#include <stdio.h>
#include <stdlib.h> //exit
#include <unistd.h> //fork, readlink
#include <string.h> //strndup
#include <libgen.h> //dirname
#include <sys/types.h> //pid_t
#include "alert.h"

static char *curdir(void);


void alert(int barks) {
	pid_t child;
	char path[4096];
	char strbark[1024];
	char *cdir;

	if ((child = fork()) == -1) {
		perror("fork");
	}

	if (child == 0){
		cdir = curdir();
		snprintf(path,  4096, "%s/extra/sendmail",cdir);
		free(cdir);
		snprintf(strbark, 1024, "%d", barks);

		execlp(path,"sendmail",strbark,NULL);
		perror("exec");
		exit(1);
	} else {
		//parent
		return;
	}
}

/*
 * returned pointer should be freed when done!
 */
static char *curdir(void){
	char buf[2048];
	char *dir;
	ssize_t len;

	len = readlink("/proc/self/exe",buf,2048);
	if (len < 0){
		perror("readlink");
	}
	buf[len] = 0;

	dir = dirname(buf);
	return strndup(dir,2048);
}
