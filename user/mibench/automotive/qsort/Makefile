EXEC = qsort_small
OBJS = qsort_small.o
#FILE2 = qsort_large.c



all: $(EXEC) # qsort_small qsort_large

#qsort_small: qsort_small.c Makefile

$(EXEC): ${OBJS} Makefile

	$(CC) $(LDFLAGS) -static -o $@ $(OBJS) $(LDLIBS) -lm 
	#gcc -static qsort_small.c -O3 -o qsort_small -lm
#qsort_large: qsort_large.c Makefile
#	gcc -static qsort_large.c -O3 -o qsort_large -lm

clean:
	rm -rf $(EXEC) *.elf *.gdb *.o 


romfs: 
	$(ROMFSINST) 	/bin/qsort_small