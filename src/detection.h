#ifndef DETECT_H
#define DETECT_H

#define BARK_THRESHOLD 150
#define BARK_END 135
#define CALM_MS 300

void detection_start(void);
void *detect(void *data);
unsigned int detection_end(void);

#endif
