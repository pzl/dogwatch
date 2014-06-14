#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <portaudio.h>
#include "audioin.h"



static void close(void){

    printf("terminating audio connection\n");
    PaError err = Pa_Terminate();
    if (err != paNoError){
        printf("PA terminate Error: %s\n", Pa_GetErrorText(err));
    }
}

static void shutdown(int sig){
    printf("shutting down\n");
    (void) sig; //unused
    close();
    exit(0);
}



int main(int argc, char **argv) {
    PaStream *stream;
    PaError err=paNoError;
    sound data;

    SAMPLE max, val;
    double average;
    int numSamples, i;

    //wunused
    (void) argc;
    (void) argv;


    signal(SIGINT, shutdown);

    audio_init(&stream, &data);
    audio_start(stream);

    numSamples = data.maxFrameIndex * CHANNELS;


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

    printf("sample max amplitude: %d\n", max);
    printf("sample average = %1f\n", average);



    FILE *f;
    f = fopen("out/record.raw","wb");
    if (f == NULL){
        fprintf(stderr, "Could not open file for writing\n");
    } else {
        fwrite(data.recorded, CHANNELS*sizeof(SAMPLE), data.maxFrameIndex, f);
        fclose(f);
        printf("Wrote to file\n");
    }
    close();

    return 0;
}

