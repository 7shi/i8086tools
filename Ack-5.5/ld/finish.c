/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#ifndef lint
static char rcsid[] = "$Id: finish.c,v 3.9 1994/06/24 10:34:44 ceriel Exp $";
#endif

#include <out.h>
#include "const.h"
#include "defs.h"
#include "memory.h"
#include "orig.h"
#include "scan.h"

extern bool	incore;
extern unsigned short	NLocals;
extern int	flagword;
extern struct outname	*searchname();

static		adjust_names();
static		handle_relos();
static		put_locals();
static		compute_origins();

/*
 * We know all there is to know about the current module.
 * Now we relocate the values in the emitted bytes and write
 * those to the final output file. Then we compute the relative origins
 * for the next module.
 */
finish()
{
	struct outhead	*head;
	struct outsect	*sects;
	struct outname	*names;
	char		*chars;

	get_modul();
	head = (struct outhead *)modulptr(IND_HEAD);
	sects = (struct outsect *)modulptr(IND_SECT(*head));
	names = (struct outname *)modulptr(IND_NAME(*head));
	chars = (char *)modulptr(IND_CHAR(*head));
	adjust_names(names, head, chars);
	handle_relos(head, sects, names);
	if (!incore && !(flagword & SFLAG)) {
		put_locals(names, head->oh_nname);
#ifdef SYMDBUG
		put_dbug(OFF_DBUG(*head));
#endif /* SYMDBUG */
	}
	compute_origins(sects, head->oh_nsect);
	skip_modul(head);
}

/*
 * Adjust all local names for the move into core.
 */
static
adjust_names(name, head, chars)
	register struct outname	*name;
	struct outhead		*head;
	register char		*chars;
{
	register int		cnt;
	register long		charoff;
	struct outname		*base = name;

	cnt = head->oh_nname;
	charoff = OFF_CHAR(*head);
	while (cnt--) {
		if (name->on_foff != (long)0)
			name->on_mptr = chars + (ind_t)(name->on_foff - charoff);
		name++;
	}
	if (! incore) {
		do_crs(base, head->oh_nname);
	}
}

do_crs(base, count)
	struct outname	*base;
	unsigned	count;
{
	register struct outname	*name = base;

	while (count--) {
		if ((name->on_type & S_TYP) == S_CRS) {
			char *s;
			struct outname *p;

			s = address(ALLOGCHR, (ind_t) name->on_valu);
			p = searchname(s, hash(s));

			if (flagword & RFLAG) {
				name->on_valu = NLocals + (p -
					(struct outname *)
					  address(ALLOGLOB, (ind_t) 0));
			}
			else {
				name->on_valu = p->on_valu;
				name->on_type &= ~S_TYP;
				name->on_type |= (p->on_type & S_TYP);
			}
		}
		name++;
	}
}

/*
 * If all sections are in core, we can access them randomly, so we need only
 * scan the relocation table once. Otherwise we must for each section scan
 * the relocation table again, because the relocation entries of one section
 * need not be consecutive.
 */
static
handle_relos(head, sects, names)
	struct outhead		*head;
	struct outsect		*sects;
	struct outname		*names;
{
	register struct outrelo	*relo;
	register int		sectindex;
	register int		nrelo;
	register char		*emit;
	extern char		*getemit();
	extern struct outrelo	*nextrelo();
	static long zeros[MAXSECT];

	if (incore) {
		nrelo = head->oh_nrelo; sectindex = -1;
		startrelo(head); relo = nextrelo();
		while (nrelo--) {
			if (sectindex != relo->or_sect - S_MIN) {
				sectindex = relo->or_sect - S_MIN;
				emit = getemit(head, sects, sectindex);
			}
			relocate(head, emit, names, relo, 0L);
			relo++;
		}
	} else {
		for (sectindex = 0; sectindex < head->oh_nsect; sectindex++) {
			if (sects[sectindex].os_flen) {
				wrt_nulls(sectindex, zeros[sectindex]);
				zeros[sectindex] = 0;
				emit = getemit(head, sects, sectindex);
				if (emit) {
				    nrelo = head->oh_nrelo; startrelo(head);
				    while (nrelo--) {
					relo = nextrelo();
					if (relo->or_sect - S_MIN == sectindex) {
						relocate(head,emit,names,relo,0L);
						/*
						 * Write out the (probably changed)
						 * relocation information.
						 */
						if (flagword & (RFLAG|CFLAG))
							wr_relo(relo, 1);
					}
				    }
				    wrt_emit(emit, sectindex,
					sects[sectindex].os_flen);
				}
				else {
				    long sz = sects[sectindex].os_flen;
				    long sf = 0;
				    long blksz;
				    char *getblk();

				    emit = getblk(sz, &blksz, sectindex);
				    while (sz) {
					long sz2 = sz > blksz ? blksz : sz;

					rd_emit(emit, sz2);
				    	nrelo = head->oh_nrelo; startrelo(head);
				    	while (nrelo--) {
					    relo = nextrelo();
					    if (relo->or_sect-S_MIN==sectindex
						&&
						relo->or_addr >= sf
						&&
						relo->or_addr < sf + sz2){
						relocate(head,emit,names,relo,
							 sf);
						/*
						 * Write out the (probably changed)
						 * relocation information.
						 */
						if (flagword & (RFLAG|CFLAG))
							wr_relo(relo, 1);
					    }
					}
				        wrt_emit(emit, sectindex, sz2);
					sz -= sz2;
					sf += sz2;
				    }
				}
				endemit(emit);
			}
			zeros[sectindex] += sects[sectindex].os_size -
					    sects[sectindex].os_flen;
		}
	}
}

/*
 * Write out the local names that must be saved.
 */
static
put_locals(name, nnames)
	struct outname		*name;
	register unsigned	nnames;
{
	register struct outname *oname = name;
	register struct outname *iname = oname;

	while (nnames--) {
		if ((iname->on_type & S_EXT) == 0 && mustsavelocal(iname)) {
			namerelocate(iname);
			addbase(iname);
			wrt_name(iname, 0);
			*oname++ = *iname;
		}
		iname++;
	}
	wr_name(name, (unsigned int) (oname - name));
}

/*
 * Add all flen's and all (size - flen == zero)'s of preceding sections
 * with the same number.
 */
static
compute_origins(sect, nsect)
	register struct outsect	*sect;
	register unsigned	nsect;
{
	extern struct orig	relorig[];
	register struct orig	*orig = relorig;

	while (nsect--) {

		orig->org_size += sect->os_size;
		orig++; sect++;
	}
}
#ifdef SYMDBUG

/*
 * Write out what is after the string area. This is likely to be
 * debugging information.
 */
static
put_dbug(offdbug)
	long		offdbug;
{
	char		buf[512];
	register int	nbytes;
	register long	dbugsize;
	extern long	objectsize;

	dbugsize = objectsize - offdbug;
	while (dbugsize) {
		nbytes = dbugsize > 512 ? 512 : dbugsize;
		rd_dbug(buf, (long)nbytes);
		wr_dbug(buf, (long) nbytes);
		dbugsize -= nbytes;
	}
}
#endif /* SYMDBUG */
