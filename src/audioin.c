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
    SAMPLE *wptr = &data->recorded[data->frameIndex * CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

    /* avoid unused var warnings */
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;
    (void) udata;

    if (framesLeft < framesPerBuffer) {
        framesToCalc = framesLeft;
        finished = paComplete;
    } else {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }


    if (inputBuffer == NULL){
        for (i=0; i<framesToCalc; i++){
            *wptr++ = SAMPLE_SILENCE; //left?
            if (CHANNELS == 2 ){
                *wptr++ = SAMPLE_SILENCE; //right?
            }
        }
    } else {
        for (i=0; i<framesToCalc; i++){
            *wptr++ = *rptr++; //left
            if (CHANNELS == 2){
                *wptr++ = *rptr++; //right
            }
        }
    }

    data->frameIndex += framesToCalc;

    sem_post(&(data->writer));
    sem_post(&(data->drawer));

    return finished;

}


void audio_init(PaStream **pstream, sound *data){
    PaStreamParameters inputConfig;
    PaError err=paNoError;
    int i,
        totalFrames,
        numSamples,
        numBytes;



    data->maxFrameIndex = totalFrames = SECONDS * SAMPLE_RATE; //seconds
    data->frameIndex = 0;
    numSamples = totalFrames * CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data->recorded = (SAMPLE *) malloc(numBytes);

    sem_init(&(data->drawer),0,0);
    sem_init(&(data->writer),0,0);

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
