#ifndef WRITER_H
#define WRITER_H

#include "audioin.h"

typedef struct writer {
	FILE *fp;
	sound *data;
} writer;


FILE *init_file(const char *);
void *write_file(void *);
void close_file(FILE *);

#endif
