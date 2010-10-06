#
# or32/Makefile
#
# This file is included by the global makefile so that you can add your own
# platform-specific flags and dependencies.
#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Based on m68k/Rules.make
#

GCC_DIR = $(shell $(CC) -v 2>&1 | grep specs | sed -e 's/.* \(.*\)specs/\1\./')

LIBGCC = $(GCC_DIR)/include
LIBGCC = $(GCC_DIR)/libgcc.a

CFLAGS := $(CFLAGS) -pipe -DNO_MM -DNO_FPU -DMAGIC_ROM_PTR -DNO_FORGET -DUTS_SYSNAME='"uClinux"'
AFLAGS := $(AFLAGS) -pipe -DNO_MM -DNO_FPU -DMAGIC_ROM_PTR -DUTS_SYSNAME='"uClinux"'

LINKFLAGS = -T arch/$(ARCH)/board/$(MODEL).ld

INIT_B := arch/$(ARCH)/board/init_$(MODEL).b
STOB   := arch/$(ARCH)/tools/stob

SUBDIRS := arch/$(ARCH)/kernel arch/$(ARCH)/mm arch/$(ARCH)/lib \
           arch/$(ARCH)/board $(SUBDIRS)
ARCHIVES := arch/$(ARCH)/kernel/kernel.o arch/$(ARCH)/mm/mm.o \
            arch/$(ARCH)/board/board.o $(ARCHIVES)
LIBS += arch/$(ARCH)/lib/lib.a $(LIBGCC)

ifdef CONFIG_FRAMEBUFFER
SUBDIRS := $(SUBDIRS) arch/$(ARCH)/console
ARCHIVES := $(ARCHIVES) arch/$(ARCH)/console/console.a
endif

romfs.s19: romfs.img arch/$(ARCH)/empty.o
	$(CROSS_COMPILE)objcopy -v -R .text -R .data -R .bss --add-section=.fs=romfs.img --adjust-section-vma=.fs=$(ROMFS_LOAD_ADDR) arch/$(ARCH)/empty.o romfs.s19
	$(CROSS_COMPILE)objcopy -O srec romfs.s19

romfs.b: romfs.s19
	$(STOB) romfs.s19 > romfs.b

linux.data: linux
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.romvec --remove-section=.text --remove-section=.ramvec --remove-section=.bss --remove-section=.eram linux linux.data

linux.text: linux
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.ramvec --remove-section=.bss --remove-section=.data --remove-section=.eram --set-section-flags=.romvec=CONTENTS,ALLOC,LOAD,READONLY,CODE linux linux.text

romfs.img:
	echo creating a vmlinux rom image without root filesystem!

linux.bin: linux.text linux.data romfs.img
	if [ -f romfs.img ]; then\
		cat linux.text linux.data romfs.img > linux.bin;\
	else\
		cat linux.text linux.data > linux.bin;\
	fi

flash.s19: linux.bin arch/$(ARCH)/empty.o
	$(CROSS_COMPILE)objcopy -v -R .text -R .data -R .bss --add-section=.fs=linux.bin --adjust-section-vma=.fs=$(FLASH_LOAD_ADDR) arch/$(ARCH)/empty.o flash.s19
	$(CROSS_COMPILE)objcopy -O srec flash.s19

flash.b: flash.s19
	$(STOB) flash.s19 > flash.b

linux.trg linux.rom: linux.bin
	perl arch/$(ARCH)/tools/fixup.pl

linux.s19: linux
	$(CROSS_COMPILE)objcopy -O srec --adjust-section-vma=.data=0x`$(CROSS_COMPILE)nm linux | awk '/__data_rom_start/ {printf $$1}'` linux linux.s19

	$(CROSS_COMPILE)objcopy -O srec linux.s19
	
linux.b: linux.s19
	if [ -f $(INIT_B) ]; then\
		cp $(INIT_B) linux.b;\
	fi
	$(STOB) linux.s19 >> linux.b

archclean:
	rm -f linux.text linux.data linux.bin linux.rom linux.trg
	rm -f linux.s19 romfs.s19 flash.s19
	rm -f linux.img romdisk.img
	rm -f linux.b romfs.b flash.b
