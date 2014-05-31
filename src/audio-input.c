#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include "audio-input.h"



int num_interfaces(void){
    int err,
        cardNum=-1,
        cards=0;

    for (;;){
        if ((err = snd_card_next(&cardNum)) < 0){
            fprintf(stderr,"Error getting ALSA card number: %s",snd_strerror(err));
            break;
        }
        if (cardNum < 0){
            break;
        }
        cards++;
    }
    snd_config_update_free_global();
    return cards;
}


void get_interfaces(char * interfaces[]){
    int err,
        cardNum=-1;

    for (;;){
        snd_ctl_t *cardHandle;

        if ((err = snd_card_next(&cardNum)) < 0){
            fprintf(stderr,"Error getting ALSA card number: %s",snd_strerror(err));
            //continue or break here? how to return
            break;
        }
        if (cardNum < 0){
            break;
        }

        char str[64];
        sprintf(str, "hw:%i", cardNum);
        if ((err = snd_ctl_open(&cardHandle, str, 0)) < 0){
            fprintf(stderr, "Can't open card %i: %s\n", cardNum, snd_strerror(err));
            continue;
        }

        snd_ctl_card_info_t *cardInfo;
        snd_ctl_card_info_malloc(&cardInfo);
        if ((err = snd_ctl_card_info(cardHandle,cardInfo)) < 0){
            fprintf(stderr, "Can't get info for card %i: %s\n",cardNum, snd_strerror(err));
            continue;
        } else {
            char *p = malloc(sizeof(char) * 40);
            strcpy(p, snd_ctl_card_info_get_name(cardInfo));
            interfaces[cardNum] = p;
        }
        snd_ctl_close(cardHandle);
    }
    snd_config_update_free_global();

}

int get_audio(int card){
    if (card < 0){
        return -1;
    }
    
    int i,
        err,
        channels=1;
    unsigned int rate = 44100,
                periods = 2;
    char * buffer;
    char hwname[64];
    snd_pcm_t *capture_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
    snd_pcm_uframes_t periodsize = 8, //period size (bytes)
                        buf;

    sprintf(hwname,"hw:%i",card);


    if ((err = snd_pcm_open(&capture_handle, hwname, SND_PCM_STREAM_CAPTURE, 0)) < 0){
        fprintf(stderr, "cannot open audio device %s (%s)\n",hwname,snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0){
        fprintf(stderr,"cannot allocate space for hwparams: %s\n",snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0 ){
        fprintf(stderr, "cannot initialize hw params structure: %s\n",snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0 ){
        fprintf(stderr,  "cannot set access type: %s\n",snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, format)) < 0){
        fprintf(stderr, "cannot set sample format: %s\n", snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &rate, 0)) < 0){
        fprintf(stderr, "cannot set sample rate: %s\n", snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, 1)) < 0 ){
        fprintf(stderr, "cannot set channel count: %s\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params(capture_handle,hw_params)) < 0 ){
        fprintf(stderr, "cannot set audio params: %s\n", snd_strerror(err));
        return -1;
    }
    
    snd_pcm_hw_params_free(hw_params);

    if ((err = snd_pcm_prepare(capture_handle)) < 0 ){
        fprintf(stderr, "cannot prep audio device: %s\n", snd_strerror(err));
        return -1;
    }
    
    //buffer = malloc(buffer_frames * snd_pcm_format_width(format) / 8 * 2);
    buffer = malloc(PERIOD_SIZE * PERIODS * channels);
    if (buffer == NULL){
        fprintf(stderr, "Failed to allocate buffer to capture audio\n");
        return -1;
    }
    
    int fd = open("record.pcm", O_WRONLY | O_TRUNC);
    if (!fd){
        fprintf(stderr, "couldn't open file for writing");
        return -1;
    }

    

    for (i = 0; i< 10; ++i){
        if ((err = snd_pcm_readi(capture_handle, buffer, periodsize*periods/2)) < 0) {
            fprintf(stderr, "read from audio interface failed: %s\n",snd_strerror(err));
            return -1;
        }
        write(fd,buffer,periodsize*periods/2);
        //fprintf(stdout, "read %d done\n",i);
    }

    close(fd);


    free(buffer);

    snd_pcm_close(capture_handle);

    return card;
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

        char str[6];

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

