/* filehdr.h */

struct filehdr {
	unsigned short	f_magic;	/* Magic number		*/
	unsigned short	f_nscns;	/* number of sections	*/
	long		f_timdat;	/* time & date stamp	*/
	long		f_symptr;	/* file ptr to symtable	*/
	long		f_nsyms;	/* # of symtab entries	*/
	unsigned short	f_opthdr;	/* optional header size	*/
	unsigned short	f_flags;	/* flags		*/
	};

/* Flags */

#define F_RELFLG	00001	/* Relocation info stripped	*/
#define F_EXEC		00002	/* Executable			*/
#define F_LNNO		00004	/* Line numbers stripped	*/
#define F_LSYMS		00010	/* Local symbols stripped	*/


/* Magic Numbers */

#define i486RWRMAGIC	0400	/* Writeable non-shared	*/
#define i486RROMAGIC	0410	/* Read-only shared	*/

#define FILHDR	struct filehdr
#define FILHSZ	sizeof(FILHDR)

