include ../Makefile.inc
TARGET = a.out write.out write-libc.out $(ASMBIN)
ASMBIN = $(ASMSRC:%.asm=%.bin)
ASMSRC = test.asm idiv.asm xchg.asm write-nasm.asm

all: $(TARGET)

a.out: a.c
	m2cc -o $@ $<

.SUFFIXES: .asm .bin
.asm.bin:
	nasm -o $@ $<

.SUFFIXES: .tmp .out
.tmp.out:
	m2cv $< $@

write.tmp: write.o
	m2ld -o $@ $<
write.o: write.s
	m2as -o $@ $<

write-libc.tmp: write-libc.o
	m2ld -o $@ $< $(M2LIB)/libc.a $(M2LIB)/libe.a
write-libc.o: write-libc.s
	m2as - -o $@ $<

test: $(TARGET)
	m2run a.out
	m2run write-libc
	m2run -v write
	m2run -v test.bin
	m2run -v idiv.bin
	m2run -v xchg.bin
	m2run -v write-nasm.bin

clean:
	rm -f $(TARGET) *.o *.tmp
