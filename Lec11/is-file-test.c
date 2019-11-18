#include "is-file-test.h"

int 
name_compare(const FTSENT ** first, const FTSENT ** second) {
        return (strcmp((*first)->fts_name, (*second)->fts_name));
}

void
directory_indexing(char* path) {
	char *pp[2];
	pp[0] = path;
	pp[1] = NULL;

	ftsp = fts_open(pp, fts_options, &name_compare);
	if(ftsp == NULL) {
		perror("fts_open");
		exit(1);
	}

	char* listing = NULL;

	while(1) {
		FTSENT *ent = fts_read(ftsp);
		if(ent == NULL) {
       	        	if(errno == 0) {
               	        	break;
                       	} else {
                       		perror("fts_read");
                       		exit(1);
                       	}
                }

		if (ent->fts_info == FTS_D || ent->fts_info == FTS_F) {
			if(ent->fts_level == 1) {
				if(listing == NULL) {
					listing = ent->fts_name;
				} else {
					strncat(listing, ent->fts_name, strlen(ent->fts_name));
				}
				strncat(listing, "\n", 1);

				if(strcmp(ent->fts_name, "index.html") == 0) {
					break;
				}
			}	
		}
	}

	if(listing != NULL) {
		printf("%s", listing);
	}

	if(fts_close(ftsp) == -1) {
		perror("fts_close");
		exit(1);
	}	
}

int main(int argc, char* argv[]) {
	directory_indexing("/home/dev/APUE/Lec11/Test");
	return 0;
}
