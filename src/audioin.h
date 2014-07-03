#ifndef AUDIO_IN_H
#define AUDIO_IN_H

#define SAMPLE_RATE 8000
#define FRAMES_PER_PACKET 512
#define PACKETS_PER_BUFFER 4
#define CHANNELS 1

#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE 128

#include <semaphore.h>
#include <portaudio.h>

typedef struct sound {
    int frameIndex;
    int maxFrameIndex;
    int pstart;
    int plen;
    sem_t *drawer;
    sem_t *writer;
    sem_t *detector;
    SAMPLE *recorded;
} sound;

void audio_init(PaStream **pstream, sound *data);
void audio_start(PaStream *stream);


#endif
