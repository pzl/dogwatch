#ifndef AUDIO_IN_H
#define AUDIO_IN_H


typedef struct {
    int id;
    char name[40];
} soundcard;


//get the number of ALSA cards available
int num_interfaces(void);

//returns a list of available audio cards
void get_interfaces(char * interfaces[]);

/*
 * select a particular sound card for capture.
 *
 * Returns `card` on success, negative for error
 */
int get_audio(int card);

//prints audio card debug info to stdout
void print_cards(void);

#endif /* AUDIO_IN_H */
