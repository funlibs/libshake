#include "sndfile.h"
#include "portaudio.h"
#include <stdlib.h>
#include <stdio.h>


/* the filename is defined here because this is just a demo */
#define FILE_NAME "test.wav"


/*
 * Data structure to pass to callback
 * includes the sound file, info about the sound file, and a position
 * cursor (where we are in the sound file)
 */
struct OurData
{
    SNDFILE *sndFile;
    SF_INFO sfInfo;
    int position;
};


/*
 * Callback function for audio output
 */
int Callback(
        const void                      *input,
        void                            *output,
        unsigned long                   frameCount,
        const PaStreamCallbackTimeInfo* paTimeInfo,
        PaStreamCallbackFlags           statusFlags,
        void                            *userData)
{
    OurData *data = (OurData *)userData; /* we passed a data structure
                                            into the callback so we have something to work with */
    int *cursor; /* current pointer into the output  */
    int *out = (int *)output;
    int thisSize = frameCount;
    int thisRead;

    cursor = out; /* set the output cursor to the beginning */
    while (thisSize > 0)
    {
        /* seek to our current file position */
        sf_seek(data->sndFile, data->position, SEEK_SET);

        /* are we going to read past the end of the file?*/
        if (thisSize > (data->sfInfo.frames - data->position))
        {
            /*if we are, only read to the end of the file*/
            thisRead = data->sfInfo.frames - data->position;
            /* and then loop to the beginning of the file */
            data->position = 0;
        }
        else
        {
            /* otherwise, we'll just fill up the rest of the output buffer */
            thisRead = thisSize;
            /* and increment the file position */
            data->position += thisRead;
        }

        /* since our output format and channel interleaving is the same as
           sf_readf_int's requirements */
        /* we'll just read straight into the output buffer */
        sf_readf_int(data->sndFile, cursor, thisRead);
        /* increment the output cursor*/
        cursor += thisRead;
        /* decrement the number of samples left to process */
        thisSize -= thisRead;
    }

    return paContinue;
}


int main()
{
    OurData *data = (OurData *)malloc(sizeof(OurData));
    PaStream *stream;
    PaError error;
    PaStreamParameters outputParameters;

    /* initialize our data structure */
    data->position = 0;
    data->sfInfo.format = 0;
    /* try to open the file */
    data->sndFile = sf_open(FILE_NAME, SFM_READ, &data->sfInfo);

    if (!data->sndFile)
    {
        printf("error opening file\n");
        return 1;
    }

    /* start portaudio */
    Pa_Initialize();

    /* set the output parameters */
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* use the
                                                              default device */
    outputParameters.channelCount = data->sfInfo.channels; /* use the
                                                              same number of channels as our sound file */
    outputParameters.sampleFormat = paInt32; /* 32bit int format */
    outputParameters.suggestedLatency = 0.2; /* 200 ms ought to satisfy
                                                even the worst sound card */
    outputParameters.hostApiSpecificStreamInfo = 0; /* no api specific data */

    /* try to open the output */
    error = Pa_OpenStream(&stream,  /* stream is a 'token' that we need
                                       to save for future portaudio calls */
            0,  /* no input */
            &outputParameters,
            data->sfInfo.samplerate,  /* use the same
                                         sample rate as the sound file */
            paFramesPerBufferUnspecified,  /* let
                                              portaudio choose the buffersize */
            paNoFlag,  /* no special modes (clip off, dither off) */
            Callback,  /* callback function defined above */
            data ); /* pass in our data structure so the
                       callback knows what's up */

    /* if we can't open it, then bail out */
    if (error)
    {
        printf("error opening output, error code = %i\n", error);
        Pa_Terminate();
        return 1;
    }

    /* when we start the stream, the callback starts getting called */
    Pa_StartStream(stream);
    Pa_Sleep(2000); /* pause for 2 seconds (2000ms) so we can hear a bit
                       of the output */
    Pa_StopStream(stream); // stop the stream
    Pa_Terminate(); // and shut down portaudio
    return 0;
}


