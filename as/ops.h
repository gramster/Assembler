/* %W% %H% */


/*******************************************/
/* Ops - values of most significant 8 bits */
/*******************************************/

/***************************/
/* Group 0 - Data Movement */
/***************************/

#define LDBU 0x00
#define LDBS 0x04
#define LDSU 0x08
#define LDSS 0x0C
#define LDL  0x10
#define STB  0x14
#define STS  0x18
#define STL  0x1C

/************************/
/* Group 1 - Arithmetic */
/************************/

#define ADD  0x20
#define SUB  0x24
#define SUBR 0x28

/*****************************/
/* Group 2 - Logical/Bitwise */
/*****************************/

#define AND  0x40
#define OR   0x44
#define XOR  0x48
#define LAND 0x4C
#define LOR  0x50
#define SRA  0x54
#define SLL  0x58
#define SRL  0x5C

/**************************/
/* Group 3 - Control Flow */
/**************************/

#define TRAP 0x60
#define JMP  0x64
#define JSR  0x68
#define BSR  0x6C
#define BF   0x70
#define BT   0x74
#define RTS  0x78
#define RTI  0x7C

/***************************/
/* Group 4 - Miscellaneous */
/***************************/

#define LDHI 0x80
#define NOP  0x84

/************************/
/* Group 5 - Privileged */
/************************/

#define WAIT 0xA0
#define IOFF 0xA4
#define ION  0xA8
#define STPC 0xAC

/*************************/
/* Group 6 - Test Signed */
/*************************/

#define TEQ  0xC0
#define TNE  0xC4
#define TGE  0xC8
#define TGT  0xCC
#define TLE  0xD0
#define TLT  0xD4

/***************************/
/* Group 7 - Test Unsigned */
/***************************/

#define THS  0xE0
#define THI  0xE4
#define TLS  0xE8
#define TLO  0xEC

/******************/
/* Classification */
/******************/

#define ISSHORT(o)	((o==LDSU)||(o==LDSS)||(o==STS))
#define ISLONG(o)	((o==LDL)||(o==STL)||(o==JMP)||(o==JSR))

#define ISMOVE(o)	((o>=LDBU)&&(o<=STL))
#define ISLOAD(o)	((o>=LDBU)&&(o<=LDL))
#define ISSTORE(o)	((o>=STB)&&(o<=STL))

#define IS_BRANCH(o)	(o==BF || o==BT || o==BSR)
#define IS_SHIFT(o)	(o==SLL || o==SRL || o==SRA)

#define ISARITH(o)	((o>=ADD)&&(o<=SUBR))

#define ISLOGIC(o)	((o>=AND)&&(o<=SRL))

#define ISCONTROL(o)	((o>=TRAP)&&(o<=RTI))

#define ISMISC(o)	((o>=LDHI)&&(o<=NOP))

#define ISPRIV(o)	((o>=WAIT)&&(o<=STPC))

#define ISSGNTST(o)	((o>=TEQ)&&(o<=TLT))

#define ISUSGNTST(o)	((o>=THS)&&(o<=TLO))

#define TABL(o)		(o/4)
#define OPR(i)		(i*4)

/***********
* Op Table *
***********/

#define OP_TBL_SIZ	64

extern char *optbl[OP_TBL_SIZ];
extern unsigned long modtbl[OP_TBL_SIZ];

/**************************/
/* Possible Operand Types */
/**************************/

#define NONE	0
#define REG	1		/* %rn			*/
#define IREG	2		/* $nn(%rn) or Id(%rn)	*/
#define SIREG	4		/* $nn*s(%rn)		*/
#define RREG	8		/* %rd(%rs)		*/
#define SRREG	16		/* %rd*s(%rs)		*/
#define IMMED	32		/* $nnnn or $Id		*/
#define RELAD	64		/* Reloc address (label)*/

#define SHIFT	128
#define ADDRESS	(IREG|RREG|SIREG|SRREG)	/* Loads/stores	*/
#define ARITH	(REG|IMMED)		/* Arithmetic	*/

/******************/
/* Mode Encodings */
/******************/

#define MODE(o1,o2,o3)	((unsigned long)(o1+SHIFT*(o2+SHIFT*o3)))
#define MODE1(m)		(m%SHIFT) 		/* Extract operand 1 mode */
#define MODE2(m)		((m/SHIFT)%SHIFT)	/* Extract operand 2 mode */
#define MODE3(m)		((m/SHIFT)/SHIFT)	/* Extract operand 3 mode */


/*********************/
/* Pseudo-operations */
/*********************/

/************************
* Location counter	*
* and segment control	*
************************/

#define TEXT	0
#define DATA	1
#define ORG	2
#define ALIGN	3

/************************
* Data initialisation	*
************************/

#define BYTE	4	/* Assign bytes		*/
#define SHORT	5	/* Assign 16-bits	*/
#define LONG	6	/* Assign words		*/
#define SPACE	7	/* Assign zero bytes	*/

/************************
* Symbol Definition	*
************************/

#define SET	8	/* Set <id> to <val>	*/
#define LCOMM	9	/* Allocate common mem.	*/
#define COMM	10	/* Assign common memory	*/
#define GLOBAL	11	/* Make <id> global	*/

/************************
* Debugging		*
************************

#define FILE	12	* Name of source file	*
#define LN	13	* Line number		*
#define DEF	14	* Start/end of symbol	*
#define ENDDEF	15	*	table entry	*

************************
* Attribute operators	*
* for symbol table	*
************************

#define ATT_VAL	16	* Value			*
#define ATT_SCL	17	* Storage Class		*
#define ATT_TYP	18	* Type			*
#define ATT_TAG	19	* Structured type tag	*
#define ATT_LN	20	* Block line number	*
#define ATT_SIZ	21	* Size			*
#define ATT_DIM	22	* Array dimension	*/

/************************
* Number of pseudo-ops	*
************************/

#define PSOP_TBL_SIZ	12

/************************
* Pseodo-op table	*
************************/

extern char *psoptbl[];

/************************
* Assembly pass		*
************************/

extern int pass;

