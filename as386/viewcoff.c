#include <stdio.h>
#include "include/filehdr.h"
#include "include/scnhdr.h"
#include "include/reloc.h"
#include "include/storclass.h"
#include "include/syms.h"

#define FLG	(fhead.f_flags)
#define SFLG	(sechdr.s_flags)
#define FHSIZ	(sizeof(FILHDR))
#define SHSIZ	SCNHSZ
#define RLSIZ	RELSZ
#define SYSIZ	SYMESZ

FILE *fp;
long fnow;
FILHDR fhead;
SCNHDR sechdr;
int strip;

RELOC  relent;
SYMENT sentry;

/*************************
DOS has a nasty habit of
expanding and compressing
newlines into carriage
return/newline sequences
when reading and writing to
files. While this is usually
transparent, it affects the
operation of fseek very
detrimentally. Hence the
following poor patch....
**************************/

#ifdef 0
/* Absolute seek - third argument ignored */

FSEEK(FILE *f, unsigned long o, short t) {
	unsigned long ol=0L,r;
	char buff[512];
	
	fseek(f,0,0);
	while (r=o-ol) {
		if (r>512) r=512;
		fread(buff,r,1,f);
		ol+=r;
	}
}

#else
	#define FSEEK	fseek
#endif

/***************************************/

unsigned short all,hdrs,relcs,stabs;

main(argc,argv)
int argc;
char *argv[];
{
unsigned short argno,secno;

if (argc<2) {printf("Usage: viewcoff [-h] [-r] [-s] file\n"); exit(0); }
if (argc==2) all=1;
for (argno=1;argno<argc;argno++)
	if (argv[argno][0]=='-') switch(argv[argno][1])
		{
		case 'h' : all=0; hdrs=1; break;
		case 'r' : all=0; relcs=1; break;
		case 's' : all=0; stabs=1; break;
		default  : fprintf(stderr,"Illegal option %s\n",argv[argno]);
				break;
		}
	else if ((fp=fopen(argv[argno],"r"))==NULL)
		{
		fprintf(stderr,"Cannot open %s\n",argv[argno]);
		exit(0);
		}
	else break; /* Skip anything after file name */

printf("\n\n\n\n\n\n\n\n\n\n\n\n");
printf("COFF details of %s\n",argv[argno]);
printf("===============================================================\n\n");
print_header();
if (all | hdrs | relcs) for (secno=1;secno <= fhead.f_nscns;secno++)
	print_sec_hdr(secno);
if (all | stabs) print_symtab();
fclose(fp);
}



print_header()
{
fread(&fhead,FHSIZ,1,fp);
printf("Magic number		: %o\n",fhead.f_magic);
printf("Number of sections		: %d\n",fhead.f_nscns);
printf("Time/date stamp		: %d\n",fhead.f_timdat);
printf("Symbol table pointer	: %d\n",fhead.f_symptr);
printf("Symbol table entries	: %d\n",fhead.f_nsyms);
printf("Optional header size	: %d\n",fhead.f_opthdr);
printf("File header flags		: %d\n",FLG);
if (FLG & F_RELFLG) {printf("\tRelocation info stripped\n");strip=1;}
else strip=0;
if (FLG & F_EXEC) printf("\tFile is executable\n");
if (FLG & F_LNNO) printf("\tLine numbers stripped\n");
if (FLG & F_LSYMS) printf("\tLocal symbols stripped\n");
fnow=FHSIZ+fhead.f_opthdr;
}


print_sec_hdr(secno)
unsigned short secno;
{
unsigned short relnow;
long fnow2;
char *rt,*nm;

FSEEK(fp,fnow,0);
fread(&sechdr,SHSIZ,1,fp);
fnow+=SHSIZ;
if (all | hdrs)
	{
	printf("\n\n\nSection			: %s\n",sechdr.s_name);
	printf("=============================================\n");
	printf("Physical address	: %ld\n",sechdr.s_paddr);
	printf("Virtual address		: %ld\n",sechdr.s_vaddr);
	printf("Size			: %ld\n",sechdr.s_size);
	printf("Section pointer		: %ld\n",sechdr.s_scnptr);
	printf("Relocation pointer	: %ld\n",sechdr.s_relptr);
	printf("Relocation entries	: %d\n",sechdr.s_nreloc);
	printf("Flags			: %lx\n",SFLG);
	}

if (!strip && (sechdr.s_nreloc) && (all || relcs))
	{
	fnow2=sechdr.s_relptr;
	printf("\n\nRelocation entries for section %s:\n\n",sechdr.s_name);
	printf("Reference virtual address     Symbol table index       Relocation type\n");
	printf("=======================================================================\n\n");
	for (relnow=0;relnow < sechdr.s_nreloc; relnow++)
		{
		FSEEK(fp,fnow2,0);
		fread(&relent,RLSIZ,1,fp);
		fnow2+=RLSIZ;
		FSEEK(fp,fhead.f_symptr+SYSIZ*(relent.r_symndx),0);
		fread(&sentry,SYSIZ,1,fp);
		nm=sentry.n_name;
		if (sentry.n_zeroes==0L) nm="(long id)";
		switch (relent.r_type)
			{
			case R_ABS:	rt="Absolute";
					break;
			case R_RELBYTE:	rt="Byte relative";
					break;
			case R_RELWORD:	rt="Word relative";
					break;
			case R_RELLONG: rt="Long relative";
					break;
			case R_PCRBYTE: rt="Byte PC relative";
					break;
			case R_PCRWORD:	rt="Word PC relative";
					break;
			case R_PCRLONG: rt="Long PC relative";
					break;
			}
		printf("%10ld                    %4ld : %-8s          %s\n",relent.r_vaddr,relent.r_symndx,nm,rt);
		}
	}
}


char *class(cd)
char cd;
{
switch(cd)
	{
	case C_EFCN : return "Function end";
	case C_NULL : return "None";
	case C_AUTO : return "Auto variable";
	case C_EXT : return "Extern symbol";
	case C_STAT : return "Static";
	case C_REG : return "Reg variable";
	case C_EXTDEF : return "Extern defn";
	case C_LABEL : return "Label";
	case C_ULABEL : return "Undef label";
	case C_MOS : return "Struct member";
	case C_ARG : return "Funct argument";
	case C_STRTAG : return "Structure tag";
	case C_MOU : return "Union member";
	case C_UNTAG : return "Union tag";
	case C_TPDEF : return "Type defn";
	case C_USTATIC : return "Uninit static";
	case C_ENTAG : return "Enum tag";
	case C_MOE : return "Enum member";
	case C_REGPARM : return "Reg parameter";
	case C_FIELD : return "Bit field";
	case C_BLOCK : return "Block start/end";
	case C_FCN : return "Func start/end";
	case C_EOS : return "Structure end";
	case C_FILE : return "File name";
	case C_ALIAS : return "Duplicate tag";
	case C_HIDDEN: return "Hidden";
	default: return "Unknown";
	}
}

char *secn(cd)
short cd;
{
switch (cd)
	{
	case N_DEBUG:	return "Debug info";
	case N_ABS:	return "Abs symbol";
	case N_UNDEF:	return "Undef ext";
	case 1:		return "Text";
	case 2:		return "Data";
	case 3:		return "Bss";
	default:	return "Unknown";
	}
}


char *vtyp(sc)
unsigned short sc;
{
switch (sc)
	{
	case C_AUTO:
	case C_ARG:	return "Stack offset";
	case C_EXT:
	case C_STAT:
	case C_LABEL:
	case C_BLOCK:
	case C_FCN:
	case C_HIDDEN:	return "Reloc address";
	case C_REG:
	case C_REGPARM:	return "Reg number";
	case C_MOS:	return "Offset";
	case C_MOE:	return "Enum value";
	case C_FIELD:	return "Bit displace";
	case C_EOS:	return "Size";
	case C_ALIAS:	return "Tag index";
	case C_FILE:	return "^Next file";
	default:	return "";
	}
}

print_symtab()
{
int symnow,skipaux=1;

FSEEK(fp,fhead.f_symptr,0);
if (fhead.f_nsyms) {
	printf("\n\n\n\nSymbol Table\n============\n\n");
	printf("Name      Value    Value Type      Storage Class   Section     Auxents\n");
	printf("======================================================================\n");

	for (symnow=0;symnow<(fhead.f_nsyms);symnow+=skipaux)
		{
		while (skipaux--)
			if (fread(&sentry,SYSIZ,1,fp)==0) return;
		printf("%-10s%-9lX",sentry.n_name,sentry.n_value);
		printf("%-16s%-16s",vtyp(sentry.n_sclass),class(sentry.n_sclass));
		printf("%-12s%-2d\n",secn(sentry.n_scnum),sentry.n_numaux);
		skipaux=1+sentry.n_numaux;
		}
	}
}

