all: main.o bitmap.o dct.o
	gcc -Wall -std=c99 -o main main.o bitmap.o dct.o

dct.o: dct.c dct.h
	gcc -Wall -std=c99 -c dct.c

main.o: main.c bitmap.h
	gcc -Wall -std=c99 -c main.c

bitmap.o: bitmap.c bitmap.h
	gcc -Wall -std=c99 -c bitmap.c

run:
	./main

clean:
	rm -f *.o main