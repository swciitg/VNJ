# Copyright (C) 2004-2010
# Sigma Designs, Inc. All Rights Reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.


# To use Code Sourcery toolchain, add "codesourcery" to COMPILKIND, and set your path:
# $ export PATH=/utils/gcc/4.2.3/codesourcery/i386/bin/:$PATH
ifneq '$(filter codesourcery, $(COMPILKIND))' ''
CROSS = mips-linux-gnu-
else
CROSS = mipsel-linux-
endif

# Define this to use boot authentication
USE_BOOT_AUTH := y

MYKEY_PUB := $(wildcard mykey.pub)

YAFFS2_DIR := $(wildcard yaffs2)
ifeq ($(YAFFS2_DIR),)
$(error yaffs2 directory does not exist. Please install the yaffs2 archive here)
endif

CC=$(CROSS)gcc
CPP=$(CROSS)cpp
LD=$(CROSS)ld
AR=$(CROSS)ar
OBJCOPY=$(CROSS)objcopy
OBJDUMP=$(CROSS)objdump
SED=sed

INCS = \
	-Iyaffs2/ \
	-Iyaffs2/direct/

# Applet starting address and stack size.
# We're using this applet address because it is placed immediately after ezboot's buffers, assuming
# ezboot is using the maximum size for its heap/reading/decryption buffers, which is 2/32/32 MB.
APPLET_ADDR = 0x87840000
APPLET_NAME = yaffs_launcher

IMAGE_BIN = $(APPLET_NAME).bin

# Applet version should be updated for each release of the yaffs_launcher
APPLET_VERSION = R004

STACK_SIZE = 0x4000

CFLAGS = $(RMCFLAGS) -mips32r2 -EL

CFLAGS +=\
	-EL \
	-D_32_ \
	-Wall \
	-Werror \
	-Wno-pointer-sign \
	-O2 \
	-mno-abicalls \
	-fno-pic \
	-fno-builtin

# CFLAGS +=\
# 	-Wsign-compare \
# 	-Wmissing-prototypes


CFLAGS_MIPS32=-march=mips32r2 -Wa,-32 -Wa,-march=mips32r2 -Wa,-mips32r2 -Wa,--trap -mabi=32

# You may optionally build in mips16 mode to save on code size, or compile in mips32 exclusively
ifneq '$(filter on, $(MIPS16))' ''
CFLAGS_MIPS16=-mips16 -DYAFFS_LAUNCHER_COMPILEMODE_MIPS16=1
else
CFLAGS_MIPS16=
endif

CFLAGS += $(INCS)

CFLAGS += -DRMPLATFORM=RMPLATFORMID_TANGO3

CFLAGS += -DAPPLET_ADDR=$(APPLET_ADDR)

CFLAGS += -DCONFIG_YAFFS_DIRECT=1 -DCONFIG_YAFFS_YAFFS2=1 -DCONFIG_YAFFSFS_PROVIDE_VALUES=1 -DCONFIG_YAFFS_PROVIDE_DEFS=1

ifeq ($(USE_BOOT_AUTH), y)
CFLAGS += -DUSE_BOOT_AUTH
endif

ASFLAGS = $(CFLAGS) -D__ASSEMBLY__=1


#######################
### Device Type Support
#######################
ifneq '$(filter -DXBOOT3_SMP8910=1, $(RMCFLAGS))' ''
# 8910 family
CFLAGS += -DWITH_MLC_NAND=1
else
ifneq '$(filter -DXBOOT2_SMP8656=1, $(RMCFLAGS))' ''
# 8656 family
ifneq '$(filter -DXBOOT3=1, $(RMCFLAGS))' ''
# 8656 OTP uses old controller
CFLAGS += -DWITH_PB_NAND=1
else
CFLAGS += -DWITH_MLC_NAND=1
endif # XBOOT3
else
ifneq '$(filter -DXBOOT2_SMP8646=1, $(RMCFLAGS))' ''
# 8646 family
CFLAGS += -DWITH_MLC_NAND=1
else
ifneq '$(filter -DXBOOT2_SMP8670=1, $(RMCFLAGS))' ''
# 8670 family
CFLAGS += -DWITH_MLC_NAND=1
else
# 8642, 8644, 8652, 8654 family
CFLAGS += -DWITH_PB_NAND=1
endif # SMP8670
endif # SMP8646
endif # SMP8656
endif # SMP8910


# Setup certificate for signing/encryption
CERT_SET := 8644
ifneq '$(filter -DXBOOT2_SMP8656=1, $(RMCFLAGS))' ''
CERT_SET := 8656
endif

ifneq '$(filter -DWITH_PROD=1, $(RMCFLAGS))' ''
REV    := $(CERT_SET)_ES1_prod
CERTID := 000f
else
ifneq '$(filter -DWITH_FACSPROD=1, $(RMCFLAGS))' ''
REV    := $(CERT_SET)_ES1_prod
CERTID := 000f
else
REV    := $(CERT_SET)_ES1_dev
CERTID := 0001
endif
endif

ifneq '$(filter -DXBOOT3=1, $(RMCFLAGS))' ''
CERTID := 0x000d
endif

# Override with environment variables if they are set
ifneq ($(strip $(XSDK_DEFAULT_KEY_DOMAIN)),)
REV	:= $(XSDK_DEFAULT_KEY_DOMAIN)
endif
ifneq ($(strip $(XSDK_DEFAULT_CPU_CERTID)),)
CERTID	:= $(XSDK_DEFAULT_CPU_CERTID)
endif

PRIVATE_KEY := $(REV)_$(CERTID)

ifneq '$(findstring prod, $(REV))' ''
BOOTAUTH_KEY  := 8644_ES1_prod_000f
else
BOOTAUTH_KEY  := 8644_ES1_dev_000f
endif


SRC_C=$(APPLET_NAME).c
SRC_C += yaffs_nand_if.c yaffs_utils.c
SRC_C += yaffs_client.c kunzip.c
SRC_C += uart.c util.c vsprintf.c uartbase.c
ifeq ($(USE_BOOT_AUTH), y)
SRC_C += fastsha1.c rsa_enc_dec.c boot_auth.c
endif
ifneq '$(filter -DWITH_PB_NAND=1, $(CFLAGS))' ''
SRC_C += nandv2_drv.c
SRC_C_MIPS32=
else
CFLAGS += -DMLC_READONLY=1
SRC_C += nandv3_drv.c
SRC_C_MIPS32= pio.c mlc_pa.c
endif
SRC_C += yaffs2/direct/basic-test/yaffscfg2k.c yaffs2/direct/yaffsfs.c yaffs2/yaffs_packedtags2.c yaffs2/yaffs_verify.c \
	yaffs2/yaffs_tagsvalidity.c yaffs2/yaffs_ecc.c yaffs2/yaffs_nand.c yaffs2/yaffs_qsort.c \
	yaffs2/yaffs_guts.c yaffs2/yaffs_tagscompat.c yaffs2/yaffs_checkptrw.c yaffs2/yaffs_yaffs2.c \
	yaffs2/yaffs_allocator.c yaffs2/yaffs_bitmap.c yaffs2/yaffs_nameval.c yaffs2/yaffs_yaffs1.c
SRC_S=start.S mips_memcopy.S gbus.S pio_asm.S

SRC_H=$(wildcard *.h)
SRC_H += $(wildcard include/*.h)

SRC_MIPS32_ASM=mips32_utilfunctions_asm.S
SRC_MIPS32=mips32_utilfunctions.c

OBJ_C=$(patsubst %.c, %.o, $(SRC_C))
OBJ_MIPS32_ASM=$(patsubst %.S, %.o, $(SRC_MIPS32_ASM))
OBJ_MIPS32=$(patsubst %.c, %.o, $(SRC_MIPS32))
OBJ_S=$(patsubst %.S, %.o, $(SRC_S))
OBJ_C_MIPS32=$(patsubst %.c, %.o, $(SRC_C_MIPS32))

ifneq '$(filter -DYAFFS_LAUNCHER_COMPILEMODE_MIPS16=1, $(CFLAGS_MIPS16))' ''
OBJS= $(OBJ_C) $(OBJ_S) $(OBJ_C_MIPS32) $(OBJ_MIPS32) $(OBJ_MIPS32_ASM)
else
OBJS= $(OBJ_C) $(OBJ_S) $(OBJ_C_MIPS32)
endif

ifeq ($(USE_BOOT_AUTH), y)
OBJS += mykey.o
endif

SRC_H += Makefile

O_FORMAT 	:= $(shell $(OBJDUMP) -i | head -2 | grep elf32)

CFLAGS += -DAPPLET_VERSION=\"$(APPLET_VERSION)\"

LDFLAGS		:= -EL


all: zbimage-yaffs_launcher-xload

clean: 
	rm -f $(OBJS) *.bin *.elf yaffs_launcher.bin.gz *.map *.dasm *.zbf *.xload *.o *.ld *~ yaffs2/*~ yaffs2/direct/*~
	rm -f zbimage-yaffs_launcher-xload

zbimage-yaffs_launcher-xload: $(IMAGE_BIN)
	gzip -c9nf $(IMAGE_BIN) > $(IMAGE_BIN).gz
	zeropad.bash $(IMAGE_BIN).gz 16
	mkdir -p romfs
	cp -f $(IMAGE_BIN).gz yaffs_launcher_gz.zbf
ifneq '$(filter -DXBOOT3=1, $(RMCFLAGS))' ''
	create_xload3.bash -p yaffs_launcher_gz.zbf -c $(CERTID)
	mv yaffs_launcher_gz.zbf.xload3 yaffs_launcher_gz_$(REV).xload
else
	cg_display_cert.bash -c $(XSDK_ROOT)/items/xload_certificates/xload_certificate_$(PRIVATE_KEY).bin
	openssl sha1 -sign $(XSDK_ROOT)/dummy_private_keys/$(PRIVATE_KEY)_keyboth.pem < yaffs_launcher_gz.zbf \
		| revbytes.pl - > yaffs_launcher_gz.zbf.$(PRIVATE_KEY).bin
	mkxload.bash $(XSDK_ROOT) $(REV) $(CERTID) yaffs_launcher_gz.zbf yaffs_launcher_gz.zbf.$(PRIVATE_KEY).bin \
		yaffs_launcher_gz_$(REV).xload
endif
	genzbf -l $(APPLET_ADDR) -s $(APPLET_ADDR) -a lzef -o $(IMAGE_BIN)-xload.zbf yaffs_launcher_gz_$(REV).xload
	cp -f $(IMAGE_BIN)-xload.zbf romfs/
	genromfs -V YAFFS_LAUNCHER_XLOAD -d romfs -f zbimage-yaffs_launcher-xload
	rm -rf romfs
	@echo Your image is ready as "zbimage-yaffs_launcher-xload"

$(APPLET_NAME).elf: $(OBJS) yaffs_launcher.ld 
	$(LD) $(LDFLAGS) -T yaffs_launcher.ld -o $@ -Map $(APPLET_NAME).map $(OBJS) 

$(APPLET_NAME).dasm: $(APPLET_NAME).elf
	$(OBJDUMP) -S --line-number $< > $@

$(APPLET_NAME).bin: $(APPLET_NAME).elf 
	$(OBJCOPY) -O binary $< $@

$(OBJ_S) : %.o : %.S
	$(CC) -c $(ASFLAGS) $(CFLAGS_MIPS32) -o $@ $<

$(OBJ_MIPS32_ASM) : %.o : %.S
	$(CC) -c $(ASFLAGS) $(CFLAGS_MIPS32) -o $@ $<

$(OBJ_MIPS32) : %.o : %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_MIPS32) -o $@ $<

$(OBJ_C) : %.o : %.c $(SRC_H)
ifneq '$(filter on, $(MIPS16))' ''
	$(CC) -c $(CFLAGS) $(CFLAGS_MIPS16) -o $@ $<
else
	$(CC) -c $(CFLAGS) $(CFLAGS_MIPS32) -o $@ $<
endif

$(OBJ_C_MIPS32) : %.o : %.c
	$(CC) -c $(CFLAGS) $(CFLAGS_MIPS32) -o $@ $<

ifeq ($(USE_BOOT_AUTH), y)
mykey.o: mykey.pub mykey.lds
	$(LD) $(LDFLAGS) -T mykey.lds -b binary --oformat elf32-tradlittlemips -o $@ mykey.pub

mykey.pub: 
ifeq ($(MYKEY_PUB),)
# Public key file mykey.pub does not exist.  Try to make it from the CPU KEYS package...
	echo mykey.pub does not exist.  Pulling public key from CPU KEYS package.
	openssl rsa -pubin -in $(XSDK_ROOT)/public_keys/$(BOOTAUTH_KEY)_pubkey.pem -modulus | grep Modulus | sed 's/Modulus=//g' | xargs tobin.bash > mykey.pub
else
	echo mykey.pub already exists, using that as the public key to authenticate with.
endif
endif # USE_BOOT_AUTH

yaffs_launcher.ld: yaffs_launcher.ld.S
	$(CPP) -P $(ASFLAGS) -DSTACK_SIZE=$(STACK_SIZE) -DAPPLET_ADDR=$(APPLET_ADDR) $^ >$@


FORCE:
