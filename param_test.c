#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXPARAMLENGTH 15

#define OPTION_USER "-user"
#define OPTION_NAME "-name"
#define OPTION_TYPE "-type"
#define OPTION_PRINT "-print"
#define OPTION_LS "-ls"
#define OPTION_NOUSER "-nouser"
#define OPTION_PATH "-path"


const char params[][MAXPARAMLENGTH] = { OPTION_USER, OPTION_NAME, OPTION_TYPE, OPTION_PRINT, OPTION_LS, OPTION_NOUSER, OPTION_PATH };
const int paramcnt = sizeof(params)/MAXPARAMLENGTH;

int main(int argc, const char * const * argv) {
	int i = 0;
	for(i=1; i<=paramcnt; i++) {
		printf("%s\n", params[i-1]);
	} 
}
