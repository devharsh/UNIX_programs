#include<stdio.h>
#include<unistd.h>

int main(int argc, char* argv[]) {
	printf("%s\n", argv[1]);
	if(chdir(argv[1]) != 0) {
		perror("chdir() failed");
		return 1;
	}
	return 0;
}
