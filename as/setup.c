/* %W% %H%  */

#include "ops.h"
#include "errors.h"

/****************************************
* Tables:				*
*	optbl - instruction mnemonics	*
*	psoptbl - pseudo operations	*
*	modtbl - permitted operands	*
*	errtbl - error messages		*
****************************************/

unsigned long
	modtbl  [OP_TBL_SIZ];
char	*optbl  [OP_TBL_SIZ];
char	*psoptbl[PSOP_TBL_SIZ];
char	*errtbl [ERR_TBL_SIZ];

/************************
* setupops:		*
*	Sets up the	*
*	mnemonic table	*
************************/

setupops()
{
int i;

for (i=0;i<64;i++) optbl[i]="";

/*****************/
/* Data Movement */
/*****************/

optbl[ 0]="ldbu"; optbl[ 1]="ldbs"; optbl[ 2]="ldsu"; optbl[ 3]="ldss";
optbl[ 4]="ldl";  optbl[ 5]="stb";  optbl[ 6]="sts";  optbl[ 7]="stl";
modtbl[0]=MODE(ADDRESS,REG,NONE); modtbl[1]=MODE(ADDRESS,REG,NONE);
modtbl[2]=MODE(ADDRESS,REG,NONE); modtbl[3]=MODE(ADDRESS,REG,NONE);
modtbl[4]=MODE(ADDRESS,REG,NONE); modtbl[5]=MODE(REG,ADDRESS,NONE);
modtbl[6]=MODE(REG,ADDRESS,NONE); modtbl[7]=MODE(REG,ADDRESS,NONE);

/**************/
/* Arithmetic */
/**************/

optbl[ 8]="add"; optbl[ 9]="sub";  optbl[10]="subr";
modtbl[8]= MODE(REG,ARITH,REG);	modtbl[9]= MODE(REG,ARITH,REG);
modtbl[10]=MODE(REG,IMMED,REG);

/***********/
/* Logical */
/***********/

optbl[16]="and"; optbl[17]="or";   optbl[18]="xor"; optbl[19]="land";
optbl[20]="lor"; optbl[21]="sra";  optbl[22]="sll"; optbl[23]="srl";
for (i=16;i<=23;i++) modtbl[i]=MODE(REG,ARITH,REG);

/***********/
/* Control */
/***********/

optbl[24]="trap"; optbl[25]="jmp"; optbl[26]="jsr"; optbl[27]="bsr";
optbl[28]="bf";	  optbl[29]="bt";  optbl[30]="rts"; optbl[31]="rti";
modtbl[24]=MODE(REG,IMMED,NONE); modtbl[25]=MODE(ADDRESS,NONE,NONE);
modtbl[26]=MODE(REG,ADDRESS,NONE); modtbl[27]=MODE(REG,RELAD,NONE);
modtbl[28]=MODE(REG,RELAD,NONE);
modtbl[29]=MODE(REG,RELAD,NONE);
modtbl[30]=MODE(REG,NONE,NONE); modtbl[31]=MODE(REG,NONE,NONE);

/*****************/
/* Miscellaneous */
/*****************/

optbl[32]="ldhi";		 optbl[33]="nop";
modtbl[32]=MODE(REG,IMMED,NONE); modtbl[33]=MODE(NONE,NONE,NONE);

/**************/
/* Privileged */
/**************/

optbl[40]="wait"; optbl[41]="ioff"; optbl[42]="ion"; optbl[43]="stpc";
modtbl[40]=MODE(NONE,NONE,NONE); modtbl[41]=MODE(NONE,NONE,NONE);
modtbl[42]=MODE(NONE,NONE,NONE); modtbl[43]=MODE(REG,NONE,NONE);

/****************/
/* Signed Tests */
/****************/

optbl[48]="teq"; optbl[49]="tne"; optbl[50]="tge";
optbl[51]="tgt"; optbl[52]="tle"; optbl[53]="tlt";

/******************/
/* Unsigned Tests */
/******************/

optbl[56]="ths"; optbl[57]="thi"; optbl[58]="tls"; optbl[59]="tlo";
for (i=48;i<=59;i++) modtbl[i]=MODE(REG,ARITH,REG);

}

/****************************************
setuppsops:
	This function sets up the table
	of assembler pseudo-operations
*****************************************/

setuppsops()
{
/****************
* Control	*
*****************/

psoptbl[TEXT	]="text";	psoptbl[DATA	]="data";
psoptbl[ORG	]="org";	psoptbl[ALIGN	]="align";

/****************
* Data		*
****************/

psoptbl[BYTE	]="byte";	psoptbl[SHORT	]="short";
psoptbl[LONG	]="long";	psoptbl[SPACE	]="space";

/****************
* Symbols	*
****************/

psoptbl[SET	]="set";	psoptbl[LCOMM	]="lcomm";
psoptbl[COMM	]="comm";	psoptbl[GLOBAL	]="global";

}

/************************
* setuperrs:		*
*	Sets up the	*
*	error message	*
*	table		*
************************/

setuperrs()
{
errtbl[ERR_DUP_DEC]	= "Illegal attempt to redefine %s";
errtbl[ERR_UNDEF]	= "Label %s used but not defined";
errtbl[ERR_UNEXPCT]	= "%c expected";
errtbl[ERR_UNKNOWN]	= "Unrecognised instruction %s";
errtbl[ERR_NOT_NUM]	= "Numeric constant expected";
errtbl[ERR_NOT_IMM]	= "Immediate value expected";
errtbl[ERR_BAD_OPR]	= "Incorrect operand type %s";
errtbl[ERR_NO_REG]	= "Register number expected";
errtbl[ERR_BAD_REG]	= "Invalid register number %d";
errtbl[ERR_BAD_SHF]	= "Invalid shift size %d for this instruction";
errtbl[ERR_OVERFLOW]	= "Immediate value exceeds allowed range";
errtbl[ERR_ORIGIN]	= "New origin is less than old";
errtbl[ERR_IN_EXPR]	= "Bad expression";
errtbl[ERR_UID_EXPR]	= "Unknown identifier %s in expression";
errtbl[ERR_BAD_ARG]	= "Invalid or bad argument type : %s";
errtbl[ERR_ALGN_EXPR]= "Align value must be 2 or 4";
errtbl[ERR_NOT_RELAD]= "Destination %s must be relocatable";
errtbl[WARN_TOTEXT]	= "Switching output to text section";
errtbl[WARN_TODATA]	= "Switching output to data section";
errtbl[ERR_EOF]		= "\nUnexpected end-of-file";
errtbl[WARN_NOT_ABS]	= "Identifier %s should be absolute";
}


