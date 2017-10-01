#include "link.h"
#include <string.h>

/* Table Sizes */

#define STABSIZ  100	/* Static/Global stabs */
#define TSTABSIZ 150	/* Temp File Stab      */

#define RELCSIZ		50
#define TRELCSIZ	50

#define STORSIZ	512	/* output file stringstore */
#define TSTORSIZ	256	/* input file stringstore	*/

RELOC  *outreloc[3],*tmpreloc;
unsigned oreltop[3]={
	0,0,0};

SYMENT *statstab,*globstab,*tmpstab;
unsigned short filesym[50];
unsigned long filepc[50][4];


#ifdef FLEXNAMES
char *stringstore,*tstringstore;
int stortop=4,tstortop=0;
#endif

unsigned int stattop=0,globtop=0,filetop=0,tmptop=0;
unsigned int fileglob[50];	/* Index into globstab of first entry for file */

/* Externally visible functions */

extern void maketables  ();
extern void mergestabs  ();
extern void enter_udefsym(	/* name        */ );
extern int  showundefs  ();

/*******************************/

void maketables()
{
	/* Make stabs */
	if ((statstab  = (SYMENT *)(malloc(STABSIZ * SYMESZ)))==NULL)
		fatal("cannot make symbol table for output file static symbols");
	if ((globstab  = (SYMENT *)(malloc(STABSIZ * SYMESZ)))==NULL)
		fatal("cannot make symbol table for output file external symbols");
	if ((tmpstab  = (SYMENT *)(malloc(TSTABSIZ* SYMESZ)))==NULL)
		fatal("cannot make symbol table for input files");
	if (makehash(HASHTMP,3*TSTABSIZ/2)==NULL)
		fatal("cannot make hash table for temporary files");
	if (makehash(HASHEXT,3*STABSIZ/2)==NULL)
		fatal("cannot make hash table for external symbols");
#ifdef FLEXNAMES
	if ((stringstore=(char *)malloc(STORSIZ))==NULL)
		fatal("cannot make string store for output");
	if ((tstringstore=(char *)malloc(TSTORSIZ))==NULL)
		fatal("cannot make temporary string store");
#endif

	if ((outreloc[N_TEXT] = (RELOC  *)(malloc(RELCSIZ * RELSZ )))==NULL)
		fatal("cannot make relocation table for output file text section");
	if ((outreloc[N_DATA] = (RELOC  *)(malloc(RELCSIZ * RELSZ )))==NULL)
		fatal("cannot make relocation table for output file data section");
	if ((tmpreloc = (RELOC  *)(malloc(TRELCSIZ* RELSZ )))==NULL)
		fatal("cannot make relocation table for input file");
}

/***************************
   Symbol Table Handling
*****************************/

void enter_udefsym(name)  /* Enter undefined symbol - NB no flexname */
char *name;
{
	if (findhash(HASHEXT,name)!=NULL)
		fprintf(stderr,"ld : -u %s : already exists\n",name);
		else
	{
		strcpy(globstab[globtop].n_name,name);
		globstab[globtop].n_value=0;
		globstab[globtop].n_scnum=N_UNDEF;
		globstab[globtop].n_type=0;
		globstab[globtop].n_sclass=C_EXT;
		globstab[globtop].n_numaux=0;
		enterhash(HASHEXT,globstab[globtop].n_name,&globstab[globtop]);
		globtop++;
	}
}

void mergestabs()
{
	int stabnow;
	SYMENT *symnow,*symold;
	static void copystabs();

	for (stabnow=0;stabnow<tmptop;stabnow++)
	{
		symnow=&tmpstab[stabnow];
		if (symnow->n_sclass!=C_EXT)
		{
			if (!strip || symnow->n_sclass==C_FILE)
			{
				short secn;
				copystabs(symnow,&statstab[stattop],symnow->n_numaux+1);
				if (symnow->n_sclass==C_FILE)
				{
					/* Save section & stab offsets for file for diagnostics */
					for (secn=N_TEXT;secn<=N_BSS;secn++)
						filepc[filetop][secn]=globalpc[secn];
					fileglob[filetop]=globtop;
					filesym[filetop]=stattop;
					if (filetop) statstab[filesym[filetop-1]].n_value=stattop;
					filetop++;
				}
				stattop+=(symnow->n_numaux+1);
				stabnow+=symnow->n_numaux;
			}
		}
		else	{
			/* External symbol */
			if ((symold=(SYMENT *)findhash(HASHEXT,symnow->n_name))==NULL)
			{
				/* No entry yet - make one */
				copystabs(symnow,&globstab[globtop],1+symnow->n_numaux);

				/* NB enterhash here might be wrong for FLEXNAMES */

				enterhash(HASHEXT,globstab[globtop].n_name,&globstab[globtop]);
				globtop+=1+symnow->n_numaux;
			}
			else if (symnow->n_scnum!=N_UNDEF)
			{
				/* Already in stab; now being defined */
				/* If old entry is defined,error	*/
				/* Else add this entry			*/
				if (symold->n_scnum!=N_UNDEF)
				{	/* Redefinition */
					short filenum=filetop;
					redeferrs++;
					fprintf(stderr,"ld : redefinition of %s in %s ",
					    symnow->n_name,filename);
					while (--filenum>0)
					{
						if (symold->n_value>=filepc[filenum][symold->n_scnum]) break;
					}
					fprintf(stderr,"(%s)\n",statstab[filesym[filenum]+1].n_name);
				}
				else
				{	/* Update the entry */
					copystabs(symnow,symold,symnow->n_numaux+1);
				}
			}
		}
	}
}


static void copystabs(from,to)
SYMENT *from,*to;
{
	strcpy(to->n_name,from->n_name);
	to->n_value=from->n_value;
	to->n_sclass=from->n_sclass;
	to->n_scnum=from->n_scnum;
	to->n_type=from->n_type;
	to->n_numaux=from->n_numaux;

#ifdef FLEXNAMES
	if (from->n_zeroes==0)	/* name in stringstore */
	{
		strcpy(&stringstore[stortop],&tstringstore[from->n_offset]);
		to->n_zeroes=0L;
		to->n_offset=stortop;
		stortop+=strlen(&stringstore[stortop]);
	}
#endif

	if (to->n_numaux)
		strcpy((to+1)->n_name,(from+1)->n_name);
}

int showundefs()	/* Error messages for undefined externals */
{
	short snow,filenum;
	static short first=TRUE;

	for (snow=0;snow<globtop;snow++)
		if (globstab[snow].n_scnum==N_UNDEF)
		{
			if (first)
			{
				if (strip)
					fprintf(stderr,"ld : Unresolved externals\n=========================\n\n");
				first=FALSE;
			}
			undeferrs++;
			filenum=filetop;
			while (--filenum>0)
				if (snow>=fileglob[filenum]) break;
			if (strip)
				fprintf(stderr,"%s - first referenced in module %s\n",
				    globstab[snow].n_name,statstab[filesym[filenum]+1].n_name);
		}

	return (!first);	/* Return true if there are still undefed externals */
}


