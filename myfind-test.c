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
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>


/*
 * --------------------------------------------------------------- defines --
 */

#define MYFIND_DEBUG

#define OPTION_USER "-user"
#define OPTION_NAME "-name"
#define OPTION_TYPE "-type"
#define OPTION_PRINT "-print"
#define OPTION_LS "-ls"
#define OPTION_NOUSER "-nouser"
#define OPTION_PATH "-path"

#define MAXPATHLENGTH 1024

/*
 * -------------------------------------------------------------- typedefs --
 */

typedef enum boolean { false, true } boolean_t;


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
void ls(const struct stat * file, const char * file_name);
boolean_t nouser(const struct stat * file);

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

	int i = 0;

	/* Syntax check of passed params */
	if (argc <2) {
		usage(&argv[0]);
	}
	for(i=3; i<= argc; i++) {
		/* OPTION_TYPE must be either bcdpfls */
		if  (strcmp(argv[i-1], OPTION_TYPE) == 0) {
			if (i == argc || (strcmp(argv[i], "b") != 0 && strcmp(argv[i], "c") != 0
			&& strcmp(argv[i], "d") != 0 && strcmp(argv[i], "p") != 0
			&& strcmp(argv[i], "f") != 0 && strcmp(argv[i], "l") != 0
			&& strcmp(argv[i], "s") != 0)
			) {
				fprintf(stderr, "Option %s needs an argument of [bcdpfls]\n\n", argv[i-1]);
				usage(&argv[0]);
			}
		}
		/* OPTION_USER, OPTION_NAME and OPTION_PATH need an argument */
		if ( (strcmp(argv[i-1], OPTION_USER) == 0 || strcmp(argv[i-1], OPTION_NAME) == 0 || strcmp(argv[i-1], OPTION_PATH) == 0) && i == argc) {
				fprintf(stderr, "Option %s needs an argument\n\n", argv[i-1]);
				usage(&argv[0]);
		}
	}	

	do_dir(argv[1], &argv[2]);

	return EXIT_SUCCESS;
}

void usage(const char * const * myname) {
	fprintf(stderr, "Usage: %s <DIRECTORY> [PARAMETER]\n", myname[0]);
	exit(EXIT_FAILURE);
}

void do_file(const char * file_name, const char * const * params) {

	struct stat myfile;

	params = params;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_file was called for file: %s\n", file_name);
	#endif

	if ( lstat(file_name, &myfile) == -1 ) {
		fprintf(stderr, "Could not stat %s\n", file_name);
	}
	else {
		ls(&myfile, file_name);

		if ( get_file_type(&myfile) == 'd' ) {
			do_dir(file_name, params);
		}
	}
}

void do_dir(const char * dir_name, const char * const * params) {

	DIR *mydirp = NULL;
	struct dirent *thisdir = NULL;
	char path[MAXPATHLENGTH] = {0};

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
				strcpy(path, dir_name);
				strcat(path, "/");
				strcat(path, thisdir->d_name);
				do_file(path, params);
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

void ls(const struct stat * file, const char * file_name) {

	char permissions[10] = {0};
	char timestring[30] = {0};
	char linkdestination[MAXPATHLENGTH] = {0};
	int linkbytesread = 0;
	char filetype = 0;
	struct passwd *pwd = NULL;
	struct group *grp = NULL;
	struct tm *ptime = NULL;

	filetype = get_file_type(file);

	/* initialize 9 of 10 chars of permissions with - */
	memset(permissions, '-', 9 );

	/* user permissions */
	if ( file->st_mode & S_IRUSR ) { permissions[0] = 'r'; };
	if ( file->st_mode & S_IWUSR ) { permissions[1] = 'w'; };
        if ( file->st_mode & S_ISUID ) {
		if ( file->st_mode & S_IXUSR ) {
			permissions[2] = 's';
		}
		else {
			permissions[2] = 'S';
		}
	}
	else {
		if ( file->st_mode & S_IXUSR ) { permissions[2] = 'x'; };
	}
	
	/* group permissions */
	if ( file->st_mode & S_IRGRP ) { permissions[3] = 'r'; };
	if ( file->st_mode & S_IWGRP ) { permissions[4] = 'w'; };
        if ( file->st_mode & S_ISGID ) {
		if ( file->st_mode & S_IXGRP ) {
			permissions[5] = 's';
		}
		else {
			permissions[5] = 'S';
		}
	}
	else {
		if ( file->st_mode & S_IXGRP ) { permissions[5] = 'x'; };
	}

	/* all permissions */
	if ( file->st_mode & S_IROTH ) { permissions[6] = 'r'; };
	if ( file->st_mode & S_IWOTH ) { permissions[7] = 'w'; };
        if ( file->st_mode & S_ISVTX ) {
		if ( file->st_mode & S_IXGRP ) {
			permissions[8] = 't';
		}
		else {
			permissions[8] = 'T';
		}
	}
	else {
		if ( file->st_mode & S_IXGRP ) { permissions[8] = 'x'; };
	}


	/* print inodenumber */
	printf("%lu ", file->st_ino);
	/* print blockcount as 1k blocks*/
	printf("%lu ", (file->st_blocks / 2));
	/* print type and permissions */
	printf("%c%s ", filetype, permissions);
	/* print hardlink count */
	printf("%lu ", file->st_nlink);

	/* lookup name for UID */
	if ( (pwd = getpwuid(file->st_uid)) == NULL) {
		printf("%d ", file->st_uid);
	}
	else {
		printf("%s ", pwd->pw_name);
	}

	pwd = NULL;

	/* lookup name for GID */
        grp = getgrgid(file->st_gid);

        if (grp == NULL) {
                printf("%d ", file->st_gid);
        }
        else {
                printf("%s ", grp->gr_name);
        }

        grp = NULL;

	/* print file size */
        printf("%lu ", file->st_size);

	/* parse st_mtime, write it into timestring and print it */
	ptime = localtime(&file->st_mtime);
	strftime(timestring,30,"%b %d %H:%M %Y", ptime);
	printf("%s ", timestring);

	/* print file name */
	printf("%s", file_name);

	/* in case it's a symlink, print it's destination */
	if (filetype == 'l') {
		linkbytesread = readlink(file_name, linkdestination, MAXPATHLENGTH-1);
		if ( linkbytesread > 0 ) {
			printf(" -> %s", linkdestination);
		}
		else {
			printf(" -> ERROR READING LINK");
			/* TODO: read errno and do something with it */
		}
	}

	printf("\n");

}

boolean_t nouser(const struct stat * file) {

	struct passwd *pwd = NULL;

	if ( (pwd = getpwuid(file->st_uid)) == NULL) {
		return true;
	}
	else {
		pwd = NULL;
		return false;
	}

}

/*
 * =================================================================== eof ==
 */
