/* Esse arquivo é responsável por separar a imagem em blocos, fazer a subamostragem 
 * da crominância usando 4:2:0 e aplicar a DCT sobre esses blocos. E o inverso. */
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "codec.h"
#include "dct.h"

// Clamp usado para fazer o padding se acessar um pixel fora da imagem
// Se o pixel estiver fora da imagem, retorna o pixel da borda
int padding_clamp(int val, int max) {
    return (val < max) ? val : max - 1;
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void extract_block_y(PIXELYCBCR *image, float block[8][8], int start_x, int start_y, int width, int height) {
    /*
     * Extrai um bloco 8x8 de Y (Luminância) da imagem YCbCr linearizada.
     *
     * Parâmetros:
     * image: imagem YCbCr linearizada em um vetor
     * block: bloco de floats 8x8 a ser preenchido
     * start_x, start_y: coordenada x e y do pixel inicial
     * width, height: largura e altura da imagem
     */
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int px = padding_clamp(start_x + x, width);
            int py = padding_clamp(start_y + y, height);
            int index = py * width + px;
            block[y][x] = (float)image[index].Y;
        }
    }
}

void extract_block_chroma420(PIXELYCBCR *image, float block[8][8], int start_x, int start_y, int width, int height, char channel) {
    /*
     * Extrai um bloco 8x8 (subamostrado) de Cb ou Cr da imagem YCbCr linearizada.
     *
     * Parâmetros:
     * image: imagem YCbCr linearizada em um vetor
     * block: bloco de floats 8x8 a ser preenchido
     * start_x, start_y: coordenada x e y do pixel inicial
     * width, height: largura e altura da imagem
     * channel: 'B' para Cb, 'R' para Cr
     */
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // A subamostragem é feita pegando a média dos 4 pixels 2x2
            float sum = 0;
            for (int dy = 0; dy < 2; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    int px = padding_clamp(start_x + x * 2 + dx, width);
                    int py = padding_clamp(start_y + y * 2 + dy, height);
                    int index = py * width + px;
                    sum += (channel == 'B') ? image[index].Cb : image[index].Cr;
                }
            }
            block[y][x] = sum / 4.0f;
        }
    }
}

void reconstructBlock8x8_Y(PIXELYCBCR *dst, float block[8][8], int start_x, int start_y, int width, int height) {
    /*
     * Reconstrói um bloco 8x8 de Y (Luminância) na imagem YCbCr linearizada.
     *
     * Parâmetros:
     * dst: imagem YCbCr linearizada a ser preenchida
     * block: bloco de floats 8x8 de Y
     * start_x, start_y: coordenada x e y do pixel inicial
     * width, height: largura e altura da imagem
     */
    for (int y = 0; y < 8; y++)
        for (int x = 0; x < 8; x++) {
            int px = padding_clamp(start_x + x, width);
            int py = padding_clamp(start_y + y, height);
            // utiliza-se clamping para evitar overflow
            dst[py * width + px].Y = (unsigned char)(clamp(block[y][x] + 0.5f, 0.0f, 255.0f));
        }
}

void reconstructBlock8x8_CbCr420(PIXELYCBCR *dst, float block[8][8], int start_x, int start_y, int width, int height, char channel) {
    /*
     * Reconstrói um bloco 8x8 de Cb ou Cr (que por ser subamostrado vai virar 16x16 pixels) na imagem YCbCr linearizada.
     *
     * Parâmetros:
     * dst: imagem YCbCr linearizada a ser preenchida
     * block: bloco de floats 8x8 subamostrados de Cb ou Cr
     * start_x, start_y: coordenada x e y do pixel inicial
     * width, height: largura e altura da imagem
     * channel: 'B' para Cb, 'R' para Cr
     */
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int px = padding_clamp(start_x + x, width);
            int py = padding_clamp(start_y + y, height);
            // utiliza-se clamping para evitar overflow
            unsigned char val = (unsigned char)(clamp(block[y / 2][x / 2] + 0.5f, 0.0f, 255.0f));
            PIXELYCBCR *pix = &dst[py * width + px];
            if (channel == 'B') pix->Cb = val;
            else pix->Cr = val;
        }
    }
}

MACROBLOCO* encodeImageYCbCr(PIXELYCBCR *image, int width, int height, int *out_macroblock_count) {
    /*
     * Dado uma imagem YCbCr linearizada, aplica DCT em blocos de 16x16 pixels.
     * Cada macrobloco contém 4 blocos de Y (8x8) e 1 bloco de Cb e Cr (8x8 cada).
     * O que caracteriza uma subamostragem 4:2:0.
     * 
     * Parâmetros:
     * image: imagem YCbCr linearizada em um vetor
     * width, height: largura e altura da imagem
     * out_macroblock_count: ponteiro para armazenar o número de macroblocos
     */
    int mb_cols = (width + 15) / 16;
    int mb_rows = (height + 15) / 16;
    int num_blocks = mb_cols * mb_rows;

    *out_macroblock_count = num_blocks;

    // Aloca vetor de macroblocos
    MACROBLOCO *macroblocks = (MACROBLOCO *) malloc(num_blocks * sizeof(MACROBLOCO));
    if (!macroblocks) {
        return NULL;
    }

    int mb_index = 0;

    // Para cada macrobloco 16x16, extrai os blocos 8x8 e aplica DCT
    for (int by = 0; by < height; by += 16) {
        for (int bx = 0; bx < width; bx += 16) {
            MACROBLOCO *mb = &macroblocks[mb_index++];

            // Extrai e aplica DCT para os 4 blocos Y
            for (int i = 0; i < 4; i++) {
                int ox = bx + (i % 2) * 8;
                int oy = by + (i / 2) * 8;

                float y_temp[8][8];
                extract_block_y(image, y_temp, ox, oy, width, height);
                forwardDCTMatrix(y_temp, mb->Y[i].Y);
            }

            // Extrai e aplica DCT para os blocos Cb e Cr
            float cb_temp[8][8], cr_temp[8][8];
            extract_block_chroma420(image, cb_temp, bx, by, width, height, 'B');
            extract_block_chroma420(image, cr_temp, bx, by, width, height, 'R');
            
            forwardDCTMatrix(cb_temp, mb->Cb.C);
            forwardDCTMatrix(cr_temp, mb->Cr.C);
        }
    }

    return macroblocks;
}

float quantization_matrix_y[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68,109,103,77},
    {24,35,55,64,81,104,113,92},
    {79,64,78,87,103,121,120,101},
    {72,92 ,95 ,98 ,112 ,100 ,103 ,99}
};

float quantization_matrix_chroma[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99 ,99 ,99 ,99 ,99 ,99 ,99},
    {99 ,99 ,99 ,99 ,99 ,99 ,99 ,98}
};

void quantizeBlock(float block[8][8], float quantization_matrix[8][8], int compression_factor) {
    /*
     * Aplica a quantização em um bloco 8x8 usando uma matriz de quantização.
     *
     * Parâmetros:
     * block: bloco 8x8 a ser quantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Aplica a quantização
            block[y][x] = round(block[y][x] / (quantization_matrix[y][x] * compression_factor));
        }
    }
}

void dequantizeBlock(float block[8][8], float quantization_matrix[8][8], int compression_factor) {
    /*
     * Aplica a dequantização em um bloco 8x8 usando uma matriz de quantização.
     *
     * Parâmetros:
     * block: bloco 8x8 a ser dequantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            // Aplica a dequantização
            block[y][x] *= (quantization_matrix[y][x] * compression_factor);
        }
    }
}

void quantizeMacroblock(MACROBLOCO *mb, int compression_factor) {
    /*
     * Aplica a quantização em um macrobloco 16x16.
     *
     * Parâmetros:
     * mb: macrobloco a ser quantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < 4; i++) {
        quantizeBlock(mb->Y[i].Y, quantization_matrix_y, compression_factor);
    }
    quantizeBlock(mb->Cb.C, quantization_matrix_chroma, compression_factor);
    quantizeBlock(mb->Cr.C, quantization_matrix_chroma, compression_factor);
}

void dequantizeMacroblock(MACROBLOCO *mb, int compression_factor) {
    /*
     * Aplica a dequantização em um macrobloco 16x16.
     *
     * Parâmetros:
     * mb: macrobloco a ser dequantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < 4; i++) {
        dequantizeBlock(mb->Y[i].Y, quantization_matrix_y, compression_factor);
    }
    dequantizeBlock(mb->Cb.C, quantization_matrix_chroma, compression_factor);
    dequantizeBlock(mb->Cr.C, quantization_matrix_chroma, compression_factor);
}

void quantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, int compression_factor) {
    /*
     * Aplica a quantização em um vetor de macroblocos.
     *
     * Parâmetros:
     * mb_array: vetor de macroblocos
     * macroblock_count: número de macroblocos
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < macroblock_count; i++) {
        quantizeMacroblock(&mb_array[i], compression_factor);
    }
}
void dequantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, int compression_factor) {
    /*
     * Aplica a dequantização em um vetor de macroblocos.
     *
     * Parâmetros:
     * mb_array: vetor de macroblocos
     * macroblock_count: número de macroblocos
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < macroblock_count; i++) {
        dequantizeMacroblock(&mb_array[i], compression_factor);
    }
}

void decodeImageYCbCr(MACROBLOCO *mb_array, PIXELYCBCR *dst, int width, int height) {
    /*
     * Dado um vetor de macroblocos, reconstrói a imagem YCbCr linearizada.
     *
     * Parâmetros:
     * mb_array: vetor de macroblocos
     * dst: imagem YCbCr linearizada a ser preenchida
     * width, height: largura e altura da imagem
     */
    int mb_width = (width + 15) / 16;
    int mb_height = (height + 15) / 16;
    int mb_index = 0;

    for (int y = 0; y < mb_height * 16; y += 16) {
        for (int x = 0; x < mb_width * 16; x += 16) {
            MACROBLOCO *mb = &mb_array[mb_index++];

            // Reconstrói os 4 blocos Y
            for (int i = 0; i < 4; i++) {
                int bx = x + (i % 2) * 8;
                int by = y + (i / 2) * 8;

                float rec[8][8] = {0};
                inverseDCTMatrix(mb->Y[i].Y, rec);
                reconstructBlock8x8_Y(dst, rec, bx, by, width, height);
            }

            // Reconstrói os blocos Cb e Cr
            float cb_rec[8][8] = {0}, cr_rec[8][8] = {0};
            inverseDCTMatrix(mb->Cb.C, cb_rec);
            inverseDCTMatrix(mb->Cr.C, cr_rec);

            reconstructBlock8x8_CbCr420(dst, cb_rec, x, y, width, height, 'B');
            reconstructBlock8x8_CbCr420(dst, cr_rec, x, y, width, height, 'R');
        }
    }
}