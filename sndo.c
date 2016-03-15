#include "wave.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {

    char*       waveData;
    WAVE_INFO   waveInfo;
    int         position;
    int         bytesPerSample;

} StreamState;


StreamState* state;
PaStream*   stream;


int sndoCallback(
        const void                      *input,
        void                            *output,
        unsigned long                   frameCount,
        const PaStreamCallbackTimeInfo* paTimeInfo,
        PaStreamCallbackFlags           statusFlags,
        void                            *streamState)
{

    StreamState *state = (StreamState *) streamState;

    int mustRead =
        state->waveInfo.nChannels * state->bytesPerSample * frameCount;

    if ((state->position + mustRead) > state->waveInfo.dataSize) {

        // read end of wave buffer and go to the begining
        int readEnd = state->waveInfo.dataSize - state->position;
        memcpy(output, &state->waveData[state->position], readEnd);

        state->position = 0;
        mustRead = mustRead - readEnd;

    }

    memcpy(output, &state->waveData[state->position], mustRead);

    state->position += mustRead;

    return paContinue;

}


int sndoInit(char* fileName)
{

    state = (StreamState *) malloc(sizeof(StreamState));

    state->waveData = waveLoad(fileName, &state->waveInfo);
    if (!state->waveData) {

        printf("error opening file\n");
        return 1;

    }

    state->position = 0;
    state->bytesPerSample = state->waveInfo.wBitsPerSample / 8;


    Pa_Initialize();


    PaStreamParameters outputParameters;

    outputParameters.device = Pa_GetDefaultOutputDevice();
    outputParameters.channelCount = state->waveInfo.nChannels;
    outputParameters.suggestedLatency = 0.2;
    outputParameters.hostApiSpecificStreamInfo = 0;

    if (state->waveInfo.wBitsPerSample == 8)
        outputParameters.sampleFormat = paUInt8;

    else if (state->waveInfo.wBitsPerSample == 16)
        outputParameters.sampleFormat = paInt16;

    else if (state->waveInfo.wBitsPerSample == 32)
        outputParameters.sampleFormat = paInt32;


    PaError error;

    error = Pa_OpenStream(&stream,
            0,                              // no input
            &outputParameters,
            state->waveInfo.nSamplesPerSec,  // sample rate 
            paFramesPerBufferUnspecified,
            paNoFlag,  // no special modes (clip off, dither off)
            sndoCallback,  
            state); 

    if (error) {

        printf("error opening output, error code = %i\n", error);
        Pa_Terminate();
        return 1;

    }


    Pa_StartStream(stream);

    return 0;

}


void sndoTerminate()
{

    Pa_StopStream(stream);
    Pa_Terminate();

}


