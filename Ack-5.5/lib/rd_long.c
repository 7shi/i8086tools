/* $Id: rd_long.c,v 1.5 1994/06/24 11:19:04 ceriel Exp $ */
/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include "obj.h"

long
rd_long(fd)
{
	char buf[4];

	rd_bytes(fd, buf, 4L);
	return get4(buf);
}
