/*
* A dummy program to play around with gdb
* author: Devharsh Trivedi
* email: dtrived5@stevens.edu
*/

#include <stdio.h>
#include <stdlib.h>

int fun1(int fa1, int fb1);
int fun2(int fa2, int fb2);
int fun3(int fa3, int fb3);

/*
* This program takes two integers as user input
* and performs arithmetic operations to inspect
* registry elements of gdb
*/
int 
main(int argc, char ** argv) {
	if(argc != 3) {
		perror("argument error in main");
		return 1;
	}
	printf("output = %d\n", fun1(atoi(argv[1]), atoi(argv[2])));
	return 0;
}

int 
fun1(int fa1, int fb1) {
	return fun2(fa1+fb1, fa1-fb1);
}

int
fun2(int fa2, int fb2) {
	return fun3(fa2+fb2, fa2-fb2);
}

int
fun3(int fa3, int fb3) {
	return fa3%fb3;
}
