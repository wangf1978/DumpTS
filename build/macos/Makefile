SOURCEDIR = ../../src
OBJECTDIR := ./obj
REAL_SOURCEDIR := $(shell cd $(SOURCEDIR); pwd)
REAL_OBJECTDIR := $(shell mkdir -p $(OBJECTDIR); cd obj; pwd)
OPT_COMPILER := clang
export OPT_COMPILER

include ../common/Toolchain.mk

#CXXFLAGS += -fno-rtti 
CXXFLAGS += $(CPUOPTION)
CXXFLAGS += -fexceptions -fno-permissive -std=gnu++14

ifeq ($(OPT_DEBUG),true)
CXXFLAGS	+= -g -DDEBUG -D_DEBUG -Wall
else
CXXFLAGS	+= -O2
endif

WARN := -Wall -Wno-switch-enum -Wno-switch -Wno-multichar -Wno-shift-overflow -Wno-unused-variable -Wno-bitfield-constant-conversion 
PREPROC := -D__int64="long long"

EXECUTABLE := DumpTS

SOURCES := \
	$(SOURCEDIR)/platcomm.cpp \
	$(SOURCEDIR)/combase.cpp \
	$(SOURCEDIR)/AIFF.cpp \
	$(SOURCEDIR)/AMRingBuffer.cpp \
	$(SOURCEDIR)/AMBitArray.cpp \
	$(SOURCEDIR)/AMSHA1.cpp \
	$(SOURCEDIR)/AMRFC3986.cpp \
	$(SOURCEDIR)/AudChMap.cpp \
	$(SOURCEDIR)/Bitstream.cpp \
	$(SOURCEDIR)/AMBitStream.cpp \
	$(SOURCEDIR)/nal_com.cpp \
	$(SOURCEDIR)/nal_parser.cpp \
	$(SOURCEDIR)/mpeg2_video_parser.cpp \
	$(SOURCEDIR)/mpeg2_video.cpp \
	$(SOURCEDIR)/sei.cpp \
	$(SOURCEDIR)/h264_video.cpp \
	$(SOURCEDIR)/h265_video.cpp \
	$(SOURCEDIR)/h266_video.cpp \
	$(SOURCEDIR)/av1_parser.cpp \
	$(SOURCEDIR)/av1.cpp \
	$(SOURCEDIR)/ESRepacker.cpp \
	$(SOURCEDIR)/HuffmanTree.cpp \
	$(SOURCEDIR)/IP.cpp \
	$(SOURCEDIR)/ISO14496_1.cpp \
	$(SOURCEDIR)/ISO14496_3.cpp \
	$(SOURCEDIR)/ISO14496_12.cpp \
	$(SOURCEDIR)/ISO14496_15.cpp \
	$(SOURCEDIR)/PayloadBuf.cpp \
	$(SOURCEDIR)/MMT.cpp \
	$(SOURCEDIR)/Matroska.cpp \
	$(SOURCEDIR)/PartitialTS.cpp \
	$(SOURCEDIR)/QTFF.cpp \
	$(SOURCEDIR)/descriptors_13818_1.cpp \
	$(SOURCEDIR)/descriptors_hdmv.cpp \
	$(SOURCEDIR)/system_13818_1.cpp \
	$(SOURCEDIR)/MSEnav.cpp \
	$(SOURCEDIR)/RefactorTS.cpp \
	$(SOURCEDIR)/DumpMKV.cpp \
	$(SOURCEDIR)/DumpMMT.cpp \
	$(SOURCEDIR)/DumpMP4.cpp \
	$(SOURCEDIR)/DumpStream.cpp \
	$(SOURCEDIR)/DumpTS.cpp \
	$(SOURCEDIR)/DumpPS.cpp \
	$(SOURCEDIR)/DumpNAL.cpp \
	$(SOURCEDIR)/DumpAAC.cpp \
	$(SOURCEDIR)/DumpAV1.cpp \
	$(SOURCEDIR)/DumpMSE.cpp \
	$(SOURCEDIR)/transcode.cpp \
	$(SOURCEDIR)/transcode_es.cpp \
	$(SOURCEDIR)/Syncer.cpp \
	$(SOURCEDIR)/TSTD.cpp \
	$(SOURCEDIR)/crc.cpp \
	$(SOURCEDIR)/mediaobjprint.cpp \
	$(SOURCEDIR)/range_lex.cpp \
	$(SOURCEDIR)/tinyxml2.cpp
	
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
	@if [ ! -d $(BINDIR)/ ]; then mkdir $(BINDIR)/ ; fi
	$(CXX) -L$(BINDIR) -o $@ $^ $(LDFLAGS) -ldl -Wl
ifneq ($(OPT_DEBUG),true)
	$(STRIP) $(EXECUTABLE)
endif
	cp -f $@ $(BINDIR)/

clean:
	@echo $(SOURCEDIR)
	@echo $(OBJECTDIR)
	rm -rf $(EXECUTABLE) $(OBJECTS) $(OBJECTDIR)
