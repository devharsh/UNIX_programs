#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

int main(int argc, char **argv)
{
	char *grep_args[] = {"grep", "Devharsh", NULL};

	char line[] = "grep Devharsh  git-logs.txt > file";

	char* read = strtok(line, "<");
	char* write = strtok(line, ">");
	char* append = strstr(line, ">>");

	//while(read) {
	//	printf("%s\n", read);
	//	read = strtok(NULL, "<");
	//}

	while(write) {
		printf("%s\n", write);
		write = strtok(NULL, ">");
	}

	//execvp("grep", grep_args);
}
