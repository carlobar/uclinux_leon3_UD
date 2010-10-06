#
# Makefile for the AMBA bus specific drivers.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definition is now inherited from the
# parent makefile.
#

SUB_DIRS += apbuart 

ifdef CONFIG_GRLIB_GAISLER_GRETH
SUB_DIRS += greth
endif
ifdef CONFIG_GRLIB_GAISLER_VGA
SUB_DIRS += vga
endif
ifdef CONFIG_GRLIB_GAISLER_PS2
SUB_DIRS += ps2
endif

include $(TOPDIR)/Rules.make

