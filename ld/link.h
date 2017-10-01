#include <stdio.h>

#define stderr	stdout

#ifdef XENIX
#include "/usr/src/coff/filehdr.h"
#include "/usr/src/coff/scnhdr.h"
#include "/usr/src/coff/syms.h"
#include "/usr/src/coff/storclass.h"
#include "/usr/src/coff/reloc.h"
#else
#ifdef UNIX
#include <filehdr.h>
#include <scnhdr.h>
#include <syms.h>
#include <storclass.h>
#include <reloc.h>
#else
#include "..\include\filehdr.h"
#include "..\include\scnhdr.h"
#include "..\include\syms.h"
#include "..\include\storclass.h"
#include "..\include\reloc.h"
#endif
#endif

#define RISCOMAGIC	0711	/* RISC object file */
#define RISCEMAGIC	0712	/* RISC exeobj file */

#define N_TEXT	1
#define N_DATA	2
#define N_BSS	3

#define HASHTMP 0
#define HASHEXT 1

#define STACK	0x4000	/* Stack extent 16k 	*/

/****************************************/
/*      Function Declarations          */
/****************************************/

/* File : main.c - main program, error handling, initialisation, etc */
/* ====   ======
*/
extern void fatal       (	/* message     */ );
extern void trace       (	/* msg,msg_arg	*/ );
extern void ltrace      (	/* msg,msg_arg	*/ );

/* File : files.c - file handling functions */
/* ====   =======
*/
extern void open_tmp    (  /* fp,name     */ );
extern void killtemps   ();
extern int  get_fl_type (	/* file_name   */ );
extern void readfilhdr  ();
extern void readstab    ();
extern void appendsecn  (  /* secnum      */ );
extern void makecoffile ();

/* File : reloc.c - object file and archive relocation */
/* ====   =======
*/
extern void search_arc  ();
extern void merge_obj   ();
extern void relocate    ( /* pc,sec,typ,*stab,*data */ );

/* File : tables.c - symbol and relocation table management */
/* ====   ========
*/
extern void maketables  ();
extern void mergestabs  ();
extern void enter_udefsym(	/* name        */ );
extern int  showundefs  ();

/* File : hash.c - hash table management */
/* ====   ======
*/
extern int makehash     ( /* table,size	*/ );
extern void destroyhash	( /* table			*/ );
extern char *findhash   ( /* table,key		*/ );
extern void enterhash	( /* table,key,data*/);


/****** Variables ************/

extern FILE *infl, *outfl, *tempfl[];
extern char tempname[4][12];

extern FILHDR tfilhdr,ofilhdr;

extern SCNHDR tsechdr,texsechd,datsechd,bsssechd;

extern unsigned long base,offset;	/* Archive/file offsets */
extern unsigned long globalpc[4];

extern RELOC *outreloc[3],*tmpreloc;	/* Reloc tables */
extern unsigned oreltop[3];

extern SYMENT *statstab,*globstab,*tmpstab;	/* Symbol tables */
extern unsigned int stattop,globtop,tmptop;

#ifdef FLEXNAMES
extern char *stringstore,*tstringstore;
extern int stortop,tstortop;
#endif

extern char filename[];	/* current file argument */
extern char outname[];  /* output file name */
extern short strip;	/* strip flag */

extern unsigned short redeferrs,undeferrs; /* Counts of errors */

