/*
 * output.h - variables related to COFF output
 *
 * (c) 1987, 1992 by Graham Wheeler, All Rights Reserved
 */

#include "coff.h"

extern unsigned section;		/* Current output section	*/

/************/
/* Sections */
/************/

#define N_TEXT	1
#define N_DATA	2
#define N_BSS	3

/****************************/
/* File and Section headers */
/****************************/

extern FILHDR	filehead;
extern SCNHDR	sechead[];

/**************************/
/* Relocation information */
/**************************/

#define RELOCMAX	1024	/* Maximum number of entries	*/

extern RELOC *reltab[4]; 	/* Relocation entries		*/
extern int reltop[4];		/* Next free entry		*/

/***************************/
/* Current section offsets */
/***************************/

extern long pc[];
extern long pcold[];

/************************/
/* Temp stab entry and	*/
/* redefined stab entry	*/
/************************/

#define TEMPSTAB	(-3)
#define REDEF		(-4)

/************/
/* Files    */
/************/

extern FILE	*lis,		/* List file	*/
		*obj,		/* Object file	*/
		*err,		/* Error file	*/
		*txt,		/* Text temp	*/
		*dat;		/* Data temp	*/
extern int	list;		/* List flag	*/
extern char	*txttmp,
		*dattmp;	/* Temp file names */

/************************/
/* Symbol table info	*/
/************************/

extern int	stortop,
		stabtop,
		stabtemp;

extern char	*stringstore;

extern SYMENT	*stab,
		*stabent;

