/* %W% %H%  */

/*****************
* Error messages *
*****************/

#define	ERR_EOF		1
#define ERR_UNDEF	2
#define ERR_EXPECT	3
#define ERR_UNKNOWN	4
#define ERR_NOT_NUM	5
#define ERR_NOT_IMM	6
#define ERR_BAD_OPR	7
#define ERR_NO_REG	8
#define ERR_BAD_REG	9
#define ERR_OVERFLOW	10
#define ERR_UNEXPCT	11
#define ERR_ORIGIN	12
#define ERR_ALGN_EXPR	13
#define ERR_FATAL	14
#define ERR_BAD_SHF	15
#define ERR_IN_EXPR	16
#define ERR_UID_EXPR	17
#define ERR_BAD_ARG	18
#define ERR_DUP_DEC	19
#define ERR_NOT_RELAD	20
#define ERR_TOP		20

#define ISERROR(c)	((c>0)&&(c<=ERR_TOP))

#define WARN_TOTEXT	(ERR_TOP+1)
#define WARN_TODATA	(ERR_TOP+2)
#define WARN_NOT_ABS	(ERR_TOP+3)
#define WARN_TOP	(ERR_TOP+3)

#define ISWARNING(c)	(c>ERR_TOP)

/********************************
* Number of error messages	*
********************************/

#define ERR_TBL_SIZ	(WARN_TOP+1)

/****************
* Error data	*
****************/

extern char	*errtbl[],	/* Table of error messages	*/
		errstring[];	/* Custom built error message	*/
extern int	errcode,	/* Error code			*/
		errcnt;		/* Number of errors so far	*/
extern unsigned	errcol;		/* Column (position) of error	*/
extern char	*errval;	/* Optional arg for message	*/

