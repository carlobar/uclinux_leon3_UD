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

L_OBJS   := ps2_keyboard.o
L_TARGET := gaisler_ps2.a

include $(TOPDIR)/Rules.make

CFLAGS += -I ../../../../drivers/char