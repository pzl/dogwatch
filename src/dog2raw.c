#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "audioin.h"
#include "file.h"

#define BUF_BYTES 2048

int main(int argc, char **argv){
	if (argc != 3){
		fprintf(stderr, "Usage: %s <inputfile> <outputfile>\n", argv[0]);
		return -1;
	}

	SAMPLE buf[BUF_BYTES];
	int n;
	struct tm *timeinfo;
	FILE * outfile = fopen(argv[2], "wb");
	dogfile inf = open_dogfile(argv[1]);

	if (inf.fp == NULL){
		fprintf(stderr, "Could not open input file %s\n", argv[1]);
		return -1;
	}
	if (outfile == 0){
		fprintf(stderr, "Could not open output file %s\n", argv[2]);
		close_file(inf);
		return -1;
	}

	timeinfo = localtime(&inf.date);
	printf("opened dogfile from %s\n", asctime(timeinfo));

	while ((n = read_dogfile(&inf, buf, BUF_BYTES)) > 0){
		printf("read %d bytes\n", n);
		fwrite(buf, CHANNELS*sizeof(SAMPLE), n, outfile);
	}



	//fclose(outfile);
	//close_file(inf);

	return 0;
}
