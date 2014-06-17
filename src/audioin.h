#ifndef AUDIO_IN_H
#define AUDIO_IN_H

#define SAMPLE_RATE 8000
#define FRAMES_PER_BUFFER   512
#define CHANNELS 1

#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE 128

#define SECONDS 5

typedef struct sound {
    int frameIndex;
    int maxFrameIndex;
    sem_t drawer;
    sem_t writer;
    SAMPLE *recorded;
} sound;

void audio_init(PaStream **pstream, sound *data);
void audio_start(PaStream *stream);


#endif
