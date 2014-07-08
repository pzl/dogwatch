#include <stdio.h>
#include <cairo.h>
#include <time.h>
#include "audioin.h"
#include "detection.h"
#include "file.h"
#include "reviewer.h"

static float midline(int row);
static void data_config(cairo_t *);
static void label_config(cairo_t *);
static void timecode(cairo_t *, float x, float y, float time);
static void axis_break(cairo_t *, float *x, float y, float time);
static void meta_row(cairo_t *, const char *, dogfile);


void png_view_create(const char *readfile, const char *outfile){
	dogfile d;
	long long samples_of_silence=0,
			  samples_seen=0;
	int i, j, rd,
		max_rows = 30,
		quiet_axis_break=0,
		lastY=0;
	float posX = 0;
	SAMPLE buf[REVIEW_BUFFER_SIZE];
	static const double wide_dash[] = {14.0, 6.0},
						thin_dash[] = {4.0,8.0};


	d = open_dogfile(readfile);
	if (d.fp == NULL){
		fprintf(stderr, "error reading file\n");
		return;
	}

	/*
		@todo; get filesize info
	*/


	printf("max rows: %d\n", max_rows);


	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, REVIEW_FILE_WIDTH, REVIEW_ROW_HEIGHT*max_rows + META_ROW_HEIGHT);
	cairo_t *cr = cairo_create(surface);

	//background all white
	cairo_set_source_rgba(cr,1.0,1.0,1.0,1.0);
	cairo_rectangle(cr,0,0,REVIEW_FILE_WIDTH,REVIEW_ROW_HEIGHT*max_rows + META_ROW_HEIGHT);
	cairo_fill(cr);

	//center lines for each row
	cairo_set_source_rgba(cr,0.3,0.3,0.3,0.6);
	cairo_set_line_width(cr,1);
	for (i=0; i<max_rows; i++){
		cairo_move_to(cr,0, midline(i) + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_stroke(cr);

	//extent/limit lines between rows
	cairo_set_source_rgba(cr,0.0,0.0,0.0,0.4);
	//i starts at 1 since we don't need line at top of file
	for (i=1; i<max_rows; i++){
		cairo_move_to(cr,0,i*REVIEW_ROW_HEIGHT+0.5+META_ROW_HEIGHT);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,wide_dash,1,0);
	cairo_stroke(cr);

	//detector amplitude marker
	cairo_set_source_rgba(cr,1.0,0.4,0.4,0.4);
	for (i=0; i<max_rows; i++){
		cairo_move_to(cr,0,midline(i) + BARK_THRESHOLD - SAMPLE_SILENCE + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0, midline(i) - (BARK_THRESHOLD - SAMPLE_SILENCE) + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);

	//cooldown/calm amplitude marker
	cairo_set_source_rgba(cr,0.4,0.8,1.0,0.2);
	for (i=0; i<max_rows; i++){
		cairo_move_to(cr,0,midline(i) + BARK_END - SAMPLE_SILENCE + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0,midline(i) - (BARK_END - SAMPLE_SILENCE) + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);

	//uninteresting noise level
	cairo_set_source_rgba(cr,0.4,0.8,0.4,0.2);
	for (i=0; i<max_rows; i++){
		cairo_move_to(cr,0,midline(i) + NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0,midline(i) - (NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE) + 0.5);
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);


	//file meta information, top row
	meta_row(cr, readfile, d);



	//make actual data

	//first row setup
	timecode(cr,5,META_ROW_HEIGHT,0);
	cairo_move_to(cr,0,midline(0)+0.5);
	data_config(cr);

	lastY = META_ROW_HEIGHT;
	while ((rd = read_dogfile(&d, buf, REVIEW_BUFFER_SIZE)) > 0){

		//if new row
		if (posX >= REVIEW_FILE_WIDTH) {
			lastY+=REVIEW_ROW_HEIGHT;
			posX=0.0;
			//row beginning label
			//@todo time value is way off
			timecode(cr,5,lastY,
						(lastY/REVIEW_ROW_HEIGHT * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL)
						/(SAMPLE_RATE*1.0));
			cairo_move_to(cr,0,lastY + REVIEW_ROW_HEIGHT/2+0.5);
			data_config(cr);

			if (lastY+REVIEW_ROW_HEIGHT > max_rows*REVIEW_ROW_HEIGHT+META_ROW_HEIGHT){
				lastY -= REVIEW_ROW_HEIGHT;
				break;
			}
		}


		//process samples in buffer
		for (j=0; j<rd; j++, samples_seen++){
			//if skipping data due to quietness
			if (quiet_axis_break){
				if (buf[j] > NOISE_OF_INTEREST_LEVEL || buf[j] < SAMPLE_SILENCE - (NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE)){
					//broke the silence
					samples_of_silence=0;
					quiet_axis_break=0;
					timecode(cr, posX, lastY, 
								(lastY/REVIEW_ROW_HEIGHT * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL +
								samples_seen)
								/(SAMPLE_RATE*1.0) );
					data_config(cr);
					cairo_move_to(cr,posX, lastY + REVIEW_ROW_HEIGHT - buf[j] + 0.5);
					posX += 1/(SAMPLES_PER_PIXEL*1.0);
				} else {
					//still quiet, move on to next sample
					continue;
				}
			} else {
				if (buf[j] < NOISE_OF_INTEREST_LEVEL && buf[j] > SAMPLE_SILENCE - (NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE)){
					samples_of_silence++;
				} else {
					//peaked above NOIL
					samples_of_silence=0;
				}

				if (samples_of_silence >= SEC_OF_QUIET_TILL_SKIP  * SAMPLE_RATE){
					//been quiet long enough, start breaking the axis
					quiet_axis_break=1;
					axis_break(cr,&posX, lastY + REVIEW_ROW_HEIGHT/2,
						(lastY/REVIEW_ROW_HEIGHT * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL + 
									samples_seen)
								/(SAMPLE_RATE*1.0));
				} else {
					cairo_line_to(cr,posX, lastY + REVIEW_ROW_HEIGHT - buf[j] + 0.5);
					posX += 1/(SAMPLES_PER_PIXEL*1.0);
				}
			}
		}
	}


	close_file(d);

	cairo_stroke(cr);

	cairo_surface_t *surface2 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,REVIEW_FILE_WIDTH,lastY+REVIEW_ROW_HEIGHT);
	cairo_t *cr2 = cairo_create(surface2);
	cairo_set_source_surface(cr2,surface,0,0);
	cairo_paint(cr2);

	cairo_destroy(cr);
	cairo_destroy(cr2);
	cairo_surface_write_to_png(surface2, outfile);
	cairo_surface_destroy(surface);
	cairo_surface_destroy(surface2);
	return;
}


static float midline(int row){
	return row*REVIEW_ROW_HEIGHT + REVIEW_ROW_HEIGHT/2 + META_ROW_HEIGHT;
}

static void data_config(cairo_t *cr){
	cairo_set_dash(cr,NULL,0,0); //disable any dashes
	cairo_set_source_rgba(cr,0.0,1.0,1.0,1.0);
	cairo_set_line_width(cr,1);
}

static void label_config(cairo_t *cr){
	cairo_select_font_face(cr,"sans serif",CAIRO_FONT_SLANT_NORMAL,CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr,13);
	cairo_set_source_rgba(cr,0.0,0.0,0.0,1.0);
}

static void timecode(cairo_t *cr, float x, float y, float t){
	char label[80]; /* @todo guarantee enough length here */

	label_config(cr);
	cairo_move_to(cr,x,y+13.5);
	if (t < 60.0){
		sprintf(label, "%-.2fs",t);
	} else if (t < 3600.0){
		sprintf(label, "%-.2fm",t/60.0);
	} else if (t < 86400.0){
		sprintf(label, "%-.2fh",t/3600.0);
	} else {
		sprintf(label, "%-.2fd",t/86400.0);
	}
	cairo_show_text(cr,label);
}

static void axis_break(cairo_t *cr, float *x, float y, float t){
	int gap = 5,
		skew = 2,
		height = 20;

	//close any previous paths
	cairo_stroke(cr);

	timecode(cr,*x-50,y - REVIEW_ROW_HEIGHT/2-0.5,t);

	cairo_set_source_rgba(cr,0.0,0.0,0.0,0.6);
	cairo_set_dash(cr,NULL,0,0);
	cairo_set_line_width(cr,1.5);

	cairo_move_to(cr,*x-skew,y+height);
	cairo_line_to(cr,*x+skew,y-height);

	cairo_move_to(cr,*x+gap-skew,y+height);
	cairo_line_to(cr,*x+gap+skew,y-height);
	*x += gap+skew;
	cairo_stroke(cr);

	data_config(cr);

}

static void meta_row(cairo_t *cr, const char *filename, dogfile d){
	//cairo_text_extents_t ext;
	const char *dfmt = "%a, %b %e, %Y %r";
	char cmp_label[15],
		lossy_label[13],
		date_label[30];


	//line separator
	cairo_set_source_rgba(cr,0.3,0.3,0.3,0.1);
	cairo_move_to(cr,0,META_ROW_HEIGHT+0.5);
	cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	cairo_set_dash(cr,NULL,0,0);
	cairo_stroke(cr);


	//filename
	label_config(cr);
	cairo_move_to(cr,5,13.5);
	//cairo_text_extents(cr, filename, &ext);
	cairo_show_text(cr,filename);

	if (d.date){
		struct tm *tm_dt;
		tm_dt = localtime(&d.date);
		cairo_rel_move_to(cr,20,0);
		strftime(date_label, 30, dfmt, tm_dt);
		cairo_show_text(cr,date_label);
	}

	//add spacing after name
	//cairo_rel_move_to(cr,ext.x_advance + 45,0);
	cairo_rel_move_to(cr,75,0);
	
	//file compression
	sprintf(cmp_label, "Compression: %d", d.compression);
	//cairo_text_extents(cr,cmp_label,&ext);
	cairo_show_text(cr,cmp_label);

	//cairo_rel_move_to(cr,ext.x_advance+10,0);
	cairo_rel_move_to(cr,25,0);


	//lossiness
	sprintf(lossy_label, "Lossiness: %d", d.lossiness);
	//cairo_text_extents(cr,lossy_label,&ext);
	cairo_show_text(cr,lossy_label);

}
