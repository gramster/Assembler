/****************************/
/* toke.h - Lexical tokens */
/****************************/

/* Note - for the actual token symbolic values, powers of two
	(such as CONSTANT) represent classes of tokens, while
	all other values represent specific tokens. Thus we
	can test membership of a class with a bitwise and, and
	individual identity with equality.
*/


#define UNDEF	0	/* Undefined		*/
#define FILEND	(-1)	/* End of file		*/

/* Immediate arguments for assembly */

#define IMMEDIATE	8	/* Immediate class	*/
#define IMEDSHF		9	/* Bit shift value	*/
#define IMEDDAT		10	/* Immediate data	*/
#define IMEDHI		11	/* LDHI	operand		*/
#define IMEDOFF		12	/* Branch offset	*/
#define IMED32		13	/* 32-bit immediate	*/

#define IS_IMMED(c)	(c & IMMEDIATE)

/* Maximum values for different data sizes */

#define U5BITS	31
#define S5BITS	16
#define U8BITS	255
#define S8BITS	128
#define U14BITS	0x3FFF
#define S14BITS	0x2000
#define U16BITS	0xFFFF
#define S16BITS	0x8000
#define U18BITS	0x3FFFFL
#define S18BITS	0x20000L
#define U19BITS	0x7FFFFL
#define S19BITS	0x40000L
#define U32BITS	0xFFFFFFFFL
#define S32BITS	0x80000000L

/* Maximum values for unsigned immediates */

#define VALUSHF	U5BITS
#define VALUTRAP	U8BITS
#define VALUI	U14BITS
#define VALUHI	U18BITS
#define VALUOFF	U19BITS

/* Maximum values for signed immediates */

#define VALSSHF	S5BITS
#define VALSI	S14BITS
#define VALSHI	S18BITS
#define VALSOFF	S19BITS

/* Constants for data segment */

#define CONSTANT	16	/* Constant class	*/
#define BCON		17	/* Byte constant	*/
#define SCON		18	/* Short constant	*/
#define LCON		19	/* Long constant	*/
#define STCON		20	/* String constant	*/

/* Tokens for assembler instructions */

#define REGSTR		64	/* Register		*/
#define IDENT		128	/* Identifier		*/
#define DEFIDENT	256	/* Identifier defn	*/
#define UIDENT		512	/* Unknown id		*/

#define DELIM		1024	/* Delimeter		*/
#define EOLN		2048	/* Line end		*/


/************************
* Lexical information	*
*************************/

extern char token[],	/* Current token	*/
c;	/* Lookahead character	*/
extern int	tktyp;	/* Current token type	*/
extern long	valu;	/* Current token value	*/
extern int	ln,	/* Current line number	*/
filend;	/* End-of-file flag	*/
extern FILE	*fp;	/* Input file		*/
extern char	buff[];	/* Input stream buffer	*/
extern int	bufnow;	/* Buffer position	*/
extern int	opnow;	/* Current operation	*/
extern unsigned long res; /* Assembly result	*/

