#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "file.h"

dogfile create_dogfile(const char *name){
	dogfile d;
	unsigned char header[FILE_HEADER_SIZE] = { 255, 'D', 'O', 'G', 1, 0 };

	d.fp = fopen(name,"wb");
	if (d.fp == NULL){
        fprintf(stderr, "Could not open file \"%s\" for writing\n", name);
        exit(1);
    }
    fwrite(header, sizeof(unsigned char), FILE_HEADER_SIZE, d.fp);
    return d;
}

dogfile open_dogfile(const char *name){
	dogfile d;
	unsigned char header[FILE_HEADER_SIZE];

	d.fp = fopen(name,"r+b");
	if (d.fp == NULL){
        fprintf(stderr, "Could not open file for writing\n");
        exit(1);
    }
    fread(header, sizeof(unsigned char), FILE_HEADER_SIZE, d.fp);
    if (header[0] == 255 && 
    	header[1] == 'D' &&
    	header[2] == 'O' &&
    	header[3] == 'G' ){
		d.version = header[4];
		d.lossiness = header[5];
    } else {
    	fprintf(stderr, "error parsing %s: header incorrect.\n", name);
    }
    return d;
}

void *write_file(void *wargs){
	writer *args = (writer *)wargs;
	SAMPLE *packet;
	SAMPLE buf[2*FRAMES_PER_PACKET];
	int i, start, plen,
		repeat=0,
		buflen=0;

	while (1){
		sem_wait(args->data->writer);
		buflen=0;

		//store packet pointers in case they change
		start = args->data->pstart;
		plen = args->data->plen;

		packet = &(args->data->recorded[start]);

		for (i=0; i<plen; i++) {

			//use compression to represent data val 0
			if (packet[i] == 0 ){
				repeat=0; //reset continuous silence counter

				buf[buflen++] = 0;
				buf[buflen++] = 0;
				buf[buflen++] = 1;

				continue;
			} else if (packet[i] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
			           packet[i] <= SAMPLE_SILENCE + LOSSY_LEVEL){
				repeat++;

				if (repeat >= COMPRESS_AFTER_TIMES){
					i+=1; //we already counted the sample we're on 

					while (i < plen && 
					       i<255 && 
					       packet[i] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
					       packet[i] <= SAMPLE_SILENCE + LOSSY_LEVEL){
						i++;
						repeat++;
					}

					i-=1; //will increment one more on continue

					while (buf[buflen-1] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
					       buf[buflen-1] <= SAMPLE_SILENCE + LOSSY_LEVEL){
						//rewind the buffer
						buflen--;
					}

					//compress escape, value, Nrepeats
					buf[buflen++] = 0;
					buf[buflen++] = SAMPLE_SILENCE;
					buf[buflen++] = repeat;

					repeat=0;
					continue;
				}
			} else {
				repeat = 0;
			}

			buf[buflen++] = packet[i];
		}

		fwrite(buf, CHANNELS*sizeof(SAMPLE), buflen, args->df.fp);
	}

	return NULL;
}

void close_file(dogfile d){
	fclose(d.fp);
}
