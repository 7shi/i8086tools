TARGET = a.out write write-libc $(ASMBIN)
ASMBIN = $(ASMSRC:%.asm=%.bin)
ASMSRC = test.asm idiv.asm xchg.asm write-nasm.asm

all: $(TARGET)

a.out: a.c
	m2cc -o $@ $<

.SUFFIXES: .asm .bin
.asm.bin:
	nasm -o $@ $<

write: write.out
	m2cv $< $@
write.out: write.o
	m2ld -o $@ $<
write.o: write.s
	m2as -o $@ $<

write-libc: write-libc.s
	m2cc -o $@ $<

test: $(TARGET)
	m2run a.out
	m2run write-libc
	m2run -v write
	m2run -v test.bin
	m2run -v idiv.bin
	m2run -v xchg.bin
	m2run -v write-nasm.bin

clean:
	rm -f $(TARGET) *.out *.o
