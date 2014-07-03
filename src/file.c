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
	int start, len;

	while (1){
		sem_wait(args->data->writer);
		//store packet pointers in case they change
		start = args->data->pstart;
		len = args->data->plen;


		packet = &(args->data->recorded[start]);
		fwrite(packet, CHANNELS*sizeof(SAMPLE), len, args->df.fp);
	}

	return NULL;
}

void close_file(dogfile d){
	fclose(d.fp);
}
