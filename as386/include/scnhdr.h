/* scnhdr.h	*/

struct scnhdr {
	char		s_name[8];	/* section name		*/
	long		s_paddr;	/* physical address	*/
	long		s_vaddr;	/* virtual address	*/
	long		s_size;		/* size			*/
	long		s_scnptr;	/* file ptr to scn data	*/
	long		s_relptr;	/* file ptr to reloc	*/
	long		s_lnnoptr;	/* file ptr to line nms */
	unsigned short	s_nreloc;	/* # of reloc entries	*/
	unsigned short	s_nlnno;	/* # of line num entries*/
	long		s_flags;	/* flags		*/
	};

#define	SCNHDR	struct scnhdr
#define SCNHSZ	sizeof(SCNHDR)

/* Constants for names of special sections */

#define _TEXT	".text"
#define _DATA	".data"
#define _BSS	".bss"

/* Section types (lower 4 bits of flag) */
/* Acronyms	A - allocated		*/
/*		R - relocated		*/
/*		L - loaded		*/


#define STYP_REG	0x00	/* Regular - ARL	*/
#define STYP_DSECT	0x01	/* Dummy - !AR!L	*/
#define STYP_NOLOAD	0x02	/* Noload - AR!L	*/
#define STYP_GROUP	0x04	/* Grouped		*/
#define STYP_PAD	0x08	/* Padding - !A!RL	*/
#define STYP_COPY	0x10	/* Copy - !A!RL		*/
#define STYP_TEXT	0x20	/* Executable text	*/
#define STYP_DATA	0x40	/* Initialised data	*/
#define STYP_BSS	0x80	/* Unitialised data	*/
