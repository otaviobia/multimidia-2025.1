all: main.o bitmap.o
	gcc -Wall -std=c99 -o main main.o bitmap.o

main.o: main.c bitmap.h
	gcc -Wall -std=c99 -c main.c

bitmap.o: bitmap.c bitmap.h
	gcc -Wall -std=c99 -c bitmap.c

run:
	./main

clean:
	rm -f *.o main