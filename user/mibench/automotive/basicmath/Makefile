FILE =    rad2deg.c  cubic.c   isqrt.c

EXEC = basicmath_small
OBJS = basicmath_small.o

all: $(EXEC)

basicmath_small: ${OBJS} Makefile
	$(CC) $(LDFLAGS) -static ${FILE} -o $@ $(OBJS) $(LDLIBS) -lm

clean:
	rm -rf $(EXEC) *.elf *.gdb *.o

romfs:
	$(ROMFSINST) -e CONFIG_USER_MIBENCH_AUTO_BASICMATH /bin/basicmath_small
