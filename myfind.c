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
 * Last Modified: $Author: KleHe $
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
void ls(const struct stat* info); /** ls-Funktionalitaet **/
do_file(const char * file_name, const char * const * parms);
do_dir(const char * dir_name, const char * const * parms);

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
    /* prevent warnings regarding unused params */
	argc = argc;
	argv = argv;

	printf("Hello world!\n");
	return EXIT_SUCCESS;
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

do_file(const char * file_name, const char * const * parms)
{
}

do_dir(const char * dir_name, const char * const * parms)
{
}

/*
 * =================================================================== eof ==
 */
