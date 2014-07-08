#ifndef REVIEW_H
#define REVIEW_H

#define REVIEW_BUFFER_SIZE 512

#define REVIEW_ROW_HEIGHT 64
#define REVIEW_FILE_WIDTH 1080
#define SAMPLES_PER_PIXEL 16
#define MAX_ROWS 50

#define META_ROW_HEIGHT 20

#define NOISE_OF_INTEREST_LEVEL 140
#define SEC_OF_QUIET_TILL_SKIP	0.3

void png_view_create(const char *audiofile, const char *pngfile);

#endif
