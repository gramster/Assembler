#include "link.h"

/* Externally visible functions */

extern void search_arc();
extern void merge_obj();
extern void relocate();
/**********************
	Archive Search
************************/

void search_arc()
{
	trace("Searching library archive");
	fclose(infl);
}

/*************************
	Object File Relocate
**************************/

void merge_obj()
{
	short secnum;

	base=0L; /* No 'archive' offset */
	readfilhdr();
	offset=FILHSZ+tfilhdr.f_opthdr; /* Offset of first section header */

	readstab();
	mergestabs();

	for (secnum=1;secnum<=tfilhdr.f_nscns;secnum++)
		appendsecn(secnum);
	fclose(infl);
}


#define TSHRTMSKL ((unsigned long) 0x3FFFL)
#define TSHRTMSKU ((unsigned long) 0xFFFFC000L)
#define TLNGMSKL  ((unsigned long) 0x7FFFFL)
#define TLNGMSKU  ((unsigned long) 0xFFF80000L)

#define LONGEDIT(l,u)	((TLNGMSKL &  (l)) | (TLNGMSKU  & (u)))
#define SHORTEDIT(l,u)	((TSHRTMSKL & (l)) | (TSHRTMSKU & (u)))

void relocate(pc,secn,type,stabentry,data)
unsigned long pc;
unsigned short secn,type;
SYMENT *stabentry;
char *data;
{
	unsigned long val=stabentry->n_value,
	*ldata=((unsigned long  *)data),
	lval=*ldata;
	unsigned short *wdata=((unsigned short *)data);

	trace("RELOCATE at vaddr %d",(short)pc);
	trace("reference to %s",stabentry->n_name);
	trace("defined in section %d",(short)stabentry->n_scnum);
	trace("with value %d",(short)val);

	switch(secn)
	{
	case N_TEXT:
		switch(type)
		{
		case R_RELWORD:
			ltrace("relocate: txt relwrd : %08lX",*ldata);
			*ldata = SHORTEDIT(val+lval,lval);
			ltrace("changed to %08lX",*ldata);
			break;
		case R_RELLONG:
			ltrace("relocate: txt rellng : %08lX",*ldata);
			*ldata = LONGEDIT(val+lval,lval);
			ltrace("changed to %08lX",*ldata);
			break;
		case R_PCRWORD:
			ltrace("relocate: txt pcrwrd : %08lX",*ldata);
			*ldata=SHORTEDIT(val+lval-pc-8,lval);
			ltrace("changed to %08lX",*ldata);
			break;
		case R_PCRLONG:
			ltrace("relocate: txt pcrlng : %08lX",*ldata);
			*ldata=LONGEDIT(val+lval-pc-8,lval);
			ltrace("changed to %08lX",*ldata);
			break;
		default:
			fprintf(stderr,"ld : bad relocation type in text section\n");
			break;
		};
		break;
	default: 
		switch(type)
		{
		case R_RELBYTE:
			trace("relocate: dat relbyt : %02X",*data);
			*data +=(char )val;
			trace("changed to %02X",*data);
			break;
		case R_RELWORD:
			trace("relocate: dat relwrd : %04X",*wdata);
			*wdata+=(short)val;
			trace("changed to %04X",*wdata);
			break;
		case R_RELLONG:
			ltrace("relocate: dat rellng : %08lX",*ldata);
			*ldata+=(long )val;
			ltrace("changed to %08lX",*ldata);
			break;
		default:
			fprintf(stderr,"ld : bad relocation type in data section\n");
			break;
		};
		break;
	}
}
	
