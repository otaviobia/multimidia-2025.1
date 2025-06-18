# Compila o compressor e o decompressor
all:
	gcc -o compressor compressor.c utils/*.c -Wall -std=c99 -lm
	gcc -o decompressor decompressor.c utils/*.c -Wall -std=c99 -lm

# Remove arquivos gerados na execução
clean:
	rm -f compressor decompressor *.o *.exe *.bmp *.bin