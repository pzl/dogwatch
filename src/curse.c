#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "curse.h"


static wave_pos map(unsigned char);

void nc_setup(void){

	/* ncurses setup */
	initscr();
	noecho();
	curs_set(FALSE);

	if (has_colors()){
		use_default_colors();
		start_color();
		init_pair(1,COLOR_RED,-1);
		init_pair(2,COLOR_BLUE,COLOR_BLUE);
	}

	/* draw baseline */
	if (has_colors()){
		attron(COLOR_PAIR(1));
	}
	mvhline(LINES/2,0,'-',COLS);
	refresh();
}

void nc_stop(void){
	endwin();
}


void *nc_view(void *snd){
	sound *data = (sound *)snd;
	SAMPLE *packet;
	int i;
	wave_pos ln;

	unsigned int j;
	size_t bytes_read = 0;
	struct timespec wait;
	wait.tv_sec=0;


	unsigned char buffer[COLS];
	unsigned char *bp;


	while (1){
		sem_wait(&(data->drawer));

		clear();
		if (has_colors()){
			attron(COLOR_PAIR(1));
		}
		mvhline(LINES/2,0,'-',COLS);

		if (has_colors()){
			attron(COLOR_PAIR(2));
		}

		packet = &(data->recorded[data->frameIndex * CHANNELS]);
		packet -= FRAMES_PER_BUFFER;
		for (i=0; i<COLS; i++){
			ln = map(*packet++);
			mvvline(ln.y,i,'|',ln.height);
		}
		refresh();
	}
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
