## start toolchain select
OPT_TARGET_PLATFORM ?=
OPT_COMPILER ?=  

ifeq ($(strip $(OPT_COMPILER)), clang)
	CC = ${PREFIX}clang
	CXX = ${PREFIX}clang++
	AR = ${PREFIX}ar
	RANLIB = ${PREFIX}ranlib
	AS = ${PREFIX}as
	LD = ${PREFIX}ld
	NM = ${PREFIX}nm
	STRIP = ${PREFIX}strip
	BINDIR:=$(REAL_SOURCEDIR)/../bin/macos
else
ifeq ($(strip $(OPT_TARGET_PLATFORM)), USER_DEFINED_TARGET1)
	CPUOPTION := -mips2 -DUSE_MIPS -DUSE_OVIA_MIPS -DUSE_BIGENDIAN -fpermissive -fPIC
	CROSS_DIR := /opt/wrl30/host-cross/mips-wrs-linux-gnu
	SYSROOT_DIR := /opt/wrl30/host-cross/mips-wrs-linux-gnu/sysroot
	TARGET_DIR := /opt/wrl30/export/dist
	BUILD_DIR := /opt/emma/wrl30/build
	KERNEL_SOURCE_DIR := /opt/wrl30/build/linux
	KERNEL_BUILD_DIR := /opt/wrl30/build/linux-xxx-standard-build
	PREFIX := $(CROSS_DIR)/x86-linux2/mips-wrs-linux-gnu-mips_softfp-glibc_std-
	BINDIR:=$(REAL_SOURCEDIR)/../bin/$(OPT_TARGET_PLATFORM)
else
ifeq ($(strip $(OPT_TARGET_PLATFORM)), USER_DEFINED_TARGET2)
	CPUOPTION := -DUSE_MIPS -DUSE_OVIA_MIPS -DUSE_BIGENDIAN -D__EMMA__ -fpermissive -fPIC
	CROSS_DIR := /opt/xxx/host-cross/mips-wrs-linux-gnu
	SYSROOT_DIR := /opt/xxx/host-cross/mips-wrs-linux-gnu/sysroot
	TARGET_DIR := /opt/xxx/export/dist
	BUILD_DIR := /opt/xxx/build
	KERNEL_SOURCE_DIR := /opt/xxx/build/linux
	KERNEL_BUILD_DIR := /opt/xxx/build/linux-nec_emma3r-standard-build
	PREFIX := /opt/xxx/host-cross/mips-wrs-linux-gnu/bin/mips-wrs-linux-gnu-mips_xxxx-glibc_small-
	BINDIR:=$(REAL_SOURCEDIR)/../bin/$(OPT_TARGET_PLATFORM)
else # x86
	CROSS_DIR :=
	SYSROOT_DIR :=
	TARGET_DIR :=
	BUILD_DIR :=
	KERNEL_SOURCE_DIR :=
	KERNEL_BUILD_DIR :=
	PREFIX :=
	BINDIR:=$(REAL_SOURCEDIR)/../bin/linux
endif
endif

CC = ${PREFIX}gcc
CXX = ${PREFIX}g++
AR = ${PREFIX}ar
RANLIB = ${PREFIX}ranlib
AS = ${PREFIX}as
LD = ${PREFIX}ld
NM = ${PREFIX}nm
STRIP = ${PREFIX}strip
endif

