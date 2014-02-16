/**
 * @file myfind.c
 * Betriebssysteme myfind 
 * Beispiel 1
 *
 * @author Leandros Athanasiadis <leandros.athanasiadis@technikum-wien.at>
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
<<<<<<< HEAD
 * Last Modified: $Author: ichbinderbernd $
=======
 * Last Modified: $Author: KleHe $
>>>>>>> 9496c416371d3b843b423891c4e12c8a45c0e2bf
 */

/*
 * -------------------------------------------------------------- includes --
 */

/* testcomment */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fnmatch.h>       /** filename finden **/
#include <pwd.h>           /** PW Library **/
#include <grp.h>           /** Gruppenstruktur **/
#include <limits.h>        /** Könnte eventuell bei maximalen Pfadlaengen bzw. Dateilaengen gebraucht werden **/
#include <sys/types.h>     /** Library fuer die ganzen Socketfunktionen **/
#include <sys/stat.h>      /** Hiermit sollte es moeglich sein Infos über Dateien zu ermitteln **/
#include <errno.h>         /** Fehlerhandling **/
#include <time.h>          /** Aktuelle Zeit **/
/*
 * --------------------------------------------------------------- defines --
 */
/*
 * -------------------------------------------------------------- typedefs --
 */

/*
 * --------------------------------------------------------------- globals --
 */

/*
 * ------------------------------------------------------------- functions --
 */
void usage(const char *argv[]);
void ls(const struct stat* info); /** ls-Funktionalitaet **/
void scan(const char * path, const char * const * parms);
void do_file(const char * file_name, const char * const * parms);
void do_dir(const char * dir_name, const char * const * parms);

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

int main(int argc, const char *argv[])
{
	/* local variable definitions */
	int i = 1;
	int paramsplit = 0;

	/* prevent warnings regarding unused params */
	argc = argc;
	argv = argv;

	/* find out where parameters start */
	while(i <= argc)
	{
		if( ! strcmp(argv[i-1], "-user") )
		{
			paramsplit = i;
			printf("paramsplit: %d\n", paramsplit);
			break;
		}
		i++;
	}

	i = 2;
	while ( i < paramsplit )
	{
		printf("Directory: %s\n", argv[i-1]);
		i++;
	}

	scan(argv[1], NULL);

	return EXIT_SUCCESS;
}

void usage(const char *argv[]) {
	argv = argv;
	fprintf(stdout, "Usage: %s <directory> [params]\n", argv[0]);
}

void ls(const struct stat* info)
{
	/** Blocks ausgeben **/
	fprintf(stdout, "%2lu ", (unsigned long) info->st_blocks);
	/** Filegroesse **/
	fprintf(stdout, "%9lu ", (unsigned long) info->st_size);
	/** iNodes ausgeben **/
	fprintf(stdout, "%8lu ", (unsigned long) info->st_ino);
	/** %lu wegen unsigned long **/
}

void scan(const char * path, const char * const * parms)
{
	struct stat myentry;
	parms = parms;

	if (path == NULL)
	{
		printf("Kein zu lesender Eintrag übergeben\n");
		return;
	}

	if( stat(path, &myentry) == 0 )
	{
		if( myentry.st_mode & S_IFDIR )
		{
			printf("is dir\n");
		}
		if( myentry.st_mode & S_IFREG )
		{
			printf("is file\n");
		}
	}
	else
	{
		printf("Fehler beim Lesen von Eintrag %s", path);
		/* Todo: Fehlerbehandlung */
	}
}

void do_file(const char * file_name, const char * const * parms)
{
	file_name = file_name;
	parms = parms;
}

void do_dir(const char * dir_name, const char * const * parms)
{
	dir_name = dir_name;
	parms = parms;

}

/*
 * =================================================================== eof ==
 */
