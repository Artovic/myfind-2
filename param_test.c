#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#define MAXPARAMLENGTH 8

#define OPTION_USER "-user"
#define OPTION_NAME "-name"
#define OPTION_TYPE "-type"
#define OPTION_PRINT "-print"
#define OPTION_LS "-ls"
#define OPTION_NOUSER "-nouser"
#define OPTION_PATH "-path"


const char knownparams[][MAXPARAMLENGTH] = { OPTION_USER, OPTION_NAME, OPTION_TYPE, OPTION_PRINT, OPTION_LS, OPTION_NOUSER, OPTION_PATH };
const int knownparamcnt = sizeof(knownparams)/MAXPARAMLENGTH;

const char *progname = NULL;
int paramcnt = 0;

int main(int argc, const char * const * argv) {

	int i = 0;
	for(i=1; i<=knownparamcnt; i++) {
		printf("%s\n", knownparams[i-1]);
	}

	progname = basename((char*) argv[0]);
	paramcnt = argc-1;

	printf("argc: %d\n", argc);
	printf("paramcnt: %d\n", paramcnt);
	printf("progname: %s\n", progname);

}
