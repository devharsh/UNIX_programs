#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

int main(void) {
	int fd;
	int nochdir;
	int noclose;

	printf("forking...\n");

	switch (fork()) {
		case -1:
			return (-1);
		case 0:
			break;
		default:
			_exit(0);
	}

	printf("setting session...\n");

	if (setsid() == -1)
		return (-1);

	printf("changing directories...\n");

	if (!nochdir)
		(void)chdir("/");

	printf("duping outputs...\n");

	if (!noclose && (fd = open(_PATH_DEVNULL, O_RDWR, 0)) != -1){
		(void)dup2(fd, STDIN_FILENO);
		(void)dup2(fd, STDOUT_FILENO);
		(void)dup2(fd, STDERR_FILENO);
		if (fd > STDERR_FILENO)
			(void)close(fd);
	}

	printf("daemon is closing...\n");

	return (0);
}
