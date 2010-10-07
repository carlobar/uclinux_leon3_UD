FILE = bitcnt_1.c bitcnt_2.c bitcnt_3.c bitcnt_4.c  bitfiles.c bitstrng.c bstr_i.c 
#bitcnts.c
EXEC = bitcnts
OBJS = bitcnts.o

all:$(EXEC)

$(EXEC): ${OBJS} Makefile
	$(CC) $(LDFLAGS) -static ${FILE} -o $@ $(OBJS) $(LDLIBS) -lm

clean:
	rm -rf $(EXEC) *.elf *.gdb *.o 

romfs:
	$(ROMFSINST) /bin/bitcnts
