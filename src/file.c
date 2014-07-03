#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "file.h"

FILE *create_file(const char *name){
	FILE *f;
	unsigned char header[6] = { 255, 'D', 'O', 'G', 1, 0 };

	f = fopen(name,"wb");
	if (f == NULL){
        fprintf(stderr, "Could not open file for writing\n");
        exit(1);
    }
    fwrite(header, sizeof(unsigned char), 6, f);
    return f;
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
		fwrite(packet, CHANNELS*sizeof(SAMPLE), len, args->fp);
	}

	return NULL;
}

void close_file(FILE *fp){
	fclose(fp);
}
