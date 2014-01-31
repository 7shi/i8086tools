include ../Makefile.inc

all:

install:
	tar cvf - v6root 8086v6-ack | tar xf - -C $(PREFIX)

uninstall:
	for f in `find v6root -type f`; do rm -f $(PREFIX)/$$f; done
	for f in `find 8086v6-ack -type f`; do rm -f $(PREFIX)/$$f; done
	-rmdir $(V6BIN) $(V6LIB) $(V6ROOT)
	-rmdir $(ACKBIN) $(ACKLIB) $(ACKROOT)
