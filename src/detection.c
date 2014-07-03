#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "audioin.h"
#include "detection.h"

unsigned int barks = 0;

void detection_start(void){
	barks=0;
}

void *detect(void *snd){
	sound *data = (sound *)snd;
	int i;
	unsigned char barking=0;
	unsigned int ms=0;
	time_t rawtime;
	struct tm * timeinfo;


	while (1){
		sem_wait(data->detector);

		for (i=0; i<data->plen; i++){
			if (!barking){
				//if not currently inside a bark cooldown, check for spikes
				if (abs(data->recorded[data->pstart+i]-SAMPLE_SILENCE) >= BARK_THRESHOLD - SAMPLE_SILENCE){
					barking=1;
					barks++;
					time(&rawtime);
					timeinfo = localtime(&rawtime);
					printf("barked! %s", asctime(timeinfo));
				}
			} else {
				if (abs(data->recorded[data->pstart+i] - SAMPLE_SILENCE) <= CALM - SAMPLE_SILENCE){
					//count the consecutive quiet frames
					ms++;

					if (ms >= SAMPLE_RATE*CALM_MS/1000){
						barking=0;
						time(&rawtime);
						timeinfo = localtime(&rawtime);
						printf("barking stopped %s", asctime(timeinfo));
					}
				} else {
					//counter starts over since we just broke calm threshold
					ms = 0;
				}
			}

		}
	}

	return NULL;
}

unsigned int detection_end(void){
	return barks;
}
