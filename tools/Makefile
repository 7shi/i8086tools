TARGET = pe2aout aout-tool
PREFIX = /usr/local
MINGW  = #i686-mingw32-
CFLAGS = -g
CROSSLDFLAGS = -s -Ttext=0 --image-base=0 --section-alignment=0 --file-alignment=0

all: $(TARGET)

pe2aout: pe2aout.c pe.h a.out.h
	$(CC) $(CFLAGS) -o $@ -s $<

aout-tool: aout-tool.c a.out.h
	$(CC) $(CFLAGS) -o $@ -s $<

test: exit-32 write-16
	./aout-tool $^

exit-32: exit-32.exe $(TARGET)
	./pe2aout $<

write-16: write-16.exe $(TARGET)
	./pe2aout -16 $<

exit-32.exe: exit-32.o
write-16.exe: write-16.o

exit-32.exe write-16.exe:
	$(MINGW)ld -o $@ $(CROSSLDFLAGS) $<

exit-32.o: exit-32.c
	$(MINGW)gcc -c -o $@ -masm=intel $<

write-16.o: write-16.s
	$(MINGW)as -o $@ $<

install: $(TARGET)
	install -cs $(TARGET) $(PREFIX)/bin

clean:
	rm -f $(TARGET) *.exe *.o exit-32 write-16
