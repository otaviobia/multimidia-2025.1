#ifndef CODEC_H
    #define CODEC_H

    #include "bitmap.h"

    // Estrutura que representa um bloco de 8x8 pixels da luminância (Y)
    typedef struct {
        float Y[8][8];
    } BLOCOY;

    // Estrutura que representa um bloco de 8x8 pixels da crominância (Cb ou Cr)
    typedef struct {
        float C[8][8];
    } BLOCOCROMINANCIA;

    // Estrutura que representa um macrobloco de 16x16 pixels, contendo 4 blocos de Y e 1 bloco de Cb e Cr
    // Essa quantidade de blocos é o que caracteriza uma subamostragem 4:2:0
    typedef struct {
        BLOCOY Y[4];
        BLOCOCROMINANCIA Cb;
        BLOCOCROMINANCIA Cr;
    } MACROBLOCO;

    MACROBLOCO* encodeImageYCbCr(PIXELYCBCR *image, int width, int height, int *out_macroblock_count);
    void decodeImageYCbCr(MACROBLOCO *mb_array, PIXELYCBCR *dst, int width, int height);
#endif