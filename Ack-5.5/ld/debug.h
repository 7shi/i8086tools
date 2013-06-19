/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Id: debug.h,v 3.3 1994/06/24 10:34:32 ceriel Exp $ */

#ifdef NDEBUG

#define debug(s, a1, a2, a3, a4)

#else
extern int DEB;

#define debug(s, a1, a2, a3, a4)	(DEB && printf(s, a1, a2, a3, a4))

#endif

extern int Verbose;
#define verbose(s, a1, a2, a3, a4)	(Verbose && do_verbose(s, a1, a2, a3, a4))
