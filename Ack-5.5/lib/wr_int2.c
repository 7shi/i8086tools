/* $Id: wr_int2.c,v 1.5 1994/06/24 11:19:22 ceriel Exp $ */
/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include "obj.h"

void
wr_int2(fd, i)
{
	char buf[2];

	put2(i, buf);
	wr_bytes(fd, buf, 2L);
}
