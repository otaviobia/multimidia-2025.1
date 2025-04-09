all:
	gcc bitmap.c -o bitmap.o -std=c99 -Wall
run:
	./bitmap.o
