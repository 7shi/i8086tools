TARGET   = i8086tools
TEST     = test.bin
CXX      = g++
CXXFLAGS = -Wall -g -O2
LDFLAGS  =
OBJECTS  = $(SOURCES:%.cpp=%.o)
SOURCES  = main.cpp utils.cpp disasm.cpp \
	   OpCode.cpp Operand.cpp File.cpp \
	   VM.cpp VM.inst.cpp VM.sys.cpp VM.signal.cpp

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

$(TEST): test.asm
	nasm -o $@ $<

test: $(TEST) $(TARGET)
	./$(TARGET) -v $(TEST)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core $(TEST)

depend:
	rm -f dependencies
	for cpp in $(SOURCES); do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
