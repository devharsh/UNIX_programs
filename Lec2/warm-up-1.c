#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef BUFFSIZE
#define BUFFSIZE 32768
#endif 

int main()
{
	int n;
	int n2;
	char buf[BUFFSIZE];

	while((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0)
	{
		printf("STDIN_FILENO: %d\n", STDIN_FILENO);
		printf("STDOUT_FILENO: %d\n", STDOUT_FILENO);

		n2 = write(STDOUT_FILENO, buf, n);
		printf("STDIN_FILENO: %d\n", STDIN_FILENO);
		printf("STDOUT_FILENO: %d\n", STDOUT_FILENO);

		if(n2 != n)
		{
			printf("STDIN_FILENO: %d\n", STDIN_FILENO);
			printf("STDOUT_FILENO: %d\n", STDOUT_FILENO);

			fprintf(stderr, "write error\n");
			return(1);
		}
	}
	
	if(n < 0)
	{
		fprintf(stderr, "read error\n");
		return(1);
	}

	return(0);
}
