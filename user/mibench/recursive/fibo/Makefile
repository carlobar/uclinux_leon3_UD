FILE=fibo.c
EXEC=fibo

all:$(EXEC)

$(EXEC): fibo.c Makefile
	$(CC) $(LDFLAGS) -static -o $@ $(FILE)  $(LDLIBS) -lm

clean: rm -rf fibo output* *.o *.elf* *.flash *.bin *.out *.srec

romfs: 
	$(ROMFSINST) 	/bin/fibo
