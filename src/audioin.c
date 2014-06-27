#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"

static int get_audio(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *udata){

    sound *data = (sound*)udata;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr;
    int fpb = (int) framesPerBuffer;
    long i;

    /* avoid unused var warnings */
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) udata;


    /* circle back if we'd fill past the end */
    if ((data->frameIndex + fpb) > data->maxFrameIndex){
        data->frameIndex=0;
    }

    wptr = &(data->recorded[data->frameIndex]);

    /* populate the data */
    if (inputBuffer == NULL){
        for (i=0; i<fpb; i++){
            *wptr++ = SAMPLE_SILENCE; //left?
            if (CHANNELS == 2 ){
                *wptr++ = SAMPLE_SILENCE; //right?
            }
        }
    } else {
        for (i=0; i<fpb; i++){
            *wptr++ = *rptr++; //left
            if (CHANNELS == 2){
                *wptr++ = *rptr++; //right
            }
        }
    }

    data->pstart = data->frameIndex;
    data->frameIndex += fpb;
    data->plen = fpb;

    sem_post(data->writer);
    sem_post(data->drawer);
    sem_post(data->detector);

    return paContinue;

}


void audio_init(PaStream **pstream, sound *data){
    PaStreamParameters inputConfig;
    PaError err=paNoError;
    int i,
        totalFrames,
        numSamples,
        numBytes;



    data->maxFrameIndex = totalFrames = FRAMES_PER_BUFFER * PACKETS_PER_BUFFER;
    data->frameIndex = 0;
    data->pstart = 0;
    data->plen = 0;
    numSamples = totalFrames * CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data->recorded = (SAMPLE *) malloc(numBytes);

    data->drawer = sem_open("dogwatch-drawer",O_CREAT,0644,0);
    data->writer = sem_open("dogwatch-writer",O_CREAT,0644,0);
    data->detector = sem_open("dogwatch-detector",O_CREAT,0644,0);

    if (data->drawer == SEM_FAILED){
        fprintf(stderr, "unable to create drawer semaphore\n");
        sem_unlink("dogwatch-drawer");
        exit(1);
    }
    if (data->writer == SEM_FAILED){
        fprintf(stderr, "unable to create writer semaphore\n");
        sem_unlink("dogwatch-writer");
        exit(1);
    }
    if (data->detector == SEM_FAILED){
        fprintf(stderr, "unable to create detector semaphore\n");
        sem_unlink("dogwatch-detector");
        exit(1);
    }


    if (data->recorded == NULL){
        printf("Could not allocate audio buffer\n");
        exit(1);
    }

    for (i=0; i<numSamples; i++){
        data->recorded[i] = 0;
    }

    /* PortAudio init */
    err = Pa_Initialize();
    if (err != paNoError){
        printf("PA Init Error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }


    /* set recording parameters */
    inputConfig.channelCount = CHANNELS;
    inputConfig.sampleFormat = PA_SAMPLE_TYPE;
    inputConfig.hostApiSpecificStreamInfo=NULL;
    inputConfig.device = Pa_GetDefaultInputDevice();
    if (inputConfig.device == paNoDevice){
        fprintf(stderr, "Error: No default input device\n");
        exit(1);
    }
    inputConfig.suggestedLatency = Pa_GetDeviceInfo(inputConfig.device)->defaultLowInputLatency;


    /* start recording */
    err = Pa_OpenStream(pstream,
                       &inputConfig,
                       NULL, //output config
                       SAMPLE_RATE,
                       FRAMES_PER_BUFFER,
                       paClipOff,
                       get_audio,
                       data); //pointer to be passed to callback
    if (err != paNoError){
        printf("PA open default Error: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
}

void audio_start(PaStream *stream){
    PaError err=paNoError;

    err = Pa_StartStream(stream);
    if (err != paNoError){
        fprintf(stderr, "Error: could not start stream: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
    printf("Recording started\n");

}
