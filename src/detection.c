#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "detection.h"



void detection_start(void){

}

void *detect(void *snd){
	sound *data = (sound *)snd;
	unsigned int i, barks;

	barks = 0;

	while (1){
		sem_wait(&(data->detector));
		for (i=data->pstart; i<data->pstart + data->plen; i++){
			if (abs(data->recorded[i]-SAMPLE_SILENCE) >= BARK_THRESHOLD - SAMPLE_SILENCE){
				barks++;
				printf("barked! %d\n", data->recorded[i]);
			}
		}
	}
}
