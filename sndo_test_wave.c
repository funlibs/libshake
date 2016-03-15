#include "wave.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    char*       waveData;
    WAVE_INFO   waveInfo;
    int         position;
    int         bytesPerSample;
} StreamState;


int Callback(
        const void                      *input,
        void                            *output,
        unsigned long                   frameCount,
        const PaStreamCallbackTimeInfo* paTimeInfo,
        PaStreamCallbackFlags           statusFlags,
        void                            *userData)
{
    StreamState *state = (StreamState *) userData;

    int mustRead = state->waveInfo.nChannels * state->bytesPerSample * frameCount;

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



int main(int argc, char* argv[])
{
    StreamState*    state = (StreamState *)malloc(sizeof(StreamState));

    state->waveData = waveOpen(argv[1], &state->waveInfo);
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
        outputParameters.sampleFormat = paInt8;
    else if (state->waveInfo.wBitsPerSample == 16)
        outputParameters.sampleFormat = paInt16;
    else if (state->waveInfo.wBitsPerSample == 32)
        outputParameters.sampleFormat = paInt32;


    PaStream*   stream;
    PaError     error;

    error = Pa_OpenStream(&stream,
            0,                              // no input
            &outputParameters,
            state->waveInfo.nSamplesPerSec,  // sample rate 
            paFramesPerBufferUnspecified,
            paNoFlag,  // no special modes (clip off, dither off)
            Callback,  
            state ); 

    /* if we can't open it, then bail out */
    if (error)
    {
        printf("error opening output, error code = %i\n", error);
        Pa_Terminate();
        return 1;
    }

    /* when we start the stream, the callback starts getting called */
    Pa_StartStream(stream);
    Pa_Sleep(300000);
    Pa_StopStream(stream);
    Pa_Terminate();
    return 0;
}


