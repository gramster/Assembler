/*
 * temps.c - operations on temporary files (used for intermediate
 *		storage for the .text and .data sections).
 *
 * (c) 1987, 1992 by Graham Wheeler, all rights reserved.
 */

#include <stdio.h>

#include "output.h"	/* Output variables		*/

unsigned long txtbuff[128];	/* Buffer for text	*/
unsigned char datbuff[512];	/* Buffer for data	*/
int txtbufp=0,datbufp=0;	/* Buffer indexes	*/

extern int pass;	/* Current pass			*/
extern int errcnt;	/* Number of errors found	*/

/**********************************
   Functions to write the sections
***********************************/

void writetxt(unsigned long instr) {
	txtbuff[txtbufp++]=instr;
	pc[N_TEXT]+=4;
	if (txtbufp==128) flushtxt();
}

void writedat(char chr) {
	datbuff[datbufp++]=chr;
	pc[N_DATA]+=1;
	if (datbufp==512)  flushdat();
}

static void flushtxt(void) {
	if (errcnt==0)
		fwrite(txtbuff,4,txtbufp,txt);
	txtbufp=0;
}

static void flushdat(void) {
	if (errcnt==0)
		fwrite(datbuff,1,datbufp,dat);
	datbufp=0;
}

/*******************************
Operations on temporary files
********************************/

void maketmps(void) {
	mktemp(txttmp);
	mktemp(dattmp);
}

void opentmps(void) {
	if (((txt=fopen(txttmp,"w"))==NULL) || \
		((dat=fopen(dattmp,"w"))==NULL))
			fatal("cannot create intermediate files\n");
}

void reopentmps(void) {
	if (((txt=fopen(txttmp,"r"))==NULL) || \
		((dat=fopen(dattmp,"r"))==NULL))
			fatal("cannot open intermediate files\n");
}

void closetmps(void) {
	flushtxt(); fclose(txt);
	flushdat(); fclose(dat);
}

void removetmps(void) {
	unlink(txttmp);
	unlink(dattmp);
}

