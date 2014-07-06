#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "audioin.h"
#include "file.h"

static void extend_buf(SAMPLE **, int bytes);
static int drain_overflow(filebuf *request, int expand);
static int _read(FILE *, filebuf *, int nbytes);
static int sample_cpy(filebuf *from, filebuf *to, int expand);


filebuf overflow = {
	.index = 0,
	.len = 0
};


/*
 * creates new file following DOG file spec, writing basic header to file
 * 
 * name: file name, including extension
 * @return: dogfile object representing new file
 */
dogfile create_dogfile(const char *name, unsigned char compressed, unsigned char lossy){
	dogfile d;
	unsigned char header[FILE_HEADER_SIZE] = { 255, 'D', 'O', 'G', 1, 6 },
				  meta[6] = { 1, 1, compressed, 1, 2, lossy };
	d.fp = fopen(name,"wb");
	if (d.fp == NULL){
        fprintf(stderr, "Could not open file \"%s\" for writing\n", name);
        exit(1);
    }
    fwrite(header, sizeof(unsigned char), FILE_HEADER_SIZE, d.fp);
    fwrite(meta, sizeof(unsigned char), 6, d.fp);

    d.version=1;
    d.compression=compressed;
    d.lossiness=lossy;

    return d;
}


/*
 * opens dogfile, parses and validates header. 
 *
 * name: file path + name, including extension
 * @return: dogfile of open file, seeked to data portion
 */
dogfile open_dogfile(const char *name){
	dogfile d;
	unsigned char headerID[FILE_HEADER_SIZE];
	unsigned char *metaheader;
	unsigned char dsize, key;
	int header_meta_length, i;

	d.fp = fopen(name,"r+b");
	if (d.fp == NULL){
        fprintf(stderr, "Could not open file for writing\n");
        exit(1);
    }
    fread(headerID, sizeof(unsigned char), FILE_HEADER_SIZE, d.fp);

    //validate ID header
    if (headerID[0] == 255 && 
    	headerID[1] == 'D' &&
    	headerID[2] == 'O' &&
    	headerID[3] == 'G' ){
		d.version = headerID[4];

		//meta info defaults
		d.compression = DF_COMPRESSED;
		d.lossiness = DF_LOSSLESS;

		//parse Meta header
		header_meta_length = headerID[5];
		metaheader = malloc(header_meta_length * sizeof(unsigned char));
		fread(metaheader, sizeof(unsigned char), header_meta_length, d.fp);
		for (i=0; i<header_meta_length; i++){
			dsize=metaheader[i++];
			key=metaheader[i++];

			/*
				@todo: not doing anything with dsize yet
			*/

			if (key == 1){
				d.compression = metaheader[i];
			} else if (key == 2){
				d.lossiness = metaheader[i];
			}
		}

		free(metaheader);

    } else {
    	fprintf(stderr, "error parsing %s: header incorrect.\n", name);
    }
    return d;
}

/*
 * file writing thread. Waits on semaphore signal
 *
 * param should be writer *
 */
void *file_writer(void *wargs){
	writer *args = (writer *)wargs;
	SAMPLE *packet;

	while (1){
		sem_wait(args->data->writer);

		packet = &(args->data->recorded[args->data->pstart]);
		write_packet(&(args->df), packet, args->data->plen);
	}

	return NULL;
}

/*
 * writes from SAMPLE * to FILE *, and may compress data according to DOG spec
 *
 * FILE *: file to write to
 * SAMPLE *: buffer to write from
 * plen: number of bytes from SAMPLE * to read and encode. May not match
 *			bytes written
 *
 * @return: 0 for now (@todo)
 */
int write_packet(dogfile *d, SAMPLE *packet, int plen){
	SAMPLE buf[2*FRAMES_PER_PACKET];
	int i,
		repeat=0,
		buflen=0;

	if (d->compression == DF_COMPRESSED){

		for (i=0; i<plen; i++) {

			//use compression to represent data val 0
			if (packet[i] == 0 ){
				repeat=0; //reset continuous silence counter

				buf[buflen++] = 0;
				buf[buflen++] = 0;
				buf[buflen++] = 1;

				continue;
			} else if (packet[i] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
			           packet[i] <= SAMPLE_SILENCE + LOSSY_LEVEL){
				repeat++;

				if (repeat >= COMPRESS_AFTER_TIMES){
					i+=1; //we already counted the sample we're on 

					while (i < plen && 
					       repeat<255 && 
					       packet[i] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
					       packet[i] <= SAMPLE_SILENCE + LOSSY_LEVEL){
						i++;
						repeat++;
					}

					i-=1; //will increment one more on continue

					while (buf[buflen-1] >= SAMPLE_SILENCE - LOSSY_LEVEL &&
					       buf[buflen-1] <= SAMPLE_SILENCE + LOSSY_LEVEL){
						//rewind the buffer
						buflen--;
					}

					//compress escape, value, Nrepeats
					buf[buflen++] = 0;
					buf[buflen++] = SAMPLE_SILENCE;
					buf[buflen++] = repeat;

					repeat=0;
					continue;
				}
			} else {
				repeat = 0;
			}

			buf[buflen++] = packet[i];
		}

		fwrite(buf, CHANNELS*sizeof(SAMPLE), buflen, d->fp);
	} else {
		//uncompressed
		fwrite(packet, CHANNELS*sizeof(SAMPLE), plen, d->fp);
	}
	
	return 0;
}

int read_dogfile(dogfile *d, SAMPLE *reqbuf, int reqbytes){
	filebuf readin;
	filebuf requested;

	readin.index=0;
	readin.len = 0;
	requested.buf = reqbuf;
	requested.index=0;
	requested.len = reqbytes;


	int copied=0;


	/*
		If we have buffered bytes available, send those from overflow buf
		before doing another read
	*/
	if (d->compression == DF_COMPRESSED){
		requested.index = drain_overflow(&requested, _FILECPY_EXPAND);
	} else {
		requested.index = drain_overflow(&requested, _FILECPY_NOEXPAND);
	}
	if (requested.index == requested.len){
		//buffer completely filled up the request!
		return requested.len;
	}
	copied = requested.index;


	/*
		Requested bytes not satisfied yet, read remaining bytes
		from file. May get slightly more or less bytes than
		asked for. Compressions not expanded yet
	*/
	readin.len = _read(d->fp, &readin, requested.len-requested.index);

	/*
		at this point we should have an a full readin buffer, (possibly overfull)
		of and an emptied overflow buffer

		copy and expand from readin bytes to provided result buffer
	*/
	if (d->compression == DF_COMPRESSED){
		copied += sample_cpy(&readin, &requested, _FILECPY_EXPAND);
	} else {
		//uncompressed file, don't perform expansion
		copied += sample_cpy(&readin, &requested, _FILECPY_NOEXPAND);
	}

	/*
	printf("df: requested %d bytes. copied %d bytes. read in %d bytes. readin index: %d, remaining: %d bytes\n",
	 	requested.len,
	 	copied,
	 	readin.len,
	 	readin.index,
	 	readin.len - readin.index);
	*/


	//info left to throw into buffer
	if (readin.len > readin.index){
		//at least enough for samples as read
		overflow.len = readin.len - readin.index;
		overflow.buf = malloc( (overflow.len) * sizeof(SAMPLE));
		if (overflow.buf == NULL){
			fprintf(stderr, "Could not allocate mem for overflow read buffer\n");
			return 0;
		}
		//don't expand when holding in mem
		sample_cpy(&readin, &overflow, _FILECPY_NOEXPAND);
		overflow.index=0;

	}

	free(readin.buf);

	return copied;
}



void close_file(dogfile d){
	fclose(d.fp);
}


static void extend_buf(SAMPLE **buf, int bytes){
	SAMPLE *t;

	t = realloc(*buf, bytes);
	if (t){
		*buf = t;
	} else {
		fprintf(stderr, "Error extending buffer!\n");
	}
}


static int drain_overflow(filebuf *request, int expand){
	int r=0;

	//make sure there IS a buffer to use
	if (overflow.index >= overflow.len || overflow.buf == NULL){
		return 0;
	}

	//copy while we haven't fulfilled request or until buffer empty
	r = sample_cpy(&overflow, request, expand);

	if (overflow.index >= overflow.len){
		//exhausted overflow buffer, reset it
		overflow.index=0;
		overflow.len=0;
		free(overflow.buf);
	}

	return r;
}

static int _read(FILE *fp, filebuf *rbuf, int bytes){
	SAMPLE *temp;
	int byt_read;

	/*
		Allocate space and read from file as normal
	*/
	rbuf->buf = malloc( (bytes) * sizeof(SAMPLE));
	if (rbuf->buf == NULL){
		fprintf(stderr, "error creating buffer for file reading\n");
		return 0;
	}

	//let's read some data!
	byt_read = fread(rbuf->buf, sizeof(SAMPLE), bytes, fp);
	if (byt_read == 0){
		//it wasn't EOF that caused underflow
		if (!feof(fp)){
			fprintf(stderr, "Problem reading dogfile!\n");
		}
		return 0;
	}



	/* 
		Check if we cut off reading in the middle of a compression sequence,
		and finish that sequence into read_buf
	 */
	if (rbuf->buf[byt_read-1] == 0){
		//printf("extending read buffer by 2\n");
		//last byte was 0, need two more

		//extend read_buf by 2b
		extend_buf(&(rbuf->buf), (bytes+2) * sizeof(SAMPLE));

		//use temp as tiny buf to read the next two bytes
		temp = malloc(2*sizeof(SAMPLE));
		fread(temp, sizeof(SAMPLE), 2, fp);

		//copy the two bytes to read_buf
		rbuf->buf[byt_read++] = temp[0];
		rbuf->buf[byt_read++] = temp[1];

		free(temp);

	} else if (rbuf->buf[byt_read-2] == 0){
		//printf("extending read buffer by 1\n");
		//second to last byte was 0, missing repetition byte of compress seq.

		//extend read_buf by a single byte
		extend_buf(&(rbuf->buf), (bytes+1) * sizeof(SAMPLE));

		//read the next byte
		temp = malloc(sizeof(SAMPLE));
		fread(temp, sizeof(SAMPLE), 1, fp);

		//copy byte to end of read_buf
		rbuf->buf[byt_read++] = temp[0];

		free(temp);
	}

	return byt_read;
}

static int sample_cpy(filebuf *from, filebuf *to, int expand){
	SAMPLE compress_val;
	int repeat, //if repeat was SAMPLE, would overflow at 0
		copied=0;

	//while in both bounds
	while (from->index < from->len &&
		   to->index < to->len){

		//if compressed sequence
		if (expand == _FILECPY_EXPAND && from->buf[from->index] == 0 ){
			compress_val = from->buf[++(from->index)];
			repeat = from->buf[++(from->index)];

			while (repeat--){
				//while room left in destination buffer
				if (to->index < to->len){
					to->buf[(to->index)++] = compress_val;
					copied++;
				} else {
					//filled up dest
					//rewind src buf to reinclude remainder
					//of compression sequence
					from->buf[from->index] = ++repeat; //+1 accounts for --
					from->index -= 2;

					break;
				}
			}
			//emptied compressed sequence, point to next byte
			if (repeat <= 0){
				from->index++;
			}

		} else {
			//simple copy over
			to->buf[(to->index)++] = from->buf[(from->index)++];
			copied++;
		}
	}

	return copied;
}
