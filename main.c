#include <stdlib.h>
#include <math.h>


#include "bitmap.h"
#include "dct.h"
#include "test.h"

int main(void)
{
    /*
     * Programa de exemplo para verificar as funcionalidades do arquivo bitmap.c.
     * O programa lê um arquivo BMP, troca os canais R e B dos pixels, e escreve o resultado
     * em um novo arquivo BMP.
     */
    FILE *input;
    if (!(input = fopen("images/colors.bmp", "rb"))) {
        printf("Error: could not open input file." );
        exit(1);
    }

    // Carrega os cabeçalhos do arquivo BMP nas estruturas FileHeader e InfoHeader
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER InfoHeader;
    loadBMPHeaders(input, &FileHeader, &InfoHeader);

    // Aloca memória para o vetor de pixels (tam = largura * altura)
    PIXELRGB *Pixels;
    PIXELRGB *Pixelsrec;
    int tam = InfoHeader.Width * InfoHeader.Height;
    Pixels = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));
    Pixelsrec = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));

    // Lê os pixels do arquivo BMP e armazena no vetor de pixels
    readPixels(input, InfoHeader, Pixels);
        
    FILE *output;
    if (!(output = fopen("out.bmp", "wb"))) {
        printf("Error: could not open output file." );
        exit(1);
    }

    /* Troca os canais R e B dos pixels
    unsigned char aux_;
    for (int i = 0; i < tam; i++) {
        aux_ = Pixels[i].R;
        Pixels[i].R = Pixels[i].B;
        Pixels[i].B = aux_;
    }*/

    // Converte os pixels de RGB para YCbCr
    PIXELYCBCR *PixelsYCbCr;
    PixelsYCbCr = (PIXELYCBCR *) malloc(tam * sizeof(PIXELYCBCR));
    
    convertToYCBCR(Pixels, PixelsYCbCr, tam);

    // Volta os pixels de YCbCr para RGB
    convertToRGB(PixelsYCbCr, Pixelsrec, tam);
    compareRGB(Pixels, Pixelsrec, tam);

    // Escreve os cabeçalhos e os pixels no arquivo de saída
    writeBMP(output, FileHeader, InfoHeader, Pixels);


    //bloco ficticio

    float block[8][8] = {
        {52, 55, 61, 66, 70, 61, 64, 73},
        {63, 59, 55, 90, 109, 85, 69, 72},
        {62, 59, 68, 113, 144, 104, 66, 73},
        {63, 58, 71, 122, 154, 106, 70, 69},
        {67, 61, 68, 104, 126, 88, 68, 70},
        {79, 65, 60, 70, 77, 68, 58, 75},
        {85, 71, 64, 59, 55, 61, 65, 83},
        {87, 79, 69, 68, 65, 76, 78, 94}
    };

    float Dctfrequencies[8][8];
    float reconstructedBlock[8][8];

    forwardDCT(block, Dctfrequencies);
    inverseDCT(Dctfrequencies, reconstructedBlock);

    // Funcao Teste , talvez dps colocar no .test (sumarizado)
    printf("Original Block vs Reconstructed Block:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("Original: %6.2f, DCT: %6.2f, Reconstructed: %6.2f, Diff: %6.2f\n",
                block[i][j], Dctfrequencies[i][j], reconstructedBlock[i][j], fabs(block[i][j] - reconstructedBlock[i][j]));
        }
    }

    // Libera a memória alocada e fecha os arquivos
    free(Pixels);
    free(PixelsYCbCr);
    free(Pixelsrec);
    fclose(input);
    fclose(output);
    exit(0);

    return 0;
}
