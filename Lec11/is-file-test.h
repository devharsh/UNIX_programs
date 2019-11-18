#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int fts_options = FTS_COMFOLLOW | FTS_NOCHDIR | FTS_PHYSICAL;

FTS *ftsp;

int 
name_compare(const FTSENT ** first, const FTSENT ** second); 

void
directory_indexing(char* path);
