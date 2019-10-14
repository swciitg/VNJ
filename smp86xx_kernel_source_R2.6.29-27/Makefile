#
# This is the Sigma Designs Makefile for the preparation of the MIPS kernel 
# source for the SMP86xx family of chips.
#
# Copyright (c) 2005 Sigma Designs, Inc <http://www.sigmadesigns.com>
#
SHELL = /bin/bash
.SUFFIXES:

ifndef $(PWD)
PWD := $(shell pwd)
endif

patch_dir := pf/patches
current_dir := $(shell basename $(PWD))

wget          := wget --passive-ftp
download_site := ftp://armutils:sigmadesigns@gw.sigmadesigns.com/smp86xx


all:
	@echo
	@echo "Linux kernel; Sigma Designs release is $(RELEASE_NAME)"
	@echo
	@echo "Available targets:"
	@echo "  kernel-source-2.6.29: prepares the kernel source under ./linux-2.6.29"
	@echo "  proprietary: making proprietary programs/drivers under ./proprietary"
	@echo "  superclean          : cleans all created files away"
	@echo
	@echo "Once you have prepared the kernel source, you can build the"
	@echo "Linux kernel in a regular manner (make menuconfig, make dep,"
	@echo "make). Your end target is vmlinux.bin under the kernel base"
	@echo "directory".
	@echo

.PHONY: superclean
superclean:
	-$(MAKE) -C ./proprietary cleanall
	-rm $(kernel_src_2_6_29)
	-rm -r $(kernel_src_2_6_29_dir)

.PHONY: proprietary
proprietary:
	LINUX_KERNEL=$(PWD)/linux-2.6.29 $(MAKE) -C ./proprietary

###############################################################################
#
# Kernel 2.6.29
#
###############################################################################

kernel_src_2_6_29           := linux-2.6.29.tar.bz2
kernel_src_2_6_29_dir       := linux-2.6.29
ifneq '$(filter -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2,$(RMCFLAGS))' ''
kernel_src_2_6_29_config    := config-2.6.29-smp86xx
else
kernel_src_2_6_29_config    := config-2.6.29-smp86xx-T3
endif

$(kernel_src_2_6_29_dir)/.unpacked: $(kernel_src_2_6_29)
.PHONY: kernel-source-2.6.29-unpacked
kernel-source-2.6.29-unpacked: $(kernel_src_2_6_29_dir)/.unpacked

ifneq '$(filter -DEM86XX_CHIP=EM86XX_CHIPID_TANGO2,$(RMCFLAGS))' ''
patch_list_2_6_29 :=  \
    0000.shared.patch \
    1000.kernel-updates.patch \
    1001.tangox.patch \
    1002.zbf.patch \
    1003.rootfs.patch \
    1004.gpio.patch \
    1005.mbus.patch \
    1006.pcidma.patch \
    1007.enet.patch \
    1009.usb.patch \
    1010.ir.patch \
    1011.fip.patch \
    1012.scard.patch \
    1014.mtd.patch \
    1015.bmide.patch \
    1017.ipfilter.patch \
    1018.fb.patch \
    1021.memop.patch \
    1022.dtcpip.patch \
    1024.multirecv.patch \
    1034.highmem.patch
else
ifneq '$(filter codesourcery, $(COMPILKIND))' ''
patch_list_2_6_29 :=  \
    0000.shared.patch \
    1000.kernel-updates.patch \
    1001.tangox.patch \
    1002.zbf.patch \
    1003.rootfs.patch \
    1004.gpio.patch \
    1005.mbus.patch \
    1006.pcidma.patch \
    1007.enet.patch \
    1008.watchdog.patch \
    1009.usb.patch \
    1010.ir.patch \
    1011.fip.patch \
    1012.scard.patch \
    1013.uir.patch \
    1014.mtd.patch \
    1015.bmide.patch \
    1016.sata.patch \
    1017.ipfilter.patch \
    1018.fb.patch \
    1019.otg.patch \
    1020.unaligned.patch \
    1021.memop.patch \
    1022.dtcpip.patch \
    1023.codesourcery.patch \
    1024.multirecv.patch \
    1026.sdio.patch \
    1029.gcc44x.patch \
    1032.zxenv.patch \
    1033.yaffs.patch \
    1034.highmem.patch
else
patch_list_2_6_29 :=  \
    0000.shared.patch \
    1000.kernel-updates.patch \
    1001.tangox.patch \
    1002.zbf.patch \
    1003.rootfs.patch \
    1004.gpio.patch \
    1005.mbus.patch \
    1006.pcidma.patch \
    1007.enet.patch \
    1008.watchdog.patch \
    1009.usb.patch \
    1010.ir.patch \
    1011.fip.patch \
    1012.scard.patch \
    1013.uir.patch \
    1014.mtd.patch \
    1015.bmide.patch \
    1016.sata.patch \
    1017.ipfilter.patch \
    1018.fb.patch \
    1019.otg.patch \
    1020.unaligned.patch \
    1021.memop.patch \
    1022.dtcpip.patch \
    1024.multirecv.patch \
    1026.sdio.patch \
    1029.gcc44x.patch \
    1032.zxenv.patch \
    1033.yaffs.patch \
    1034.highmem.patch
endif
endif
patch_executed_list_2_6_29 := $(patch_list_2_6_29:%=$(kernel_src_2_6_29_dir)/README.%)

$(patch_executed_list_2_6_29): $(kernel_src_2_6_29_dir)/.unpacked

$(kernel_src_2_6_29_dir)/.patched: $(patch_executed_list_2_6_29)
.PHONY: kernel-source-2.6.29-patched
kernel-source-2.6.29-patched: $(kernel_src_2_6_29_dir)/.patched

$(kernel_src_2_6_29_dir)/.configured: $(kernel_src_2_6_29_config) $(kernel_src_2_6_29_dir)/.patched
.PHONY: kernel-source-2.6.29-configured
kernel-source-2.6.29-configured: $(kernel_src_2_6_29_dir)/.configured

.PHONY: kernel-source-2.6.29
kernel-source-2.6.29: $(kernel_src_2_6_29_dir)/.configured

###############################################################################
#
# Rules
#
###############################################################################
$(kernel_src_2_6_29):
	$(wget) $(download_site)/$(@F)

$(kernel_src_2_6_29_dir)/.unpacked:
	bzcat $< | tar -x -f -
	touch $@

$(patch_executed_list_2_6_29): 
	patch -d $(@D) -p1 < $(patch_dir)/$(@D)/$(subst README.,,$(@F))

$(kernel_src_2_6_29_dir)/.patched:
	touch $@

$(kernel_src_2_6_29_dir)/.configured:
	sed -e "s/@CURRENT_UID@/$(shell id -u)/;s/@CURRENT_GID@/$(shell id -g)/" < $< > $(@D)/.config
	echo | make -C $(@D) oldconfig
	touch $@

kernel-source-2.6.29:
	@echo
	@echo "Your $(subst kernel-source-,,$@) kernel source is now available under ./$(<D)."
	@echo "Please set LINUX_KERNEL/UCLINUX_KERNEL environment to point to this directory"
	@echo "before making proprietary programs/drivers."
	@echo
	@rm -f linux
	@ln -sf $(kernel_src_2_6_29_dir) linux

