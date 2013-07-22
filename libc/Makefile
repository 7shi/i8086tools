include ../Makefile.inc
TARGET = crt0.o libc.a

all: $(TARGET)

crt0.o: crt0.s
	8086v6-cc -c $<

libc.a: cret.o
	m2aal r $@ $^

cret.o: cret.s
	m2cc -c $<

install: $(TARGET)
	install -c $(TARGET) $(ACKLIB)

clean:
	rm -f *.o $(TARGET)
