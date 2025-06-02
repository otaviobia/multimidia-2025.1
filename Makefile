all: main.o bitmap.o dct.o test.o codec.o
	gcc -Wall -std=c99 -o main main.o bitmap.o test.o dct.o codec.o -lm

dct.o: dct.c dct.h
	gcc -Wall -std=c99 -c dct.c

main.o: main.c bitmap.h
	gcc -Wall -std=c99 -c main.c

bitmap.o: bitmap.c bitmap.h
	gcc -Wall -std=c99 -c bitmap.c

test.o: test.c test.h
	gcc -Wall -std=c99 -c test.c

codec.o: codec.c codec.h
	gcc -Wall -std=c99 -c codec.c

run:
	./main

clean:
	rm -f *.o main *.exe *.bmp