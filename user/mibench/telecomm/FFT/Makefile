OBJ = main.o fftmisc.o  fourierf.o
FILE = main.c fftmisc.c  fourierf.c
CFLAGS = -static  -v  -lc -lgcc
EXEC=fft


all:$(EXEC)

$(EXEC): ${OBJ} Makefile

	$(CC) $(LDFLAGS) ${CFLAGS} $(OBJ) -o $@  -lm $(LDLIBS) -lm
	#$(CC) $(LDFLAGS) $(CFLAGS) $(OBJ) -o $@ $(LDLIBS) -lm
fftmisc.o: fftmisc.c
	$(CC) $(LDFLAGS) ${CFLAGS} -c fftmisc.c
fourierf.o: fourierf.c
	$(CC) $(LDFLAGS) ${CFLAGS} -c fourierf.c
main.o: main.c
	$(CC) $(LDFLAGS) ${CFLAGS} -c main.c


clean:
	rm -rf $(EXEC) *.elf* *.gdb *.o 


romfs: 
	$(ROMFSINST)	/bin/fft
