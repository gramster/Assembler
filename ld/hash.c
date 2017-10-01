
#include "link.h"

#define TRUE		1
#define FALSE		0

typedef char *POINTER;
typedef struct entry {
	POINTER key,data;
} ENTRY;

#define SHIFT ((sizeof(char) * sizeof(int)) - m) /* Shift factor */
#define FACTOR 035761254233	/* Magic multiplication factor */

#define NUMTABS	10		/* Up to 10 tables	*/

static ENTRY *table[NUMTABS];	/* The tables */
static unsigned int count[NUMTABS] = {
	0};	/* Number of entries in tables */
static unsigned int length[NUMTABS];	/* Size of tables */
static unsigned int m[NUMTABS];		/* Log base 2 of length */

extern void free();
extern char *malloc(), *calloc(), *strcpy();

int makehash();
void destroyhash();
POINTER findhash();
void enterhash();

/*****************************************************
	Basic Functions
*******************************************************/

static unsigned int
crunch(key)		/* Convert multicharacter key to unsigned int */
POINTER key;
{
	unsigned int sum = 0;	/* Results */
	int s;			/* Length of the key */

	for(s = 0; *key; s++)	/* Simply add up the bytes */
		sum += *key++;

	return(sum + s);
}


/***************************************************************
    NOTE: The following algorithm only works on machines where
    the results of multiplying two integers is the least
    significant part of the double word integer required to hold
    the result.  It is adapted from Knuth, Volume 3, section 6.4.
*****************************************************************/

static unsigned hash(key,m)		/* Multiplication hashing scheme */
unsigned int m;
POINTER key;		/* Key to be hashed */
{
	return((int) (((unsigned) (crunch(key) * FACTOR)) >> SHIFT));
}

/****************************************************************
 * Secondary hashing, for use with multiplicative hashing scheme.
 * Adapted from Knuth, Volume 3, section 6.4.
 *****************************************************************/

static unsigned int rehash(key,m)	/* Secondary hashing routine */
unsigned int m;
POINTER key;		/* String to be hashed */
{
	return((int) (((unsigned) ((crunch(key) * FACTOR) << m) >> SHIFT) | 1));
}

/*****************************************************/
/* Hash search of a fixed-capacity table.  Open addressing used to
   resolve collisions.  Algorithm modified from Knuth, Volume 3,
   section 6.4, algorithm D.  Labels flag corresponding actions.
*/

static unsigned int hashindex(tablnum,key)
int tablnum;
POINTER key;
{
	ENTRY *Table=table[tablnum]; /* Hash table base */
	unsigned int i;	/* Insertion index */
	unsigned int c;	/* Secondary probe displacement */

	i = hash(key,m[tablnum]);	/* Primary hash on key */
	if ((Table[i].key == NULL)||(strcmp(Table[i].key,key)==0)) return i;

	c = rehash(key,m[tablnum]);	/* No match => compute secondary hash */
	while (1)
	{
		i = (i + c) % length[tablnum];	/* Advance to next slot */
		if ((Table[i].key == NULL)||(strcmp(Table[i].key, key)==0)) return i;
	}
}

/******************************************
	External Functions
********************************************/

makehash(tablnum,size)		/* Create a hash table no smaller than size */
int tablnum;
int size;		/* Minimum size for hash table */
{
	unsigned int unsize;	/* Holds the shifted size */
	unsigned int ll,mm;

	if(size<=0) return(FALSE);

	unsize = size;	/* +1 for empty table slot; -1 for ceiling */
	ll = 1;		/* Maximum entries in table */
	mm = 0;		/* Log2 length */
	while(unsize) {
		unsize >>= 1;
		ll <<= 1;
		mm++;
	}
	length[tablnum]=ll;
	m[tablnum]=mm;
	return ((table[tablnum] = (ENTRY *) calloc(ll, sizeof(ENTRY)))!=NULL);
}

void destroyhash(tablnum)	/* Reset the module to its initial state */
int tablnum;
{
	free((POINTER) table[tablnum]);
	count[tablnum] = 0;
}


POINTER findhash(tablnum,key)	/* Find item in table */
int tablnum;
POINTER key;
{
	ENTRY *Table=table[tablnum];
	unsigned int i=hashindex(tablnum,key);	/* Insertion index */

	if(Table[i].key == NULL) return (ENTRY *)NULL;
	else return(Table[i].data);
}


void enterhash(tablnum,key,data)	/* Insert item into table */
int tablnum;
POINTER key,data;
{
	ENTRY *Table=table[tablnum];
	unsigned int i=hashindex(tablnum,key);	/* Insertion index */

	if ((Table[i].key==NULL) && (count[tablnum] < (length[tablnum] - 1)))
	{
		count[tablnum]++;		/* Increment table occupancy count */
		Table[i].key=key;
		Table[i].data=data;
	}
}

