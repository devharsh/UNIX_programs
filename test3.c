#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <assert.h>
#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

static char line[1024];
static int n = 0;
static void split(char* cmd);

static char* args[512];
pid_t pid;
char buf[1024];
char shell_buf[1024];
char* query = NULL;
char* buf_cmd;
char ch;

int status = 0;
int is_c_on = 0;
int is_x_on = 0;
int data = 0;
int cnt = 0;
int startp[2];
int endp[2];
int is_bg_proc = 0;

void
sish_help() {
        fprintf(stderr,
        "Usage: %s [-x] [-c command]\n"
        "\t-x            Enable tracing mode\n"
        "\t-c command    Execute the given command\n",
        getprogname());
        exit(EXIT_FAILURE);
}

void execute (char *cmd)
{
	char* token;
	char* args[20];
	char *delim = " \n";
	int argIndex = 0;
	
	for (int i = 0; i < 20; i++) {
            args[i] = NULL;
	}

	for (token = strtok (cmd, delim);
                token && argIndex + 1 < 20; 
                token = strtok (NULL, delim)) {
            args[argIndex++] = token;
        }
 
	if(is_x_on) {
		if(args[0] != NULL) {
			fprintf(stderr, "+ %s", args[0]);
			if(args[1] != NULL) {
				fprintf(stderr, " %s", args[1]);
			}
			fprintf(stderr, "\n"); 
		}
	}
 
	/* shell built-in calls */
	if (strcmp(args[0], "exit") == 0) {
		exit(0);
      	} else if (strcmp(args[0], "cd") == 0) {
       		if(args[1]==NULL) {
       			if(chdir(getenv("HOME")) == -1) {
         			fprintf(stderr, "getenv error: %s\n", strerror(errno));
         			exit(1);
         		}
         	} else {
         		if(chdir(args[1]) == -1) {
                                fprintf(stderr, "chdir error: %s\n", strerror(errno));
                                exit(1);
                        }
                }
        } else {
		pid_t pid, status;
		pid = fork ();

    		if (pid < 0) {
        		perror ("fork");
        		return;
    		} else if (pid > 0) {
        		while (wait (&status) != pid) {
            			continue;
			}
    		} else if (pid == 0) {
        		int idx = 0,
            			fd;
        		while (args[idx]) {   
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
            			} else if (*args[idx] == '^' && args[idx+1]) {
                			if ((fd = open (args[idx+1], 
                            			O_WRONLY | O_APPEND, 
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
				} else if (*args[idx] == '<' && args[idx+1]) {
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
			if (strcmp(args[0], "echo") == 0) {
                        	if(strcmp(args[1], "$$") == 0) {
                                	printf("%d\n", (int)getpid());
                        	} else if(strcmp(args[1], "$?") == 0) {
                                	printf("%d\n", status);
                        	} else {
                                	printf("%s\n", args[1]);
                        	}
        		} else if (execvp (args[0], args) == -1) {
            			perror ("execvp");
        		}
        		_exit (EXIT_FAILURE); 
    		} 
	}                          
}

static int command(int input, int first, int last)
{
	if(is_x_on) {
		if(args[0] != NULL) {
			fprintf(stderr, "+ %s", args[0]);
			if(args[1] != NULL) {
				fprintf(stderr, " %s", args[1]);
			}
			fprintf(stderr, "\n"); 
		}
	}
	/* shell built-in calls */
	if (strcmp(args[0], "exit") == 0) {
		exit(0);
        } else if (strcmp(args[0], "echo") == 0) {
               	if(strcmp(args[1], "$$") == 0) {
			printf("%d\n", (int)getpid());
		} else if(strcmp(args[1], "$?") == 0) {
			printf("%d\n", status);
		} else {
			printf("%s\n", args[1]);
		}
	} else if (strcmp(args[0], "cd") == 0) {
		if(args[1]==NULL) {
			if(chdir(getenv("HOME")) == -1) {
				fprintf(stderr, "getenv error: %s\n", strerror(errno));
				exit(1);
			}
                } else {
                        if(chdir(args[1]) == -1) {
                                fprintf(stderr, "chdir error: %s\n", strerror(errno));
                                exit(1);
                        }
                }
	} else {
		int pipes[2];
       		pipe(pipes);

       		if((pid=fork()) == -1) {
                       	fprintf(stderr, "shell: can't fork: %s\n", strerror(errno));
                       	exit(EX_DATAERR);
       		} else if (pid == 0) {
               		if (first == 1 && last == 0 && input == 0) {
                       		dup2(pipes[1], STDOUT_FILENO );
               		} else if (first == 0 && last == 0 && input != 0) {
                       		dup2(input, STDIN_FILENO);
                       		dup2(pipes[1], STDOUT_FILENO);
               		} else {
                       		dup2( input, STDIN_FILENO );
               		}
               		execvp(args[0], args);
			fprintf(stderr, "shell: couldn't exec %s\n", strerror(errno));
			exit(EX_DATAERR);
		}

       		if (input != 0) {
               		close(input);
		}

       		close(pipes[1]);

       		if (last == 1) {
               		close(pipes[0]);
		}

       		return pipes[0];
	}
}

int 
main(int argc, char** argv) {
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "signal error: %s\n", strerror(errno));
		exit(1);
	}

	if(getcwd(shell_buf, 1024) == NULL) {
        	fprintf(stderr, "getcwd error: %s\n", strerror(errno));
		exit(1);
	}

    	if(setenv("SHELL", shell_buf, 1) == -1) {
        	fprintf(stderr, "setenv error: %s\n", strerror(errno));
		exit(1);
	}

	while ((ch = getopt(argc, argv, "c:x")) != -1) {
		switch (ch) {
			case 'x':
				is_x_on = 1;
				break;
			case 'c':
				if (optarg) {
					query = optarg;
					is_c_on = 1;
				} else {
					sish_help();
					exit(0);
				}
				break;
			default:
				break;
			}
	}

	while (1) {
		char* cmd;
		int input = 0;
		int first = 1;

		if(!is_c_on) {
			printf("sish$ ");
                	fflush(NULL);
                	if (!fgets(line, 1024, stdin)) {
				return 0;
			}
			cmd = line;
		} else {
			cmd = query;
		}

                char* next = strchr(cmd, '|');
		char* read = strstr(cmd, "<");
		char* write = strstr(cmd, ">");
		char* append = strstr(cmd, ">>");

		if(append) {
			int j_second = 0;
			for(int j=0; j<strlen(cmd); j++) {
				if(j_second) {
					cmd[j] = '^';
					break;
				} else if(cmd[j] == '>') {
					cmd[j] = ' ';
					j_second = 1;
				}
			}
		}

		if(read || write || append) {
			execute(cmd);
			continue;
		}

		while (next != NULL) {
                	*next = '\0';
			split(cmd);
        		if (args[0] != NULL) {
                		n += 1;
                		input = command(input, first, 0);
        		} else {
				input = 0;
			}
                       	cmd = next + 1;
                       	next = strchr(cmd, '|');
                       	first = 0;
               	}
		split(cmd);
        	if (args[0] != NULL) {
			n += 1;
               		input = command(input, first, 1);
        	} else {
			input = 0;
		}
               	if(!is_bg_proc) {
			for (int i = 0; i < n; ++i)
                       		wait(NULL);
		}
                n = 0;
		if(is_c_on) {
			break;
		}
        }
        return 0;
}

static void split(char* cmd)
{
	while (isspace(*cmd)) {
                cmd++;
        }
        char* next = strchr(cmd, ' ');
        int i = 0;
        while(next != NULL) {
                next[0] = '\0';
                args[i] = cmd;
                ++i;
                cmd = next + 1;
                while (isspace(*cmd)) {
                        cmd++;
                }
                next = strchr(cmd, ' ');
        }
	cmd[strlen(cmd)] = '\n';
        if (cmd[0] != '\0') {
                args[i] = cmd;
                next = strchr(cmd, '\n');
                next[0] = '\0';
                ++i;
        }
        if('&' == args[i-1][strlen(args[i-1])-1]) {
		is_bg_proc = 1;
		args[i-1] = NULL;
	}
	args[i] = NULL;
}
