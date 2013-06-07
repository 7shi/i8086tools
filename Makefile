TARGET = i8086tools

all: $(TARGET)

$(TARGET): utils.o disasm.o main.o
	g++ -o $@ $^

utils.o: utils.cpp utils.h
disasm.o: disasm.cpp disasm.h utils.h
main.o: main.cpp disasm.h utils.h

clean:
	rm -f $(TARGET) *.o
