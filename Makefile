all: main.o bitmap.o dct.o test.o
	gcc -Wall -std=c99 -o main main.o bitmap.o test.o dct.o

dct.o: dct.c dct.h
	gcc -Wall -std=c99 -c dct.c

main.o: main.c bitmap.h
	gcc -Wall -std=c99 -c main.c

bitmap.o: bitmap.c bitmap.h
	gcc -Wall -std=c99 -c bitmap.c

test.o: test.c
	gcc -Wall -std=c99 -c test.c

run:
	./main

clean:
	rm -f *.o main