#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "file.h"


/*
 * creates new file following DOG file spec, writing basic header to file
 * 
 * name: file name, including extension
 * @return: dogfile object representing new file
 */
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


/*
 * opens dogfile, parses and validates header. 
 *
 * name: file path + name, including extension
 * @return: dogfile of open file, seeked to data portion
 */
dogfile open_dogfile(const char *name){
	dogfile d;
	unsigned char header[FILE_HEADER_SIZE];
	int header_meta_length;

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
		header_meta_length = header[5];

		//consume rest of header
		fread(NULL, sizeof(unsigned char), header_meta_length, d.fp);

		d.lossiness = 0;
    } else {
    	fprintf(stderr, "error parsing %s: header incorrect.\n", name);
    }
    return d;
}

/*
 * file writing thread. Waits on semaphore signal
 *
 * param should be writer *
 */
void *file_writer(void *wargs){
	writer *args = (writer *)wargs;
	SAMPLE *packet;

	while (1){
		sem_wait(args->data->writer);

		packet = &(args->data->recorded[args->data->pstart]);
		write_packet(args->df.fp, packet, args->data->plen);
	}

	return NULL;
}

/*
 * writes from SAMPLE * to FILE *, and may compress data according to DOG spec
 *
 * FILE *: file to write to
 * SAMPLE *: buffer to write from
 * plen: number of bytes from SAMPLE * to read and encode. May not match
 *			bytes written
 *
 * @return: 0 for now (@todo)
 */
int write_packet(FILE *fp, SAMPLE *packet, int plen){
	SAMPLE buf[2*FRAMES_PER_PACKET];
	int i,
		repeat=0,
		buflen=0;

#ifndef COMPRESSION_OFF

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
				       repeat<255 && 
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

	fwrite(buf, CHANNELS*sizeof(SAMPLE), buflen, fp);
#else
	fwrite(packet, CHANNELS*sizeof(SAMPLE), plen, fp);
#endif
	
	return 0;
}


void close_file(dogfile d){
	fclose(d.fp);
}
