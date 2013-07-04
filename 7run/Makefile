include ../Makefile.inc
TARGET   = 7run
CXX      = g++
CXXFLAGS = -Wall -O2 -g
LDFLAGS  =
OBJECTS  = $(SOURCES:%.cpp=%.o)
SOURCES  = main.cpp utils.cpp \
	   File.cpp VMUnix.cpp VMUnix.sys.cpp \
	   i8086/OpCode.cpp i8086/Operand.cpp \
	   i8086/VM.cpp i8086/VM.inst.cpp i8086/disasm.cpp \
	   VMMinix2.cpp VMMinix2.signal.cpp \
	   PDP11/VM.cpp
COMMANDS = bin/aal bin/ar bin/cc bin/nm bin/strip \
	   bin/crc bin/install bin/m2 \
	   lib/as lib/ld lib/cv

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core

install: $(TARGET)
	mkdir -p $(PREFIX)/bin $(M2ROOT)/m2bin
	install -cs $(TARGET) $(PREFIX)/bin
	for cmd in $(COMMANDS); do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 $$cmd; \
	  cp $(PREFIX)/bin/m2`basename $$cmd` $(M2ROOT)/m2bin/`basename $$cmd`; \
	done

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) $(TARGET).exe
	for cmd in $(COMMANDS); do \
	  rm -f $(PREFIX)/bin/m2`basename $$cmd`; \
	  rm -f $(M2ROOT)/m2bin/`basename $$cmd`; \
	done
	-rmdir $(M2ROOT)/m2bin

depend:
	rm -f dependencies
	for cpp in $(SOURCES); do \
		g++ -MM $(CXXFLAGS) $$cpp >> dependencies; \
	done

-include dependencies
