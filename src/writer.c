#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "writer.h"

FILE *init_file(const char *name){
	FILE *f;
    f = fopen(name,"wb");
    if (f == NULL){
        fprintf(stderr, "Could not open file for writing\n");
        exit(1);
    }
    return f;
}

void *write_file(void *wargs){
	writer *args = (writer *)wargs;
	SAMPLE *packet;


	while (1){
		sem_wait(&(args->data->writer));
		packet = &(args->data->recorded[args->data->pstart]);
		fwrite(packet, CHANNELS*sizeof(SAMPLE), args->data->plen, args->fp);
	}

	return NULL;
}

void close_file(FILE *fp){
	fclose(fp);
}
