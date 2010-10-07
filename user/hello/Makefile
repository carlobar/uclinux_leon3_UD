EXEC = hello
OBJS = hello.o

all: $(EXEC)

$(EXEC) : $(OBJS)

	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	@echo = $(LDFLAGS)
	@echo = $(LDLIBS)

romfs:
	$(ROMFSINST)    /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
