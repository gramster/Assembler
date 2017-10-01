/*
 *	%Z% %M% %I% %D% %Q%
 *
 *	Copyright (C) The Santa Cruz Operation, 1985
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 *
 */

/*	@(#)search.h	1.1	*/

/* HSEARCH(3C) */
typedef struct entry { char *key, *data; } ENTRY;
typedef enum { FIND, ENTER } ACTION;

/* TSEARCH(3C) */
typedef enum { preorder, postorder, endorder, leaf } VISIT;
