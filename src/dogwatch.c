#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "portaudio.h"


#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER   512
#define CHANNELS 2

#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;


#define SAMPLE_SILENCE 0.0f


typedef struct sound {
    int frameIndex;
    int maxFrameIndex;
    SAMPLE *recorded;
} sound;


static void shutdown(int sig){
    printf("closing\n");

    (void) sig; //unused

    PaError err = Pa_Terminate();
    if (err != paNoError){
        printf("PA terminate Error: %s\n", Pa_GetErrorText(err));
    }

    exit(0);
}

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
    return finished;

}


int main(int argc, char **argv) {

    //wunused
    (void) argc;
    (void) argv;


    signal(SIGINT, shutdown);

    PaStreamParameters inputConfig;
    PaStream *stream;
    PaError err=paNoError;
    sound data;
    int i,
        totalFrames,
        numSamples,
        numBytes;
    SAMPLE max, val;
    double average;


    data.maxFrameIndex = totalFrames = 5 * SAMPLE_RATE; //seconds
    data.frameIndex = 0;
    numSamples = totalFrames * CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recorded = (SAMPLE *) malloc(numBytes);

    if (data.recorded == NULL){
        printf("Could not allocate audio buffer\n");
        return -1;
    }

    for (i=0; i<numSamples; i++){
        data.recorded[i] = 0;
    }

    /* PortAudio init */
    err = Pa_Initialize();
    if (err != paNoError){
        printf("PA Init Error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    /* Device enumeration */
    int nDevices;
    nDevices  = Pa_GetDeviceCount();
    printf("PA found %d devices\n", nDevices);
    const PaDeviceInfo *dInfo;
    for (i=0; i<nDevices; i++){
        dInfo = Pa_GetDeviceInfo(i);
        if (dInfo->maxInputChannels > 0){
            printf("device %d: %s\n", i, dInfo->name);
        }
    }
    printf("using default\n");




    inputConfig.device = Pa_GetDefaultInputDevice();
    if (inputConfig.device == paNoDevice){
        fprintf(stderr, "Error: No default input device\n");
        return -1;
    }
    inputConfig.channelCount = CHANNELS;
    inputConfig.sampleFormat = PA_SAMPLE_TYPE;
    inputConfig.suggestedLatency = Pa_GetDeviceInfo(inputConfig.device)->defaultLowInputLatency;
    inputConfig.hostApiSpecificStreamInfo=NULL;



    err = Pa_OpenStream(&stream,
                       &inputConfig,
                       NULL, //output config
                       SAMPLE_RATE,
                       FRAMES_PER_BUFFER,
                       paClipOff,
                       get_audio,
                       &data); //pointer to be passed to callback
    if (err != paNoError){
        printf("PA open default Error: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError){
        fprintf(stderr, "Error: could not start stream: %s\n", Pa_GetErrorText(err));
        return -1;
    }
    printf("Recording started\n");


    while ((err = Pa_IsStreamActive(stream)) == 1){
        Pa_Sleep(1000);
        printf("index = %d\n", data.frameIndex);
        fflush(stdout);
    }
    if (err < 0){
        printf("Error recording: %s\n", Pa_GetErrorText(err));
        return -1;
    }

    max = 0;
    average = 0.0;
    for (i=0; i<numSamples; i++){
        val = data.recorded[i];
        if (val < 0){
            val = -val;
        }
        if (val > max){
            max = val;
        }
        average += val;
    }
    average = average / (double)numSamples;

    printf("sample max amplitude: %.8f\n", max);
    printf("sample average = %1f\n", average);



    FILE *f;
    f = fopen("out/record.raw","wb");
    if (f == NULL){
        fprintf(stderr, "Could not open file for writing\n");
    } else {
        fwrite(data.recorded, CHANNELS*sizeof(SAMPLE), totalFrames, f);
        fclose(f);
        printf("Wrote to file\n");
    }


    return 0;
}

