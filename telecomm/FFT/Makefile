OBJ = main.o fftmisc.o  fourierf.o
FILE = main.c fftmisc.c  fourierf.c
CFLAGS = -static -O3 -g

fft: ${OBJ} Makefile
	clang ${PARAMS}   ${CFLAGS} ${OBJ} -o fft -lm
fftmisc.o: fftmisc.c
	clang ${PARAMS}  ${CFLAGS} -c fftmisc.c
fourierf.o: fourierf.c
	clang ${PARAMS}  ${CFLAGS} -c fourierf.c
main.o: main.c
	clang ${PARAMS}  ${CFLAGS} -c main.c

clean:
	rm -rf *.o fft output*
