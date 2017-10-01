/* %W% %H%  */

#include <stdio.h>
#include <ctype.h>
#include "output.h"
#include "oprnd.h"
#include "errors.h"
#include "ops.h"
#include "toke.h"

#define ERROR		(-1)

struct oprnd opr[3];

/**********************************
*
*	parse: NOTPASSED
*
* This is the main parser.
* It opens the input stream, and
* reads tokens till the end of the
* file. It always expects either
* an operation (in which case
* arguments are read as necessary)
* or a label.
*
*************************************/

parse()
{
	extern unsigned long res;	/* assembly result */

	setuplex();			/* Reset lexical variables */
	while (!filend) {

		/* Skip comments and blank lines */
		do	{
			gettoken(IDENT|EOLN);
			res=(-1L);	/* Clear result (to avoid printing
								unless it has changed	*/
			pcold[N_TEXT]=pc[N_TEXT];
			pcold[N_DATA]=pc[N_DATA];
		} while (tktyp==EOLN && !filend);

		/* Process the token according to type */

		if (tktyp!=EOF)
		{
			if (tktyp==DEFIDENT)		/* Label defn		*/
				do_occur();
			else if ((opnow=findpsop())>=0)	/* Pseudo-operation	*/
				dopseudop();
			else if ((opnow=findop())>=0)	/* RISC operation	*/
			{
				getargs(modtbl[opnow]);
				compose(); /* assemble */
			}
			else if (!filend)		/* Unknown token	*/
				error(ERR_UNKNOWN,token);
		}
	}
}


/****************************
*
*	findop,findpsop: PASSED
*
* Lookup routines for RISC
* instructions and pseudo-
* instructions
*
*****************************/

findop()
{
	return find(optbl,OP_TBL_SIZ);
}

findpsop()
{
	return find(psoptbl,PSOP_TBL_SIZ);
}

find(tabl,tablsiz)
char *tabl[];
int tablsiz;
{
	int t;
	for (t=0;t<tablsiz;t++)
		if (strcmp(token,tabl[t])==0) break;
	if (t<tablsiz) return t;
else return ERROR;
}


/********************************
*
*	getargs,getarg: NOTPASSED
*
* Reads and processes args.
*
**********************************/

getargs(mode)
unsigned long mode;
{
	if (getarg(0,MODE1(mode)))
		if (getarg(1,MODE2(mode)))
			getarg(2,MODE3(mode));
}

getarg(arg,amode)
unsigned arg,amode;
{
	if (ISERROR(errcode)||(amode==NONE))
		goto noarglbl;			/* Error or no operand - skip	*/
	if (arg) if (!accept(',')) goto noarglbl; /* No comma separator - skip	*/

	opr[arg].sz=1;				/* Default - no shift field	*/

	if (c=='%')
		/********************
		* reg[*size][(reg)] *
		*********************/
		{
			if (gettoken(REGSTR)==REGSTR)
				{
					opr[arg].r1=valu;
					if (!get_sz_and_reg(arg,REG,RREG,SRREG)) goto noarglbl;
				}
else goto noarglbl;		/* error - not a valid register	*/
		}

else if (isalnum(c) && (amode==RELAD))
/***********
* reloc *
* address *
************/
{
	getexpression(R_PCRLONG);
	opr[arg].imed=valu;
	opr[arg].admode=RELAD;
}

else if (isalnum(c)|| c=='-')
/***********************
* immed[*size][(reg)] *
* label[(reg)]
************************/
{
	long tmptyp=IS_SHIFT(OPR(opnow))? VALUSHF:
	(IS_BRANCH(OPR(opnow))? VALUOFF: VALUI);
	if (OPR(opnow)==TRAP) tmptyp=VALUTRAP;
	else if (OPR(opnow)==LDHI) tmptyp=VALUHI;
	if (IS_IMMED(gettoken(IMMEDIATE | IDENT)))
	{
		if (valu > tmptyp || valu < (-((tmptyp+1)/2)))
		{
			error(ERR_OVERFLOW);
			goto noarglbl;
		}
		else opr[arg].imed=(valu & tmptyp);
	}
	else /* Identifier - make a relocation entry */
	{
		makereloc(token,pc[N_TEXT],R_RELLONG);
		opr[arg].imed=( (stabent->n_value) & tmptyp);
	}
	if (!get_sz_and_reg(arg,IMMED,IREG,SIREG)) goto noarglbl;

	if ((opr[arg].admode==IMMED) && !IS_IMMED(tktyp)
	    && (tktyp!=IDENT) && (tktyp!=UIDENT))
	{
		error(ERR_NOT_IMM,0);
		goto noarglbl;
	}
}

else
	/**********
	* Unknown *
	***********/
	{
		error(ERR_BAD_ARG,token);
		goto noarglbl;
	}

if (opr[arg].admode & amode) return 1;	/* Operand type ok	*/

error(ERR_BAD_OPR,token);			/* Operand type not ok	*/

noarglbl: 
opr[arg].admode=NONE;		/* No operand returned	*/
return 0;
}

/*********************************
*
*	get_sz_and_reg:	PASSED
*
* This function processes
* register-indirect arguments
* for the shift value and
* base register. It returns
* FALSE if an error occurs.
*
**********************************/

get_sz_and_reg(arg,simpl,ind,sh_ind)
unsigned arg,
simpl,ind,sh_ind;	/* Choice of operand types to return:
					simpl - no shift, no register
					ind   - no shift, register
					sh_ind- shift and register
				*/
{
	if ((c=='*')||(c=='('))		/* Shift-indirect or indirect	*/
	{
		if (c=='*')		/* Shift field present		*/
		{
			accept('*');
			gettoken(BCON);	/* Get shift amount		*/

			if ((!ISSHORT(OPR(opnow))||(valu!=2))&&
			    (!ISLONG(OPR(opnow))||(valu!=4)))
			{
				error(ERR_BAD_SHF,valu);
				return FALSE;
			}
			else opr[arg].sz=valu;
			opr[arg].admode=sh_ind;	/* Got a shift-indirect	*/
		}
		else opr[arg].admode=ind;	/* Got an indirect	*/

		if (accept('('))
		{
			gettoken(REGSTR);
			opr[arg].r2=opr[arg].r1;
			opr[arg].r1=valu;
			if (!accept(')')) return FALSE;
		}
		else return FALSE;
	}
	else opr[arg].admode=simpl;		/* Not shift or shift-indirect */
	return TRUE;
}

