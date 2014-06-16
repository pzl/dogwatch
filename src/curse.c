#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include "curse.h"


static wave_pos map(unsigned char);

void nc_view(const char *fn){

	unsigned int i, j;
	wave_pos ln;
	size_t bytes_read = 0,
			fsize = 0;
	struct timespec wait;
	wait.tv_sec=0;

	initscr();


	unsigned char buffer[COLS];
	unsigned char *bp;
	FILE *fp;

	fp = fopen(fn,"rb");
	fseek(fp,0L,SEEK_END);
	fsize = ftell(fp);
	fseek(fp,0L,SEEK_SET);



	initscr();
	noecho();
	curs_set(FALSE);

	if (has_colors()){
		use_default_colors();
		start_color();
		init_pair(1,COLOR_RED,-1);
		attron(COLOR_PAIR(1));
	}

	mvhline(LINES/2,0,'-',COLS);

	if (has_colors()){
		init_pair(2,COLOR_BLUE,COLOR_BLUE);
		attron(COLOR_PAIR(2));
	}


	for (i=0; i<(fsize/COLS); i++){
		clear();
		if (has_colors()){
			attron(COLOR_PAIR(1));
		}
		mvhline(LINES/2,0,'-',COLS);

		if (has_colors()){
			attron(COLOR_PAIR(2));
		}

		fseek(fp,COLS*i,SEEK_SET);
		bytes_read = fread(buffer, sizeof(unsigned char), COLS, fp);
		bp = buffer;

		for (j=0; j<bytes_read; j++){		
			ln = map(*bp++);
			mvvline(ln.y,j,'|',ln.height);
		}
		refresh();
		wait.tv_nsec = 16750000; //8000Hz to ns times page len (COL)
		nanosleep(&wait,NULL);
	}
	refresh();

	endwin();



}

static wave_pos map(unsigned char c){
	wave_pos p;
	int peak;

	peak = (c * LINES)/255;

	if (peak < LINES/2){
		p.y = peak;
		p.height = LINES/2 - peak;
	} else if (peak > LINES/2){
		p.y = LINES/2+1;
		p.height = peak - LINES/2-1;
	} else {
		p.y = peak;
		p.height = 1;
	}
	return p;
}
