-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)

LINUXFILE = linux
ROMFSFILE = romfs.img
LINUXIMG  = $(IMAGEDIR)/$(LINUXFILE)
ROMFSIMG  = $(IMAGEDIR)/$(ROMFSFILE)
ROMFSIMGOBJ  = $(IMAGEDIR)/rdimage.o
FLASH     = $(IMAGEDIR)/flash.bin
TOOLS     = $(ROOTDIR)/tools
TSIMIMAGE = $(IMAGEDIR)/image-2.0.x.tsim
DSUIMAGE = $(IMAGEDIR)/image-2.0.x.dsu
TMPDIR = $(IMAGEDIR)/progs
FLASHIMAGE = $(IMAGEDIR)/image-2.0.x.flash
FLASHBZIMAGE = $(IMAGEDIR)/image-2.0.x.flashbz
COMONDIR = $(ROOTDIR)/vendors/gaisler/common
COMONLIBDIR = $(COMONDIR)/lib
COMONLIB = $(COMONDIR)/lib/boot.a

CC=$(CROSS_COMPILE)gcc -I../common -I$(ROOTDIR)/$(LINUXDIR)/include -I. -D__KERNEL__ -Wall -mno-fpu -nostdinc -iwithprefix include -O2

LIBGCC_DIR = $(dir $(shell $(CROSS_COMPILE)gcc $(CFLAGS) -print-libgcc-file-name))

DIRS = 

ROMFS_DIRS = \
	bin \
	dev dev/flash dev/pts \
	etc etc/config etc/init.d etc/default \
	home home/nfs home/httpd home/httpd/cgi-bin \
	lib lib/modules \
	mnt \
	proc \
	sbin \
	usr usr/bin usr/sbin \
	var sys


ifdef CONFIG_ROMFS_FS
FSIMG = $(ROMFSIMG)
endif

DEVICES = \
	ram,c,1,1 	null,c,1,3	tty1,c,4,1	tty2,c,4,2 \
	console,c,4,64	\
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	tty0,c,4,0     tty1,c,4,1         tty2,c,4,2       tty3,c,4,3 \
	ttyS0,c,4,64   ttyS1,c,4,65 \
	\
	ttyp0,c,3,0    ttyp1,c,3,1        ttyp2,c,3,2      ttyp3,c,3,3 \
	ttyp4,c,3,4    ttyp5,c,3,5        ttyp6,c,3,6      ttyp7,c,3,7 \
	ttyp8,c,3,8    ttyp9,c,3,9        ttypa,c,3,10     ttypb,c,3,11 \
	ttypc,c,3,12   ttypd,c,3,13       ttype,c,3,14     ttypf,c,3,15 \



all:
	dirs=$(DIRS) ; \
	for i in $$dirs ; do  make -C $$i || exit $? ; done

clean:
	-dirs=$(DIRS) ; \
	for i in $$dirs; do [ ! -d $$i ] || make -C $$i clean; done
	rm -rf *.o
	rm -f flashbzpiggy.o flashbzpiggy flashbzpiggy.gz piggy.o piggy piggy.flash
	make -C $(COMONLIBDIR) clean

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	for i in $(DEVICES); do \
		touch $(ROMFSDIR)/dev/@$$i; \
	done
	dirs=$(DIRS) ; for i in $$dirs; do  make -C $$i romfs || exit $? ; done
	cp $(ROOTDIR)/vendors/gaisler/leon2/rcS $(ROMFSDIR)/etc/init.d/rcS
	cp $(ROOTDIR)/vendors/gaisler/leon2/rc $(ROMFSDIR)/etc/rc
	chmod a+rwx $(ROMFSDIR)/etc/init.d/rcS
	chmod a+rwx $(ROMFSDIR)/etc/rc

ifdef CONFIG_ROMFS_FS
	$(ROMFSINST) -s /var/tmp /tmp
endif
	$(ROMFSINST) -s /var/tmp/log /dev/log
	#echo "ttyS0::respawn:/sbin/getty 38400 -" >> $(ROMFSDIR)/etc/inittab
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	date '+%Y.%m.%d-%T' > $(ROMFSDIR)/etc/build-date


$(ROMFSIMG):
	#cp $(ROOTDIR)/user/sash/sh.2  $(ROMFSDIR)/bin/sh
	#chmod a+rwx $(ROMFSDIR)/bin/sh
	[ -d $(TMPDIR) ] || mkdir -p $(TMPDIR)
	for i in `find $(ROMFSDIR)`; \
	do \
	if [ -f "$$i" -a ! -h "$$i" ]; then \
		$(CROSS_COMPILE)objdump -h $$i 2>&1 | grep -q .text ;\
		if [ $$? -eq 0 ]; then \
			make TARGET_FILE=$$i Convert; \
		fi ;\
	fi ;\
done
	#sh ./convertflatfs $(ROMFSDIR)
	-$(STRIP) $(ROMFSDIR)/home/httpd/cgi-bin/*
	-$(STRIP) $(ROMFSDIR)/sbin/*
	-$(STRIP) $(ROMFSDIR)/bin/*
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	#$(TOOLS)/cksum -b -o 2 $(ROMFSIMG) >> $(ROMFSIMG)
	#gzip -f $(ROMFSIMG)
	#$(CROSS_COMPILE)ld -r -b binary $(ROMFSIMG).gz -o $(ROMFSIMGOBJ)


Convert:
	echo "Converting "$(TARGET_FILE)" executable"
	#$(CROSS_COMPILE)ld -r -d -o $(TMPDIR)/$(notdir $(TARGET_FILE)).elf2flt $(TARGET_FILE)
	#$(CROSS_COMPILE)ld -T elf2flt.ld -Ur -o $(TMPDIR)/$(notdir $(TARGET_FILE)).elf $(TMPDIR)/$(notdir $(TARGET_FILE)).elf2flt
	#$(CROSS_COMPILE)ld -T elf2flt.ld -o $(TMPDIR)/$(notdir $(TARGET_FILE)).gdb $(TMPDIR)/$(notdir $(TARGET_FILE)).elf2flt     
	#$(CROSS_COMPILE)objdump -d $(TMPDIR)/$(notdir $(TARGET_FILE)).gdb >$(TMPDIR)/$(notdir $(TARGET_FILE)).dis
	#elf2flt -o $(TARGET_FILE) -r $(TMPDIR)/$(notdir $(TARGET_FILE)).elf
	chmod a+rwx $(TARGET_FILE)


###################################################################################
# dummy rule

genrom: 
	#cp $(ROOTDIR)/user/sash/sh.2  $(RFDIR)/bin/sh
	#chmod a+rwx $(RFDIR)/bin/sh
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG).2 -d $(RFDIR)
	ETEXT=`$(CROSS_COMPILE)objdump --headers $(LINUXIMG) | \
	  grep .text` ; \
	ADDR=`set -- $${ETEXT} ; echo 0x$${3}` ; \
	$(CROSS_COMPILE)objcopy --add-section=.romfs=$(ROMFSIMG).2 \
	  --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
	  --set-section-flags=.romfs=alloc,load,data   \
	  $(LINUXIMG) $(TSIMIMAGE).2 2> /dev/null

###################################################################################
# image creation

.PHONY: image $(FSIMG)
image: $(TSIMIMAGE) $(DSUIMAGE) $(FLASHIMAGE) $(FLASHBZIMAGE) dis

$(LINUXIMG): $(FSIMG)
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	cp $(ROOTDIR)/$(LINUXDIR)/$(LINUXFILE) $(LINUXIMG)

dis: $(LINUXIMG)
	$(CROSS_COMPILE)objdump -d $(LINUXIMG) >$(LINUXIMG).dis
	$(CROSS_COMPILE)objdump -d $(FLASHIMAGE) >$(FLASHIMAGE).dis
	$(CROSS_COMPILE)objdump -d $(FLASHBZIMAGE) >$(FLASHBZIMAGE).dis
	$(CROSS_COMPILE)objdump -d $(TSIMIMAGE) >$(TSIMIMAGE).dis
	$(CROSS_COMPILE)objdump -d $(DSUIMAGE) >$(DSUIMAGE).dis

###################################################################################
# tsim loadable image: tsimlocore.S in rom, rest in ram 

$(TSIMIMAGE): tsim.lds.o tsimlocore.o $(LINUXIMG) $(ROMFSIMGOBJ) 
	$(CROSS_COMPILE)ld -X -T $^ -o $@ #-L$(LIBGCC_DIR) -lgcc

###################################################################################
# dsu loadable image: locore_stage1.o at beginning of ram, rest in ram 

$(DSUIMAGE): dsu.lds.o locore_stage1.o $(LINUXIMG) $(ROMFSIMGOBJ) 
	$(CROSS_COMPILE)ld -X -T $^ -o $@ #-L$(LIBGCC_DIR) -lgcc

###################################################################################
# flash loadable image: .text in rom, .data in ram, copies .data to ram at bootup 

$(FLASHIMAGE): piggy.flash flash.romonly.lds.o locore.o
	echo "Note: The flash image is for hardware only. "
	echo "Note: If you want to build a tsim flashloader, remove the BZIMAGE flag (otherwise no sram flag will cause problems)"
	echo "Note: if you download with dsumon use the -nosram switch if you configured for sdram "
	$(CROSS_COMPILE)objcopy --only-section .data -O binary piggy.flash piggy
	$(CROSS_COMPILE)ld -r -b binary -o piggy.o piggy
	$(CROSS_COMPILE)ld -X -T flash.romonly.lds.o locore.o $(LINUXIMG) $(ROMFSIMGOBJ)  -o $(FLASHIMAGE) -M #-L$(LIBGCC_DIR) -lgcc

piggy.flash: flash.lds.o locore.o $(LINUXIMG) $(ROMFSIMGOBJ) 
	$(CROSS_COMPILE)ld -X -T $^  -o $@ #-L$(LIBGCC_DIR) -lgcc

$(COMONDIR)/piggy: $(DSUIMAGE) 
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S  $(DSUIMAGE) $(COMONDIR)/piggy


###################################################################################
# flashbz compressed loadable image: .text in ram, .data in ram, decompresses to ram at bootup 

$(FLASHBZIMAGE): flashbz.lds.o bzlocore.o  bzmain.o $(COMONLIB) flashbzpiggy.o 
	$(CROSS_COMPILE)ld -X -T $^ -o $@ -M

flashbz.lds.o: flashbz.lds.S flashbzpiggy 
	$(CROSS_COMPILE)gcc -E -C -P -DDATA_START=$(call _GETSIZE,flashbzpiggy)  -DPIGGY="flashbzpiggy.o" flashbz.lds.S -o flashbz.lds.o


$(COMONLIB): $(COMONDIR)/lib/_put.c $(COMONDIR)/lib/_gunzip.c $(COMONDIR)/lib/inflate.c
	make -C $(COMONLIBDIR)


flashbzpiggy: $(DSUIMAGE) 
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S  $(DSUIMAGE) flashbzpiggy

flashbzpiggy.gz: flashbzpiggy 
	gzip -f -9 < flashbzpiggy > flashbzpiggy.gz

flashbzpiggy.o:  flashbzpiggy.gz 
	$(CROSS_COMPILE)ld -r -b binary -o flashbzpiggy.o flashbzpiggy.gz
	$(CROSS_COMPILE)size flashbzpiggy.o


$(ROMFSIMGOBJ): $(ROMFSIMG)
	$(CROSS_COMPILE)ld -r -b binary $(ROMFSIMG) -o $(ROMFSIMGOBJ)


###################################################################################

locore_stage1.o: locore_stage1.S ../common/bootloader.h
	$(CROSS_COMPILE)gcc -DBZIMAGE -D__ASSEMBLY__ -I$(ROOTDIR)/$(LINUXDIR)/include -c -o $@ $<

locore.o: locore.S ../common/bootloader.h
	$(CROSS_COMPILE)gcc -DBZIMAGE -D__ASSEMBLY__ -I$(ROOTDIR)/$(LINUXDIR)/include -c -o $@ $<

bzlocore.o: bzlocore.S ../common/bootloader.h
	$(CROSS_COMPILE)gcc -DBZIMAGE -D__ASSEMBLY__ -I$(ROOTDIR)/$(LINUXDIR)/include -c -o $@ $<

tsimlocore.o: tsimlocore.S ../common/bootloader.h
	$(CROSS_COMPILE)gcc -DBZIMAGE -D__ASSEMBLY__ -I$(ROOTDIR)/$(LINUXDIR)/include -c -o $@ $<



$(COMONDIR)/prom_stage1.S: $(COMONDIR)/prom_config.h


###################################################################################

dsu.lds.o: dsu.lds.S
	$(CROSS_COMPILE)gcc -E -C -P -DRDIMAGE="$(ROMFSIMGOBJ)" -DLINUXIMAGE="$(LINUXIMG)" -I$(ROOTDIR)/$(LINUXDIR)/include dsu.lds.S -o dsu.lds.o

tsim.lds.o: tsim.lds.S
	$(CROSS_COMPILE)gcc -E -C -P -DRDIMAGE="$(ROMFSIMGOBJ)" -DLINUXIMAGE="$(LINUXIMG)" -I$(ROOTDIR)/$(LINUXDIR)/include tsim.lds.S -o tsim.lds.o

flash.lds.o: flash.lds.S
	$(CROSS_COMPILE)gcc -E -C -P -DRDIMAGE="$(ROMFSIMGOBJ)" -DLINUXIMAGE="$(LINUXIMG)" -I$(ROOTDIR)/$(LINUXDIR)/include flash.lds.S -o flash.lds.o

flash.romonly.lds.o: flash.romonly.lds.S
	$(CROSS_COMPILE)gcc -E -C -P -DDATAIMAGE="piggy.o"  -DRDIMAGE="$(ROMFSIMGOBJ)" -DLINUXIMAGE="$(LINUXIMG)" -I$(ROOTDIR)/$(LINUXDIR)/include flash.romonly.lds.S -o flash.romonly.lds.o

###################################################################################

_GETSIZE = $(shell ls -la $(1) | awk '{print $$5}')
