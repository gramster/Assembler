#include "link.h"
#include <string.h>
#include <signal.h>


#ifdef TURBOC
#define signal ssignal
#endif

#define RISCAMAGIC	0x213C /* first 2 chars of an archive magic */
#define F_AR32WR	0400

char outname[32]={
	'a','.','o','u','t',0};
char libdir[32] ={
	'/','u','s','r','/','l','i','b',0};

char epsym[32]={
	'_','c','s','t','a','r','t',0};
char filename[32],tempname[4][12];	/* scratch/temps */

short strip=TRUE,version=1,release=0;
unsigned short redeferrs=0,undeferrs=0;

/*********************************/
/* Functions externally visible */
/*********************************/

extern void fatal();
extern void trace();
extern void ltrace();

/*************************************/
/* Functions private to this module */
/*************************************/

static void reveal();
static void showsym();
static void ldabort();
static void errorquit();
static void initialise();
static void load();

/*****************************************/

static short fileargs=FALSE; /* File arguments in command line? */

main(argc,argv)
int argc;
char *argv[];
{
	short argnow=1;

	if (argc<2)
		fatal("\nUsage: ld [-e epsym][-lxxx][-o outfl][-r][-s][-u sym][-L dir][-V] files...");

	initialise();

	for (argnow=1;argnow<argc;argnow++)
	{
		if (argv[argnow][0]=='-')		/* Process option */
			switch (argv[argnow][1])
			{
			case 'e': 
				strcpy(epsym,argv[++argnow]);
				break;
			case 'l': 
				strcpy(filename,"/usr/lib/lib");
				strcat(filename,argv[++argnow]);
				search_arc(filename);
				break;
			case 'o': 
				strcpy(outname,argv[++argnow]);
				break;
			case 'r': 
				strip=FALSE;
				break;
			case 's': 
				strip=TRUE;
				break;
			case 'u': 
				enter_udefsym(argv[++argnow]);
				break;
			case 'L': 
				strcpy(libdir,argv[++argnow]);
				break;
			case 'V': 
				printf("ld v%d.%02d\n",version,release);
				break;
			}

		else
		{						/* Process file */
			strcpy(filename,argv[argnow]);
			switch(get_fl_type(filename))
			{
			case 0:
				fprintf(stderr,"ld : cannot open %s\n",filename);
				break;
			case RISCOMAGIC:
				fileargs=TRUE;
				merge_obj();
				break;
			case RISCAMAGIC:
				search_arc();
				break;
			default:
				fprintf(stderr,"ld : %s - bad magic\n",filename);
				break;
			}
		}
	}
	if (fileargs) load();
}

/***************************
	Fatal Error Handlers
*****************************/

void fatal(msg)
char *msg;
{
	fprintf(stderr,"ld : fatal - %s\n",msg);
	killtemps();
	exit(0);
}

static void ldabort()
{
	trace("Caught a signal - terminating",0);
	killtemps();
	unlink(outname);
	fatal("terminated");
}

static void errorquit()
{
	trace("Unresolved externals or redefinitions - no output file made");
	killtemps();
}

/**********************
	Debugging
************************/

void trace(msg,arg)
char *msg,*arg;
{
#ifdef DEBUG
	fprintf(stderr,msg,arg);
	fprintf(stderr," ** trace\n");
	fflush(stderr);
#endif
}

void ltrace(msg,arg)
char *msg;
long arg;
{
#ifdef DEBUG
	fprintf(stderr,msg,arg);
	fprintf(stderr," ** trace\n");
#endif
}

static void reveal()
{
	short snow,rnow;

	if (!strip)
	{
		printf("Static symbols\n====== =======\n\n");
		for (snow=0;snow<stattop;snow++) showsym(&statstab[snow],snow);
	}
	printf("\n\n\nGlobal Symbols\n====== =======\n\n");
	for (snow=0;snow<globtop;snow++) showsym(&globstab[snow],snow);

	for (snow=N_TEXT;snow<=N_DATA;snow++)
	{
		printf("\n\n\nSection %d Global Relocations\n======= = ====== ===========\n\n",snow);
		for (rnow=0;rnow<oreltop[snow];rnow++)
			printf("Address %04ld   Symndx %04ld   Type %04d\n",outreloc[snow][rnow].r_vaddr,
			    outreloc[snow][rnow].r_symndx,outreloc[snow][rnow].r_type);
	}
}

static void showsym(s,snow)
SYMENT *s;
short snow;
{
	if (s[-1].n_numaux && (!(s[-2].n_numaux) || snow<2) && snow)
		printf("%d : %8s (aux)\n",snow,s->n_name);
		else
		printf("%d : %8s (%-6ld) (sec %d) (stcl %d)\n",snow,(s->n_zeroes?s->n_name:"(ss)")
		    ,s->n_value,s->n_scnum,s->n_sclass);
}

/***************************
	Initialisation
*****************************/

static void initialise()
{
	/*signal(SIGABRT,ldabort);*/
	signal(SIGINT,ldabort);
	/*signal(SIGTERM,ldabort);*/

	infl=outfl=tempfl[N_TEXT]=tempfl[N_DATA]=NULL;
	strcpy(tempname[N_DATA],"lddatXXXXXX");
	strcpy(tempname[N_TEXT],"ldtexXXXXXX");
	mktemp(tempname[N_TEXT]);
	mktemp(tempname[N_DATA]);

	open_tmp(N_TEXT,"w");
	open_tmp(N_DATA,"w");

	maketables();

	ofilhdr.f_magic=RISCEMAGIC;
	ofilhdr.f_nscns=3;
	time(&ofilhdr.f_timdat);
	ofilhdr.f_symptr=0L;
	ofilhdr.f_nsyms=0L;
	ofilhdr.f_opthdr=0;

	strcpy(texsechd.s_name,".text");
	strcpy(datsechd.s_name,".data");
	strcpy(bsssechd.s_name,".bss");
	texsechd.s_paddr=texsechd.s_vaddr=0L;
	datsechd.s_paddr=datsechd.s_vaddr=0L;
	texsechd.s_lnnoptr=datsechd.s_lnnoptr=bsssechd.s_lnnoptr=0L;
	texsechd.s_nlnno=datsechd.s_nlnno=bsssechd.s_nlnno=0;
	texsechd.s_flags=STYP_TEXT;
	datsechd.s_flags=STYP_DATA;
	bsssechd.s_flags=STYP_BSS;

	globalpc[N_TEXT]=globalpc[N_DATA]=globalpc[N_BSS]=0;
}


/*******************************************/

static void load()
{
	trace("Starting load");
	fclose(tempfl[N_TEXT]);
	fclose(tempfl[N_DATA]);
#ifdef DEBUG
	trace("Calling reveal to show stab");
	reveal();
#endif

	if ((showundefs() && strip) || redeferrs) errorquit();
	else makecoffile();
}

