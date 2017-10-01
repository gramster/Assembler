/* %W% %H%  */

#include <stdio.h>
#include <ctype.h>
#include "toke.h"	/* Token types			*/
#include "errors.h"	/* Error table and messages	*/
#include "include/storclass.h"
#include "output.h"

#define BUFSZ		256
#define OCTAL		8
#define DECIMAL		10
#define HEXADECIMAL	16

#define TRUE	1
#define FALSE	0

#define REGISTERS	32	/* Number of registers */

extern FILE *fp;	/* File being read		*/

char	c,		/* Current lookahead character	*/
	token[BUFSZ],	/* Current lexical token	*/
	*tp;		/* Current token pointer	*/
long	valu;		/* Value of token if numeric	*/
int tktyp;		/* Token type			*/

char	buff[BUFSZ];	/* Input stream buffer		*/
int	bufnow,		/* Current position in buffer	*/
	bufend;		/* End of buffer		*/
extern int filend;	/* End-of-file flag		*/

/*
 * setuplex: Initialises all lexical vars.
 */

void setuplex(void) {
	bufend=errcode=bufnow=buff[0]=0;
	c='\n';
}

/*
 * nextline: list previous line, if necessary, and read the next line.
 */

void nextline(void) {
	writeline();
	if (errcode) writerror();
	bufend=bufnow=0;
	while (!filend && !bufend) {
		fgets(buff,BUFSZ,fp);
		filend=feof(fp)?1:0;
		bufend=strlen(buff);
	}
}

/*
 * nextchar: return next character from input stream.
 */

char nextchar(void) {
	if (bufnow>=bufend) nextline();
	c=buff[bufnow++];
	return c;
}

/*
 * gettoken: return next token in the string `token', and 
 *	its type in `typ'. `typ' can be any type in 'toke.h'.
 *	If it is numeric, the value is returned in `valu'.
 */

#define EXPECT(is)	(expect & (is))

int gettoken(unsigned expect) {
	extern void readnumber();
	extern char getescape();

	tp=token;

	skipspace();
	while (c=='#') skip('\n');  /* Skip comments */

	tktyp=UNDEF;
	if (EXPECT(IDENT|IMMEDIATE|CONSTANT)
		&& (isalpha(c)||(c=='_')||(c=='~')))
			tktyp=getident();

	else if (EXPECT(REGSTR) && (c=='%'))
		tktyp=getreg();

	else if (EXPECT(IMMEDIATE|CONSTANT)
		&& (isdigit(c)||(c=='-')))
			readnumber(&valu,&tktyp,expect);

	else if (EXPECT(BCON) && (c=='\''))
		tktyp=get_ch_con();

	else if (EXPECT(BCON) && (c=='"'))
		tktyp=get_str_con();

	else if (EXPECT(DELIM|EOLN) && (ispunct(c) || (c=='\n')))
		tktyp=get_punc();

	else if (filend) tktyp=FILEND;

	else error(ERR_EXPECT,expect);

	*tp=0;
	skipspace(); /* This might be redundant? */
	return tktyp;
}

/*
 * getident: read an identifier	name, and return its 'type'.			*
 */

int getident(void) {
	do	{
		*tp++=(c=='~'?'.':c);
		nextchar();
	} while(isalpha(c)||isdigit(c)||(c=='_')||(c=='%')||(c=='~'));

	*tp=0; /* Terminate name with ASCII nul */

	skipspace();
	if (c==':') {	/* Label definition		*/
		accept(':');
		return DEFIDENT;
	} else {
		/* Try to get value from symbol table	*/
		if (getstabval(token,&valu)) return IDENT;
		else return UIDENT; /* Undef - not in symbol table	*/
	}
}

/*
 * getreg: read a register name.
 */

int getreg(void) {
	if (accept('%'))
		if (accept('r'))
			if (isdigit(c)) {
				valu=c-'0';
				nextchar();
				if (isdigit(c)) {
					valu=valu*10+c-'0';
					nextchar();
					}
				if (valu<REGISTERS) return REGSTR;
				else error(ERR_BAD_REG,valu);
				}
			else error(ERR_NO_REG,0);
	return UNDEF;
}

/*
 * getescape: read and return an escaped character.
 */

char getescape(void) {
	switch(nextchar()) {
	case 'b': return '\b';
	case 't': return '\t';
	case 'n': return '\n';
	case 'r': return '\r';
	case 'f': return '\f';
	case 'v': return '\v';
	case 'h': return '\h';
	case '"': return '"';
	default : return c;
	}
}


/*
 * get_ch_con: reads and returns a character constant.
 */

int get_ch_con(void) {
	if (nextchar()=='\\') valu=getescape();
	else valu=(unsigned)c;
	nextchar();
	return BCON;
}

/*
 * get_str_con:	reads and returns a string constant.
 */

int get_str_con(void) {
	while (nextchar()!='"')  {
		if (c=='\\') *tp++=getescape();
		else *tp++=c;
	}
	nextchar();
	return STCON;
}

/*
 * get_punc: reads and returns a delimeter or newline character.
 */

int get_punc(void) {
	*tp++=c;
	nextchar();
	return (token[0]=='\n')?EOLN:DELIM;
}

/********************************
* Basic lexical functions	*
*********************************/

void skipspace(void) {
	while (isspace(c) && !filend && (c!='\n')) nextchar();
}

void skip(char ch) {
	while ((c!=ch)&&!filend) nextchar();
	nextchar();
	skipspace();
}

int accept(char ch) {
	skipspace();
	if (toupper(c)!=toupper(ch)) {
		error(ERR_UNEXPCT,ch);
		return FALSE;
	}
	nextchar();
	skipspace();
	return TRUE;
}


/****************************************
* Routines to read unsigned numbers in	*
* octal, decimal or hexadecimal.	*
****************************************/

int isnumeric(int bs, char *c) {
	if (*c<'0') return FALSE;
	if (bs==16) {
		if ((*c>='A') && (*c<='F')) *c-=('A'-10);
		else if ((*c>='a') && (*c<='f')) *c-=('a'-10);
		else if (*c<='9') *c-='0';
		else return FALSE;
	} else {
		if (bs==10) {
			if (*c<='9') *c-='0';
			else return FALSE;
		} else {
			if (*c<='7') *c-='0';
			else return FALSE;
		}
	}
	return TRUE;
}

long getnum(int bs) {
	long res=0;
	while (isnumeric(bs,&c)) {
		res=res*bs+c;
		nextchar();
	}
	return res;
}

void readnumber(long *val, int *typ, unsigned exp) {
	long mag;
	int issgnd;


	/* Check if signed... */

	if (c=='-')	{ *val=(-1);	issgnd=1;	accept('-');}
	else		{ *val=1;	issgnd=0; }

	/* Get the magnitude in the appropriate base... */

	if (c!='0') mag=getnum(DECIMAL);
	else if ((nextchar())=='x') {
		nextchar();
		mag=getnum(HEXADECIMAL);
	} else mag=getnum(OCTAL);

	/* Set val to sign*magnitude */

	*val *= mag;

	/* Find the appropriate type */

	if (exp & IMMEDIATE)
		if ((mag>VALUOFF)||((mag>VALSOFF)&&issgnd)) *typ=IMED32;
		else if ((mag>VALUHI)||((mag>VALSHI)&&issgnd)) *typ=IMEDOFF;
		else if ((mag>VALUI)||((mag>VALSI)&&issgnd)) *typ=IMEDHI;
		else if ((mag>VALUSHF)||((mag>VALSSHF)&&issgnd)) *typ=IMEDDAT;
		else *typ=IMEDSHF;
	else
		if ((mag>U16BITS)||((mag>S16BITS)&&issgnd)) *typ=LCON;
		else if ((mag>U8BITS)||((mag>S8BITS)&&issgnd)) *typ=SCON;
		else *typ=BCON;
}


/*
 * getexpression: behaves much like the gettoken routine, but reads
 *		expressions.
 */

extern long absexpression();

void getexpression(int exptype) {
	long val;
	int ptyp;

	if (c=='(')
		valu=absexpression();
	else {
		ptyp=gettoken(CONSTANT|IDENT);
		val=valu;
		if (ptyp==IDENT || ptyp==UIDENT) {
			findentry(token);
			if (stabent->n_scnum!=N_ABS) {
				val=0;
				makereloc(token,pc[section],exptype);
			} else if (exptype==R_PCRLONG)
				error(ERR_NOT_RELAD,token);
		} else if (exptype==R_PCRLONG)
			error(ERR_NOT_RELAD,token);
		if (c=='+') {
			accept('+');
			val+=absexpression();
		} else if (c=='-') {
			accept('-');
			val-=absexpression();
		}
		valu=val;
		if (exptype==R_RELWORD && (valu>>16))
			error(ERR_OVERFLOW,0);
	}
}


static long primary(void) {
	int ptyp;
	long val;

	if (c=='(') {
		accept('(');
		val=absexpression();
		accept(')');
	} else {
		ptyp=gettoken(CONSTANT|IDENT);
		val=valu;
		if (ptyp==UIDENT) error(ERR_UID_EXPR,token);
		else if (!(ptyp&(CONSTANT|IDENT))) error(ERR_IN_EXPR);
		else if (ptyp==IDENT) { /* Must be absolute id */
			if (getentry(token,FALSE,C_EXT)!=NULL) {
				if ((stabent->n_scnum)!=N_ABS)
					error(WARN_NOT_ABS,token);
			} else error(ERR_UID_EXPR,token);
		}
	}
	return val;
}

static long factor(void) {
	unsigned short sign=1;

	if (c=='-') { sign=(-1); accept('-'); }
	return (sign*primary());
}

static long term(void) {
	long lvalu,rvalu;
	char op;

	lvalu=factor();
	if ((c=='*')||(c=='/')) {
		op=c;
		accept(op);
		rvalu=term();
		if (op=='*') lvalu*=rvalu; else lvalu/=rvalu;
	}
	return lvalu;
}

static long absexpression(void) {
	long lvalu,rvalu;
	char op;

	lvalu=term();
	if ((c=='+')||(c=='-')) {
		op=c;
		accept(op);
		rvalu=absexpression();
		if (op=='+') lvalu+=rvalu; else lvalu-=rvalu;
	}
	return lvalu;
}


