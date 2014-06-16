#ifndef CURSE_H
#define CURSE_H

typedef struct wave_pos {
	int y;
	int height; 
} wave_pos;

void nc_setup(void);
void nc_view(const char *);
void nc_stop(void);

#endif
