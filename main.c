#include <stdlib.h>
#include <stdio.h>

#include "bitmap.h"
#include "codec.h"
#include "dct.h"
#include "test.h"

int main(void) {
    /* COMPRESSÃO */

    float quality = 50; // Qualidade da quantização (50 é o padrão)

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
    if (width % 8 != 0 || height % 8 != 0) {
        printf("Error: image dimensions must be multiples of 8.\n");
        fclose(input);
        return 1;
    }
    int tam = width * height;

    // 4. Aloca memória para os pixels de entrada e saída e lê os pixels do arquivo BMP
    PIXELRGB *Pixels = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));
    PIXELRGB *PixelsOut = (PIXELRGB *) malloc(tam * sizeof(PIXELRGB));
    readPixels(input, InfoHeader, Pixels);
    fclose(input); // arquivo de entrada não é mais necessário

    // 5. Converte os pixels de RGB para YCbCr
    PIXELYCBCR *PixelsYCbCr = (PIXELYCBCR *) malloc(tam * sizeof(PIXELYCBCR));
    convertToYCBCR(Pixels, PixelsYCbCr, tam);

    //testImageWithoutDCT(PixelsYCbCr, width, height, FileHeader, InfoHeader);

    //testImageSubsampling(PixelsYCbCr, width, height);

    // 6. Aplica a DCT e retorna os macroblocos de 16x16
    // Essa função também faz a subamostragem 4:2:0
    int macroblock_count = 0;
    MACROBLOCO *macroblocks = encodeImageYCbCr(PixelsYCbCr, width, height, &macroblock_count);
    printf("Number of macroblocks: %d\n", macroblock_count);

    // 7. Aplica a quantização nos macroblocos
    quantizeMacroblocks(macroblocks, macroblock_count, quality);

    // 8. Aplica vetorização zig-zag
    MACROBLOCO_VETORIZADO* vectorized_macroblocks = (MACROBLOCO_VETORIZADO *) malloc(macroblock_count * sizeof(MACROBLOCO_VETORIZADO));
    vectorize_macroblocks(macroblocks, vectorized_macroblocks, macroblock_count);

    // 9. Aplica codificação por carreira para os coeficientes AC e diferencial para os coeficientes DC
    MACROBLOCO_RLE_DIFERENCIAL* rle_diff_macroblocks = (MACROBLOCO_RLE_DIFERENCIAL *) malloc(macroblock_count * sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    rle_encode_macroblocks(rle_diff_macroblocks, vectorized_macroblocks, macroblock_count);
    differential_encode_dc(rle_diff_macroblocks, macroblock_count);

    // Teste 1: Codificação DC por categoria
    testDCCategoryEncoding(rle_diff_macroblocks, macroblock_count);
    
    // Teste 2: Codificação AC por categoria
    testACCategoryEncoding(rle_diff_macroblocks, macroblock_count);
    
    // Teste 3: Teste de roundtrip (codifica e decodifica para verificar reversibilidade)
    testCategoryEncodingRoundtrip(rle_diff_macroblocks, macroblock_count);

    testBitBufferSimple();
    testBitBufferExtensive();
    testHuffmanRoundtrip();

    /* DESCOMPRESSÃO */
    // 1. Desaplica codificação por diferencial e por carreira
    MACROBLOCO_VETORIZADO* new_vectorized_macroblocks = (MACROBLOCO_VETORIZADO *) malloc(macroblock_count * sizeof(MACROBLOCO_VETORIZADO));
    differential_decode_dc(rle_diff_macroblocks, macroblock_count);
    rle_decode_macroblocks(new_vectorized_macroblocks, rle_diff_macroblocks, macroblock_count);

    // 2. Desvetoriza os macroblocos
    MACROBLOCO *new_macroblocks = (MACROBLOCO *)malloc(macroblock_count * sizeof(MACROBLOCO));
    devectorize_macroblocks(new_vectorized_macroblocks, new_macroblocks, macroblock_count);

    // 3. Aplica a dequantização nos macroblocos
    dequantizeMacroblocks(new_macroblocks, macroblock_count, quality);

    // 4. Gera a imagem YCbCr a partir dos macroblocos
    PIXELYCBCR *DecodedYCbCr = (PIXELYCBCR *) malloc(tam * sizeof(PIXELYCBCR));
    decodeImageYCbCr(new_macroblocks, DecodedYCbCr, width, height);

    // 5. Converte os pixels de YCbCr para RGB
    convertToRGB(DecodedYCbCr, PixelsOut, tam);

    // 6. Abre e escreve o arquivo BMP de saída
    FILE *output = fopen("out.bmp", "wb");
    if (!output) {
        printf("Error: could not open output file.\n");
        return 1;
    }
    writeBMP(output, FileHeader, InfoHeader, PixelsOut);
    saveChannelImages(DecodedYCbCr, width, height, FileHeader, InfoHeader);
    fclose(output);

    // Teste final: Compara a imagem original com a reconstituída
    compareRGB(Pixels, PixelsOut, tam);
    // Libera a memória alocada
    free(Pixels);
    free(PixelsOut);
    free(PixelsYCbCr);
    free(DecodedYCbCr);
    free(macroblocks);
    free(vectorized_macroblocks);
    free(rle_diff_macroblocks);
    free(new_vectorized_macroblocks);
    free(new_macroblocks);

    // Sai do programa
    printf("Image processed and saved to out.bmp\n");
    return 0;
}
