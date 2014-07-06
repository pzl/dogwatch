#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <portaudio.h>
#include "audioin.h"
#include "file.h"
#include "curse.h"
#include "detection.h"
#include "reviewer.h"



static void close(void){
    unsigned int barks;

    nc_stop();
    printf("terminating audio connection\n");
    PaError err = Pa_Terminate();
    if (err != paNoError){
        printf("PA terminate Error: %s\n", Pa_GetErrorText(err));
    }

    barks = detection_end();
    printf("barked %d times, roughly\n", barks);
}

static void shutdown(int sig){
    nc_stop();
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
    pthread_t file_write_thread, wave_viewer, crude_detector;

    //wunused
    (void) argc;
    (void) argv;

    if (argc >= 2 && *argv[1] == 'r'){
        png_view_create("out/record.dog","waveform.png");
        return 0;
    }


    signal(SIGINT, shutdown);

    wargs.df = create_dogfile("out/record.dog", DF_COMPRESSED, DF_LOSSLESS);
    wargs.data = &data;

    audio_init(&stream, &data);
    audio_start(stream);
    if (pthread_create(&file_write_thread, NULL, file_writer, &wargs)){
        fprintf(stderr, "Error creating file writer\n");
        close();
        close_file(wargs.df);
        exit(1);
    }

    if (argc >= 2 && *argv[1] == 'n'){
        nc_setup();
        if (pthread_create(&wave_viewer, NULL, nc_view, &data)){
            fprintf(stderr, "Error creating waveform viewer\n");
            nc_stop();
            close();
            close_file(wargs.df);
            exit(1);
        }
    }

    detection_start();
    if (pthread_create(&crude_detector, NULL, detect, &data)){
        fprintf(stderr, "Error creating detection thread\n");
        nc_stop();
        close();
        close_file(wargs.df);
        exit(1);
    }

    audio_wait(stream);


    //nc_stop();
    close();
    close_file(wargs.df);

    return 0;
}

