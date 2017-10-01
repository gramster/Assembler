/* %W% %H%  */

#include <stdio.h>

#include "errors.h"	/* Error table and messages	*/
#include "toke.h"

unsigned errcol;	/* Error position (column)	*/
char	*errval;	/* Optional error message	*/
			/*	argument		*/

/************************
*  Error Processing	*
*************************/

void warning(int num, char *val) {
	errcol=bufnow;
	errcode=/*(filend?ERR_EOF:num); */ num;
	errval=val;
}

void error(int num, long val) {
	errcnt++;
	warning(num,val);
	skip('\n');
}

void fatal(char *msg) {
	fprintf(stderr,"Fatal - %s\n",msg);
	fprintf(stderr,"Cannot recover from this error - goodbye!\n");
	fflush(stdout);
	fflush(stderr);
	removetbls();
	exit(0);
}

