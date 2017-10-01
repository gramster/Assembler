/* %W% %H%  */

#include <stdio.h>
#include "toke.h"	/* Token types			*/
#include "errors.h"	/* Error table and messages	*/
#include "output.h"	/* Output variables		*/

#define TRUE		1
#define FALSE		0

char	errstring[40];	/* Error message for unexpected	*/
			/*	tokens			*/
int	typcnt;		/* Used as a toggle to count	*/
			/*	"or"s in message	*/
extern int pass;	/* Current pass			*/

/********************************
*  Functions to write list file	*
********************************/

void writerror(void) {
	if (pass==2) {
		if (ISWARNING(errcode))
			fprintf(err,"%*c^ WARNING!\n* ",20+errcol,32);
		else
			fprintf(err,"%*c^ ERROR!\n***** ",20+errcol,32);
		if (errcode==ERR_EXPECT)
			expecterror(errval); /* Build error message */
		fprintf(err,errtbl[errcode],errval);
		fprintf(err,"\n");
		}
	errcode=0;
}

static void expecterror(unsigned exp) {
	typcnt=0;
	errstring[0]=0;
	checkexp("immediate value",exp,IMMEDIATE);
	checkexp("absolute value",exp,CONSTANT);
	checkexp("register",exp,REGSTR);
	checkexp("identifier",exp,IDENT);
	checkexp("delimeter",exp,DELIM);
	errstring[0]=toupper(errstring[0]);
}

static int checkexp(char *msg, unsigned exp, unsigned mask) {
	if (mask&exp == mask) {
		if (typcnt++) strcat(errstring," or ");
		strcat(errstring,msg);
		return TRUE;
	} else return FALSE;
}

void writeline(void) {
	unsigned short offset=(short)(pcold[section]);
	if (ln && (pass==2)) {
		if (errcode) fprintf(err,"%4d> %16c%s",ln,' ',buff);
		else if (list) {
			if (res!=0xFFFFFFFFL)
				fprintf(lis,"%4d> %04X : %08lX %s",
						ln,offset,res,buff);
			else
				fprintf(lis,"%4d> %04X%12c%s",
						ln,offset,' ',buff);
		}
	}
	ln++;
}


