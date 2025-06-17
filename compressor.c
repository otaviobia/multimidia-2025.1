/*
 * Compressor de imagens BMP seguindo o padrão JPEG.
 * Desenvolvido para a disciplina de Multimídia no 1º semestre de 2025.
 * Componentes do grupo:
 * - Otávio Biagioni Melo - nº USP: 15482604
 * - Christyan Paniago Nantes — nº USP: 15635906
 */
#include <stdio.h>
#include <stdlib.h>
#include "utils/bitmap.h"
#include "utils/codec.h"
#include "utils/huffman.h"
#include "utils/test.h"

int main(int argc, char *argv[]) {
    // Verifica se o número de argumentos está correto e exibe a mensagem de uso correto
    if (argc < 3 || argc > 4) {
        printf("Uso correto: ./compressor <original.bmp> <comprimido.bin> [qualidade]\n");
        printf("    -> qualidade (opcional - default 50) varia entre 1 e 100.\n");
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    int quality = 50; // Qualidade padrão

    if (argc == 4) {
        quality = atof(argv[3]);
        if (quality < 1 || quality > 100) {
            printf("Erro: Qualidade deve ser um valor entre 1 e 100.\n");
            return 1;
        }
    }

    /* --- PIPELINE DE COMPRESSÃO --- */

    // 1. Abre o arquivo BMP de entrada e lê os cabeçalhos
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        printf("Erro ao abrir o arquivo BMP\n");
        return 1;
    }

    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    loadBMPHeaders(input_file, &file_header, &info_header);

    // 2. Lê os pixels RGB do arquivo BMP
    int width = info_header.Width;
    int height = info_header.Height;
    int tam = width * height;
    PIXELRGB *pixels_rgb = (PIXELRGB *)calloc(tam, sizeof(PIXELRGB));
    if (!pixels_rgb) {
        printf("Erro ao alocar memória para os pixels RGB.\n");
        fclose(input_file);
        return 1;
    }
    readPixels(input_file, info_header, pixels_rgb);
    fclose(input_file); // Fecha o arquivo BMP após leituras finalizadas

    // 3. Converte os pixels RGB para YCbCr
    PIXELYCBCR *pixels_ycbcr = (PIXELYCBCR *)calloc(tam, sizeof(PIXELYCBCR));
    if (!pixels_ycbcr) {
        printf("Erro ao alocar memória para os pixels YCbCr.\n");
        free(pixels_rgb);
        return 1;
    }
    convertToYCBCR(pixels_rgb, pixels_ycbcr, tam);
    
    // 4. Aplica a DCT e o subsampling 4:2:0
    int macroblock_count = 0;
    MACROBLOCO *macroblocks = encodeImageYCbCr(pixels_ycbcr, width, height, &macroblock_count);

    // 5. Aplica a quantização
    quantizeMacroblocks(macroblocks, macroblock_count, quality);

    // 6. Faz a vetorização zig-zag dos macroblocos
    MACROBLOCO_VETORIZADO *vectorized_macroblocks = (MACROBLOCO_VETORIZADO *)calloc(macroblock_count, sizeof(MACROBLOCO_VETORIZADO));
    if (!vectorized_macroblocks) { 
        printf("Erro ao alocar memória para os macroblocos vetorizados.\n");
        free(pixels_rgb); free(pixels_ycbcr); free(macroblocks);
        return 1;
    }
    vectorize_macroblocks(macroblocks, vectorized_macroblocks, macroblock_count);

    // 7. Faz a codificação RLE e diferencial dos macroblocos vetorizados
    MACROBLOCO_RLE_DIFERENCIAL *rle_diff_macroblocks = (MACROBLOCO_RLE_DIFERENCIAL *)calloc(macroblock_count, sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    if (!rle_diff_macroblocks) { 
        printf("Erro ao alocar memória para os macroblocos com RLE e diferencial.\n");
        free(pixels_rgb); free(pixels_ycbcr); free(macroblocks); free(vectorized_macroblocks);
        return 1;
    }
    rle_encode_macroblocks(rle_diff_macroblocks, vectorized_macroblocks, macroblock_count);
    differential_encode_dc(rle_diff_macroblocks, macroblock_count);

    // 8. Aplica a codificação Huffman e escreve os macroblocos comprimidos em um arquivo binário
    write_macroblocks_huffman(output_filename, rle_diff_macroblocks, macroblock_count, file_header, info_header, quality);

    printf("Imagem comprimida com sucesso para %s\n", output_filename);
    printf("O arquivo comprimido eh %.2f%% menor que a imagem original.\n", (1.0f - (float)fsize(output_filename) / fsize(input_filename)) * 100.0f);

    // 9. Limpa a memória alocada
    free(pixels_rgb);
    free(pixels_ycbcr);
    free(macroblocks);
    free(vectorized_macroblocks);
    free(rle_diff_macroblocks);

    return 0;
}