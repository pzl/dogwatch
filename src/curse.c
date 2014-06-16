#include <ncurses.h>
#include <unistd.h>
#include <stdio.h>

#define BUFFER_SIZE 256

typedef struct pos {
	int y;
	int height; 
} pos;

pos map(unsigned char);

int main (int argc, char **argv){

	int x=0,
		y=0,
		nextx=0,
		dir=1,
		i, j;
	pos ln;
	unsigned int sz;

	unsigned char buffer[BUFFER_SIZE];
	unsigned char *bp;
	FILE *fp;

	fp = fopen("out/record.raw","rb");
	fseek(fp,0L,SEEK_END);
	sz = ftell(fp);
	fseek(fp,0L,SEEK_SET);

	size_t bytes_read = 0;
	//bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fp);
	//bp = buffer;

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


	for (i=0; i<(sz/BUFFER_SIZE); i++){
		clear();
		if (has_colors()){
			attron(COLOR_PAIR(1));
		}
		mvhline(LINES/2,0,'-',COLS);

		if (has_colors()){
			attron(COLOR_PAIR(2));
		}

		fseek(fp,BUFFER_SIZE*i,SEEK_SET);
		bytes_read = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, fp);
		bp = buffer;

		for (j=0; j<BUFFER_SIZE; j++){		
			ln = map(*bp++);
			mvvline(ln.y,j,'|',ln.height);
		}
		refresh();
		//sleep(1);
		usleep(80000);
	}
	refresh();

	endwin();



}

pos map(unsigned char c){
	pos p;
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
