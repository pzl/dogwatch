#ifndef WRITER_H
#define WRITER_H

#include "audioin.h"

#define FILE_HEADER_SIZE 6
#define COMPRESS_AFTER_TIMES 4
#define LOSSY_LEVEL 0

#define _FILECPY_EXPAND 1
#define _FILECPY_NOEXPAND 0

#define DF_COMPRESSED 1
#define DF_NOCOMPRESS 0

#define DF_LOSSLESS 0
#define DF_L_LOSSY 1
#define DF_M_LOSSY 2
#define DF_H_LOSSY 3

typedef struct dogfile {
	FILE *fp;
	unsigned char version;
	unsigned char lossiness;
	unsigned char compression;
} dogfile;

typedef struct writer {
	dogfile df;
	sound *data;
} writer;

typedef struct filebuf {
	SAMPLE *buf;
	int index;
	int len;
} filebuf;


dogfile create_dogfile(const char *);
dogfile open_dogfile(const char *);
int read_dogfile(dogfile *, SAMPLE *buf, int nsamples);
void *file_writer(void *);
int write_packet(dogfile *, SAMPLE *, int packetlen);
void close_file(dogfile);

#endif
