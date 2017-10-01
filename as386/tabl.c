/* %W% %H%  */

#include <stdio.h>
#include <string.h>

#ifdef DOS
#	include "search.h"
#else
#	include <search.h>
#endif

#include "ops.h"
#include "errors.h"
#include "output.h"

#define TRUE		1
#define FALSE		0

#define FLEXNAMES		/* No 8 char restriction on names	*/

#define IDLENGTH	8	/* Normal identifier length		*/

#ifndef STABSIZ			/* Number of symbol table entries	*/
#define STABSIZ		256
#endif

#ifndef TSTABSIZ		/* Number of temporary entries		*/
#define TSTABSIZ	100
#endif

#ifndef STRSTORSIZ		/* Size of string table for long names	*/
#define STRSTORSIZ	(STABSIZ*2)
#endif

#ifndef HASHSIZ			/* Hash table entries (130% * stab)	*/
#define HASHSIZ		((STABSIZ*13)/10)
#endif

#ifndef RELOCSIZ		/* Number of relocation entries		*/
#define RELOCSIZ	512
#endif

extern ENTRY *hsearch();

SYMENT	*stab,		/* Symbol table			*/
	*tmpstab,	/* Symbol table for temporaries	*/
	*stabent;	/* Current stab entry		*/

int	stabtop,	/* Index of next free stab entry*/
	stabtemp=0;	/* Number of temp entries	*/

extern char	token[],/* Symbol name			*/
		iname[];/* Input source file name	*/
extern int	opnow,	/* Current operation		*/
		pass;	/* Current pass			*/

#ifdef FLEXNAMES	/* Any length id names allowed	*/
  char	*stringstore;	/* Space for extra name chars	*/
  int	stortop;	/* Start of free stringstore	*/
#endif

ENTRY	hashent,	/* Current hash table entry	*/	
	*hashnow;	/* Current hash table pointer	*/

RELOC	*reltab[4];	/* Relocation table pointer	*/
int	reltop[4];	/* Index of next free reloc entry*/

/*
 * makehash
 *
 * This makes the hash table,
 * if memory is available.
 *
 */

void makehash(void) {
	extern int hcreate();
	if (!hcreate(HASHSIZ))
		fatal("Cannot make hash table");
}

/*
 * maketbls
 *
 * Creates the symbol table, relocation table, temporary
 * symbol table, hash table, and string store.
 *
 */

void maketbls(void) {
	int i;

	for (i=0;i<3;i++)
		if ((reltab[i]=(RELOC *)malloc(RELOCSIZ*RELSZ))==NULL)
			fatal("Cannot make relocation table");
	if ((stab=(SYMENT *)malloc(STABSIZ*SYMESZ))==NULL)
		fatal("Cannot make symbol table");
	if ((tmpstab=(SYMENT *)malloc(TSTABSIZ*SYMESZ))==NULL)
		fatal("Cannot allocate temporary symbol space");
#ifdef FLEXNAMES
	if ((stringstore=(char *)malloc(STRSTORSIZ))==NULL)
		fatal("Cannot make string store");
#endif
	makehash();
}

/*
 * removetbls
 */

void removetbls(void) {
	extern void hdestroy();
	hdestroy();
#ifdef FLEXNAMES
	free(stringstore);
#endif
	free((char *)tmpstab);
	free((char *)stab);
	free((char *)reltab);
}

/*
 * cleartbls
 *
 * Clear tables for the next source file.
 *
 */

void cleartbls(void) {
	extern void hdestroy();
	hdestroy();
	makehash();
	makefilestab();
	reltop[N_TEXT]=reltop[N_DATA]=reltop[N_BSS]=stabtemp=0;
	stortop=4;
}

/*
 * makefilestab
 *
 * This makes the first stab entry, for the file name
 * itself.
 *
 */

void makefilestab(void) {
	strcpy(stab[0].n_name,".file");
	stab[0].n_value=0;		/* Value zero till linking	*/
	stab[0].n_scnum=N_DEBUG;	/* Section is DEBUG		*/
	stab[0].n_type=0;
	stab[0].n_sclass=C_FILE;	/* Storage class is 'pointer to next file'*/
	stab[0].n_numaux=1;		/* 1 auxiliary entry		*/

	strcpy(stab[1].n_name,iname);	/* Aux entry contains name	*/
	stabtop=2;			/* Start of free stab space	*/
}

/*
 * mkentry
 *
 * Makes a symbol table entry. NO checks are performed.
 *
 */

void mkentry(char *name, unsigned scl) {
	SYMENT *symtab;
	int where;

	/* choose which stab to search. */
	
	if (scl==TEMPSTAB) {
		symtab=tmpstab;
		where=stabtemp++;
	} else	{
		symtab=stab;
		where=stabtop++;
	}

#ifdef FLEXNAMES
	if ((strlen(name)>IDLENGTH)&&(scl!=TEMPSTAB)) {
		symtab[where].n_zeroes=0L;
		symtab[where].n_offset=stortop;
		strcpy(stringstore+stortop,name);
		hashent.key=stringstore+stortop;
		stortop+=(strlen(name)+1);
	} else 
#endif
		{
		strncpy(symtab[where].n_name,name,IDLENGTH);
		hashent.key=symtab[where].n_name;
	}
	hashent.data=(char *)&symtab[where];
	symtab[where].n_value=0;
	symtab[where].n_sclass=scl;
	symtab[where].n_scnum=(scl==TEMPSTAB)?N_ABS:N_UNDEF;
	symtab[where].n_type=T_NULL;
	symtab[where].n_numaux=0;
	if (hsearch(hashent,ENTER)==NULL)
		fatal("Out of hash table space");
	stabent = (&symtab[where]);
}

/*
 * findentry
 *
 * Returns a pointer to the symbol table entry for toke,
 * or NULL if none exists.
 *
 */

SYMENT *findentry(char *name) {
	hashent.key=name;
	if ((hashnow=hsearch(hashent,FIND))==NULL) stabent=NULL;
	else stabent=(SYMENT *)(hashnow->data);
	return stabent;
}

/*
 * getentry
 *
 * Returns a pointer to the symbol table entry for toke,
 * creating the entry if necessary. This is used for
 * adding information about symbols; thus if the symbol
 * already exists and is not an undefined external or a
 * temporary entry, we may generate an error (we use a
 * flag for this, as there are cases where this is ok, eg
 * changing statics to global.)
 *
 */

SYMENT *getentry(char *name, unsigned chkdups, unsigned scl) {
	stabent=findentry(name);
	switch (pass) {
	case 1: if (stabent==NULL)
			mkentry(name,scl);
		break;
	case 2: if (chkdups && ((stabent->n_scnum) != N_UNDEF))
			if ((stabent->n_scnum)==N_ABS)
				warning(ERR_DUP_DEC,name);
			else error(ERR_DUP_DEC,name);
		break;
	}
	return stabent;
}

/*
 * getstabval
 *
 * gets the value of the stab; returns FALSE if stab doesn't
 * exist on second pass.
 *
 */

int getstabval(char *name, long *val) {
	*val=0;
	if ((stabent=findentry(name)) != NULL) 
		*val=(stabent->n_value);
	else if (pass==2)
		return FALSE;
	return TRUE;
}

/*
 * undefine
 *
 * Set all stab entry section numbers back to undefined
 * for second pass.
 *
 */

void undefine(void) {
	int s_now=2; /* Don't undef the .file entry */

	while (s_now<stabtop)
		stab[s_now++].n_scnum=N_UNDEF;
	s_now=0;
	while (s_now<stabtemp)
		tmpstab[s_now++].n_scnum=N_UNDEF;
}

/*
 * makereloc
 *
 * Makes a relocation entry for a reference to an undefined
 * identifier, creating a stab entry if necessary.
 *
 */

void makereloc(char *toke, long where, int typ) {
	if (pass==2 && getentry(toke,FALSE,C_EXT)!=NULL && (stabent->n_scnum)!=N_ABS) {
		reltab[section][reltop[section]].r_vaddr=where;
		reltab[section][reltop[section]].r_symndx=(stabent-stab);
		reltab[section][reltop[section]].r_type=typ;
		reltop[section]++;
	}
}


