#include <stdio.h> 
#define SIZE 256
int main() 
{
	char buf[SIZE];
	while(fgets(buf, SIZE, stdin) != NULL) 
   		printf("%s", buf);
	return 0; 
} 

