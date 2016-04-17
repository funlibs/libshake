/*
 * MIT License
 *
 * LIBSHAKE Copyright (c) 2016 Sebastien Serre <ssbx@sysmo.io>.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "shake.h"
#include "wave.h"
#include "portaudio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BUFFER_SIZE     1000000
#define SAMPLE_BITS     16
#define SAMPLE_FORMAT   paInt16
#define SAMPLE_RATE     44100


typedef struct sound_t {int16_t* data; int size;} Sound;
int       SHAKE_LoadedSounds;
Sound*    SHAKE_Sounds;
PaStream* SHAKE_Stream;
int16_t*  SHAKE_Buffer;
int       SHAKE_BufferNextReadPosition;



// portable mutex
#ifdef WIN32

#include <windows.h>
static HANDLE SHAKE_BufferMutex;
void shakeInitLock() { SHAKE_BufferMutex = CreateMutex(NULL, FALSE, NULL); }
void shakeLock() { WaitForSingleObject(SHAKE_BufferMutex, INFINITE); }
void shakeUnlock() { ReleaseMutex(SHAKE_BufferMutex); }

#else

#include <pthread.h>
static pthread_mutex_t  SHAKE_BufferMutex = PTHREAD_MUTEX_INITIALIZER; 
void shakeInitLock() {}
void shakeLock() { pthread_mutex_lock(&SHAKE_BufferMutex); }
void shakeUnlock() { pthread_mutex_unlock(&SHAKE_BufferMutex); }

#endif // WIN32


// The portaudio thread
int shakeCallback(
        const void                      *input,
        void                            *output,
        unsigned long                    frameCount,
        const PaStreamCallbackTimeInfo*  paTimeInfo,
        PaStreamCallbackFlags            statusFlags,
        void                            *noUserData)
{

    int mustRead = 2 * frameCount; // nchannels * framecount


    shakeLock();  // LOCK BUFFER

    if ((SHAKE_BufferNextReadPosition + mustRead) > BUFFER_SIZE) {

        // read end of wave buffer and go to the begining
        int readEnd = BUFFER_SIZE - SHAKE_BufferNextReadPosition;
        memcpy(output, &SHAKE_Buffer[SHAKE_BufferNextReadPosition], readEnd * 2);
        memset(&SHAKE_Buffer[SHAKE_BufferNextReadPosition], 0,      readEnd * 2);

        SHAKE_BufferNextReadPosition = 0;
        mustRead = mustRead - readEnd;

    }

    memcpy(output, &SHAKE_Buffer[SHAKE_BufferNextReadPosition], mustRead * 2);
    memset(&SHAKE_Buffer[SHAKE_BufferNextReadPosition], 0,      mustRead * 2);

    SHAKE_BufferNextReadPosition += mustRead;

    shakeUnlock(); // UNLOCK SHAKE_Buffer

    return paContinue;

}


// the main thread
int shakeInit(float suggestedLatency)
{

    shakeInitLock();

    SHAKE_Buffer = calloc(BUFFER_SIZE, sizeof(int16_t));
    SHAKE_BufferNextReadPosition = 0;

    SHAKE_Sounds = malloc(10 * sizeof(Sound));
    SHAKE_LoadedSounds = 0;


    Pa_Initialize();


    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2;
    outputParameters.suggestedLatency = suggestedLatency;
    outputParameters.hostApiSpecificStreamInfo = 0;
    outputParameters.sampleFormat = SAMPLE_FORMAT;


    PaError error = Pa_OpenStream(&SHAKE_Stream, 0, &outputParameters,
            SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag,
            shakeCallback, NULL);

    if (error) {

        printf("error opening output, error code = %i\n", error);
        Pa_Terminate();
        return 1;

    }

    Pa_StartStream(SHAKE_Stream);

    return 0;

}


int shakeLoad(char* fileName)
{

    WAVE_INFO info;
    void* in;
    in = (void *) waveLoad(fileName, &info);

    if (!in) {

        printf("error opening file\n");
        exit(1);

    }

    if (info.wBitsPerSample != SAMPLE_BITS ||
        info.nSamplesPerSec != 44100 ||
        info.nChannels != 2)
    {
        printf("Error it is not a 2 channels 16 bits samples at 44100Hs.\n");
        exit(1);
    }

    Sound s;
    s.data = (int16_t*) in;
    s.size = info.dataSize / 2;


    int soundId = SHAKE_LoadedSounds;
    SHAKE_Sounds[soundId] = s;
    SHAKE_LoadedSounds += 1;

    return soundId;

}


void shakeMixAverage(
        int16_t*    sample, 
        int         sampleIndex,
        int16_t*    output,
        int         outputIndex,
        int         size)
{

    // basic hard clipping
    int i;
    for (i = 0; i < size; i++)
    {

        int16_t val_a, val_b, result;
        val_a = sample[sampleIndex + i];
        val_b = output[outputIndex + i];

        // clip
        int32_t v = (int32_t) val_a + (int32_t) val_b;

        if (v > INT16_MAX)
            result = INT16_MAX;
        else if (v < INT16_MIN)
            result = INT16_MIN;
        else
            result = (int16_t) v;


        output[outputIndex + i] = result;

    }

}


void shakePlay(int soundId) {

    shakeLock(); // LOCK BUFFER

    int dataPos     = 0;
    int buffPos     = SHAKE_BufferNextReadPosition;

    int mustRead    = SHAKE_Sounds[soundId].size;
    int samplesLeft = BUFFER_SIZE - buffPos;

    if (samplesLeft < mustRead) {

        shakeMixAverage(
                SHAKE_Sounds[soundId].data, dataPos,
                SHAKE_Buffer, buffPos, samplesLeft);

        mustRead = mustRead - samplesLeft;
        dataPos  = samplesLeft;
        buffPos  = 0;

    }

    shakeMixAverage(
            SHAKE_Sounds[soundId].data, dataPos,
            SHAKE_Buffer,               buffPos, mustRead);

    shakeUnlock(); // UNLOCK BUFFER

}


void shakeTerminate()
{

    Pa_StopStream(SHAKE_Stream);
    Pa_Terminate();

    int i;
    for (i = 0; i < SHAKE_LoadedSounds; i++)
        free(SHAKE_Sounds[i].data);

    free(SHAKE_Sounds);
    free(SHAKE_Buffer);

}

