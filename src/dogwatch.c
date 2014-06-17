#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <portaudio.h>
#include "audioin.h"
#include "writer.h"
#include "curse.h"



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


static void process_audio(PaStream *stream, sound *data){
    PaError err = paNoError;
    SAMPLE max, val;
    double average;
    int numSamples, i;

    numSamples = data->maxFrameIndex * CHANNELS;
    while ((err = Pa_IsStreamActive(stream)) == 1){
        Pa_Sleep(1000);
    }
    if (err < 0){
        printf("Error recording: %s\n", Pa_GetErrorText(err));
        exit(1);
    }
    
    max = 0;
    average = 0.0;
    for (i=0; i<numSamples; i++){
        val = data->recorded[i];
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

}

int main(int argc, char **argv) {
    PaStream *stream;
    sound data;
    writer wargs;
    pthread_t file_writer, wave_viewer;

    //wunused
    (void) argc;
    (void) argv;


    signal(SIGINT, shutdown);

    wargs.fp = init_file("out/record.raw");
    wargs.data = &data;

    audio_init(&stream, &data);
    audio_start(stream);
    if (pthread_create(&file_writer, NULL, write_file, &wargs)){
        fprintf(stderr, "Error creating file writer\n");
        close();
        close_file(wargs.fp);
        exit(1);
    }

    nc_setup();
    if (pthread_create(&wave_viewer, NULL, nc_view, &data)){
        fprintf(stderr, "Error creating waveform viewer\n");
        nc_stop();
        close();
        close_file(wargs.fp);
        exit(1);
    }

    process_audio(stream, &data);


    nc_stop();
    close();
    close_file(wargs.fp);

    return 0;
}

