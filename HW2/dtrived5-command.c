#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
sig_handler_exit(int signal) {
	printf("Caught signal %d, exiting\n", signal);
	exit(1);
}

void
sig_handler_continue(int signal) {
	printf("Caught signal %d, ignoring\n", signal);
}

int
command (const char* string, char* outbuf, int outlen, char *errbuf,
int errlen) {
	signal(SIGHUP, 	sig_handler_exit);
	signal(SIGINT, 	sig_handler_exit);
	signal(SIGABRT, sig_handler_exit);

	signal(SIGQUIT, sig_handler_continue);
	signal(SIGILL, 	sig_handler_continue);
	signal(SIGTRAP, sig_handler_continue);

	if(string == NULL)
		return -1;

	pid_t pid;
	FILE *fp;
	fp = popen(string, "r+");

	if((pid = fork()) < 0) {
		fprintf(stderr, "fork error: %s\n",
			strerror(errno));
		exit(1);
	}

	if(fp == NULL) {
		perror("command execution error");
		fgets(errbuf, errlen, fp);	
	} else {
		while(1) {
			char* line;
			line = fgets(outbuf, outlen, fp);
			
			if(line == NULL)
				break;
		}
	}
	
	return 0;
}
