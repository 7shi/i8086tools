TARGET = pe2aout aout-tool
PREFIX = /usr/local
MINGW  = gcc
CFLAGS = -g

all: $(TARGET)

pe2aout: pe2aout.c pe.h a.out.h
	$(CC) $(CFLAGS) -o $@ -s $<

aout-tool: aout-tool.c a.out.h
	$(CC) $(CFLAGS) -o $@ -s $<

test: test.exe $(TARGET)
	./pe2aout $<
	./aout-tool $@

test.exe: test.c
	$(MINGW) -o $@ -masm=intel -s -Ttext=0 -Wl,--image-base=0 -Wl,--section-alignment=0x200 -nostdinc -nostdlib $<

install: $(TARGET)
	install -cs $(TARGET) $(PREFIX)/bin

clean:
	rm -f $(TARGET) *.exe test test.out test.o
