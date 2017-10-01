/* reloc.h */

struct reloc {
	long	r_vaddr;	/* virtual address of reference */
	long	r_symndx;	/* index into symbol table	*/
	unsigned short r_type;	/* relocation type		*/
	};

#define RELOC		struct reloc
#define RELSZ		10

#define R_ABS		0
#define R_RELBYTE	017
#define R_RELWORD	020
#define R_RELLONG	021
#define R_PCRBYTE	022
#define R_PCRWORD	023
#define R_PCRLONG	024
