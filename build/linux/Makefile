SOURCEDIR = ../../src
OBJECTDIR := ./obj
REAL_SOURCEDIR := $(shell cd $(SOURCEDIR); pwd)
REAL_OBJECTDIR := $(shell mkdir -p $(OBJECTDIR); cd obj; pwd)

include ../common/Toolchain.mk

#CXXFLAGS += -fno-rtti 
CXXFLAGS += $(CPUOPTION)
CXXFLAGS += -fexceptions -fno-permissive

ifeq ($(OPT_DEBUG),true)
CXXFLAGS	+= -g -DDEBUG -D_DEBUG -Wall
else
CXXFLAGS	+= -O2
endif

WARN := -Wall -Wno-switch-enum -Wno-switch -Wno-multichar -Wno-shift-overflow -Wno-unused-variable -Wno-unused-but-set-variable
PREPROC := -D__int64="long long"

EXECUTABLE := DumpTS

SOURCES := \
	$(SOURCEDIR)/AIFF.cpp \
	$(SOURCEDIR)/AMRingBuffer.cpp \
	$(SOURCEDIR)/AudChMap.cpp \
	$(SOURCEDIR)/Bitstream.cpp \
	$(SOURCEDIR)/AMBitStream.cpp \
	$(SOURCEDIR)/DumpMKV.cpp \
	$(SOURCEDIR)/DumpMMT.cpp \
	$(SOURCEDIR)/DumpMP4.cpp \
	$(SOURCEDIR)/DumpStream.cpp \
	$(SOURCEDIR)/DumpTS.cpp \
	$(SOURCEDIR)/DumpPS.cpp \
	$(SOURCEDIR)/ESRepacker.cpp \
	$(SOURCEDIR)/HuffmanTree.cpp \
	$(SOURCEDIR)/IP.cpp \
	$(SOURCEDIR)/ISO14496_1.cpp \
	$(SOURCEDIR)/ISO14496_12.cpp \
	$(SOURCEDIR)/ISO14496_15.cpp \
	$(SOURCEDIR)/MMT.cpp \
	$(SOURCEDIR)/Matroska.cpp \
	$(SOURCEDIR)/PartitialTS.cpp \
	$(SOURCEDIR)/PayloadBuf.cpp \
	$(SOURCEDIR)/QTFF.cpp \
	$(SOURCEDIR)/RefactorTS.cpp \
	$(SOURCEDIR)/StdAfx.cpp \
	$(SOURCEDIR)/combase.cpp \
	$(SOURCEDIR)/system_13818_1.cpp \
	$(SOURCEDIR)/Syncer.cpp \
	$(SOURCEDIR)/TSTD.cpp \
	$(SOURCEDIR)/crc.cpp
	
OBJECTS = $(patsubst $(SOURCEDIR)/%.c, $(OBJECTDIR)/%.o, $(patsubst $(SOURCEDIR)/%.cpp, $(OBJECTDIR)/%.o, $(SOURCES)))

INCDIRS += \
	-I. \
	-I../../src/LibPlatform
	
CPPFLAGS = $(DBG) $(OPT) $(WARN) $(COMMON_PREPROC) $(PREPROC) $(INCDIRS)

$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.cpp
	@if [ ! -d $(OBJECTDIR)/ ]; then mkdir $(OBJECTDIR)/ ; fi
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $($@_XFLAGS) $< -o $@
$(OBJECTDIR)/%.o: $(SOURCEDIR)/%.c
	@if [ ! -d $(OBJECTDIR)/ ]; then mkdir $(OBJECTDIR)/ ; fi
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $($@_XFLAGS) $< -o $@

all: $(EXECUTABLE) $(EXTRATARGETS)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) -L$(BINDIR) -o $@ $^ -Wl,-Bsymbolic
ifneq ($(OPT_DEBUG),true)
	$(STRIP) $(EXECUTABLE)
endif
	@if [ ! -d $(BINDIR)/ ]; then mkdir $(BINDIR)/ ; fi
	cp -f $@ $(BINDIR)/

clean:
	@echo $(SOURCEDIR)
	@echo $(OBJECTDIR)
	rm -rf $(EXECUTABLE) $(OBJECTS) $(OBJECTDIR)
