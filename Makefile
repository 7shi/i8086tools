TARGET = i8086tools
TEST   = test.bin

all: $(TARGET)

$(TARGET): utils.o disasm.o main.o
	g++ -o $@ $^

utils.o: utils.cpp utils.h
disasm.o: disasm.cpp disasm.h utils.h
main.o: main.cpp disasm.h utils.h

$(TEST): test.asm
	nasm -o $@ $<

test: $(TEST) $(TARGET)
	./$(TARGET) $(TEST)

clean:
	rm -f $(TARGET) *.o $(TEST)
