/* %W% %H%  */

#include <stdio.h>

#include "output.h"	/* Output variables		*/

#define i486OMAGIC	0711
#define F_AR32WR	0400

/*
 * makecoff:
 *
 * Creates a COFF file from all the info
 * available.
 *
 */

void makecoff(void) {
	int i;
	long l,s;
	long *storesize;
	union auxent flname[1];
	SCNHDR	sechead[4];	/* Secction headers		*/
	FILHDR	filehead;	/* File header			*/

	extern char iname[];

	/* Make file header */
	
	filehead.f_magic=RISCOMAGIC;	/* RISC object file */
	filehead.f_nscns=3;
	time(&filehead.f_timdat);	/* Time/date stamp */
	filehead.f_symptr=sizeof(filehead)+3*sizeof(sechead[0])
		+pc[N_TEXT]+pc[N_DATA]+
		(reltop[N_TEXT]+reltop[N_DATA]+reltop[N_BSS])*RELSZ;
	filehead.f_nsyms=stabtop;
	filehead.f_opthdr=0;
	filehead.f_flags=(F_LNNO | F_AR32WR);
	
	/* Section header names */
	
	strcpy(sechead[N_TEXT].s_name,".text");
	strcpy(sechead[N_DATA].s_name,".data");
	strcpy(sechead[N_BSS ].s_name,".bss" );
	
	/* Section physical/virtual addresses */
	
	sechead[N_TEXT].s_paddr=0L;
	sechead[N_DATA].s_paddr=sechead[N_TEXT].s_paddr+pc[N_TEXT];
	sechead[N_BSS ].s_paddr=sechead[N_DATA].s_paddr+pc[N_DATA];
	
	/* Assuming separate text / data-bss sections */
	
	sechead[N_TEXT].s_vaddr=0L;
	sechead[N_DATA].s_vaddr=0L;
	sechead[N_BSS ].s_vaddr=pc[N_DATA];
	
	/* Section sizes */

	sechead[N_TEXT].s_size=pc[N_TEXT];
	sechead[N_DATA].s_size=pc[N_DATA];
	sechead[N_BSS ].s_size=pc[N_BSS ];

	/* Section file pointers */

	l=sizeof(filehead)+3*sizeof(sechead[0]); /* Start of sections */

	sechead[N_TEXT].s_scnptr=pc[N_TEXT]?l:0;
	sechead[N_DATA].s_scnptr=pc[N_DATA]?l+pc[N_TEXT]:0;
	sechead[N_BSS ].s_scnptr=pc[N_BSS ]?l+pc[N_TEXT]+pc[N_DATA]:0;

	/* Relocation entries */

	l+=pc[N_TEXT]+pc[N_DATA]+pc[N_BSS]; /* Start of relocation entries */

	sechead[N_TEXT].s_relptr=reltop[N_TEXT]?l:0;
	sechead[N_TEXT].s_nreloc=reltop[N_TEXT];

	sechead[N_DATA].s_relptr=reltop[N_DATA]?l+RELSZ*reltop[N_TEXT]:0;
	sechead[N_DATA].s_nreloc=reltop[N_DATA];

	sechead[N_BSS ].s_relptr=reltop[N_BSS]?l+RELSZ*(reltop[N_TEXT]+reltop[N_DATA]):0;
	sechead[N_BSS ].s_nreloc=reltop[N_BSS];

	/* Line number info (not present) */

	sechead[N_TEXT].s_lnnoptr=0;
	sechead[N_DATA].s_lnnoptr=0;
	sechead[N_BSS ].s_lnnoptr=0;
	sechead[N_TEXT].s_nlnno=0;
	sechead[N_DATA].s_nlnno=0;
	sechead[N_BSS ].s_nlnno=0;

	/* Flags */

	sechead[N_TEXT].s_flags=STYP_TEXT;
	sechead[N_DATA].s_flags=STYP_DATA;
	sechead[N_BSS ].s_flags=STYP_BSS;

	/* Write the file and section headers */

	fwrite(&filehead,1,sizeof(filehead),obj);
	fwrite(&sechead[N_TEXT],1,SCNHSZ,obj);
	fwrite(&sechead[N_DATA],1,SCNHSZ,obj);
	fwrite(&sechead[N_BSS ],1,SCNHSZ,obj);

	/* Reopen temps, read and write */

	writetemp(txt,N_TEXT);
	writetemp(dat,N_DATA);

	/* Write relocation entries */
	for (i=N_TEXT;i<=N_BSS;i++)
		fwrite(reltab[i],reltop[i],RELSZ,obj);

	/* Write symbol table */

	for (i=0;i<stabtop;i++)
		fwrite(&stab[i],1,SYMESZ,obj);

	/* Write string table */

	storesize=(long *)stringstore;
	*storesize=(unsigned long) stortop;
	fwrite(stringstore,stortop,1,obj);
}

/*
 * writetemp:
 *
 * Writes out a temporary file to the object file.
 *
 */

static void writetemp(FILE *tfp, int sec) {
	char buff[512];
	int num;
	while (pc[sec]) {
		num=fread(buff,1,512,tfp);
		if (num>pc[sec]) num=pc[sec];
		fwrite(buff,1,num,obj);
		pc[sec]-=num;
	}
}


