/*
 * simple-ls2.c
 * Extremely low-power ls clone.
 * ./simple-ls2 .
 */

#include <sys/types.h>
#include <sys/stat.h>


#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char* permissions(char* file)
{
        struct stat st;
        char *modeval = malloc(11);


        if(stat(file, &st) == 0)
        {
                mode_t perm = st.st_mode;

                modeval[0] = (perm & S_IFDIR) ? 'd' : '-';
                modeval[1] = (perm & S_IRUSR) ? 'r' : '-';
                modeval[2] = (perm & S_IWUSR) ? 'w' : '-';
                modeval[3] = (perm & S_IXUSR) ? 'x' : '-';
                modeval[4] = (perm & S_IRGRP) ? 'r' : '-';
                modeval[5] = (perm & S_IWGRP) ? 'w' : '-';
                modeval[6] = (perm & S_IXGRP) ? 'x' : '-';
                modeval[7] = (perm & S_IROTH) ? 'r' : '-';
                modeval[8] = (perm & S_IWOTH) ? 'w' : '-';
                modeval[9] = (perm & S_IXOTH) ? 'x' : '-';
                modeval[10] = '\0';

                return modeval;
        }
        else
        {
                return strerror(errno);
        }
}

int
main(int argc, char **argv) {

        DIR *dp;
        struct dirent *dirp;

        if (argc != 2) {
                fprintf(stderr, "usage: %s dir_name\n", argv[0]);
                exit(1);
        }

        if ((dp = opendir(argv[1])) == NULL ) {
                fprintf(stderr, "can't open '%s'\n", argv[1]);
                exit(1);
        }

        while ((dirp = readdir(dp)) != NULL )
                printf("%s \t %s\n",permissions(dirp->d_name), dirp->d_name);

        closedir(dp);
        return(0);
}
