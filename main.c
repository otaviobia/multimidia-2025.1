#include <stdlib.h>
#include <stdio.h>

#include "bitmap.h"
#include "codec.h"
#include "dct.h"
#include "test.h"

int main(void) {
    /* COMPRESSÃO */
    // 1. Abre o arquivo BMP de entrada
    FILE *input = fopen("images/lenna.bmp", "rb");
    if (!input) {
        printf("Error: could not open input file.\n");
        return 1;
    }

    // 2. Carrega os cabeçalhos do arquivo BMP nas estruturas FileHeader e InfoHeader
    BITMAPFILEHEADER FileHeader;
    BITMAPINFOHEADER InfoHeader;
    loadBMPHeaders(input, &FileHeader, &InfoHeader);

    // 3. Calcula a quantidade de pixels da imagem
    int width = InfoHeader.Width;
    int height = InfoHeader.Height;
    int tam = width * height;

    // 4. Aloca memória para os pixels de entrada e saída e lê os pixels do arquivo BMP
    PIXELRGB *Pixels = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));
    PIXELRGB *PixelsOut = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));
    readPixels(input, InfoHeader, Pixels);
    fclose(input); // arquivo de entrada não é mais necessário

    // 5. Converte os pixels de RGB para YCbCr
    PIXELYCBCR *PixelsYCbCr = (PIXELYCBCR *) malloc(tam * sizeof(PIXELYCBCR));
    convertToYCBCR(Pixels, PixelsYCbCr, tam);

    // 6. Aplica a DCT e retorna os macroblocos de 16x16
    // Essa função também faz a subamostragem 4:2:0
    int macroblock_count = 0;
    MACROBLOCO *blocks = encodeImageYCbCr(PixelsYCbCr, width, height, &macroblock_count);
    printf("Number of macroblocks: %d\n", macroblock_count);

    /* DESCOMPRESSÃO */
    // 1. Gera a imagem YCbCr a partir dos macroblocos
    PIXELYCBCR *DecodedYCbCr = (PIXELYCBCR *) malloc(tam * sizeof(PIXELYCBCR));
    decodeImageYCbCr(blocks, DecodedYCbCr, width, height);

    // 2. Converte os pixels de YCbCr para RGB
    convertToRGB(DecodedYCbCr, PixelsOut, tam);

    // 3. Abre e escreve o arquivo BMP de saída
    FILE *output = fopen("out.bmp", "wb");
    if (!output) {
        printf("Error: could not open output file.\n");
        return 1;
    }
    writeBMP(output, FileHeader, InfoHeader, PixelsOut);
    fclose(output);

    // Libera a memória alocada
    free(Pixels);
    free(PixelsOut);
    free(PixelsYCbCr);
    free(DecodedYCbCr);
    free(blocks);

    // Sai do programa
    printf("Image processed and saved to out.bmp\n");
    return 0;
}
