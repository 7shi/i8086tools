include ../Makefile.inc
TARGET = nm strip

all: $(TARGET)

nm: nm.c
	v6cc $<
	mv a.out $@
	v6strip $@

strip: strip.s
	v6as $<
	mv a.out $@
	v6strip $@

install: $(TARGET)
	install -c $(TARGET) $(V6BIN)

clean:
	rm -f $(TARGET)
