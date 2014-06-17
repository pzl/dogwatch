#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <portaudio.h>
#include "audioin.h"
#include "writer.h"
#include "curse.h"
#include "gui.h"



static void close(void){

    printf("terminating audio connection\n");
    PaError err = Pa_Terminate();
    if (err != paNoError){
        printf("PA terminate Error: %s\n", Pa_GetErrorText(err));
    }

    nc_stop();
}

static void shutdown(int sig){
    printf("shutting down\n");
    (void) sig; //unused
    close();
    exit(0);
}


static void audio_wait(PaStream *stream){
    PaError err = paNoError;

    while ((err = Pa_IsStreamActive(stream)) == 1){
        Pa_Sleep(300);
    }
    if (err < 0){
        printf("Error recording: %s\n", Pa_GetErrorText(err));
        exit(1);
    }   
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

    gui_init();

    audio_wait(stream);


    nc_stop();
    close();
    close_file(wargs.fp);

    return 0;
}

