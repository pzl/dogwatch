#ifndef REVIEW_H
#define REVIEW_H

#define REVIEW_FILE_HEIGHT 256
#define REVIEW_FILE_WIDTH 8192
#define SAMPLES_PER_PIXEL 4

void png_view_create(const char *audiofile, const char *pngfile);

#endif