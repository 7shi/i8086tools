/* $Id: rd_int2.c,v 1.5 1994/06/24 11:19:02 ceriel Exp $ */
/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include "obj.h"

int
rd_int2(fd)
{
	char buf[2];

	rd_bytes(fd, buf, 2L);
	return get2(buf);
}
