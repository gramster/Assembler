/*
 * as386 - An 80386/80486 Assembler
 *
 * Written by Graham Wheeler, October 1992
 * (c) 1992, All Rights Reserved
 *
 * Based on a UNIX assembler for a RISC instruction set written
 * by the author in 1987.
 *
 * This assembler produces COFF output files. No macro facilities
 * are supported.
 */

#include <stdio.h>
#include "output.h"

#define TRUE	1
#define FALSE	0

int	errcode,	/* Error type number			*/
	errcnt,		/* Count of errors so far		*/
	filend,		/* End of file flag			*/
	ln;		/* Current source file line number	*/

unsigned section;	/* Current output file section		*/

long	pc[4],		/* Section offsets			*/
	pcold[4];	/* Last section offsets (for listing)	*/

FILE	*fp,		/* Input source file ptr		*/
	*lis,		/* List file ptr			*/
	*err,		/* Error file pointer			*/
	*obj,		/* Output object file ptr		*/
	*txt,		/* Text section temp file ptr		*/
	*dat;		/* Data section temp file ptr		*/

int	list,		/* Flag for output listing on/off	*/
	pass;		/* Current assembly pass		*/

char	iname[40],	/* Input source file name		*/
	oname[40],	/* Output object file name		*/

/* Temporary file names */

#ifdef DOS
	*txttmp="ASTEXTXXXXXX",
	*dattmp="ASDATAXXXXXX";
#else
	*txttmp="/usr/tmp/astXXXXXX",
	*dattmp="/usr/tmp/asdXXXXXX";
#endif

short version=3,release=2;

/************************************
*
*	main:
*
* The main program, which processes
* the command line, builds the file
* names, invokes the assembly passes
* and the object file builder for
* each file specified.
*
*************************************/

main(int argc, char *argv[]) {
	int argnow,namepos;

	/* Process command line */

	if (strcmp(argv[1],"-V")==0) {
		printf("asm %d.%02d\n",version,release);
		exit(0);
	}
	list=(strcmp(argv[1],"-g")==0)?1:0; /* Set list if -g option */
	lis=stdout;
	err=list?stdout:stderr; /* If list, send errors to stdout; else stderr */
	argnow=list+1;
	if (argc<=argnow) {
		fprintf(stderr,"Usage: as [-g] <source files>\n");
		exit(0);
	}
	
	/* Set up tables */
	
	initialise();
	
	/* Process each file in argument line */
	
	while (argnow<argc) {
	
		/* Process file names */
	
		strcpy(iname,argv[argnow]);
		namepos=strlen(iname)-2;
		if ((iname[namepos]=='.')&&(iname[namepos+1]=='s'))
			iname[namepos]=0;
		strcpy(oname,iname);
		strcat(oname,".o");
		strcat(iname,".s");
	
		cleartbls();
		pass=1;
		if (assemble()) {
			pass=2;
			undefine();
			opentmps();
			assemble();
			closetmps();
			if (errcode) { /* Unexpected EOF */
				writeline();
				writerror();
			}
	
			/* If no errors, make coff object file (.o extension) */
	
			if (errcnt==0) {
				if ((obj=fopen(oname,"w"))==NULL)
					fprintf(stderr,"%s : cannot create\n",oname);
				else	{
					reopentmps();
					makecoff();
					closetmps();
					fclose(obj);
				}
			}
			removetmps(); /* Unlink temporary files */
		}
		argnow++; /* next file... */
	}
	removetbls(); /* Reclaim memory */
}

/*****************************
*
*	assemble:
*
* This function is responsible
* for controlling a single pass
* of the assembly process.
*
******************************/

static int assemble(void)
{
	/* Open assembly source file */
	
	if ((fp=fopen(iname,"r"))==NULL) {
		fprintf(stderr,"%s : cannot open\n",iname);
		return FALSE;
	} else 	{
		/* Reinitialise main variables */
	
		filend=ln=errcnt=errcode=pc[N_TEXT]=pc[N_DATA]=pc[N_BSS]=0;
		section=N_TEXT;
	
		/* Parse and assemble input */
	
		parse();
		fclose(fp);
		return TRUE;
	}
}

/***********************************
*
*	initialise:
*
* This functions sets up the assembly
* tables of instructions, addressing
* modes, the symbol table, etc.
*
*************************************/

static void initialise(void) {
	maketbls();
	setupops();
	setuppsops();
	setuperrs();
}

