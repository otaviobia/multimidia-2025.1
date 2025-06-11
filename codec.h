#ifndef CODEC_H
    #define CODEC_H

    #include "bitmap.h"

    // Estrutura que representa um bloco de 8x8 pixels
    typedef struct {
        float block[8][8];
    } BLOCO;

    // Estrutura que representa uma vetorização de um bloco 8x8 utilizando zig-zag
    typedef struct {
        float vector[64];
    } VETORZIGZAG;

    typedef struct {
        VETORZIGZAG Y_vetor[4], Cb_vetor, Cr_vetor;
    } MACROBLOCO_VETORIZADO;

    // Estrutura que representa um macrobloco de 16x16 pixels, contendo 4 blocos de Y e 1 bloco de Cb e Cr
    // Essa quantidade de blocos é o que caracteriza uma subamostragem 4:2:0
    typedef struct {
        BLOCO Y[4], Cb, Cr;
    } MACROBLOCO;

    // Estrutura que representa um par do Run-length Encoding
    typedef struct {
        int zeros; // Número de zeros antes do coeficiente não-zero
        float valor;    // Valor do coeficiente não-zero (ou 0.0 para representar EOB)
    } PAR_RLE;

    typedef struct {
        float coeficiente_dc;
        PAR_RLE pares[64]; // Tamanho no pior caso
        int quantidade; // Quantidade real de pares
    } BLOCO_RLE;

    typedef struct {
        BLOCO_RLE Y_vetor[4], Cb_vetor, Cr_vetor;
    } MACROBLOCO_RLE;

    MACROBLOCO* encodeImageYCbCr(PIXELYCBCR *image, int width, int height, int *out_macroblock_count);
    void decodeImageYCbCr(MACROBLOCO *mb_array, PIXELYCBCR *dst, int width, int height);
    void extract_block_y(PIXELYCBCR *image, float block[8][8], int start_x, int start_y, int width, int height);
    void extract_block_chroma420(PIXELYCBCR *image, float block[8][8], int start_x, int start_y, int width, int height, char channel);
    void reconstructBlock8x8_Y(PIXELYCBCR *dst, float block[8][8], int start_x, int start_y, int width, int height);
    void reconstructBlock8x8_CbCr420(PIXELYCBCR *dst, float block[8][8], int start_x, int start_y, int width, int height, char channel);
    void quantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, float quality);
    void dequantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, float quality);
    void vectorize_macroblocks(MACROBLOCO *macroblocks, MACROBLOCO_VETORIZADO *vectorized_macroblocks, int macroblock_count);
    void devectorize_macroblocks(MACROBLOCO_VETORIZADO *vectorized_macroblocks, MACROBLOCO *macroblocks, int macroblock_count);
    void rle_encode_macroblocks(MACROBLOCO_RLE *rle_macroblocks, MACROBLOCO_VETORIZADO *vectorized_macroblocks, int macroblock_count);
    void rle_decode_macroblocks(MACROBLOCO_VETORIZADO *vectorized_macroblocks, MACROBLOCO_RLE *rle_macroblocks, int macroblock_count);
    void vectorize_block(float block[8][8], VETORZIGZAG *return_vector);
    void devectorize_block(VETORZIGZAG *vector, float block[8][8]);
#endif