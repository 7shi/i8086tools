TARGET = a.out $(ASMBIN)
ASMBIN = $(ASMSRC:%.asm=%.bin)
ASMSRC = test.asm idiv.asm xchg.asm

all: $(TARGET)

a.out: a.c
	m2cc -o $@ $<

.SUFFIXES: .asm .bin
.asm.bin:
	nasm -o $@ $<

test: $(TARGET)
	m2run a.out
	m2run -v test.bin
	m2run -v idiv.bin
	m2run -v xchg.bin

clean:
	rm -f $(TARGET)
