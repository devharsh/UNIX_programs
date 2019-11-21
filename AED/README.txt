localhost$ ls -a
.      ..     aed.c  test   test.c
localhost$ ./test < test.c > test.out
localhost$ ls -a
.        ..       aed.c    test     test.c   test.out
localhost$ diff test.c test.out
localhost$ cat test.out
#include <stdio.h> 
#define SIZE 256
int main() 
{
        char buf[SIZE];
        while(fgets(buf, SIZE, stdin) != NULL) 
                printf("%s", buf);
        return 0; 
} 

localhost$ cat test.c
#include <stdio.h> 
#define SIZE 256
int main() 
{
        char buf[SIZE];
        while(fgets(buf, SIZE, stdin) != NULL) 
                printf("%s", buf);
        return 0; 
} 


