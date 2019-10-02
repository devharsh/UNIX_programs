/*
 * HW-1 - bbcp.c - bare-bones copy a file
 * Author: Devharsh Trivedi
 * Email: dtrived5@stevens.edu
*/

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 1 MB buffer size for copying */
#ifndef BUF_SIZE
#define BUF_SIZE 1048576 
#endif

/* limit for a file name */
#ifndef BUF_LIMIT
#define BUF_LIMIT 512
#endif

/*
* This is a very trivial program to copy a file
* It takes two arguments: source-file and target-file
* Source must be a valid file and not a directory
* Target can either be a file or a directory
*/
int
main(int argc, char** argv) {
	int inputFD, outputFD;
	char* link_path;
	char filebuf[BUF_SIZE];
	char target_path[BUF_LIMIT];
	struct stat buf;
	ssize_t number, len;

	if(argc != 3) {
		fprintf(stderr, "usage: ./bbcp source-file target-file\n");
		exit(1);
	}
	
	if (strcmp(argv[1], argv[2]) == 0) {
		fprintf(stderr, "both files are same (no action)\n");
		exit(1);
	}

	link_path = argv[2];
	len = readlink(link_path, target_path, sizeof(target_path)-1);
	if (len != -1) {
		target_path[len] = '\0';
		if (strcmp(argv[1], target_path) == 0) {
			fprintf(stderr, "both files are same (no action)\n");
			exit(1);
		}
	}
	
	if (access(argv[1], R_OK) != 0) {
		fprintf(stderr, "%s is not readable (access denied)\n", argv[1]);
		exit(1);
	}
	
	stat(argv[1], &buf);

	if(S_ISREG(buf.st_mode)) {
	} else {
		fprintf(stderr, "Not a valid file: %s\n", argv[1]);
		exit(1);
	}
	
	inputFD = open(argv[1], O_RDONLY);
	if(inputFD == -1) {
		fprintf(stderr, "Error opening file: %s\n", argv[1]);
		exit(1);
	}

	if(access((dirname(strdup(argv[2]))), W_OK) != 0) {
		fprintf(stderr, "%s is not writable(access denied)", dirname(strdup(argv[2])));
		exit(1);
	}

	stat(argv[2], &buf);

	/* To create a subdirectory if it does not exist */
	mkdir(dirname(strdup(argv[2])),0777);

	if(S_ISDIR(buf.st_mode)) {
		strcat(argv[2],"/");
		strcat(argv[2],basename(strdup(argv[1])));
	}

	outputFD = open(argv[2], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if(outputFD == -1) {
		fprintf(stderr, "Error opening file:%s\n", argv[2]);
		exit(1);
	}

	while((number = read(inputFD, filebuf, BUF_SIZE)) > 0) {
		if(write(outputFD, filebuf, number) != number) {
			fprintf(stderr, "Error writing file\n");
			exit(1);
		}
	}
	if(number == -1) {
		fprintf(stderr, "Error writing file\n");
		exit(1);
	}

	if(close(inputFD) == -1) {
		fprintf(stderr, "Error closing file:%s\n", argv[1]);
		exit(1);
	}
	if(close(outputFD) == -1) {
		fprintf(stderr, "Error closing file:%s\n", argv[2]);
		exit(1);
	}

	return(0);
}
