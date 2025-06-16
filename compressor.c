#include <stdio.h>
#include <stdlib.h>
#include "bitmap.h"
#include "codec.h"
#include "huffman.h"

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <input_image.bmp> <output_file.bin> [quality]\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = argv[2];
    float quality = 50.0f; // Default quality

    if (argc == 4) {
        quality = atof(argv[3]);
        if (quality < 1 || quality > 100) {
            fprintf(stderr, "Error: Quality must be a value between 1 and 100.\n");
            return 1;
        }
    }

    // --- Compression Pipeline ---

    // 1. Open and load BMP headers
    FILE *input_file = fopen(input_filename, "rb");
    if (!input_file) {
        perror("Error opening input BMP file");
        return 1;
    }

    BITMAPFILEHEADER file_header;
    BITMAPINFOHEADER info_header;
    loadBMPHeaders(input_file, &file_header, &info_header);

    // 2. Read pixel data
    int width = info_header.Width;
    int height = info_header.Height;
    int tam = width * height;
    PIXELRGB *pixels_rgb = (PIXELRGB *)calloc(tam, sizeof(PIXELRGB));
    if (!pixels_rgb) {
        fprintf(stderr, "Error: Memory allocation failed for pixels.\n");
        fclose(input_file);
        return 1;
    }
    readPixels(input_file, info_header, pixels_rgb);
    fclose(input_file);

    // 3. Convert RGB -> YCbCr
    PIXELYCBCR *pixels_ycbcr = (PIXELYCBCR *)calloc(tam, sizeof(PIXELYCBCR));
    if (!pixels_ycbcr) {
        fprintf(stderr, "Error: Memory allocation failed for YCbCr pixels.\n");
        free(pixels_rgb);
        return 1;
    }
    convertToYCBCR(pixels_rgb, pixels_ycbcr, tam);
    
    // 4. Encode YCbCr to macroblocks (includes DCT and subsampling)
    int macroblock_count = 0;
    MACROBLOCO *macroblocks = encodeImageYCbCr(pixels_ycbcr, width, height, &macroblock_count);
    printf("\n--- COMPRESSOR TRACE ---\n");
    printf("[1] After DCT, MB 0, Cb DC value: %f\n", macroblocks[0].Cb.block[0][0]);

    // 5. Quantize
    quantizeMacroblocks(macroblocks, macroblock_count, quality);
    printf("[2] After Quantization, MB 0, Cb DC value: %f\n", macroblocks[0].Cb.block[0][0]);

    // 6. Vectorize (Zig-zag)
    MACROBLOCO_VETORIZADO *vectorized_macroblocks = (MACROBLOCO_VETORIZADO *)calloc(macroblock_count, sizeof(MACROBLOCO_VETORIZADO));
    if (!vectorized_macroblocks) { /* ... error handling ... */ }
    vectorize_macroblocks(macroblocks, vectorized_macroblocks, macroblock_count);

    // 7. RLE & Differential Encoding
    MACROBLOCO_RLE_DIFERENCIAL *rle_diff_macroblocks = (MACROBLOCO_RLE_DIFERENCIAL *)calloc(macroblock_count, sizeof(MACROBLOCO_RLE_DIFERENCIAL));
    if (!rle_diff_macroblocks) { /* ... error handling ... */ }
    rle_encode_macroblocks(rle_diff_macroblocks, vectorized_macroblocks, macroblock_count);
    differential_encode_dc(rle_diff_macroblocks, macroblock_count);

    // --- ADD THIS DEBUG BLOCK TO DUMP RLE FOR THE FAILING BLOCK ---
    printf("\n\n---[DEBUG] RLE DUMP FOR MACROBLOCK 130 ---\n");
    int mb_to_debug = 130;
    if (mb_to_debug < macroblock_count) {
        MACROBLOCO_RLE_DIFERENCIAL *mb = &rle_diff_macroblocks[mb_to_debug];
        
        // Print Y0 block
        printf("  Y0 block (%d pairs):\n", mb->Y_vetor[0].quantidade);
        for (int i = 0; i < mb->Y_vetor[0].quantidade; i++) {
            printf("    Pair %d: (zeros=%d, value=%.1f)\n", i, mb->Y_vetor[0].pares[i].zeros, mb->Y_vetor[0].pares[i].valor);
        }
        
        // Print Cb block
        printf("  Cb block (%d pairs):\n", mb->Cb_vetor.quantidade);
        for (int i = 0; i < mb->Cb_vetor.quantidade; i++) {
            printf("    Pair %d: (zeros=%d, value=%.1f)\n", i, mb->Cb_vetor.pares[i].zeros, mb->Cb_vetor.pares[i].valor);
        }

        // Print Cr block
        printf("  Cr block (%d pairs):\n", mb->Cr_vetor.quantidade);
        for (int i = 0; i < mb->Cr_vetor.quantidade; i++) {
            printf("    Pair %d: (zeros=%d, value=%.1f)\n", i, mb->Cr_vetor.pares[i].zeros, mb->Cr_vetor.pares[i].valor);
        }
    }
    printf("-------------------------------------------\n\n");
    // --- END DEBUG BLOCK ---

    printf("[3] Final Encoded DC value for MB 0, Cb: %d\n", rle_diff_macroblocks[0].Cb_vetor.coeficiente_dc);
    printf("------------------------\n\n");

    // 8. Huffman encode and write to file
    // Note: We pass the headers BY VALUE, which was the final fix.
    write_macroblocks_huffman(output_filename, rle_diff_macroblocks, macroblock_count, file_header, info_header, quality);

    // 9. Clean up
    free(pixels_rgb);
    free(pixels_ycbcr);
    free(macroblocks);
    free(vectorized_macroblocks);
    free(rle_diff_macroblocks);
    
    printf("Compression complete. Output written to %s\n", output_filename);
    return 0;
}