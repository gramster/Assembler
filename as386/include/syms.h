#define SYMNMLEN	8	/* Symbol name length	*/
#define	FILNMLEN	14	/* File name length	*/
#define DIMNUM		4	/* # of array dims	*/
struct syment
{
union	{
		char _n_name[SYMNMLEN];	/* old COFF symbol name	*/
		struct	{
			long _n_zeroes;	/* symbol name?		*/
			long _n_offset;	/* string table location*/
			} _n_n;
		char *_n_nptr[2];
	}	_n;
long		n_value;	/* value of symbol	*/
short		n_scnum;	/* section number	*/
unsigned short	n_type;		/* type and derived type*/
char		n_sclass;	/* storage class	*/
char		n_numaux;	/* # of aux entries	*/
};

#define n_name		_n._n_name
#define n_nptr		_n._n_nptr[1]
#define n_zeroes	_n._n_n._n_zeroes
#define n_offset	_n._n_n._n_offset

/* Special 'section numbers' */

#define N_UNDEF	0	/* undefined symbol	*/
#define N_ABS	-1	/* value is absolute	*/
#define N_DEBUG	-2	/* symbol meaningless	*/

/* Fundamental types (low 4 bits)	*/

#define _EF		".ef"
#define T_NULL		0
#define T_ARG		1	/* function argument	*/
#define T_CHAR		2	/* character		*/
#define T_SHORT		3	/* short integer	*/
#define T_INT		4	/* integer		*/
#define T_LONG		5	/* long integer		*/
#define T_FLOAT		6	/* floating point	*/
#define T_DOUBLE	7	/* double word		*/
#define T_STRUCT	8	/* structure		*/
#define T_UNION		9	/* union		*/
#define T_ENUM		10	/* enumeration		*/
#define T_MOE		11	/* member of enum	*/
#define T_UCHAR		12	/* unsigned char	*/
#define T_USHORT	13	/* unsigned short	*/
#define T_UINT		14	/* unsigned integer	*/
#define T_ULONG		15	/* unsigned long	*/

/* derived types */

#define DT_NON		0	/* no derived type	*/
#define DT_PTR		1	/* pointer		*/
#define DT_FCN		2	/* function		*/
#define DT_ARY		3	/* array		*/

/* type packing constants	*/

#define N_BTMASK	017
#define N_TMASK		060
#define N_TMASK1	0300
#define N_TMASK2	0360
#define N_BTSHFT	4
#define N_TSHIFT	2

/* MACROS	*/

#define BTYPE(x)	((x)&N_BTMASK)		/* basic type	*/
#define ISPTR(x)	(((x)&N_TMASK)==(DT_PTR<<N_BTSHFT))
#define ISFCN(x)	(((x)&N_TMASK)==(DT_FCN<<N_BTSHFT))
#define ISARY(x)	(((x)&N_TMASK)==(DT_FCN<<N_BTSHFT))
#define ISTAG(x)	((x)==C_STRTAG || (x)==C_UNTAG || (x)==C_ENTAG)
#define INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(DT_PTR<<N_BTSHFT)|(x&N_BTMASK))
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))

/* Auxiliary entries */

union auxent
{
struct	{
	long	x_tagndx;
	union	{
		struct	{
			unsigned short	x_lnno;	/* decl. line num	*/
			unsigned short	x_size;	/* str, union, ary size	*/
			} x_lnsz;
		long	x_fsize;	/* function size	*/
		} x_misc;
	union	{
		struct	{
			long	x_lnnoptr;	/* ptr to fn line	*/
			long	x_endndx;	/* entry ndx past .be	*/
			} x_fcn;
		struct	{
			unsigned short	x_dimen[DIMNUM];
			} x_ary;
		} x_fcnary;
	unsigned short	x_tvndx;
	} x_sym;
struct {
	char x_fname[FILNMLEN];
	}	x_file;
struct	{
	long		x_scnlen;	/* section length	*/
	unsigned short	x_nreloc;	/* # of reloc entries	*/
	unsigned short	x_nlinno;	/* number of line nums	*/
	} x_scn;
struct	{
	long		x_tvfill;
	unsigned short	x_tvlen;
	unsigned short	x_tvran[2];
	} x_tv;
};

#define SYMENT		struct syment
#define SYMESZ		18

#define AUXENT		union auxent
#define AUXESZ		18

#define _ETEXT		"_etext"
#define _EDATA		"_edata"
#define _END		"_end"

#define _START		"_start"

#define _TVORIG		"_tvorig"
#define _TORIGIN	"_torigin"
#define _DORIGIN	"_dorigin"

#define _SORIGIN	"_sorigin"
