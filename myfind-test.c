/**
 * @file myfind.c
 * Betriebssysteme myfind 
 * Beispiel 1
 *
 * @author Leandros Athanasiadis <leandros.athanasiadis@technikum-wien.at>
 * @author Klemens Henk <klemens.henk@technikum-wien.at>
 * @author Davor Dadic <d.dadic@technikum-wien.at>
 * @date 2014/02/14
 *
 * @version $Revision: 470 $
 *
 * @todo Test it more seriously and more complete.
 * @todo Review it for missing error checks.
 * @todo Review it and check the source against the rules at
 *       https://cis.technikum-wien.at/documents/bic/2/bes/semesterplan/lu/c-rules.html
 *
 * URL: $HeadURL$
 *
 * Last Modified: $Author: KleHe $
 */

/*
 * -------------------------------------------------------------- includes --
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


/*
 * --------------------------------------------------------------- defines --
 */

#define MYFIND_DEBUG

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 */

void usage(const char * const * myname);
void do_file(const char * file_name, const char * const * params);
void do_dir(const char * dir_name, const char * const * params);
char get_file_type(const struct stat * file);

/**
 *
 * \brief The most minimalistic C program
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return always "success"
 * \retval 0 always
 *
 */

int main(int argc, const char * const * argv) {
	argc = argc;
	argv = argv;

	if (argc <2) {
		usage(&argv[0]);
	}

	do_dir(argv[1], &argv[2]);

	return EXIT_SUCCESS;
}

void usage(const char * const * myname) {
	fprintf(stderr, "Usage: %s <VERZEICHNIS> [PARAMETER]\n", myname[0]);
	exit(EXIT_FAILURE);
}

void do_file(const char * file_name, const char * const * params) {

	struct stat myfile;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_file was called for file: %s\n", file_name);
	#endif

	if ( lstat(file_name, &myfile) == -1 ) {
		fprintf(stderr, "Could not stat %s\n", file_name);
	}
	else {
		printf("%s   size: %lu type: %c\n", file_name, myfile.st_size, get_file_type(&myfile));

		/* descend in hierarchy */
		if ( get_file_type(&myfile) == 'd' ) {
			do_dir(file_name, params);
		}
	}
}

void do_dir(const char * dir_name, const char * const * params) {

	DIR *mydirp = NULL;
	struct dirent *thisdir = NULL;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_dir was called for dir: %s\n", dir_name);
	#endif

	mydirp = opendir(dir_name);

        if (mydirp == NULL) {
                fprintf(stderr, "Directory %s could not be opened.\n", dir_name);
        }
        else {
		/* directory successfully opened, now read contents */
		while ( (thisdir = readdir(mydirp)) != NULL ) {
			/* prevent infinite loops */
			if ( (strcmp(thisdir->d_name, ".") != 0 ) && (strcmp(thisdir->d_name, "..") != 0 ) ) {
				/* do_file(thisdir->d_name, params); */
				do_file(strcat(strcat((char*) dir_name,"/"),thisdir->d_name), params);
			}
		}

        }

	closedir(mydirp);

	thisdir = NULL;
	mydirp = NULL;
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

/*
 * =================================================================== eof ==
 */
