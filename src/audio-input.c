#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "audio-input.h"

int interfaces(int a){
    return a+2;
}

void print_cards(void){
    int err,
        cardNum=-1;
    
    for (;;){
    
        snd_ctl_t *cardHandle;

        if ((err = snd_card_next(&cardNum)) < 0) {
            printf("Can't get the next card number: %s\n",snd_strerror(err));
            break;
        }
        if (cardNum < 0){
            break;
        }

        char str[64];

        sprintf(str, "hw:%i", cardNum);
        if ((err = snd_ctl_open(&cardHandle, str, 0)) < 0) {
            printf("Can't open card %i: %s\n",cardNum,snd_strerror(err));
            continue;
        }
        snd_ctl_card_info_t *cardInfo;

        snd_ctl_card_info_malloc(&cardInfo);
        if ((err = snd_ctl_card_info(cardHandle, cardInfo)) < 0){
            printf("Can't get info for card %i: %s\n", cardNum, snd_strerror(err));
        } else {
            printf("Card %i = %s\n", cardNum, snd_ctl_card_info_get_name(cardInfo));
            printf("\tid = %s\n\tlongname = %s\n\tdriver = %s\n\tmixername = %s\n\tcomponents = %s\n",
                    snd_ctl_card_info_get_id(cardInfo),
                    snd_ctl_card_info_get_longname(cardInfo),
                    snd_ctl_card_info_get_driver(cardInfo),
                    snd_ctl_card_info_get_mixername(cardInfo),
                    snd_ctl_card_info_get_components(cardInfo));
        }

        snd_ctl_close(cardHandle);
        
    }

    snd_config_update_free_global();
}

