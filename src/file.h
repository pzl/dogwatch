#ifndef WRITER_H
#define WRITER_H

#include "audioin.h"

#define FILE_HEADER_SIZE 6


typedef struct writer {
	FILE *fp;
	sound *data;
} writer;

typedef struct dogfile {
	FILE *fp;
	int version;
	int lossiness;
} dogfile;

FILE *create_file(const char *);
dogfile open_dogfile(const char *);
void *write_file(void *);
void close_file(FILE *);

#endif
