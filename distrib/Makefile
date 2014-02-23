include ../Makefile.inc

all:

install:
	tar cvf - v6root 8086v6-ack | tar xf - -C $(PREFIX)
	cd $(PREFIX)/v6root/bin && chmod 755 ar as cc ld nm strip
	cd $(PREFIX)/v6root/lib && chmod 755 as2 c0 c1 c2
	cd $(PREFIX)/8086v6-ack/bin && chmod 755 cc
	cd $(PREFIX)/8086v6-ack/lib && chmod 755 as cv irrel ld ncem ncg ncpp nopt

uninstall:
	for f in `find v6root -type f`; do rm -f $(PREFIX)/$$f; done
	for f in `find 8086v6-ack -type f`; do rm -f $(PREFIX)/$$f; done
	-rmdir $(V6BIN) $(V6LIB) $(V6ROOT)
	-rmdir $(ACKBIN) $(ACKLIB) $(ACKROOT)
