TARGET = i8086tools
TEST   = test.bin

all: $(TARGET)

$(TARGET): utils.o disasm.o VM.o main.o
	g++ -o $@ $^

utils.o: utils.cpp utils.h
disasm.o: disasm.cpp disasm.h utils.h
VM.o: VM.cpp VM.h disasm.h utils.h
main.o: main.cpp VM.h disasm.h utils.h

$(TEST): test.asm
	nasm -o $@ $<

test: $(TEST) $(TARGET)
	./$(TARGET) -v $(TEST)

clean:
	rm -f $(TARGET) *.o $(TEST)
