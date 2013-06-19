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
COMMANDS = bin/ar bin/cc bin/nm bin/strip bin/crc \
	   lib/as lib/ld lib/cv

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core
	$(MAKE) $@ -C tests
	$(MAKE) $@ -C tools
	$(MAKE) $@ -C Ack-5.5

install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -cs $(TARGET) $(PREFIX)/bin
	for cmd in $(COMMANDS); do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 $$cmd; done

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) $(TARGET).exe
	for cmd in $(COMMANDS); do \
		rm -f $(PREFIX)/bin/m2`basename $$cmd`; done

depend:
	rm -f dependencies
	for cpp in $(SOURCES); do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
