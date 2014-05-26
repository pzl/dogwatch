#include <stdio.h>
#include <stdlib.h>
#include "audio-input.h"

int main(int argc, char **argv) {
    int err, cards;

    cards = num_interfaces();
    printf("%i sound cards available\n", cards);

    char *cardnames[cards];

    get_interfaces(cardnames);
    for (int i=0; i<cards; i++){
        printf("%s\n",cardnames[i]);
        free(cardnames[i]);
    }


    err = get_audio(1);
    if (err < 0){
        printf("failed reading audio\n");
        return 1;
    }

    return 0;
}
