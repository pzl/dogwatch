#include <stdio.h>
#include <cairo.h>
#include <time.h>
#include "audioin.h"
#include "detection.h"
#include "file.h"
#include "reviewer.h"

static float scale(float y);
static float midline(int row);
static float bottom(int row);
static float crisp(float val);
static void data_config(cairo_t *);
static void label_config(cairo_t *);
static void timecode(cairo_t *, float x, int row, float time);
static void axis_break(cairo_t *, float *x, int row, float time);
static void meta_row(cairo_t *, const char *, dogfile);


void png_view_create(const char *readfile, const char *outfile){
	dogfile d;
	long long samples_of_silence=0,
			  samples_seen=0;
	int i, rd,
		quiet_axis_break=0,
		row=0;
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


	printf("max rows: %d\n", MAX_ROWS);


	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, REVIEW_FILE_WIDTH, REVIEW_ROW_HEIGHT*MAX_ROWS + META_ROW_HEIGHT);
	cairo_t *cr = cairo_create(surface);

	//background all white
	cairo_set_source_rgba(cr,1.0,1.0,1.0,1.0);
	cairo_rectangle(cr,0,0,REVIEW_FILE_WIDTH,bottom(MAX_ROWS-1));
	cairo_fill(cr);

	//center lines for each row
	cairo_set_source_rgba(cr,0.3,0.3,0.3,0.6);
	cairo_set_line_width(cr,1);
	for (i=0; i<MAX_ROWS; i++){
		cairo_move_to(cr,0, crisp(midline(i)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_stroke(cr);

	//extent/limit lines between rows
	cairo_set_source_rgba(cr,0.0,0.0,0.0,0.4);
	for (row=0; row<MAX_ROWS; row++){
		cairo_move_to(cr,0, crisp(bottom(row)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,wide_dash,1,0);
	cairo_stroke(cr);

	//detector amplitude marker
	cairo_set_source_rgba(cr,1.0,0.4,0.4,0.4);
	for (row=0; row<MAX_ROWS; row++){
		cairo_move_to(cr,0, crisp(bottom(row) - scale(BARK_THRESHOLD)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0, crisp(midline(row) + scale(BARK_THRESHOLD - SAMPLE_SILENCE)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);

	//cooldown/calm amplitude marker
	cairo_set_source_rgba(cr,0.4,0.8,1.0,0.2);
	for (row=0; row<MAX_ROWS; row++){
		cairo_move_to(cr,0, crisp(bottom(row) - scale(BARK_END)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0, crisp(midline(row) + scale(BARK_END - SAMPLE_SILENCE)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);

	//uninteresting noise level
	cairo_set_source_rgba(cr,0.4,0.8,0.4,0.2);
	for (row=0; row<MAX_ROWS; row++){
		cairo_move_to(cr,0, crisp(bottom(row) - scale(NOISE_OF_INTEREST_LEVEL)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
		cairo_move_to(cr,0, crisp(midline(row) + scale(NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE)));
		cairo_rel_line_to(cr,REVIEW_FILE_WIDTH,0);
	}
	cairo_set_dash(cr,thin_dash,1,0);
	cairo_stroke(cr);


	//file meta information, top row
	meta_row(cr, readfile, d);



	//make actual data

	//first row setup
	timecode(cr,5,META_ROW_HEIGHT,0);
	cairo_move_to(cr,0,crisp(midline(0)));
	data_config(cr);

	row=0;
	while ((rd = read_dogfile(&d, buf, REVIEW_BUFFER_SIZE)) > 0){

		//if new row
		if (posX >= REVIEW_FILE_WIDTH) {
			row++;
			posX=0.0;
			//row beginning label
			//@todo time value is way off
			/*
			timecode(cr,5,row,
						(row * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL)
						/(SAMPLE_RATE*1.0));
			*/
			cairo_move_to(cr,0,crisp(midline(row)));
			data_config(cr);

			if (bottom(row) > bottom(MAX_ROWS-1) ){
				row--;
				break;
			}
		}


		//process samples in buffer
		for (i=0; i<rd; i++, samples_seen++){
			//if skipping data due to quietness
			if (quiet_axis_break){
				if (buf[i] > NOISE_OF_INTEREST_LEVEL || buf[i] < SAMPLE_SILENCE - (NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE)){
					//broke the silence
					samples_of_silence=0;
					quiet_axis_break=0;
					timecode(cr, posX, row, 
								(row * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL +
								samples_seen)
								/(SAMPLE_RATE*1.0) );
					data_config(cr);
					cairo_move_to(cr,posX, bottom(row) - scale(buf[i]));
					posX += 1/(SAMPLES_PER_PIXEL*1.0);
				} else {
					//still quiet, move on to next sample
					continue;
				}
			} else {
				if (buf[i] < NOISE_OF_INTEREST_LEVEL && buf[i] > SAMPLE_SILENCE - (NOISE_OF_INTEREST_LEVEL - SAMPLE_SILENCE)){
					samples_of_silence++;
				} else {
					//peaked above NOIL
					samples_of_silence=0;
				}

				if (samples_of_silence >= SEC_OF_QUIET_TILL_SKIP  * SAMPLE_RATE){
					//been quiet long enough, start breaking the axis
					quiet_axis_break=1;
					axis_break(cr,&posX, row,
						(row * REVIEW_FILE_WIDTH*SAMPLES_PER_PIXEL + 
									samples_seen)
								/(SAMPLE_RATE*1.0));
				} else {
					cairo_line_to(cr,posX, bottom(row) - scale(buf[i]));
					posX += 1/(SAMPLES_PER_PIXEL*1.0);
				}
			}
		}
	}


	close_file(d);

	cairo_stroke(cr);

	cairo_surface_t *surface2 = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,REVIEW_FILE_WIDTH,bottom(row));
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


/*
 * takes float originally scaled on 0-255
 * and scales to our row domain
 */
static float scale(float y){
	return (y*REVIEW_ROW_HEIGHT)/256.0;
}

static float midline(int row){
	return row*REVIEW_ROW_HEIGHT + REVIEW_ROW_HEIGHT/2 + META_ROW_HEIGHT;
}

static float bottom(int row){
	return (row+1)*REVIEW_ROW_HEIGHT + META_ROW_HEIGHT;
}

static float crisp(float val){
	int v;
	v = (int) val;
	return v + 0.5;
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

static void timecode(cairo_t *cr, float x, int row, float t){
	char label[80]; /* @todo guarantee enough length here */

	label_config(cr);
	cairo_move_to(cr,x,bottom(row-1)+13.5);
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

static void axis_break(cairo_t *cr, float *x, int row, float t){
	int gap = 5,
		skew = 2,
		height = 20;

	//close any previous paths
	cairo_stroke(cr);

	timecode(cr,*x-50,row,t);

	cairo_set_source_rgba(cr,0.0,0.0,0.0,0.6);
	cairo_set_dash(cr,NULL,0,0);
	cairo_set_line_width(cr,1.5);

	cairo_move_to(cr,*x-skew,midline(row)+height);
	cairo_line_to(cr,*x+skew,midline(row)-height);

	cairo_move_to(cr,*x+gap-skew,midline(row)+height);
	cairo_line_to(cr,*x+gap+skew,midline(row)-height);
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
	cairo_move_to(cr,0,crisp(META_ROW_HEIGHT));
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
