#include "sndo.h"
#include "wave.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h>
static HANDLE SNDO_BufferMutex;
void sndoLock() { WaitForSingleObject(SNDO_BufferMutex, INFINITE); }
void sndoUnlock() { ReleaseMutex(SNDO_BufferMutex); }
#else
#include <pthread.h>
static pthread_mutex_t  SNDO_BufferMutex = PTHREAD_MUTEX_INITIALIZER; 
void sndoLock() { pthread_mutex_lock(&SNDO_BufferMutex); }
void sndoUnlock() { pthread_mutex_unlock(&SNDO_BufferMutex); }
#endif

#define BUFFER_SIZE     1000000
#define SAMPLE_BITS     16
#define SAMPLE_FORMAT   paInt16
#define SAMPLE_TYPE     int16_t
#define SAMPLE_RATE     44100

int sndoCallback(
        const void                      *input,
        void                            *output,
        unsigned long                    frameCount,
        const PaStreamCallbackTimeInfo*  paTimeInfo,
        PaStreamCallbackFlags            statusFlags,
        void                            *noUserData);
void sndoMixAverage(
        int16_t*    sample, 
        int         sampleIndex,
        int16_t*    output,
        int         outputIndex,
        int         size);

typedef struct sound_t {int16_t* data; int size;} Sound;

// GLOBAL_Variables 
int                     SNDO_LoadedSounds;
Sound*                  SNDO_Sounds;
PaStream*               SNDO_Stream;
int16_t*                SNDO_Buffer;
int                     SNDO_BufferPosition;

int sndoInit(float suggestedLatency)
{

#ifdef WIN32
    SNDO_BufferMutex = CreateMutex(NULL, FALSE, NULL);
#endif


    SNDO_Buffer = calloc(BUFFER_SIZE, sizeof(int16_t));
    SNDO_BufferPosition = 0;

    SNDO_Sounds = malloc(10 * sizeof(Sound));
    SNDO_LoadedSounds = 0;


    Pa_Initialize();


    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = 2;
    outputParameters.suggestedLatency = suggestedLatency;
    outputParameters.hostApiSpecificStreamInfo = 0;
    outputParameters.sampleFormat = SAMPLE_FORMAT;


    PaError error = Pa_OpenStream(&SNDO_Stream, 0, &outputParameters,
            SAMPLE_RATE, paFramesPerBufferUnspecified, paNoFlag,
            sndoCallback, NULL);

    if (error) {

        printf("error opening output, error code = %i\n", error);
        Pa_Terminate();
        return 1;

    }

    Pa_StartStream(SNDO_Stream);

    return 0;

}

int sndoLoad(char* fileName)
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


    int soundId = SNDO_LoadedSounds;
    SNDO_Sounds[soundId] = s;
    SNDO_LoadedSounds += 1;

    return soundId;

}


// main thread
void sndoPlay(int soundId) {

    sndoLock(); // LOCK BUFFER

    int dataPos     = 0;
    int buffPos     = SNDO_BufferPosition;

    int mustRead    = SNDO_Sounds[soundId].size;
    int samplesLeft = BUFFER_SIZE - SNDO_BufferPosition;

    if (samplesLeft < mustRead) {

        sndoMixAverage(
                SNDO_Sounds[soundId].data, dataPos,
                SNDO_Buffer,         buffPos, samplesLeft);

        mustRead = mustRead - samplesLeft;
        dataPos  = samplesLeft;
        buffPos  = 0;

    }

    sndoMixAverage(
            SNDO_Sounds[soundId].data, dataPos,
            SNDO_Buffer,         buffPos, mustRead);

    sndoUnlock(); // UNLOCK BUFFER

}


// portaudio thread
int sndoCallback(
        const void                      *input,
        void                            *output,
        unsigned long                    frameCount,
        const PaStreamCallbackTimeInfo*  paTimeInfo,
        PaStreamCallbackFlags            statusFlags,
        void                            *noUserData)
{

    int mustRead = 2 * frameCount; // nchannels * framecount


    sndoLock();  // LOCK BUFFER

    if ((SNDO_BufferPosition + mustRead) > BUFFER_SIZE) {

        // read end of wave buffer and go to the begining
        int readEnd = BUFFER_SIZE - SNDO_BufferPosition;
        memcpy(output, &SNDO_Buffer[SNDO_BufferPosition], readEnd * 2);
        memset(&SNDO_Buffer[SNDO_BufferPosition], 0,      readEnd * 2);

        SNDO_BufferPosition = 0;
        mustRead = mustRead - readEnd;

    }

    memcpy(output, &SNDO_Buffer[SNDO_BufferPosition], mustRead * 2);
    memset(&SNDO_Buffer[SNDO_BufferPosition], 0,      mustRead * 2);

    SNDO_BufferPosition += mustRead;

    sndoUnlock(); // UNLOCK SNDO_Buffer

    return paContinue;

}


/*
 * @briev mix sounds
 * Very basic with hard clipping.
 */
void sndoMixAverage(
        int16_t*    sample, 
        int         sampleIndex,
        int16_t*    output,
        int         outputIndex,
        int         size)
{

    int i;
    for (i = 0; i < size; i++)
    {

        int16_t val_a, val_b, result;
        val_a = sample[sampleIndex + i];
        val_b = output[outputIndex + i];

        // clip
        int32_t v = (int32_t) val_a + (int32_t) val_b;

        if (v > 32767)
            result = 32767;
        else if (v < -32767)
            result = -32767;
        else
            result = (SAMPLE_TYPE) v;


        output[outputIndex + i] = result;

    }

}



void sndoTerminate()
{

    Pa_StopStream(SNDO_Stream);
    Pa_Terminate();

    int i;
    for (i = 0; i < SNDO_LoadedSounds; i++)
        free(SNDO_Sounds[i].data);

    free(SNDO_Sounds);
    free(SNDO_Buffer);

}


