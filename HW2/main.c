#include "command.h"

#define BUFSIZ 1024

int
main (/*int argc, char **argv*/) {
	char out[BUFSIZ], err[BUFSIZ];
	
	if (command("ls -l", out, BUFSIZ, err, BUFSIZ) == -1) {
		perror("command");
		exit(1);
	}

	printf("stdout:\n%s\n", out);
	printf("stderr:\n%s\n", err);

	return 0;
}
