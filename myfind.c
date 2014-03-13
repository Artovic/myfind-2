/**
 * \file myfind.c
 * Betriebssysteme myfind 
 * Beispiel 1
 *
 * \author Leandros Athanasiadis <leandros.athanasiadis@technikum-wien.at>
 * \author Klemens Henk <klemens.henk@technikum-wien.at>
 * \author Davor Dadic <davor.dadic@technikum-wien.at>
 * \date 2014/03/08
 *
 * \version 1.0
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
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>



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
#define OPTION_NOUSER_TEST -nouser
#define OPTION_PATH "-path"

#define FILETYPEMODE_LS 0
#define FILETYPEMODE_TYPE 1

#define DOFILEMODE_SELF 0
#define DOFILEMODE_OTHER 1 

/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

/* declare program name instead of allways invoking basename(argv[0]) */
const char *progname = NULL;

/*
 * ------------------------------------------------------------- functions --
 */

void usage(void);
void do_file(const char *file_name, const int mode, const char * const *argv, int argc);
void do_dir(const char *dir_name, const char * const *argv, int argc);
char get_file_type(const struct stat *file, const int mode);
void ls(const struct stat *file, const char *file_name);
bool nouser(const struct stat *file);
bool usernamematch(const struct stat *file, const char *arg);
bool isnumeric(const char *arg);

/**
 *
 * \brief main() of custom find implementation 
 *
 * This is the main entry point for any C program.
 *
 * \param argc The number of arguments
 * \param argv The arguments itselves (including the program name in argv[0]).\n
 * Expects first parameter to be a directory.
 *
 * \return Always EXIT_SUCCESS 
 *
 */

int main(int argc, const char * const *argv) {

	int i = 0;
	struct stat myfile;
	struct passwd *pwd = NULL;

	/* set global *progname to stripped argv[0] */
	progname = basename((char*) argv[0]);

	/* Syntax check of passed params */
	if (argc <2) {
		usage();
	}
	for(i=2; i<argc; i++) {
		/* OPTION_USER, OPTION_NAME, OPTION_PATH and OPTION_TYPE need an argument */
		if (strcmp(argv[i], OPTION_USER) == 0 || strcmp(argv[i], OPTION_NAME) == 0 || strcmp(argv[i], OPTION_PATH) == 0 || strcmp(argv[i], OPTION_TYPE) == 0) {
			if (i+1 == argc) {
				fprintf(stderr, "%s: Option %s needs an argument.\n\n", progname, argv[i]);
				usage();
                        }
			else {
				/* OPTION_TYPE's argument must be in [bcdpfls] */
				if (strcmp(argv[i], OPTION_TYPE) == 0 && strcmp(argv[i+1], "b") != 0 && strcmp(argv[i+1], "c") != 0
				&& strcmp(argv[i+1], "d") != 0 && strcmp(argv[i+1], "p") != 0
				&& strcmp(argv[i+1], "f") != 0 && strcmp(argv[i+1], "l") != 0 && strcmp(argv[i+1], "s") != 0)
				{
				fprintf(stderr, "%s: Option %s needs an argument of [bcdpfls].\n\n", progname, argv[i]);
				usage();
				}

				/* if not numeric, OPTION_USER's argument must be a valid user name at this system */
				if (strcmp(argv[i], OPTION_USER) == 0 && isnumeric(argv[i+1]) == false && (pwd = getpwnam(argv[i+1])) == NULL) {
					fprintf(stderr, "%s: User not found: %s\n", progname, argv[i+1]);
					exit(EXIT_FAILURE);
				}

			}

			/* skip next argument as it's an argument for this one */
			i++;
		}

		/* unexpected parameter */
		else if (strcmp(argv[i], OPTION_LS) != 0 && strcmp(argv[i], OPTION_PRINT) != 0 && strcmp(argv[i], OPTION_NOUSER) != 0) {
			fprintf(stderr, "%s: Unknown parameter given: %s .\n\n", progname, argv[i]);
			usage();
		}
	}


	/* call functions depending on file type  */
	errno = 0;
	if ( lstat(argv[1], &myfile) == -1 ) {
		fprintf(stderr, "%s: Could not stat %s - %s\n", progname, argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}
	else {
		if (get_file_type(&myfile, FILETYPEMODE_TYPE) == 'd') {
			do_dir(argv[1], argv, argc);
		}
		else {
			do_file(argv[1], DOFILEMODE_SELF, argv, argc);
		}
	}

	return EXIT_SUCCESS;
}


/**
 *
 * \brief usage - print synopsis and exit 
 *
 * This function is used to print synopsis and exit 
 *
 * \return void but terminates execution with EXIT_FAILURE
 *
 */
void usage(void) {
	fprintf(stderr, "Usage: %s <DIRECTORY> [PARAMETER]\n", progname);
	exit(EXIT_FAILURE);
}

/**
 *
 * \brief Get inode information for file
 *
 * \param file_name Path of the file name
 * \param argv argv passed through from main()
 * \param argc argc passed through from main()
 *
 */
void do_file(const char *file_name, const int mode, const char * const *argv, int argc) {	

	struct stat myfile;
	int i = 0;
	bool printed = false;
	bool lsed = false;
	char *file_name_copy = NULL;
	bool match = false;

	/* basename/dirname may modify passed string so make a copy... */
	file_name_copy = malloc((strlen(file_name)+1)*sizeof(char));
	if (file_name_copy == NULL) {
		fprintf(stderr, "%s: memory allocation for file_name_copy failed!\n", progname);
		exit(EXIT_FAILURE);
	}
	strcpy(file_name_copy, file_name);
	

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_file was called for file: %s\n", file_name);
	#endif

	/* go get inode infos */
	errno = 0;
	if ( lstat(file_name, &myfile) == -1 ) {
		fprintf(stderr, "%s: Could not stat %s - %s\n", progname, file_name, strerror(errno));
	}
	else {

		/* make sure we print if no furhter arguments given */
		if(argc <= 2) {
			match = true;
		}
		
		/* parse passed parameters, break loop if there is no match between file and criteria */
		for(i=2; i<argc; i++) {

			if (strcmp(argv[i], OPTION_NOUSER) == 0) {
				if (nouser(&myfile) == false) {
					match = false;
					break;
				}
				else {
					match = true;
				}
			}
			else if (strcmp(argv[i], OPTION_USER) == 0) {
				if (usernamematch(&myfile, argv[++i]) == false) {
					match = false;
					break;
				}
				else {
					match = true;
				}
			}
			else if (strcmp(argv[i], OPTION_NAME) == 0) {
				if (fnmatch(argv[++i], basename((char*)file_name_copy), 0) != 0) {
					match = false;
					break;
				}
				else {
					match = true;
				}
			}
			else if (strcmp(argv[i], OPTION_PATH) == 0) {
				if (fnmatch(argv[++i], file_name, FNM_PATHNAME) != 0) {
					match = false;
					break;
				}
				else {
					match = true;
				}
			}
			else if (strcmp(argv[i], OPTION_TYPE) == 0) {
				if (argv[++i][0] != get_file_type(&myfile, FILETYPEMODE_TYPE)) {
					match = false;
					break;
				}
				else {
					match = true;
				}
			}
			else if (strcmp(argv[i], OPTION_LS) == 0) {
				ls(&myfile, file_name);
				lsed = true;
			}
			/* print it if invoked explicitely */
			else if (strcmp(argv[i], OPTION_PRINT) == 0) {
				printf("%s\n", file_name);
				printed = true;
			}
			else {
				assert(0);
			}

		}

		/* print it if matched and not printed/lsed yet */
		if (match == true && printed == false && lsed == false) {
			printf("%s\n", file_name);
		}

		/* if file is subdirectory descend into hierarchy */
		if ( get_file_type(&myfile, FILETYPEMODE_TYPE) == 'd' && mode == DOFILEMODE_OTHER ) {
			do_dir(file_name, argv, argc);
		}
	}

	free(file_name_copy);
	file_name_copy = NULL;
}

/**
 *
 * \brief Get contents of directory
 *
 * \param dir_name Path of the directory name
 * \param argv argv passed through from main()
 * \param argc argc passed through from main()
 *
 */
void do_dir(const char *dir_name, const char * const *argv, int argc) {

	DIR *mydirp = NULL;
	struct dirent *thisdir = NULL;
	char *path = NULL;

	#ifdef MYFIND_DEBUG
	fprintf(stderr, "do_dir was called for dir: %s\n", dir_name);
	#endif

	/* do_file for self, but just if top of search hierarchy */
	if (strcmp(dir_name, argv[1]) == 0) {
		do_file(dir_name, DOFILEMODE_SELF, argv, argc);
	}

	errno = 0;
        if ( (mydirp = opendir(dir_name)) == NULL) {
		fprintf(stderr, "%s: Error opening %s - %s\n", progname, dir_name, strerror(errno));	
        }
        else {
		/* directory successfully opened, now read contents */
		errno = 0;
		while ( (thisdir = readdir(mydirp)) != NULL)  {
			/* prevent infinite loops */
			if ( (strcmp(thisdir->d_name, ".") != 0 ) && (strcmp(thisdir->d_name, "..") != 0 ) ) {

				/* allocate memory for path, /, d_name and \0 */
				path = malloc( (strlen(dir_name)+strlen(thisdir->d_name)+2) * sizeof(char) );
				if (path == NULL) {
					fprintf(stderr, "%s: Memory allocation for path in do_dir failed, exiting.\n", progname);
					exit(EXIT_FAILURE);
				}

				/* Extend current path directory entry */
				strcpy(path, dir_name);
				if (strcmp(dir_name, "/") != 0) {
					strcat(path, "/");
				}
				strcat(path, thisdir->d_name);

				/* go get infos */
				do_file(path, DOFILEMODE_OTHER, argv, argc);

				/* free path, set pointer NULL */
				free(path);
				path = NULL;
			}
		}

		/* checking errno in while loop makes no sense as it's allways 0 (thisdir != NULL) */
		if (errno != 0) {
			fprintf(stderr, "%s: Error reading directory entry in %s - %s\n", progname, dir_name, strerror(errno));	
		}

		/* close directory */
		errno = 0;
		if (closedir(mydirp) != 0) {
			fprintf(stderr, "%s: Error closing directory %s - %s\n", progname, dir_name, strerror(errno));	
		} 

		mydirp = NULL;

        }


}

/**
 *
 * \brief Get file type of file
 *
 * \param file Pointer to struct stat of current file 
 * \param mode Adapts output for further use - filetype for regular file is "f", ls prints it as "-" 
 * \return One single char
 *
 */

char get_file_type(const struct stat *file, const int mode) {

	char retval = 'u';

	if(S_ISBLK(file->st_mode)) { retval = 'b'; }
	if(S_ISCHR(file->st_mode)) { retval = 'c'; }
	if(S_ISDIR(file->st_mode)) { retval = 'd'; }
	if(S_ISLNK(file->st_mode)) { retval = 'l'; }
	if(S_ISSOCK(file->st_mode)) { retval = 's'; }
	if(S_ISFIFO(file->st_mode)) { retval = 'p'; }

	if (mode==FILETYPEMODE_LS) {
		if(S_ISREG(file->st_mode)) { retval = '-'; }
	}
	else if (mode==FILETYPEMODE_TYPE) {
		if(S_ISREG(file->st_mode)) { retval = 'f'; }
	}

	return retval;
}

/**
 *
 * \brief Prints ls-style output
 *
 * \param file Pointer to struct stat of current file 
 * \param file_name Path to current file 
 *
 */

void ls(const struct stat *file, const char *file_name) {

	char permissions[10] = {0};
	char timestring[18] = {0};
	char *linkdestination = NULL;
	int linkbytesread = 0;
	int linklength = 0;
	char filetype = 0;
	struct passwd *pwd = NULL;
	struct group *grp = NULL;
	struct tm *ptime = NULL;

	filetype = get_file_type(file, FILETYPEMODE_LS);

	/* initialize 9 of 10 chars of permissions with - */
	memset(permissions, '-', 9 );

	/* parse st_mtime, write it into timestring and print it */
	ptime = localtime(&file->st_mtime);
	/* TODO 
	strftime(timestring,18,"%b %d %H:%M %Y", ptime); */
	strftime(timestring,18,"%b %d %H:%M", ptime);

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


	/* print inodenumber, blockcount as 1k blocks, type+permissions, hardlink count */
	/* typecast file->st_nlink to long for 32/64 bit compatibility */
	printf("%lu %lu %c%s %lu", file->st_ino, (file->st_blocks / 2), filetype, permissions, (long) file->st_nlink);

	/* lookup name for UID */
	if ((pwd = getpwuid(file->st_uid)) == NULL) {
		printf(" %d", file->st_uid);
	}
	else {
		printf(" %s", pwd->pw_name);
		pwd = NULL;
	}


	/* lookup name for GID */
        if ((grp = getgrgid(file->st_gid)) == NULL) {
                printf(" %d", file->st_gid);
        }
        else {
                printf(" %s", grp->gr_name);
        	grp = NULL;
        }

	/* print file size, timestring, filename*/
        printf(" %lu %s %s", file->st_size, timestring, file_name);

	/* in case it's a symlink, print it's destination */
	if (filetype == 'l') {

		linklength = (file->st_size + 1) * sizeof(char);

		linkdestination = malloc(linklength);
		if (linkdestination == NULL) {
			fprintf(stderr, "%s: Memory allocation for linkdestination in ls failed, exiting.\n", progname);
			exit(EXIT_FAILURE);
		}

		memset(linkdestination, '\0', linklength);

		errno = 0;
		if ( (linkbytesread = readlink(file_name, linkdestination, linklength-1)) > 0 ) {
			printf(" -> %s", linkdestination);
		}
		else {
			printf(" -> ERROR READING LINK");
			fprintf(stderr, "%s: Error reading link %s - %s\n", progname, file_name, strerror(errno));	
		}

		free(linkdestination);
		linkdestination = NULL;
	}

	printf("\n");

}

/**
 *
 * \brief Used to return if file's uid is found in local /etc/passwd
 *
 * \param file Pointer to struct stat of current file 
 * \return true if no username found for uid
 *
 */
bool nouser(const struct stat *file) {

	struct passwd *pwd = NULL;

	if ( (pwd = getpwuid(file->st_uid)) == NULL) {
		return true;
	}
	return false;

}

/**
 *
 * \brief Used to return if given username matches file's uid
 *
 * \param file Pointer to struct stat of current file 
 * \param arg String to match
 * \return true if file's uid matches given username's uid 
 *
 */
bool usernamematch(const struct stat *file, const char *arg) {
	struct passwd *pwd = NULL;
	pwd = getpwnam(arg);

	if ( ((pwd = getpwnam(arg))!= NULL && file->st_uid == pwd->pw_uid) ) {
		return true;
	}
	else {
		return false;
	}
}


/**
 *
 * \brief Used to return if passed string is numeric 
 *
 * \param arg String to check for alphanumeric chars 
 * \return true if string is just numeric, otherwise false 
 *
 */
bool isnumeric(const char *arg) {

	int i = 0;

	while(arg[i] != '\0') {
		if(!isdigit(arg[i])) {
			return false;
		}
		i++;
	}

	return true;
}

/*
 * =================================================================== eof ==
 */
