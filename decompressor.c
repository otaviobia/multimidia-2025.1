#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bitmap.h"
#include "codec.h"
#include "huffman.h"

// This is the only function needed from test.c for the final output
void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int tam);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file.bin> <output_image.bmp>\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];

    // --- Decompression Pipeline ---
    BITMAPFILEHEADER fhead;
    BITMAPINFOHEADER ihead;
    float quality_read;
    int count_read;
    MACROBLOCO_RLE_DIFERENCIAL *read_blocks = NULL;

    if (!read_macroblocks_huffman(input_filename, &read_blocks, &count_read, &fhead, &ihead, &quality_read)) {
        fprintf(stderr, "Aborting: Failed to read and decode the compressed file.\n");
        if (read_blocks) free(read_blocks);
        return 1;
    }
    printf("\n--- DECOMPRESSOR TRACE ---\n");
    printf("[A] After Huffman Decode, MB 0, Cb DC value: %d\n", read_blocks[0].Cb_vetor.coeficiente_dc);

    MACROBLOCO_VETORIZADO *vectorized_macroblocks = (MACROBLOCO_VETORIZADO *)calloc(count_read, sizeof(MACROBLOCO_VETORIZADO));
    MACROBLOCO *macroblocks = (MACROBLOCO *)calloc(count_read, sizeof(MACROBLOCO));
    int width = ihead.Width;
    int height = ihead.Height;
    int tam = width * height;
    PIXELYCBCR *pixels_ycbcr = (PIXELYCBCR *)calloc(tam, sizeof(PIXELYCBCR));
    PIXELRGB *pixels_rgb = (PIXELRGB *)calloc(tam, sizeof(PIXELRGB));

    if (!vectorized_macroblocks || !macroblocks || !pixels_ycbcr || !pixels_rgb) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return 1;
    }

    // 2. Differential and RLE Decode
    differential_decode_dc(read_blocks, count_read);
    printf("[B] After Differential Decode, MB 0, Cb DC value: %d\n", read_blocks[0].Cb_vetor.coeficiente_dc);
    rle_decode_macroblocks(vectorized_macroblocks, read_blocks, count_read);
    
    // 3. De-vectorize
    devectorize_macroblocks(vectorized_macroblocks, macroblocks, count_read);

    // 4. De-quantize
    dequantizeMacroblocks(macroblocks, count_read, quality_read);
    printf("[C] After Dequantization, MB 0, Cb DC value: %f\n", macroblocks[0].Cb.block[0][0]);
    printf("--------------------------\n\n");
    
    // 5. Decode macroblocks to YCbCr image (Inverse DCT)
    decodeImageYCbCr(macroblocks, pixels_ycbcr, width, height);

    // 6. Convert YCbCr -> RGB
    convertToRGB(pixels_ycbcr, pixels_rgb, tam);
    
    // 7. Write to output BMP file
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Error opening output BMP file");
    } else {
        writeBMP(output_file, fhead, ihead, pixels_rgb);
        fclose(output_file);
        printf("Decompressed image successfully written to %s\n", output_filename);
    }
    
    // 8. Clean up
    free(read_blocks);
    free(vectorized_macroblocks);
    free(macroblocks);
    free(pixels_ycbcr);
    free(pixels_rgb);
    
    return 0;
}