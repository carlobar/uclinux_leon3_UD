EXEC = susan
OBJS = susan.o

all: $(EXEC)

$(EXEC): $(OBJS) Makefile

	#gcc -static -O4 -o susan susan.c -lm 

	$(CC) $(LDFLAGS) -static -o $@ $(OBJS) $(LDLIBS) -lm 

clean:
	rm -rf $(EXEC) *.elf *.gdb *.o 


romfs: 
	$(ROMFSINST)	/bin/susan