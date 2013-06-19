TARGET   = m2run
PREFIX   = /usr/local
M2ROOT   = $(PREFIX)/minix2
CXX      = g++
CXXFLAGS = -Wall -O2 -g
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

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core
	$(MAKE) $@ -C tests

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -cs $(TARGET) $(PREFIX)/bin
	for cmd in bin/ar lib/as bin/cc lib/ld bin/nm bin/strip; do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 $$cmd; done

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) $(TARGET).exe v6ar v6as v6cc v6ld v6nm v6strip

depend:
	rm -f dependencies
	for cpp in $(SOURCES); do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
