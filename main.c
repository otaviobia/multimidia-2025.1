#include <stdio.h>
#include <stdlib.h>

#include "bitmap.h"
       
int main(void)
{
    /*
     * Programa de exemplo para verificar as funcionalidades do arquivo bitmap.c.
     * O programa lê um arquivo BMP, troca os canais R e B dos pixels, e escreve o resultado
     * em um novo arquivo BMP.
     */
    FILE *input;
    if (!(input = fopen("colors.bmp", "rb"))) {
        printf("Error: could not open input file." );
        exit(1);
    }

    // Carrega os cabeçalhos do arquivo BMP nas estruturas FileHeader e InfoHeader
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER InfoHeader;
    loadBMPHeaders(input, &FileHeader, &InfoHeader);

    // Aloca memória para o vetor de pixels (tam = largura * altura)
    PIXEL *Pixels;
    int tam = InfoHeader.Width * InfoHeader.Height;
    Pixels = (PIXEL *) malloc(tam * sizeof(PIXEL));

    // Lê os pixels do arquivo BMP e armazena no vetor de pixels
    readPixels(input, InfoHeader, Pixels);
        
    /* Brincadeira com RGB: Troca os canais R e B */
    FILE *output;
    if (!(output = fopen("out.bmp", "wb"))) {
        printf("Error: could not open output file." );
        exit(1);
    }

    // Troca os canais R e B dos pixels
    unsigned char aux_;
    for (int i = 0; i < tam; i++) {
        aux_ = Pixels[i].R;
        Pixels[i].R = Pixels[i].B;
        Pixels[i].B = aux_;
    }

    // Escreve os cabeçalhos e os pixels no arquivo de saída
    writeBMP(output, FileHeader, InfoHeader, Pixels);

    // Libera a memória alocada e fecha os arquivos
    free(Pixels);
    fclose(input);
    fclose(output);
    exit(0);

    return 0;
}
