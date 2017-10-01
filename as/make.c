/*
 * make.c - (c) 1987, 1992 by Graham Wheeler
 * All Rights Reserved
 */

#include <stdio.h>
#include "toke.h"
#include "ops.h"
#include "oprnd.h"
#include "output.h"
#include "errors.h"

#define TRUE		1
#define FALSE		0

/*
 * The following macros are used to build a 32-bit RISC
 * instruction from its constituent parts. PRS requires special
 * treatment as it spans the boundary between 16- and 32-bit values.
 */

#define POPT(o)		res=o*0x01000000L	/* Put operation	*/
#define PMODE(o)	res+=o*0x01000000L	/* Put addressing mode	*/
#define IBIT		res+=0x01000000L	/* Immediate addressing	*/
#define SBIT		res+=0x02000000L	/* Shift addressing	*/
#define PRD(o)		res+=(o*0x00080000L)	/* Destination register	*/
#define PRS(o)		res+=((unsigned long)o*0x00004000L) /* 1st source reg*/
#define PRSS(o)		res+=(o*0x00000200L)	/* 2nd source register	*/
#define PIMED(o)	res+=(o)		/* Immediate operands	*/

unsigned long res;		/* 32-bit result	*/
int opnow;			/* Current operation	*/

char tmptoke[20];		/* Temp storage for remembering a token */

/*
 * do_comm: This function handles the 'comm' pseudo-operation
 *	which is used for defining 'extern's. It searches for a symbol
 *	table entry, checking for redefinitions, and creates a new entry
 *	if necessary. The new entry has its value set to the size of the item
 *	being declared.
 */

void do_comm(void) {
	gettoken(IDENT);
	strcpy(tmptoke,token);
	accept(',');
	gettoken(CONSTANT);
	if ((void *)getentry(tmptoke,TRUE,C_EXT)!=NULL)
		stabent->n_value=valu;
}

/*
 * do_lcomm: This function handles the 'lcomm' pseudo-op for
 *	declaring unitialised data items. The symbol table is
 *	searched, checking for redefinitions, and an entry is
 *	made if needed. The entry is attached to the bss section,
 *	it's value set to its offset in the section, and the bss
 *	pc updated.
 */

void do_lcomm(void) {
	gettoken(IDENT);
	strcpy(tmptoke,token);
	accept(',');
	gettoken(CONSTANT);
	if ((void *)getentry(tmptoke,TRUE,C_STAT)!=NULL) {
		stabent->n_scnum=N_BSS;
		stabent->n_value=pc[N_BSS];
		pc[N_BSS]+=valu;
	}
}

/*
 * do_global: This handles the 'global' pseudo-op, for declaring
 *	external identifiers. The symbol table is searched, and a new
 *	entry made if needed. The entry is then made externally visible.
 */

void do_global(void) {
	gettoken(IDENT);
	if ((void *)getentry(token,FALSE,C_EXT)!=NULL)
		stabent->n_sclass=C_EXT;
}

/*
 * do_occur: This handles assembly label declarations (a label
 *	declaration is an occurence of the label suffixed with * a ':').
 *	The label is associated with the current section, and its value
 *	set to its address in the section.
 */

void do_occur(void) {
	if ((void *)getentry(token,TRUE,C_STAT)!=NULL) {
		stabent->n_scnum=section;
		stabent->n_value=pc[section];
	}
}

/*
 * do_set: This handles the 'set' pseudo-operation. A temporary
 *	entry in the symbol table is found or made, and its value is set
 *	to the value of the set expression. This is the only way in
 *	which absolute valued (ie not relocatable) ids can be put into
 *	the stab.
 */

void do_set(void) {
	long val;
	gettoken(IDENT);
	strcpy(tmptoke,token);
	val=absexpression();
	if ((void *)getentry(tmptoke,TRUE,TEMPSTAB)!=NULL) {
		stabent->n_value=val;
		stabent->n_scnum=N_ABS;
	}
}

/*
 * bytedata: This handles an argument of the 'byte' pseudo-op.
 *	Arguments can be short integers, characters or strings.
 *	The argument is written to the temporary file for the data section.
 */

void bytedata(void) {
	char *tmp;
	gettoken(BCON);
	if (tktyp==STCON) {
		tmp=token;
		while (*tmp) writedat(*tmp++);
	} else if (tktyp&BCON==BCON) /* Shaky! */
		writedat((char)(valu & 0xFF));
}

/*
 * shortdata: This handles an argument of the 'short' pseudo-op.
 *	An expression is read, and its least significant 16 bits
 *	are written in two 8-bit halves.
 */

void shortdata(void) {
	getexpression(R_RELWORD);
	writedat((char)(valu & 0xFF));
	writedat((char)((valu>>8) & 0xFF));
}

/*
 * longdata: This handles an argument of the 'long' pseudo-op.
 *	It reads an expression, and writes the value of the
 *	expression in 4 8-bit pieces.
 */

void longdata(void) {
	int bitpos;
	getexpression(R_RELLONG);
	for (bitpos=0;bitpos<=24;bitpos+=8)
		writedat((char)(0xFF & (valu>>bitpos)));
}

/*
 * setpc: This function is used by the 'org' and 'align'
 *	pseudo-operations, to increase the value of
 *	the pc of the current section.
 */

void setpc(int sec, unsigned long new) {
	while (pc[sec]<new)
		if (sec==N_TEXT) writetxt(0x01000000L*(long)NOP);
		else if (sec==N_DATA) writedat(0);
		else pc[sec]=new;
}

/*
 * dopseudop: This function processes the assembler's pseudo-operations.
 */

void dopseudop(void) {
	long val;
	switch (opnow) {
	case TEXT:	section=N_TEXT; break;
	case DATA:	section=N_DATA; break;

	case ORG:	val=absexpression();
			if (errcnt==0)
				if (pc[section]>val) error(ERR_ORIGIN,0);
				setpc(section,val);
			break;

	case ALIGN:	gettoken(BCON);
			if ((valu!=2)&&(valu!=4)) error(ERR_ALGN_EXPR);
			else setpc(section,valu*((pc[section]-1)/valu+1));
			break;

	case BYTE:	if (section==N_TEXT) {
				warning(WARN_TODATA,0);
				section=N_DATA;
			}
			bytedata();
			while (c==',') {
				accept(',');
				bytedata();
			}
			break;

	case SHORT:	if (section==N_TEXT) {
				warning(WARN_TODATA,0);
				section=N_DATA;
			}
			shortdata();
			while (c==',') {
				accept(','); 
				shortdata(); 
			}
			break;

	case LONG:	if (section==N_TEXT) {
				warning(WARN_TODATA,0);
				section=N_DATA;
			}
			longdata();
			while (c==',') { 
				accept(','); 
				longdata(); 
			}
			break;

	case SPACE:	if (section==N_TEXT) {
				warning(WARN_TODATA,0);
				section=N_DATA;
			}
			gettoken(CONSTANT);
			while (valu--) writedat(FALSE,0);
			break;

	case SET:	do_set();
			break;

	case LCOMM:	do_lcomm();
			break;

	case COMM :	do_comm();
			break;

	case GLOBAL:	do_global();
			break;
	}

}

/*
 * compose: This function uses the information provided by the
 *	parser to create the 32-bit instruction word.
 */

void compose(void) {
	short t;
	unsigned opp;

	if (section!=N_TEXT) {
		warning(WARN_TOTEXT,0);
		section=N_TEXT;
	}
	opp=OPR(opnow);

	POPT(opp);
	if (ISMOVE(opp)) {
		t=(ISSTORE(opp)?1:0);
		PMODE(abits(t));
		PRD(opr[1-t].r1);
		PRS(opr[t].r1);
		if (opr[t].admode&(RREG|SRREG)) PRSS(opr[t].r2);
		if (opr[t].admode&(IREG|SIREG)) PIMED(opr[t].imed);
	} else if (ISARITH(opp)||ISLOGIC(opp)) {
		PRD(opr[2].r1);
		PRS(opr[0].r1);
		if (opr[1].admode==IMMED) {
			IBIT; 
			PIMED(opr[1].imed);
		} else PRSS(opr[1].r1);
	} else if (ISCONTROL(opp)) {
		if (opp!=JMP) PRD(opr[0].r1);
		else PIMED(opr[0].imed);
		if ((opp!=RTS)&&(opp!=RTI)&&(opp!=JMP)) {
			PMODE(abits(1));
			switch(opr[1].admode) {
			case RELAD:
			case IMMED:	PIMED(opr[1].imed);
					break;
			case IREG :	PIMED(opr[1].imed);
					PRS(opr[1].r1);
					break;
			case RREG :	PRSS(opr[1].r2);
					PRS(opr[1].r1);
			default: break;
			}
		}
	} else if (ISMISC(opp)) {
		if (opp==LDHI) {
			IBIT;
			PRD(opr[0].r1);
			PIMED(opr[1].imed);
		}
	} else if (ISPRIV(opp)) {
		if (opp==STPC) PRD(opr[0].r1);
	} else if (ISSGNTST(opp)||ISUSGNTST(opp)) {
		PRD(opr[2].r1);
		PRS(opr[0].r1);
		if (opr[1].admode==IMMED) {
			IBIT;
			PIMED(opr[1].imed);
		} else PRSS(opr[1].r1);
	}
	writetxt(res);
}

/*
 * abits: This function returns the value of the address mode
 *	bits for each special addressing mode.
 */

int abits(short arg) {
	switch(opr[arg].admode) {
	case IREG    : return 1;
	case SIREG   : return 3;
	case RREG    : return 0;
	case SRREG   : return 2;
	case IMMED   : return 1;
	default      : return 0;
	}
}


