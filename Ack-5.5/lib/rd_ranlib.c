/* $Id: rd_ranlib.c,v 1.6 1994/06/24 11:19:07 ceriel Exp $ */
/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include "obj.h"

void
rd_ranlib(fd, ran, cnt)
	register struct ranlib	*ran;
	register long	cnt;
{
	rd_bytes(fd, (char *) ran, cnt * SZ_RAN);
#if BYTE_ORDER == 0x0123
	if (sizeof (struct ranlib) != SZ_RAN)
#endif
	{
		register char *c = (char *) ran + cnt * SZ_RAN;

		ran += cnt;
		while (cnt--) {
			ran--;
			c -= 4; ran->ran_pos = get4(c);
			c -= 4; ran->ran_off = get4(c);
		}
	}
}
