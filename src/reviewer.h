#ifndef REVIEW_H
#define REVIEW_H

#define REVIEW_BUFFER_SIZE 512

#define REVIEW_ROW_HEIGHT 256
#define REVIEW_FILE_WIDTH 5000
#define SAMPLES_PER_PIXEL 16

#define META_ROW_HEIGHT 20

#define NOISE_OF_INTEREST_LEVEL 140
#define SEC_OF_QUIET_TILL_SKIP	1

void png_view_create(const char *audiofile, const char *pngfile);

#endif
