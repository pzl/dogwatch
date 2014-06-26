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
	int flen, i, rd, rows, cur_row;
	SAMPLE buf[REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL];
	static const double dashed[] = {14.0, 6.0};

	fd = fileno(infile);
	fstat(fd, &st);
	fsize = st.st_size;

	flen = (int) fsize/4;
	rows = 3;

	printf("file size: %lld\n", fsize);
	printf("png length: %d\n", flen);
	printf("rows: %d\n", rows);


	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, REVIEW_FILE_WIDTH, REVIEW_ROW_HEIGHT*rows);
	cairo_t *cr = cairo_create(surface);

	//background all white
	cairo_set_source_rgb(cr,1.0,1.0,1.0);
	cairo_rectangle(cr,0,0,REVIEW_FILE_WIDTH,REVIEW_ROW_HEIGHT*rows);
	cairo_fill(cr);

	//baselines for each row
	cairo_set_source_rgb(cr,0.3,0.3,0.3);
	cairo_set_line_width(cr,1);
	for (i=0; i<rows; i++){
		cairo_move_to(cr,0,i*REVIEW_ROW_HEIGHT + REVIEW_ROW_HEIGHT/2+0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_stroke(cr);

	//extent lines between rows
	cairo_set_source_rgb(cr,0.8,0.8,0.8);
	//i starts at 1 since we don't need line at top of file
	for (i=1; i<rows; i++){
		cairo_move_to(cr,0,i*REVIEW_ROW_HEIGHT+0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,dashed,1,0);
	cairo_stroke(cr);

	//turn off dash
	cairo_set_dash(cr,NULL,0,0);


	//make actual data
	cairo_move_to(cr,0,REVIEW_ROW_HEIGHT/2+0.5);
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
