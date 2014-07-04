#ifndef WRITER_H
#define WRITER_H

#include "audioin.h"

#define FILE_HEADER_SIZE 6
#define COMPRESS_AFTER_TIMES 4
#define LOSSY_LEVEL 2


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
void *file_writer(void *);
int write_packet(FILE *, SAMPLE *, int packetlen);
void close_file(dogfile);

#endif
