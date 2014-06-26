#include <stdio.h>
#include <cairo.h>
#include <sys/stat.h>
#include "audioin.h"
#include "reviewer.h"

void png_view_create(const char *readfile, const char *outfile){
	FILE *infile = fopen(readfile, "r+b");
	int fd;
	struct stat st;
	long long fsize;
	int flen, i, rd;
	SAMPLE buf[REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL];

	fd = fileno(infile);
	fstat(fd, &st);
	fsize = st.st_size;

	flen = (int) fsize/4;

	printf("file size: %lld\n", fsize);
	printf("png length: %d\n", flen);


	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, REVIEW_FILE_WIDTH, REVIEW_FILE_HEIGHT);
	cairo_t *cr = cairo_create(surface);

	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_rectangle(cr,0,0,REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL,REVIEW_FILE_HEIGHT);
	cairo_fill(cr);

	cairo_set_source_rgb(cr,0.3,0.3,0.3);
	cairo_set_line_width(cr,1);
	cairo_move_to(cr,0,REVIEW_FILE_HEIGHT/2+0.5);
	cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	cairo_stroke(cr);
	cairo_move_to(cr,0,REVIEW_FILE_HEIGHT/2+0.5);
	cairo_set_source_rgb(cr,0.0,1.0,1.0);


	rd = fread(buf, CHANNELS * sizeof(SAMPLE), REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL, infile);
	if (rd != REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL){
		fprintf(stderr, "error reading %d samples from file\n", REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL);
	}

	for (i=0; i<REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL; i++){
		cairo_line_to(cr,i/(SAMPLES_PER_PIXEL*1.0),buf[i]+0.5);
	}



	fclose(infile);


	cairo_set_line_width(cr,1);
	cairo_stroke(cr);

	cairo_destroy(cr);
	cairo_surface_write_to_png(surface, outfile);
	cairo_surface_destroy(surface);
	return;
}
