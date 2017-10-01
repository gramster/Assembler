#include "link.h"

#define MINIMISE(a,b)	((a)=(a)<(b) ? (a) : (b))

#define F_AR32WR	0400

FILE *infl,*tempfl[4],*outfl;

FILHDR tfilhdr,ofilhdr; /* temp & output file headers */

SCNHDR	tsechdr,texsechd,datsechd,bsssechd;	/* temp & 3 output section hdrs */

unsigned long globalpc[4]={
	0L,0L,0L,0L};

unsigned long base=0;	/* Relative base for file offsets (to allow archives) */
unsigned long offset=0; /* Offset into current object file */

/*********************************/
/* Externally visible functions */
/*********************************/

extern void open_tmp    (  /* fp,name     */ );
extern void killtemps   ();
extern int  get_fl_type (	/* file_name   */ );
extern void readfilhdr  ();
extern void readstab    ();
extern void appendsecn  (  /* secnum      */ );
extern void makecoffile ();

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
***************************/

#ifdef DOS
/* Absolute seek - third argument ignored */

FSEEK(f,o,t)
FILE *f;
unsigned long o;
{
	unsigned long ol=0L,r;
	char buff[512];

	fseek(f,0,0);
	while (r=o-ol)
	{
		if (r>512) r=512;
		fread(buff,r,1,f);
		ol+=r;
	}
}

#else
#define FSEEK	fseek
#endif

/***************************
	File Handling
*****************************/

void open_tmp(secn,fmode)
char *fmode;
{
	if ((tempfl[secn]=fopen(tempname[secn],fmode))==NULL)
		fatal("cannot open temp file");
}



void killtemps()
{
	unlink(tempname[N_TEXT]);
	unlink(tempname[N_DATA]);
}


int get_fl_type(name)
char *name;
{
	unsigned short magic;

	if ((infl=fopen(name,"r"))==NULL) return 0;
	fread(&magic,2,1,infl);
	return magic;
}

void readfilhdr()
{
	FSEEK(infl,base,0);
	fread(&tfilhdr,FILHSZ,1,infl);
}



void readstab()
{
	unsigned int symnum;
	unsigned long storesize;

	FSEEK(infl,base+tfilhdr.f_symptr,0);
	for (symnum=0;symnum<tfilhdr.f_nsyms;symnum++)
	{
		short defsec;
		fread(&tmpstab[symnum],sizeof(SYMENT),1,infl);
		if ((defsec=tmpstab[symnum].n_scnum)>0)
			tmpstab[symnum].n_value+=globalpc[defsec];
	}
	tmptop=symnum;
#ifdef FLEXNAMES
	fread(&storesize,4,1,infl);
	trace("Reading %d bytes of stringstore",storesize-4);
	fread(tstringstore,1,storesize-4,infl);
#endif
}

/*******************************/

void appendsecn(secnum)
unsigned short secnum;
{
	FSEEK(infl,base+offset,0);
	fread(&tsechdr,sizeof(SCNHDR),1,infl);
	offset+=SCNHSZ; /* Position of next section header */

	/* Read and do relocation entries */
	FSEEK(infl,base+tsechdr.s_relptr,0);
	fread(tmpreloc,RELSZ,tsechdr.s_nreloc,infl);

	FSEEK(infl,base+tsechdr.s_scnptr,0); /* Seek to section data */

	relocsecn(globalpc[secnum],secnum,tsechdr.s_size,tsechdr.s_nreloc,tmpreloc,infl,tempfl[secnum],tmpstab,1);

	/* Update global pc's */
	globalpc[secnum]+=tsechdr.s_size;
}

/*************************************/


void makecoffile()
{
	long rawdatastart,relocstart,stabstart;
	short i,s;

	rawdatastart=sizeof(FILHDR)/*opthdr+*/+3*SCNHSZ;
	relocstart=rawdatastart+globalpc[N_TEXT]+globalpc[N_DATA];
	stabstart=relocstart;
	if (!strip) stabstart+=(oreltop[N_TEXT]+oreltop[N_DATA])*RELSZ;

	if ((outfl=fopen(outname,"w"))==NULL) fatal("ld : cannot open output file");

	if (!strip)
	{
		ofilhdr.f_magic=RISCOMAGIC;
		ofilhdr.f_symptr=stabstart;
		ofilhdr.f_nsyms=(unsigned long)stattop+globtop;
	}
	ofilhdr.f_opthdr=0; /* change later */
	ofilhdr.f_flags=F_LNNO+F_AR32WR+(undeferrs ? 0 : F_EXEC)
	    +(strip ? (F_RELFLG+F_LSYMS) : 0);

	datsechd.s_paddr=datsechd.s_vaddr=globalpc[N_TEXT];
	bsssechd.s_paddr=bsssechd.s_vaddr=globalpc[N_TEXT]+globalpc[N_DATA];

	texsechd.s_size=globalpc[N_TEXT];
	datsechd.s_size=globalpc[N_DATA];
	bsssechd.s_size=globalpc[N_BSS ];

	texsechd.s_scnptr=globalpc[N_TEXT] ? rawdatastart : 0L;
	datsechd.s_scnptr=globalpc[N_DATA] ? rawdatastart+globalpc[N_TEXT] : 0L;
	bsssechd.s_scnptr=0L;  /* Correct? */

	if (!strip)
	{
		if (oreltop[N_TEXT])
		{
			texsechd.s_nreloc=oreltop[N_TEXT];
			texsechd.s_relptr=relocstart;
		}
		if (oreltop[N_DATA])
		{
			datsechd.s_nreloc=oreltop[N_DATA];
			datsechd.s_relptr=relocstart+RELSZ*oreltop[N_TEXT];
		}
	}

	fwrite(&ofilhdr,sizeof(ofilhdr),1,outfl);

	/* write optional header here */

	fwrite(&texsechd,SCNHSZ,1,outfl);
	fwrite(&datsechd,SCNHSZ,1,outfl);
	fwrite(&bsssechd,SCNHSZ,1,outfl);

	/**** Reopen temps, relocate (then add stattop to reloc symndx) and write */
	writetemp(N_TEXT);
	writetemp(N_DATA);


	if (!strip)
	{
		for (s=N_TEXT;s<=N_DATA;s++)
		{
			trace("Writing %d reloc entries",oreltop[s]);
			/*@@@@@@@ just for now, correct here */
			for (i=0;i<oreltop[s];i++)
			{
				ltrace("Old global symndx was %ld",outreloc[s][i].r_symndx);
				outreloc[s][i].r_symndx+=stattop;
				ltrace("New symndx is %ld",outreloc[s][i].r_symndx);
			}
			trace("Writing reloc table for section %d",s);
			if (oreltop[s]) fwrite(&outreloc[s][0],RELSZ,oreltop[s],outfl);
		}
		trace("Writing symbol tables");
		fwrite(statstab,stattop,SYMESZ,outfl);
		fwrite(globstab,globtop,SYMESZ,outfl);
#ifdef FLEXNAMES
		*((long *)stringstore)=(unsigned long)stortop;
		fwrite(stringstore,stortop,1,outfl);
#endif
	}
	fclose(outfl);
	killtemps();
}

/***************************/

writetemp(sec)
int sec;
{
	RELOC *reloc;

	open_tmp(sec,"r");

	relocsecn(0L,sec,globalpc[sec],oreltop[sec],outreloc[sec],tempfl[sec],outfl,globstab,0);

	fclose(tempfl[sec]);
}


relocsecn(base,secnum,secsiz,numrels,reltab,infile,outfile,stab,copyflag)
long base,secsiz;
unsigned short secnum,numrels,copyflag;
RELOC *reltab;
FILE *infile,*outfile;
SYMENT *stab;
{
	char buff[512];
	unsigned long secnpc=0;
	unsigned short readsize,nextrel=0,relnow=0;

	while (secsiz)
	{
		readsize=512;
		MINIMISE(readsize,secsiz);
		if (nextrel<numrels)
			MINIMISE(readsize,reltab[nextrel].r_vaddr-secnpc);
		trace("Reading %d bytes into buffer",readsize);
		fread(buff,1,readsize,infile);
		while (relnow < numrels && reltab[relnow].r_vaddr==secnpc)
		{
			SYMENT *s;

			s=&stab[reltab[relnow].r_symndx];
			trace("Reloc reference is to %s",s->n_name);
			if (s -> n_scnum > 0)
				relocate(base+secnpc,secnum,reltab[relnow].r_type,s,buff);
			else if (copyflag)
			{
				/* Copy to global relocation table outreloc */
				RELOC *from=&tmpreloc[relnow],
				    *to=&outreloc[secnum][oreltop[secnum]++];
				trace("non-local relocation - copying to global relocs");
				trace("old vaddr is %d",(short)(from->r_vaddr));
				to->r_vaddr=from->r_vaddr+base;
				ltrace("new vaddr is %ld",to->r_vaddr);
				to->r_type=from->r_type;
				to->r_symndx=(long) ( (SYMENT *)findhash(HASHEXT,s->n_name)
				    -globstab);
				ltrace("new symndx is %ld",to->r_symndx);
			}
			relnow++;
		}
		nextrel=relnow+1;
		while (nextrel<numrels &&
		    reltab[nextrel].r_vaddr==reltab[relnow].r_vaddr)
			nextrel++;
		secnpc+=readsize;
		secsiz-=readsize;
		trace("Writing %d bytes to file",readsize);
		fwrite(buff,readsize,1,outfile);
	}
	/********************************
	** for a very weird effect, try
	
	fflush(outfile);
	
	** here - on XENIX it causes a 
	** segmentation violation; under
	** DOS, an extra byte 0x0D is inserted
	** somewhere in the section!!!
	**********************************/
}


/*****************@@@@@@@@@@@@@@@@@@@@@@@

	What happens if we keep relocation entries? Is reloc still done?
	If so, how do we know in a future ld run that the already done
	reloc entries must be ignored?

**************@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

