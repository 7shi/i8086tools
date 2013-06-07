TARGET = i8086tools

all: $(TARGET)

$(TARGET): disasm.o main.o
	g++ -o $@ $^

disasm.o: disasm.cpp disasm.h
main.o: main.cpp disasm.h

clean:
	rm -f $(TARGET) *.o
