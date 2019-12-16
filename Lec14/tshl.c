#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

enum {ARGSIZE = 20, BUF_SIZE = 1024};    /* constants */

void execute (char **args);

int main (void) {

    while (1) {

        char line[BUF_SIZE] = "",
            *args[ARGSIZE],
            *delim = " \n",
            *token;
        int argIndex = 0;

        for (int i = 0; i < ARGSIZE; i++)  /* set all pointers NULL */
            args[i] = NULL;

        printf ("shell> ");             /* prompt */

        if (!fgets (line, BUF_SIZE, stdin)) {
            fprintf (stderr, "Input canceled - EOF received.\n");
            return 0;
        }
        if (*line == '\n')              /* Enter alone - empty line */
            continue;

        for (token = strtok (line, delim);        /* parse tokens */
                token && argIndex + 1 < ARGSIZE; 
                token = strtok (NULL, delim)) {
            args[argIndex++] = token;
        }

        if (!argIndex) continue;        /* validate at least 1 arg */

        if (strcmp (args[0], "quit") == 0 || strcmp (args[0], "exit") == 0)
            break;
	
	for(int i=0;i<ARGSIZE;i++) {
		if(args[i]!=NULL) {
			printf("%s\n", args[i]);
		}
	}
        execute (args);  /* call function to fork / execvp */

    }
    return 0;
}

void execute (char **args)
{
    pid_t pid, status;
    pid = fork ();

    if (pid < 0) {
        perror ("fork");
        return;
    }
    else if (pid > 0) {
        while (wait (&status) != pid)
            continue;
    }
    else if (pid == 0) {
        int idx = 0,
            fd;
        while (args[idx]) {   /* parse args for '<' or '>' and filename */
            if (*args[idx] == '>' && args[idx+1]) {
                if ((fd = open (args[idx+1], 
                            O_WRONLY | O_CREAT, 
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) {
                    perror (args[idx+1]);
                    exit (EXIT_FAILURE);
                }
                dup2 (fd, 1);
                dup2 (fd, 2);
                close (fd);
                while (args[idx]) {
                    args[idx] = args[idx+2];
                    idx++; 
                }
                break;
            }
            else if (*args[idx] == '<' && args[idx+1]) {
                if ((fd = open (args[idx+1], O_RDONLY)) == -1) {
                    perror (args[idx+1]);
                    exit (EXIT_FAILURE);
                }
                dup2 (fd, 0);
                close (fd);
                while (args[idx]) {
                    args[idx] = args[idx+2];
                    idx++; 
                }
                break;
            }
            idx++;
        }
        if (execvp (args[0], args) == -1) {
            perror ("execvp");
        }
        _exit (EXIT_FAILURE);   /* must _exit after execvp return, otherwise */
    }                           /* any atext calls invoke undefine behavior  */
}
