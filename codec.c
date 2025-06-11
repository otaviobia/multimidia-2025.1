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
            block[y][x] = (float)image[index].Y - 128.0f; // Subtrai 128 para centralizar os valores
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
            float sum = 0;
            for (int dy = 0; dy < 2; dy++) {
                for (int dx = 0; dx < 2; dx++) {
                    int px = padding_clamp(start_x + x * 2 + dx, width);
                    int py = padding_clamp(start_y + y * 2 + dy, height);
                    int index = py * width + px;
                    sum += ((channel == 'B') ? image[index].Cb : image[index].Cr) - 128.0f; // Subtrai 128 para centralizar os valores
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
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int px = padding_clamp(start_x + x, width);
            int py = padding_clamp(start_y + y, height);
            dst[py * width + px].Y = (unsigned char)(clamp(block[y][x] + 128.5f, 0.0f, 255.0f)); // Adiciona 128 para reverter a centralização
        }
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
            unsigned char val = (unsigned char)(clamp(block[y / 2][x / 2] + 128.5f, 0.0f, 255.0f)); // Adiciona 128 para reverter a centralização
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
                forwardDCTMatrix(y_temp, mb->Y[i].block);
            }

            // Extrai e aplica DCT para os blocos Cb e Cr
            float cb_temp[8][8], cr_temp[8][8];
            extract_block_chroma420(image, cb_temp, bx, by, width, height, 'B');
            extract_block_chroma420(image, cr_temp, bx, by, width, height, 'R');
            
            forwardDCTMatrix(cb_temp, mb->Cb.block);
            forwardDCTMatrix(cr_temp, mb->Cr.block);
        }
    }

    return macroblocks;
}

float base_quantization_matrix_y[8][8] = {
    {16, 11, 10, 16, 24, 40, 51, 61},
    {12, 12, 14, 19, 26, 58, 60, 55},
    {14, 13, 16, 24, 40, 57, 69, 56},
    {14, 17, 22, 29, 51, 87, 80, 62},
    {18, 22, 37, 56, 68,109,103,77},
    {24,35,55,64,81,104,113,92},
    {79,64,78,87,103,121,120,101},
    {72,92 ,95 ,98 ,112 ,100 ,103 ,99}
};

float base_quantization_matrix_chroma[8][8] = {
    {17, 18, 24, 47, 99, 99, 99, 99},
    {18, 21, 26, 66, 99, 99, 99, 99},
    {24, 26, 56, 99, 99, 99, 99, 99},
    {47, 66, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99, 99, 99, 99, 99, 99, 99},
    {99, 99 ,99 ,99 ,99 ,99 ,99 ,99},
    {99 ,99 ,99 ,99 ,99 ,99 ,99 ,98}
};

void quantizeBlock(float block[8][8], float quantization_matrix[8][8]) {
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
            block[y][x] = round(block[y][x] / quantization_matrix[y][x]);
        }
    }
}

void dequantizeBlock(float block[8][8], float quantization_matrix[8][8]) {
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
            block[y][x] *= quantization_matrix[y][x];
        }
    }
}

void quantizeMacroblock(MACROBLOCO *mb, float quantization_matrix_y[8][8], float quantization_matrix_chroma[8][8]) {
    /*
     * Aplica a quantização em um macrobloco 16x16.
     *
     * Parâmetros:
     * mb: macrobloco a ser quantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < 4; i++) {
        quantizeBlock(mb->Y[i].block, quantization_matrix_y);
    }
    quantizeBlock(mb->Cb.block, quantization_matrix_chroma);
    quantizeBlock(mb->Cr.block, quantization_matrix_chroma);
}

void dequantizeMacroblock(MACROBLOCO *mb, float quantization_matrix_y[8][8], float quantization_matrix_chroma[8][8]) {
    /*
     * Aplica a dequantização em um macrobloco 16x16.
     *
     * Parâmetros:
     * mb: macrobloco a ser dequantizado
     * quantization_matrix: matriz de quantização
     * compression_factor: fator de compressão
     */
    for (int i = 0; i < 4; i++) {
        dequantizeBlock(mb->Y[i].block, quantization_matrix_y);
    }
    dequantizeBlock(mb->Cb.block, quantization_matrix_chroma);
    dequantizeBlock(mb->Cr.block, quantization_matrix_chroma);
}

void quantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, float quality) {
    /*
     * Aplica a quantização em um vetor de macroblocos.
     *
     * Parâmetros:
     * mb_array: vetor de macroblocos
     * macroblock_count: número de macroblocos
     * compression_factor: fator de compressão
     */
    float quantization_matrix_y[8][8], quantization_matrix_chroma[8][8], multiplier;
    multiplier = quality < 50 ? 5000/quality : (200 - 2*quality);

    // Preenche as matrizes de quantização com os valores base multiplicados pelo fator de compressão
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            quantization_matrix_y[i][j] = (base_quantization_matrix_y[i][j] * multiplier + 50) / 100;
            quantization_matrix_chroma[i][j] = (base_quantization_matrix_chroma[i][j] * multiplier + 50) / 100;
        }
    }

    for (int i = 0; i < macroblock_count; i++) {
        quantizeMacroblock(&mb_array[i], quantization_matrix_y, quantization_matrix_chroma);
    }
}
void dequantizeMacroblocks(MACROBLOCO *mb_array, int macroblock_count, float quality) {
    /*
     * Aplica a dequantização em um vetor de macroblocos.
     *
     * Parâmetros:
     * mb_array: vetor de macroblocos
     * macroblock_count: número de macroblocos
     * compression_factor: fator de compressão
     */
    float quantization_matrix_y[8][8], quantization_matrix_chroma[8][8], multiplier;
    multiplier = quality < 50 ? 5000/quality : (200 - 2*quality);

    // Preenche as matrizes de quantização com os valores base multiplicados pelo fator de compressão
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            quantization_matrix_y[i][j] = (base_quantization_matrix_y[i][j] * multiplier + 50) / 100;
            quantization_matrix_chroma[i][j] = (base_quantization_matrix_chroma[i][j] * multiplier + 50) / 100;
        }
    }

    for (int i = 0; i < macroblock_count; i++) {
        dequantizeMacroblock(&mb_array[i], quantization_matrix_y, quantization_matrix_chroma);
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
                inverseDCTMatrix(mb->Y[i].block, rec);
                reconstructBlock8x8_Y(dst, rec, bx, by, width, height);
            }

            // Reconstrói os blocos Cb e Cr
            float cb_rec[8][8] = {0}, cr_rec[8][8] = {0};
            inverseDCTMatrix(mb->Cb.block, cb_rec);
            inverseDCTMatrix(mb->Cr.block, cr_rec);

            reconstructBlock8x8_CbCr420(dst, cb_rec, x, y, width, height, 'B');
            reconstructBlock8x8_CbCr420(dst, cr_rec, x, y, width, height, 'R');
        }
    }
}

void vectorize_block(float block[8][8], VETORZIGZAG *return_vector) {
    /*
     * Dado um bloco 8x8, converte em um vetor de 64 posiçöes utilizando o padrão zigue-zague.
     */
    int zigzag[64][2] = {
        {0,0},{0,1},{1,0},{2,0},{1,1},{0,2},{0,3},{1,2},
        {2,1},{3,0},{4,0},{3,1},{2,2},{1,3},{0,4},{0,5},
        {1,4},{2,3},{3,2},{4,1},{5,0},{6,0},{5,1},{4,2},
        {3,3},{2,4},{1,5},{0,6},{0,7},{1,6},{2,5},{3,4},
        {4,3},{5,2},{6,1},{7,0},{7,1},{6,2},{5,3},{4,4},
        {3,5},{2,6},{1,7},{2,7},{3,6},{4,5},{5,4},{6,3},
        {7,2},{7,3},{6,4},{5,5},{4,6},{3,7},{4,7},{5,6},
        {6,5},{7,4},{7,5},{6,6},{5,7},{6,7},{7,6},{7,7}
    };

    for (int i = 0; i < 64; i++) {
        int row = zigzag[i][0];
        int col = zigzag[i][1];
        return_vector->vector[i] = block[row][col];
    }
}

void vectorize_macroblock(MACROBLOCO *macroblock, MACROBLOCO_VETORIZADO *vetorizado) {
    /*
     * Dado um macrobloco com subamostragem de crominância, converte em um macrobloco vetorizado em zigue-zague.
     */
    for (int i = 0; i < 4; i++)
        vectorize_block(macroblock->Y[i].block, &vetorizado->Y_vetor[i]);

    vectorize_block(macroblock->Cb.block, &vetorizado->Cb_vetor);
    vectorize_block(macroblock->Cr.block, &vetorizado->Cr_vetor);
}

void vectorize_macroblocks(MACROBLOCO *macroblocks, MACROBLOCO_VETORIZADO *vectorized_macroblocks, int macroblock_count) {
    /*
     * Dado um vetor de macroblocos com subamostragem de crominância, converte em um vetor de macroblocos vetorizado em zigue-zague.
     */
    for (int i = 0; i < macroblock_count; i++) {
        vectorize_macroblock(&macroblocks[i], &vectorized_macroblocks[i]);
    }
}

void devectorize_block(VETORZIGZAG *vector, float block[8][8]) {
    /*
     * Dado um vetor de 64 posiçöes, converte em um bloco 8x8 utilizando o padrão zigue-zague.
     */
    int zigzag[64][2] = {
        {0,0},{0,1},{1,0},{2,0},{1,1},{0,2},{0,3},{1,2},
        {2,1},{3,0},{4,0},{3,1},{2,2},{1,3},{0,4},{0,5},
        {1,4},{2,3},{3,2},{4,1},{5,0},{6,0},{5,1},{4,2},
        {3,3},{2,4},{1,5},{0,6},{0,7},{1,6},{2,5},{3,4},
        {4,3},{5,2},{6,1},{7,0},{7,1},{6,2},{5,3},{4,4},
        {3,5},{2,6},{1,7},{2,7},{3,6},{4,5},{5,4},{6,3},
        {7,2},{7,3},{6,4},{5,5},{4,6},{3,7},{4,7},{5,6},
        {6,5},{7,4},{7,5},{6,6},{5,7},{6,7},{7,6},{7,7}
    };

    for (int i = 0; i < 64; i++) {
        int row = zigzag[i][0];
        int col = zigzag[i][1];
        block[row][col] = vector->vector[i];
    }
}

void devectorize_macroblock(MACROBLOCO_VETORIZADO *vetorizado, MACROBLOCO *macroblock) {
    /*
     * Dado um macrobloco vetorizado em zigue-zague, converte em um macrobloco com subamostragem de crominância.
     */
    for (int i = 0; i < 4; i++)
        devectorize_block(&vetorizado->Y_vetor[i], macroblock->Y[i].block);

    devectorize_block(&vetorizado->Cb_vetor, macroblock->Cb.block);
    devectorize_block(&vetorizado->Cr_vetor, macroblock->Cr.block);
}

void devectorize_macroblocks(MACROBLOCO_VETORIZADO *vectorized_macroblocks, MACROBLOCO *macroblocks, int macroblock_count) {
    /*
     * Dado um vetor de macroblocos vetorizado em zigue-zague, converte em um vetor de macroblocos com subamostragem de crominância.
     */
    for (int i = 0; i < macroblock_count; i++) {
        devectorize_macroblock(&vectorized_macroblocks[i], &macroblocks[i]);
    }
}

void rle_encode_block(BLOCO_RLE* rle_block, VETORZIGZAG* zigzag_block) {
    /*
     * Converte um bloco vetorizado em zigue-zague em um bloco codificado por carreira.
     */
    rle_block->quantidade = 0;
    int quantidade_zeros = 0;

    rle_block->coeficiente_dc = zigzag_block->vector[0];
    printf("%f\n", rle_block->coeficiente_dc);

    for (int i = 1; i <= 63; i++) {
        if (fabs(zigzag_block->vector[i]) < 0.0001f) {
            quantidade_zeros++;
        } else {
            if (rle_block->quantidade < 63) {
                 rle_block->pares[rle_block->quantidade].zeros = quantidade_zeros;
                 rle_block->pares[rle_block->quantidade].valor = zigzag_block->vector[i];
                 rle_block->quantidade++;
                 quantidade_zeros = 0;
            } else {
                 break;
            }
        }
    }
    // Colocar EOB
    if (rle_block->quantidade < 64) {
        rle_block->pares[rle_block->quantidade].zeros = 0;
        rle_block->pares[rle_block->quantidade].valor = 0.0f;
        rle_block->quantidade++;
    }
}

void rle_encode_macroblock(MACROBLOCO_RLE *rle_macroblock, MACROBLOCO_VETORIZADO *vectorized_macroblock) {
    /*
     * Converte um macrobloco vetorizado em zigue-zague em um macrobloco codificado por carreira.
     */
    for (int i = 0; i < 4; i++) {
        rle_encode_block(&rle_macroblock->Y_vetor[i], &vectorized_macroblock->Y_vetor[i]);
    }
    rle_encode_block(&rle_macroblock->Cb_vetor, &vectorized_macroblock->Cb_vetor);
    rle_encode_block(&rle_macroblock->Cr_vetor, &vectorized_macroblock->Cr_vetor);
}

void rle_encode_macroblocks(MACROBLOCO_RLE *rle_macroblocks, MACROBLOCO_VETORIZADO *vectorized_macroblocks, int macroblock_count) {
    /*
     * Converte um vetor de macroblocos vetorizados em zigue-zague em um vetor de macroblocos codificados por carreira.
     */
    for (int i = 0; i < macroblock_count; i++) {
        rle_encode_macroblock(&rle_macroblocks[i], &vectorized_macroblocks[i]);
    }
}

void rle_decode_block(VETORZIGZAG* zigzag_block, BLOCO_RLE* rle_block) {
    /*
     * Converte um bloco codificado por carreira em um bloco vetorizado em zigue-zague.
     */
    for (int i = 1; i <= 63; i++) {
        zigzag_block->vector[i] = 0.0f;
    }

    zigzag_block->vector[0] = rle_block->coeficiente_dc;
    int current_ac_idx = 1;

    for (int k = 0; k < rle_block->quantidade; k++) {
        const PAR_RLE* par_atual = &rle_block->pares[k];
        if (par_atual->zeros == 0 && fabs(par_atual->valor) < 0.0001f) {
            break; // EOB encontrado
        }

        for (int z = 0; z < par_atual->zeros; z++) {
            if (current_ac_idx <= 63) {
                current_ac_idx++;
            } else {
                return;
            }
        }

        if (current_ac_idx <= 63) {
            zigzag_block->vector[current_ac_idx] = par_atual->valor;
            current_ac_idx++;
        } else {
            return;
        }
    }
}

void rle_decode_macroblock(MACROBLOCO_VETORIZADO *vectorized_macroblock, MACROBLOCO_RLE *rle_macroblock) {
    /*
     * Converte um macrobloco codificado por carreira em um macrobloco vetorizado em zigue-zague.
     */
    for (int i = 0; i < 4; i++) {
        rle_decode_block(&vectorized_macroblock->Y_vetor[i], &rle_macroblock->Y_vetor[i]);
    }
    rle_decode_block(&vectorized_macroblock->Cb_vetor, &rle_macroblock->Cb_vetor);
    rle_decode_block(&vectorized_macroblock->Cr_vetor, &rle_macroblock->Cr_vetor);
}

void rle_decode_macroblocks(MACROBLOCO_VETORIZADO *vectorized_macroblocks, MACROBLOCO_RLE *rle_macroblocks, int macroblock_count) {
    /*
     * Converte um vetor de macroblocos codificados por carreira em um vetor de macroblocos vetorizados em zigue-zague.
     */
    for (int i = 0; i < macroblock_count; i++) {
        rle_decode_macroblock(&vectorized_macroblocks[i], &rle_macroblocks[i]);
    }
}