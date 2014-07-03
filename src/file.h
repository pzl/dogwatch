#ifndef WRITER_H
#define WRITER_H

#include "audioin.h"

#define FILE_HEADER_SIZE 6


typedef struct dogfile {
	FILE *fp;
	int version;
	int lossiness;
} dogfile;

typedef struct writer {
	dogfile df;
	sound *data;
} writer;


dogfile create_dogfile(const char *);
dogfile open_dogfile(const char *);
void *write_file(void *);
void close_file(dogfile);

#endif
