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
	   Minix2/VM.cpp Minix2/VM.sys.cpp Minix2/VM.signal.cpp \
	   PDP11/OpCode.cpp PDP11/Operand.cpp \
	   PDP11/VM.cpp PDP11/VM.inst.cpp PDP11/disasm.cpp \
	   UnixV6/VM.cpp UnixV6/VM.sys.cpp UnixV6/VM.signal.cpp
M2CMDS   = bin/aal bin/ar bin/cc bin/nm bin/strip \
	   bin/crc bin/install bin/m2 \
	   lib/as lib/ld lib/cv
V6CMDS   = ar as cc ld nm strip

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core

install: $(TARGET)
	mkdir -p $(PREFIX)/bin $(M2ROOT)/7bin $(V6ROOT)/7bin
	install -cs $(TARGET) $(PREFIX)/bin
	for cmd in $(M2CMDS); do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 usr/$$cmd; \
	  cp $(PREFIX)/bin/m2`basename $$cmd` $(M2ROOT)/7bin/`basename $$cmd`; \
	done
	for cmd in $(V6CMDS); do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(V6ROOT) $(PREFIX)/bin/v6 bin/$$cmd; \
	  cp $(PREFIX)/bin/v6`basename $$cmd` $(V6ROOT)/7bin/`basename $$cmd`; \
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
	  (echo -n `dirname $$cpp`/; \
	   g++ -MM $(CXXFLAGS) $$cpp) >> dependencies; \
	done

-include dependencies
