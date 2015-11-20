include ../Makefile.inc
TARGET   = 7run
CXX      = g++
CXXFLAGS = -Wall -O2 -g
LDFLAGS  =
OBJECTS  = $(SOURCES:%.cpp=%.o)
SOURCES  = main.cpp utils.cpp File.cpp \
	   VMBase.cpp UnixBase.cpp UnixBase.sys.cpp \
	   i8086/OpCode.cpp i8086/Operand.cpp \
	   i8086/VM.cpp i8086/VM.inst.cpp i8086/disasm.cpp \
	   Minix2/OS.cpp Minix2/OS.sys.cpp Minix2/OS.signal.cpp \
	   PDP11/OpCode.cpp PDP11/Operand.cpp \
	   PDP11/VM.cpp PDP11/VM.inst.cpp PDP11/disasm.cpp \
	   UnixV6/OS.cpp UnixV6/OS.sys.cpp \
	   UnixV6/OSPDP11.cpp UnixV6/OSi8086.cpp
M2CMDS   = bin/aal bin/ar bin/cc bin/nm bin/strip \
	   bin/crc bin/install bin/m2 \
	   lib/as lib/ld lib/cv
V6CMDS   = ar as cc ld nm strip
V7CMDS   = ar as cc ld

all: $(TARGET)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

clean:
	rm -f $(TARGET) $(TARGET).exe $(OBJECTS) *core

install: install-7 install-cmds

install-7: $(TARGET)
	mkdir -p $(PREFIX)/bin
	install -cs $(TARGET) $(PREFIX)/bin

install-cmds:
	mkdir -p $(M2ROOT)/7bin $(V6ROOT)/7bin $(V7ROOT)/7bin
	for cmd in $(M2CMDS); do \
	  sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(M2ROOT) $(PREFIX)/bin/m2 usr/$$cmd; \
	  cp $(PREFIX)/bin/m2`basename $$cmd` $(M2ROOT)/7bin/`basename $$cmd`; \
	done
	for cmd in $(V6CMDS); do \
	  sh mkwrap.sh -x $(PREFIX)/bin/$(TARGET) $(V6ROOT) $(PREFIX)/bin/v6 bin/$$cmd; \
	  cp $(PREFIX)/bin/v6`basename $$cmd` $(V6ROOT)/7bin/`basename $$cmd`; \
	done
	for cmd in $(V7CMDS); do \
	  sh mkwrap.sh -x $(PREFIX)/bin/$(TARGET) $(V7ROOT) $(PREFIX)/bin/v7 bin/$$cmd -7; \
	  cp $(PREFIX)/bin/v7`basename $$cmd` $(V7ROOT)/7bin/`basename $$cmd`; \
	done
	(echo '#!/bin/sh' && echo 'PATH="$(M2ROOT)/7bin:$$PATH" "$$@"') > $(PREFIX)/bin/m2path
	(echo '#!/bin/sh' && echo 'PATH="$(V6ROOT)/7bin:$$PATH" "$$@"') > $(PREFIX)/bin/v6path
	(echo '#!/bin/sh' && echo 'PATH="$(V7ROOT)/7bin:$$PATH" "$$@"') > $(PREFIX)/bin/v7path
	cd $(PREFIX)/bin && chmod 755 m2path v6path v7path
	sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(ACKROOT) $(PREFIX)/bin/8086v6- bin/cc
	sh mkwrap.sh $(PREFIX)/bin/$(TARGET) $(ACKROOT) $(PREFIX)/bin/8086v6- lib/cv

uninstall:
	cd $(PREFIX)/bin && rm -f $(TARGET) $(TARGET).exe m2path v6path v7path 8086v6-cc 8086v6-cv
	for cmd in $(M2CMDS); do \
	  rm -f $(PREFIX)/bin/m2`basename $$cmd`; \
	  rm -f $(M2ROOT)/7bin/`basename $$cmd`; \
	done
	for cmd in $(V6CMDS); do \
	  rm -f $(PREFIX)/bin/v6`basename $$cmd`; \
	  rm -f $(V6ROOT)/7bin/`basename $$cmd`; \
	done
	for cmd in $(V7CMDS); do \
	  rm -f $(PREFIX)/bin/v7`basename $$cmd`; \
	  rm -f $(V7ROOT)/7bin/`basename $$cmd`; \
	done
	-rmdir $(M2ROOT)/7bin $(V6ROOT)/7bin $(V7ROOT)/7bin

depend:
	rm -f dependencies
	for cpp in $(SOURCES); do \
	  (echo -n `dirname $$cpp`/; \
	   g++ -MM $(CXXFLAGS) $$cpp) >> dependencies; \
	done

-include dependencies
