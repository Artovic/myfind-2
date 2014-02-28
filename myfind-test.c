#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define DEBUG

void do_file(const char * file_name, const char * const * params);
void do_dir(const char * dir_name, const char * const * params);
char get_file_type(const struct stat * file);

int main(int argc, const char * const * argv) {
	argc = argc;
	argv = argv;

	if (argc <2) { exit(EXIT_FAILURE); }

	do_dir(argv[1], &argv[2]);
}

void do_file(const char * file_name, const char * const * params) {

	struct stat myfile;

	#ifdef DEBUG
	fprintf(stderr, "do_file was called for dir: %s\n", file_name);
	#endif

	if ( lstat(file_name, &myfile) == -1 ) {
		fprintf(stderr, "Could not stat %s\n", file_name);
	}
	else {
		printf("%s   size: %lu type: %c\n", file_name, myfile.st_size, get_file_type(&myfile));

		/* descend in hierarchy */
		if ( S_ISDIR(myfile.st_mode) ) {
			do_dir(file_name, params);
		}

	}

	
}

void do_dir(const char * dir_name, const char * const * params) {

        DIR *mydirp;
        struct dirent *thisdir;

	#ifdef DEBUG
	fprintf(stderr, "do_dir was called for dir: %s\n", dir_name);
	#endif

	params = params;

	mydirp = opendir(dir_name);

        if (mydirp == NULL) {
                fprintf(stderr, "Directory %s could not be opened.\n", dir_name);
        }
        else {
		/* directory successfully opened, now read contents */
		while ( (thisdir = readdir(mydirp)) != NULL ) {
			/* prevent infinite loops */
			if ( (strcmp(thisdir->d_name, ".") != 0 ) && (strcmp(thisdir->d_name, "..") != 0 ) ) {
				/* TODO: work out how to do that:
				do_file(strcat(strcat(dir_name,'/'),thisdir->d_name), params);
				*/
				do_file(thisdir->d_name, params);
			}
		}

                closedir(mydirp);
                mydirp = NULL;
        }


}

char get_file_type(const struct stat * file) {
	char retval = 'u';

	if(S_ISBLK(file->st_mode)) { retval = 'b'; }
	if(S_ISCHR(file->st_mode)) { retval = 'c'; }
	if(S_ISDIR(file->st_mode)) { retval = 'd'; }
	if(S_ISFIFO(file->st_mode)) { retval = 'f'; }
	if(S_ISREG(file->st_mode)) { retval = '-'; }
	if(S_ISLNK(file->st_mode)) { retval = 'l'; }
	if(S_ISSOCK(file->st_mode)) { retval = 's'; }

	return retval;
}
