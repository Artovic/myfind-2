/**
 * @file myfind.c
 * Betriebssysteme myfind 
 * Beispiel 1
 *
 * @author Leandros Athanasiadis <leandros.athanasiadis@technikum-wien.at>
 * @author Klemens Henk <klemens.henk@technikum-wien.at>
 * @author Davor Dadic <d.dadic@technikum-wien.at>
 * @date 2014/03/02
 *
 * @version 0.7 
 *
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
#include <fnmatch.h>
#include <libgen.h>



/*
 * --------------------------------------------------------------- defines --
 */

#undef MYFIND_DEBUG

#define MAXPARAMLENGTH 8

#define OPTION_USER "-user"
#define OPTION_NAME "-name"
#define OPTION_TYPE "-type"
#define OPTION_PRINT "-print"
#define OPTION_LS "-ls"
#define OPTION_NOUSER "-nouser"
#define OPTION_PATH "-path"

/*
 * -------------------------------------------------------------- typedefs --
 */

typedef enum boolean { false, true } boolean_t;


/*
 * --------------------------------------------------------------- globals --
 */

const char knownparams[][MAXPARAMLENGTH] = { OPTION_USER, OPTION_NAME, OPTION_TYPE, OPTION_PRINT, OPTION_LS, OPTION_NOUSER, OPTION_PATH };
const int knownparamcnt = sizeof(knownparams)/MAXPARAMLENGTH;
const char *progname = NULL;

/*
 * ------------------------------------------------------------- functions --
 */

void usage(void);
void do_file(const char * file_name, const char * const * argv, int argc);	
void do_dir(const char * dir_name, const char * const * argv, int argc);
char get_file_type(const struct stat * file);
void ls(const struct stat * file, const char * file_name, const char * const * argv);
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

	progname = basename((char*) argv[0]);

	/* Syntax check of passed params */
	if (argc <2) {
		usage();
	}
	for(i=3; i<= argc; i++) {
		/* OPTION_TYPE must be either bcdpfls */
		if  (strcmp(argv[i-1], OPTION_TYPE) == 0) {
			if (i == argc || (strcmp(argv[i], "b") != 0 && strcmp(argv[i], "c") != 0
			&& strcmp(argv[i], "d") != 0 && strcmp(argv[i], "p") != 0
			&& strcmp(argv[i], "f") != 0 && strcmp(argv[i], "l") != 0
			&& strcmp(argv[i], "s") != 0)
			) {
				fprintf(stderr, "%s: Option %s needs an argument of [bcdpfls].\n\n", argv[0], argv[i-1]);
				usage();
			}
		}
		/* OPTION_USER, OPTION_NAME and OPTION_PATH need an argument */
		if ( (strcmp(argv[i-1], OPTION_USER) == 0 || strcmp(argv[i-1], OPTION_NAME) == 0 || strcmp(argv[i-1], OPTION_PATH) == 0) && i == argc) {
				fprintf(stderr, "%s: Option %s needs an argument.\n\n", argv[0], argv[i-1]);
				usage();
		}
	}	

	do_dir(argv[1], argv, argc);

	return EXIT_SUCCESS;
}

void usage(void) {
	fprintf(stderr, "Usage: %s <DIRECTORY> [PARAMETER]\n", progname);
	exit(EXIT_FAILURE);
}

void do_file(const char * file_name, const char * const * argv, int argc) {	

	struct stat myfile;
	int i = 0;
	boolean_t printed = false;
	boolean_t lsed = false;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_file was called for file: %s\n", file_name);
	#endif

	if ( lstat(file_name, &myfile) == -1 ) {
		fprintf(stderr, "%s: Could not stat %s\n", argv[0], file_name);
	}
	else {
		/* do something according to params */
		for(i=2; i<= argc; i++) {

			if (strcmp(argv[i-1], OPTION_NOUSER) == 0) {
				if (nouser(&myfile) == false) {
					break;
				}
			}

			if (strcmp(argv[i-1], OPTION_NAME) == 0) {
				if (fnmatch(argv[i], basename((char*)file_name), FNM_NOESCAPE) != 0) {
					break;
				}
			}

			if (strcmp(argv[i-1], OPTION_LS) == 0 && lsed == false) {
				ls(&myfile, file_name, argv);
				lsed = true;
			}
			/* print it if invoked explicitely or nothing lsed yet */
			if ( (strcmp(argv[i-1], OPTION_PRINT) == 0 && printed == false) || (i==argc && printed == false && lsed == false) ) {
				printf("%s\n", file_name);
				printed = true;
			}

		}

		if ( get_file_type(&myfile) == 'd' ) {
			do_dir(file_name, argv, argc);
		}
	}
}

void do_dir(const char * dir_name, const char * const * argv, int argc) {

	DIR *mydirp = NULL;
	struct dirent *thisdir = NULL;
	char *path = NULL;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_dir was called for dir: %s\n", dir_name);
	#endif

	mydirp = opendir(dir_name);

        if (mydirp == NULL) {
                fprintf(stderr, "%s: Directory %s could not be opened.\n", argv[0], dir_name);
        }
        else {
		/* directory successfully opened, now read contents */
		while ( (thisdir = readdir(mydirp)) != NULL ) {
			/* prevent infinite loops */
			if ( (strcmp(thisdir->d_name, ".") != 0 ) && (strcmp(thisdir->d_name, "..") != 0 ) ) {

				/* allocate memory for path, /, d_name and \0 */
				path = malloc( (strlen(dir_name)+strlen(thisdir->d_name)+2) * sizeof(char) );
				if (path == NULL) {
					fprintf(stderr, "%s: Memory allocation for path in do_dir failed, exiting.\n", argv[0]);
					exit(EXIT_FAILURE);
				}

				/* Extend current path directory entry */
				strcpy(path, dir_name);
				strcat(path, "/");
				strcat(path, thisdir->d_name);

				/* go get infos */
				do_file(path, argv, argc);

				/* free path, set pointer NULL */
				free(path);
				path = NULL;
			}
		}

	mydirp = NULL;
	/* thisdir allready NULL at this time */

        }

	closedir(mydirp);

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

void ls(const struct stat * file, const char * file_name, const char * const * argv) {

	char permissions[10] = {0};
	char timestring[30] = {0};
	char *linkdestination = NULL;
	int linkbytesread = 0;
	int linklength = 0;
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

		linklength = (file->st_size + 1) * sizeof(char);

		linkdestination = malloc(linklength);
		if (linkdestination == NULL) {
			fprintf(stderr, "%s: Memory allocation for linkdestination in ls failed, exiting.\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		memset(linkdestination, '\0', linklength);

		linkbytesread = readlink(file_name, linkdestination, linklength-1);
		if ( linkbytesread > 0 ) {
			printf(" -> %s", linkdestination);
		}
		else {
			printf(" -> ERROR READING LINK");
			fprintf(stderr, "%s: Reading symlink %s failed.\n", argv[0], file_name);
			/* TODO: read errno and do something with it */
		}

		free(linkdestination);
		linkdestination = NULL;
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
