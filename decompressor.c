/*
 * Descompressor de imagens BMP seguindo o padrão JPEG.
 * Desenvolvido para a disciplina de Multimídia no 1º semestre de 2025.
 * Componentes do grupo:
 * - Otávio Biagioni Melo - nº USP: 15482604
 * - Christyan Paniago Nantes — nº USP: 15635906
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/bitmap.h"
#include "utils/codec.h"
#include "utils/huffman.h"

int main(int argc, char *argv[]) {
    // Verifica se o número de argumentos está correto e exibe a mensagem de uso correto
    if (argc != 3) {
        printf("Uso correto: ./decompressor <comprimido.bin> <reconstruido.bmp>\n");
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    /* --- PIPELINE DE DESCOMPRESSÃO --- */
    
    BITMAPFILEHEADER fhead;
    BITMAPINFOHEADER ihead;
    float quality_read;
    int count_read;
    MACROBLOCO_RLE_DIFERENCIAL *read_blocks = NULL;

    // 1. Lê o arquivo comprimido e aplica decodificação huffman nos blocos de macroblocos 
    if (!read_macroblocks_huffman(input_filename, &read_blocks, &count_read, &fhead, &ihead, &quality_read)) {
        printf("Falha ao ler ou decodificar o arquivo comprimido.\n");
        if (read_blocks) free(read_blocks);
        return 1;
    }

    // Aloca memória para as estruturas intermediárias e inicializa variáveis
    MACROBLOCO_VETORIZADO *vectorized_macroblocks = (MACROBLOCO_VETORIZADO *)calloc(count_read, sizeof(MACROBLOCO_VETORIZADO));
    MACROBLOCO *macroblocks = (MACROBLOCO *)calloc(count_read, sizeof(MACROBLOCO));
    int width = ihead.Width;
    int height = ihead.Height;
    int tam = width * height;
    PIXELYCBCR *pixels_ycbcr = (PIXELYCBCR *)calloc(tam, sizeof(PIXELYCBCR));
    PIXELRGB *pixels_rgb = (PIXELRGB *)calloc(tam, sizeof(PIXELRGB));

    if (!vectorized_macroblocks || !macroblocks || !pixels_ycbcr || !pixels_rgb) {
        printf("Erro ao alocar memória para estruturas auxiliares.\n");
        return 1;
    }

    // 2. Codificação diferencial dos valores DC e RLE nos AC dos macroblocos
    differential_decode_dc(read_blocks, count_read);
    rle_decode_macroblocks(vectorized_macroblocks, read_blocks, count_read);
    
    // 3. Desvetorização zig-zag dos macroblocos
    devectorize_macroblocks(vectorized_macroblocks, macroblocks, count_read);

    // 4. Dequantização dos macroblocos
    dequantizeMacroblocks(macroblocks, count_read, quality_read);

    // 5. Inversa da DCT e reconstrução da imagem YCbCr
    decodeImageYCbCr(macroblocks, pixels_ycbcr, width, height);

    // 6. Conversão YCbCr para RGB
    convertToRGB(pixels_ycbcr, pixels_rgb, tam);
    
    // 7. Escrita do arquivo BMP de saída
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        printf("Erro ao abrir o arquivo de saída\n");
    } else {
        writeBMP(output_file, fhead, ihead, pixels_rgb);
        fclose(output_file);
        printf("Arquivo descomprimido com sucesso para %s\n", output_filename);
    }
    
    // 8. Limpeza de memória
    free(read_blocks);
    free(vectorized_macroblocks);
    free(macroblocks);
    free(pixels_ycbcr);
    free(pixels_rgb);
    
    return 0;
}