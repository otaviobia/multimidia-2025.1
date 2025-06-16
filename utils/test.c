#include "test.h"

void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int tam) {
    /*
     * Compara duas imagens RGB pixel a pixel e contabiliza as diferenças.
     * Imprime estatísticas das diferenças encontradas.
     *
     * Parâmetros:
     * orig: ponteiro para a imagem RGB original
     * recon: ponteiro para a imagem RGB reconstruída
     * tam: quantidade de pixels nas imagens
     */   
    int cont = 0;
    int diff = 2;
    int biggestR = 0;
    int biggestG = 0;
    int biggestB = 0;

    for (int i = 0; i < tam; i++) {
        int dR = fabs((int)recon[i].R - (int)orig[i].R);
        int dG = fabs((int)recon[i].G - (int)orig[i].G);
        int dB = fabs((int)recon[i].B - (int)orig[i].B);

        if (dR > diff || dG > diff || dB > diff) {
            cont++;
            biggestR = (dR > biggestR) ? dR : biggestR;
            biggestG = (dG > biggestG) ? dG : biggestG;
            biggestB = (dB > biggestB) ? dB : biggestB;
        }
    }
    if (cont == 0){
        printf("\n*************** Rgb Comparison Info ***************\n");
        printf("No differences found.\n");
    } else {
        printf("\n*************** Rgb Comparison Info ***************\n");
        printf("Differences found: %d\n", cont);
        printf("Biggest differences is R: %d G: %d B: %d \n", biggestR, biggestG, biggestB);
    }
}

void compareBlock(const float Block[8][8], const float RecBlock[8][8]) {
    /*
     * Compara dois blocos 8x8 de valores em ponto flutuante.
     * Imprime estatísticas das diferenças encontradas.
     *
     * Parâmetros:
     * Block: bloco original 8x8
     * RecBlock: bloco reconstruído 8x8 
     */
    int diffCont = 0; 
    float maxDiff = 0.0f; 
    float diff = 0.01f; 

    printf("\n*************** Block Comparison Info ***************\n");

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float dB = fabs(Block[i][j] - RecBlock[i][j]);
            if (dB > diff) {
                diffCont++;
                maxDiff = (dB > maxDiff) ? dB : maxDiff;
            }
        }
    }

    // Print results
    if (diffCont == 0) {
        printf("No differences found between the blocks.\n");
    } else {
        printf("Differences found: %d\n", diffCont);
        printf("Largest difference: %.6f\n", maxDiff);
    }
}

void DCTBenchComparison(const float Dctfrequencies0[8][8], const float Dctfrequencies1[8][8], const float reconstructedBlock0[8][8], const float reconstructedBlock1[8][8]) {
    /*
     * Compara dois algoritmos de dct para verificar se produzem resultados equivalentes.
     * Imprime estatísticas das diferenças encontradas.
     *
     * Parâmetros:
     * Block: bloco original 8x8
     * RecBlock: bloco reconstruído 8x8 
     */
    int diffcont= 0; // count of diff in dct
    int diffcontRec = 0; // Count of diff in idct
    float maxDiffDCT = 0.0f; // Max diff in dct
    float maxDiffRec = 0.0f; // Max diff in idct
    float diff = 0.01f; // Diff analyzed

    printf("\n*************** DCT Comparison Info ***************\n");

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            float diffDCT = fabs(Dctfrequencies0[i][j] - Dctfrequencies1[i][j]);
            if (diffDCT > diff) {
                diffcont++;
                maxDiffDCT = (diffDCT > maxDiffDCT) ? diffDCT : maxDiffDCT;
            }

            float diffReconstructed = fabs(reconstructedBlock0[i][j] - reconstructedBlock1[i][j]);
            if (diffReconstructed > diff) {
                diffcontRec++;
                maxDiffRec = (diffReconstructed > maxDiffRec) ? diffReconstructed : maxDiffRec;
            }
        }
    }

    // Print results
    if (diffcont == 0 && diffcontRec == 0) {
        printf("No differences found in DCT matrices or reconstructed blocks.\n");
    } else {
        if (diffcont > 0) {
            printf("Differences found in DCT matrices: %d\n", diffcontRec);
            printf("Largest difference in DCT matrices: %.6f\n", maxDiffDCT);
        }
        if (diffcontRec > 0) {
            printf("Differences found in reconstructed blocks: %d\n", diffcontRec);
            printf("Largest difference in reconstructed blocks: %.6f\n", maxDiffRec);
        }
    }
}

int compareYBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height) {
    /*
     * Compara um bloco 8x8 do canal Y entre duas imagens YCbCr.
     * 
     * Parâmetros:
     * orig: imagem YCbCr original
     * recon: imagem YCbCr reconstruída
     * start_x, start_y: coordenadas iniciais do bloco
     * width, height: dimensões da imagem
     * 
     * Retorno:
     * 0 se não houver diferenças, 1 caso contrário || retorna o biggest diff (a depender do teste)
     */
    int cont = 0;
    int diff = 2;
    int biggestDiff = 0;
    
    //printf("\n*************** Y Block Comparison Info ***************\n");

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int px = start_x + x;
            int py = start_y + y;
            if (px >= width || py >= height) continue;
            
            int index = py * width + px;
            int dY = fabs((int)recon[index].Y - (int)orig[index].Y);
            
            if (dY > diff) {
                cont++;
                biggestDiff = (dY > biggestDiff) ? dY : biggestDiff;
            }
        }
    }

    if (cont == 0) {
        return 0;
    } else {
        return 1;
    }
}

int compareCbCrBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height) {
    /*
     * Compara um bloco 16x16 dos canais Cb e Cr entre duas imagens YCbCr.
     * 
     * Parâmetros:
     * orig: imagem YCbCr original
     * recon: imagem YCbCr reconstruída
     * start_x, start_y: coordenadas iniciais do bloco
     * width, height: dimensões da imagem
     * 
     * Retorno:
     * 0 se não houver diferenças, 1 caso contrário || retorna o biggest diff (a depender do teste)
     */

    int contCb = 0, contCr = 0;
    int diff = 2;
    int biggestCb = 0, biggestCr = 0;
    
    //printf("\n*************** CbCr Block Comparison Info ***************\n");

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int px = start_x + x;
            int py = start_y + y;
            if (px >= width || py >= height) continue;
            
            int index = py * width + px;
            int dCb = fabs((int)recon[index].Cb - (int)orig[index].Cb);
            int dCr = fabs((int)recon[index].Cr - (int)orig[index].Cr);
            
            if (dCb > diff) {
                contCb++;
                biggestCb = (dCb > biggestCb) ? dCb : biggestCb;
            }
            if (dCr > diff) {
                contCr++;
                biggestCr = (dCr > biggestCr) ? dCr : biggestCr;
            }
        }
    }

    if (contCb == 0 && contCr == 0) {
        return 0;
    } else {
        return 1;
    }
}

void testImageSubsampling(PIXELYCBCR *image, int width, int height) {
    /*
     * Testa o processo de subamostragem e reconstrução de uma imagem YCbCr.
     * Para cada macrobloco 16x16, extrai e reconstrói os canais Y, Cb e Cr,
     * comparando com a imagem original.
     * 
     * Parâmetros:
     * image: imagem YCbCr a ser testada
     * width, height: dimensões da imagem
     */

    // Create empty image for reconstruction
    PIXELYCBCR *recon = (PIXELYCBCR *)malloc(width * height * sizeof(PIXELYCBCR));
    // Initialize reconstruction buffer to zeros
    memset(recon, 0, width * height * sizeof(PIXELYCBCR));

    // For each 16x16 block in the image
    for (int by = 0; by < height; by += 16) {
        for (int bx = 0; bx < width; bx += 16) {
            // Test Y blocks (4 blocks of 8x8)
            for (int i = 0; i < 4; i++) {
                int ox = bx + (i % 2) * 8;
                int oy = by + (i / 2) * 8;
                
                // Extract and immediately reconstruct Y
                float y_temp[8][8];
                extract_block_y(image, y_temp, ox, oy, width, height);
                reconstructBlock8x8_Y(recon, y_temp, ox, oy, width, height);
                
                // Compare the block
                int cont = compareYBlock(image, recon, ox, oy, width, height);
                printf("%d ",cont);

            }

            // Test CbCr (16x16 block)
            //float cb_temp[8][8], cr_temp[8][8];
            
            // Extract and immediately reconstruct Cb and Cr
            //extract_block_chroma420(image, cb_temp, bx, by, width, height, 'B');
            //extract_block_chroma420(image, cr_temp, bx, by, width, height, 'R');
            
            //reconstructBlock8x8_CbCr420(recon, cb_temp, bx, by, width, height, 'B');
            //reconstructBlock8x8_CbCr420(recon, cr_temp, bx, by, width, height, 'R');
            
            // Compare the block
           // int cont = compareCbCrBlock(image, recon, bx, by, width, height);
            //printf("%d ",cont);
        }
        printf("\n");

    }

    printf("\nFull Image Statistics:\n");
    printf("Image dimensions: %dx%d\n", width, height);
    printf("Number of 16x16 macroblocks: %dx%d\n", 
           (width + 15) / 16, (height + 15) / 16);

    free(recon);
}

void saveChannelImages(PIXELYCBCR *pixels, int width, int height, BITMAPFILEHEADER FileHeader, BITMAPINFOHEADER InfoHeader) {
    /*
     * Salva cada canal (Y, Cb, Cr) da imagem YCbCr como uma imagem BMP separada.
     * Útil para debug visual dos canais individuais.
     * 
     * Parâmetros:
     * pixels: imagem YCbCr de entrada
     * width, height: dimensões da imagem
     * FileHeader: cabeçalho de arquivo BMP
     * InfoHeader: cabeçalho de informação BMP
     */
    int size = width * height;
    PIXELRGB *tempPixels = (PIXELRGB*)malloc(size * sizeof(PIXELRGB));
    
    // Create Y channel image (grayscale)
    for (int i = 0; i < size; i++) {
        tempPixels[i].R = pixels[i].Y;
        tempPixels[i].G = pixels[i].Y;
        tempPixels[i].B = pixels[i].Y;
    }
    
    FILE *outputY = fopen("channel_Y.bmp", "wb");
    if (outputY) {
        writeBMP(outputY, FileHeader, InfoHeader, tempPixels);
        fclose(outputY);
        printf("Y channel saved to channel_Y.bmp\n");
    }
    
    // Create Cb channel image (grayscale)
    for (int i = 0; i < size; i++) {
        // Shift to make visible (Cb is centered at 128)
        unsigned char value = pixels[i].Cb;
        tempPixels[i].R = value;
        tempPixels[i].G = value;
        tempPixels[i].B = value;
    }
    
    FILE *outputCb = fopen("channel_Cb.bmp", "wb");
    if (outputCb) {
        writeBMP(outputCb, FileHeader, InfoHeader, tempPixels);
        fclose(outputCb);
        printf("Cb channel saved to channel_Cb.bmp\n");
    }
    
    // Create Cr channel image (grayscale)
    for (int i = 0; i < size; i++) {
        // Shift to make visible (Cr is centered at 128)
        unsigned char value = pixels[i].Cr;
        tempPixels[i].R = value;
        tempPixels[i].G = value;
        tempPixels[i].B = value;
    }
    
    FILE *outputCr = fopen("channel_Cr.bmp", "wb");
    if (outputCr) {
        writeBMP(outputCr, FileHeader, InfoHeader, tempPixels);
        fclose(outputCr);
        printf("Cr channel saved to channel_Cr.bmp\n");
    }
    
    free(tempPixels);
}

void testImageWithoutDCT(PIXELYCBCR *image, int width, int height, BITMAPFILEHEADER FileHeader, BITMAPINFOHEADER InfoHeader) {
    /*
     * Testa o processo de extração e reconstrução de blocos sem aplicar DCT.
     * Isola os problemas de processamento de blocos dos problemas relacionados à DCT.
     * 
     * Parâmetros:
     * image: imagem YCbCr original
     * width, height: dimensões da imagem
     * FileHeader: cabeçalho de arquivo BMP
     * InfoHeader: cabeçalho de informação BMP
     */
    printf("\n*************** Teste Sem DCT ***************\n");
    
    // Cria imagem temporária para a reconstrução
    PIXELYCBCR *recon = (PIXELYCBCR *)malloc(width * height * sizeof(PIXELYCBCR));
    
    // É importante inicializar com zeros
    memset(recon, 0, width * height * sizeof(PIXELYCBCR));
    
    int mb_width = (width + 15) / 16;
    int mb_height = (height + 15) / 16;
    
    // Para cada macrobloco
    for (int by = 0; by < mb_height * 16; by += 16) {
        for (int bx = 0; bx < mb_width * 16; bx += 16) {
            // Processa os 4 blocos Y
            for (int i = 0; i < 4; i++) {
                int ox = bx + (i % 2) * 8;
                int oy = by + (i / 2) * 8;
                
                // Extrai bloco Y
                float y_temp[8][8];
                extract_block_y(image, y_temp, ox, oy, width, height);
                
                // Reconstrui bloco Y diretamente
                reconstructBlock8x8_Y(recon, y_temp, ox, oy, width, height);
            }
            
            // Extrai blocos Cb e Cr
            float cb_temp[8][8], cr_temp[8][8];
            extract_block_chroma420(image, cb_temp, bx, by, width, height, 'B');
            extract_block_chroma420(image, cr_temp, bx, by, width, height, 'R');
            
            // Reconstrói diretamente sem DCT
            reconstructBlock8x8_CbCr420(recon, cb_temp, bx, by, width, height, 'B');
            reconstructBlock8x8_CbCr420(recon, cr_temp, bx, by, width, height, 'R');
        }
    }
    
    // Compara as duas imagens
    //compareReconstructions(image, recon, width, height);
    
    // Para debug visual, salve a imagem reconstruída
    PIXELRGB *reconRGB = (PIXELRGB *)malloc(width * height * sizeof(PIXELRGB));
    convertToRGB(recon, reconRGB, width * height);
    
    FILE *output = fopen("recon_without_dct.bmp", "wb");
    if (output) {

        
        writeBMP(output, FileHeader, InfoHeader, reconRGB);
        fclose(output);
    }
    
    free(recon);
    free(reconRGB);
}

void testVectorization() {
    /*
     * Testa a vetorização em zigue-zague de um bloco 8x8 e sua desvetorização.
     * Cria uma matriz de teste com valores 0-63 em ordem linear, vetoriza usando o padrão
     * zigue-zague, e depois desvetoriza para verificar a corretude da implementação.
     */
    printf("\n*************** Vectorization Test ***************\n");
    
    // Create a test matrix with values 0-63
    float test_block[8][8];
    int value = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            test_block[i][j] = (float)value++;
        }
    }
    
    // Print original matrix
    printf("Original 8x8 matrix:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%3.0f ", test_block[i][j]);
        }
        printf("\n");
    }
    
    // Vectorize the block
    VETORZIGZAG vector;
    vectorize_block(test_block, &vector);
    
    // Print vectorized result
    printf("\nVectorized (zigzag order):\n");
    for (int i = 0; i < 64; i++) {
        printf("%3.0f ", vector.vector[i]);
        if ((i + 1) % 8 == 0) printf("\n");
    }
    
    // Devectorize back to matrix
    float reconstructed_block[8][8];
    devectorize_block(&vector, reconstructed_block);
    
    // Print reconstructed matrix
    printf("\nReconstructed 8x8 matrix:\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%3.0f ", reconstructed_block[i][j]);
        }
        printf("\n");
    }
    
    // Verify if original and reconstructed are identical
    int errors = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (test_block[i][j] != reconstructed_block[i][j]) {
                errors++;
                printf("Error at [%d][%d]: original=%.0f, reconstructed=%.0f\n", 
                       i, j, test_block[i][j], reconstructed_block[i][j]);
            }
        }
    }
    
    if (errors == 0) {
        printf("\nVectorization test PASSED - all values match!\n");
    } else {
        printf("\nVectorization test FAILED - %d errors found!\n", errors);
    }
    
    printf("************************************************\n\n");
}

void testDCCategoryEncoding(MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count) {
    /*
     * Testa como os coeficientes DC seriam codificados usando categorias JPEG.
     * Esta função não altera os dados, apenas mostra como seria feita a codificação.
     * Útil para debugging e validação do sistema de categorias.
     *
     * Parâmetros:
     * rle_macroblocks: Vetor de macroblocos com codificação RLE
     * macroblock_count: Quantidade de macroblocos
     */
    printf("\n*************** Teste de Codificacao DC por Categoria ***************\n");
    
    int total_dc_coeffs = 0;
    int category_count[12] = {0}; // Contador para cada categoria (0-11)
    
    for (int i = 0; i < macroblock_count && i < 3; i++) { // Testa apenas os primeiros 3 macroblocos
        printf("Macrobloco %d:\n", i);
        
        // Testa coeficientes DC dos blocos Y
        for (int j = 0; j < 4; j++) {
            int dc_value = rle_macroblocks[i].Y_vetor[j].coeficiente_dc;
            int category = get_coefficient_category(dc_value);
            int code = get_coefficient_code(dc_value, category);
            int decoded = decode_coefficient_from_category(category, code);
            
            printf("  Y[%d] DC: valor=%d, categoria=%d, codigo=%d, decodificado=%d %s\n", 
                   j, dc_value, category, code, decoded, 
                   (dc_value == decoded) ? "Y" : "X");
            
            category_count[category]++;
            total_dc_coeffs++;
        }
        
        // Testa coeficiente DC do bloco Cb
        int dc_cb = rle_macroblocks[i].Cb_vetor.coeficiente_dc;
        int cat_cb = get_coefficient_category(dc_cb);
        int code_cb = get_coefficient_code(dc_cb, cat_cb);
        int decoded_cb = decode_coefficient_from_category(cat_cb, code_cb);
        
        printf("  Cb DC: valor=%d, categoria=%d, codigo=%d, decodificado=%d %s\n", 
               dc_cb, cat_cb, code_cb, decoded_cb,
               (dc_cb == decoded_cb) ? "Y" : "X");
        
        category_count[cat_cb]++;
        total_dc_coeffs++;
        
        // Testa coeficiente DC do bloco Cr
        int dc_cr = rle_macroblocks[i].Cr_vetor.coeficiente_dc;
        int cat_cr = get_coefficient_category(dc_cr);
        int code_cr = get_coefficient_code(dc_cr, cat_cr);
        int decoded_cr = decode_coefficient_from_category(cat_cr, code_cr);
        
        printf("  Cr DC: valor=%d, categoria=%d, codigo=%d, decodificado=%d %s\n", 
               dc_cr, cat_cr, code_cr, decoded_cr,
               (dc_cr == decoded_cr) ? "Y" : "X");
        
        category_count[cat_cr]++;
        total_dc_coeffs++;
        printf("\n");
    }
    
    // Estatísticas das categorias
    // printf("Estatisticas das Categorias DC:\n");
    // for (int i = 0; i < 12; i++) {
    //     if (category_count[i] > 0) {
    //         printf("  Categoria %d: %d coeficientes (%.1f%%)\n", 
    //                i, category_count[i], 
    //                (float)category_count[i] * 100.0f / total_dc_coeffs);
    //     }
    // }
    printf("Total de coeficientes DC analisados: %d\n", total_dc_coeffs);
    printf("************************************************************\n\n");
}

void testACCategoryEncoding(MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count) {
    /*
     * Testa como os coeficientes AC seriam codificados usando categorias JPEG.
     * Esta funcao nao altera os dados, apenas mostra como seria feita a codificacao.
     * Util para debugging e validacao do sistema de categorias para coeficientes AC.
     *
     * Parametros:
     * rle_macroblocks: Vetor de macroblocos com codificacao RLE
     * macroblock_count: Quantidade de macroblocos
     */
    printf("\n*************** Teste de Codificacao AC por Categoria ***************\n");
    
    int total_ac_coeffs = 0;
    int category_count[12] = {0}; // Contador para cada categoria (0-11)
    int zero_runs[17] = {0}; // Contador para runs de zeros (0-16, onde 16 e ZRL)
    
    for (int i = 0; i < macroblock_count && i < 2; i++) { // Testa apenas os primeiros 2 macroblocos
        printf("Macrobloco %d:\n", i);
        
        // Testa o primeiro bloco Y de cada macrobloco
        BLOCO_RLE_DIFERENCIAL *block_y = &rle_macroblocks[i].Y_vetor[0];
        printf("  Componente Y[0] (%d coeficientes AC):\n", block_y->quantidade);
        
        for (int k = 0; k < block_y->quantidade && k < 5; k++) { // Mostra apenas os primeiros 5
            PAR_RLE *par = &block_y->pares[k];
            int ac_value = (int)round(par->valor);
            
            // Verifica se e EOB
            if (par->zeros == 0 && fabs(par->valor) < 0.0001f) {
                printf("    AC[%d]: EOB (End of Block)\n", k);
                break;
            }
            
            int category = get_coefficient_category(ac_value);
            int code = get_coefficient_code(ac_value, category);
            int decoded = decode_coefficient_from_category(category, code);
            
            printf("    AC[%d]: zeros=%d, valor=%.1f, categoria=%d, codigo=%d, decodificado=%d %s\n", 
                   k, par->zeros, par->valor, category, code, decoded,
                   (ac_value == decoded) ? "Y" : "X");
            
            // Estatisticas
            if (ac_value != 0) {
                category_count[category]++;
                total_ac_coeffs++;
            }
            
            if (par->zeros <= 16) {
                zero_runs[par->zeros]++;
            }
        }
        
        // TESTA COMPONENTE Cb
        BLOCO_RLE_DIFERENCIAL *block_cb = &rle_macroblocks[i].Cb_vetor;
        printf("  Componente Cb (%d coeficientes AC):\n", block_cb->quantidade);
        
        for (int k = 0; k < block_cb->quantidade && k < 5; k++) { // Mostra apenas os primeiros 5
            PAR_RLE *par = &block_cb->pares[k];
            int ac_value = (int)round(par->valor);
            
            // Verifica se e EOB
            if (par->zeros == 0 && fabs(par->valor) < 0.0001f) {
                printf("    AC[%d]: EOB (End of Block)\n", k);
                break;
            }
            
            int category = get_coefficient_category(ac_value);
            int code = get_coefficient_code(ac_value, category);
            int decoded = decode_coefficient_from_category(category, code);
            
            printf("    AC[%d]: zeros=%d, valor=%.1f, categoria=%d, codigo=%d, decodificado=%d %s\n", 
                   k, par->zeros, par->valor, category, code, decoded,
                   (ac_value == decoded) ? "Y" : "X");
            
            // Estatisticas
            if (ac_value != 0) {
                category_count[category]++;
                total_ac_coeffs++;
            }
            
            if (par->zeros <= 16) {
                zero_runs[par->zeros]++;
            }
        }
        
        // TESTA COMPONENTE Cr
        BLOCO_RLE_DIFERENCIAL *block_cr = &rle_macroblocks[i].Cr_vetor;
        printf("  Componente Cr (%d coeficientes AC):\n", block_cr->quantidade);
        
        for (int k = 0; k < block_cr->quantidade && k < 5; k++) { // Mostra apenas os primeiros 5
            PAR_RLE *par = &block_cr->pares[k];
            int ac_value = (int)round(par->valor);
            
            // Verifica se e EOB
            if (par->zeros == 0 && fabs(par->valor) < 0.0001f) {
                printf("    AC[%d]: EOB (End of Block)\n", k);
                break;
            }
            
            int category = get_coefficient_category(ac_value);
            int code = get_coefficient_code(ac_value, category);
            int decoded = decode_coefficient_from_category(category, code);
            
            printf("    AC[%d]: zeros=%d, valor=%.1f, categoria=%d, codigo=%d, decodificado=%d %s\n", 
                   k, par->zeros, par->valor, category, code, decoded,
                   (ac_value == decoded) ? "Y" : "X");
            
            // Estatisticas
            if (ac_value != 0) {
                category_count[category]++;
                total_ac_coeffs++;
            }
            
            if (par->zeros <= 16) {
                zero_runs[par->zeros]++;
            }
        }
        
        printf("\n");
    }
    
    printf("Total de coeficientes AC nao-zero analisados: %d\n", total_ac_coeffs);
    printf("************************************************************\n\n");
}

void testCategoryEncodingRoundtrip(MACROBLOCO_RLE_DIFERENCIAL *rle_macroblocks, int macroblock_count) {
    /*
     * Testa a codificacao e decodificacao por categorias para verificar se sao reverseis.
     * Esta funcao verifica se get_coefficient_code() e decode_coefficient_from_category()
     * sao funcoes inversas uma da outra.
     *
     * Parametros:
     * rle_macroblocks: Vetor de macroblocos com codificacao RLE
     * macroblock_count: Quantidade de macroblocos
     */
    printf("\n*************** Teste de Codificacao por Categoria ***************\n");
    
    int total_tests = 0;
    int failed_tests = 0;
    
    for (int i = 0; i < macroblock_count && i < 5; i++) {
        // Testa coeficientes DC
        for (int j = 0; j < 4; j++) {
            int original = rle_macroblocks[i].Y_vetor[j].coeficiente_dc;
            int category = get_coefficient_category(original);
            int code = get_coefficient_code(original, category);
            int decoded = decode_coefficient_from_category(category, code);
            
            total_tests++;
            if (original != decoded) {
                printf("ERRO DC Y[%d] do MB[%d]: %d -> cat=%d, code=%d -> %d\n", 
                       j, i, original, category, code, decoded);
                failed_tests++;
            }
        }
        
        // Testa DC Cb
        int original_cb = rle_macroblocks[i].Cb_vetor.coeficiente_dc;
        int cat_cb = get_coefficient_category(original_cb);
        int code_cb = get_coefficient_code(original_cb, cat_cb);
        int decoded_cb = decode_coefficient_from_category(cat_cb, code_cb);
        
        total_tests++;
        if (original_cb != decoded_cb) {
            printf("ERRO DC Cb do MB[%d]: %d -> cat=%d, code=%d -> %d\n", 
                   i, original_cb, cat_cb, code_cb, decoded_cb);
            failed_tests++;
        }
        
        // Testa DC Cr
        int original_cr = rle_macroblocks[i].Cr_vetor.coeficiente_dc;
        int cat_cr = get_coefficient_category(original_cr);
        int code_cr = get_coefficient_code(original_cr, cat_cr);
        int decoded_cr = decode_coefficient_from_category(cat_cr, code_cr);
        
        total_tests++;
        if (original_cr != decoded_cr) {
            printf("ERRO DC Cr do MB[%d]: %d -> cat=%d, code=%d -> %d\n", 
                   i, original_cr, cat_cr, code_cr, decoded_cr);
            failed_tests++;
        }
        
        // Testa alguns coeficientes AC
        for (int j = 0; j < 4; j++) {
            BLOCO_RLE_DIFERENCIAL *block = &rle_macroblocks[i].Y_vetor[j];
            for (int k = 0; k < block->quantidade && k < 5; k++) {
                int original_ac = (int)round(block->pares[k].valor);
                if (original_ac == 0) continue; // Pula zeros (EOB)
                
                int cat_ac = get_coefficient_category(original_ac);
                int code_ac = get_coefficient_code(original_ac, cat_ac);
                int decoded_ac = decode_coefficient_from_category(cat_ac, code_ac);
                
                total_tests++;
                if (original_ac != decoded_ac) {
                    printf("ERRO AC Y[%d][%d] do MB[%d]: %d -> cat=%d, code=%d -> %d\n", 
                           j, k, i, original_ac, cat_ac, code_ac, decoded_ac);
                    failed_tests++;
                }
            }
        }
    }
    
    printf("Resultado do Teste de Roundtrip:\n");
    printf("  Total de testes: %d\n", total_tests);
    printf("  Testes falharam: %d\n", failed_tests);
    printf("  Taxa de sucesso: %.2f%%\n", 
           total_tests > 0 ? (float)(total_tests - failed_tests) * 100.0f / total_tests : 0.0f);
    
    if (failed_tests == 0) {
        printf("  SUCESSO: Todas as codificacoes/decodificacoes sao reverseis!\n");
    } else {
        printf("  FALHA: Encontrados erros na codificacao/decodificacao!\n");
    }
    
    printf("************************************************************\n\n");
}

void printBitsInBuffer(BitBuffer* buffer) {
    /*
     * Imprime o conteúdo do buffer em formato binário para debugging
     * 
     * Parâmetros:
     * buffer: o buffer de bits a ser impresso
     */
    printf("Buffer conteudo (binario): ");
    for (size_t i = 0; i < buffer->byte_position + 1; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            printf("%d", (buffer->data[i] & (1 << bit)) ? 1 : 0);
        }
        printf(" ");
    }
    printf("\n");
    printf("Posicao atual: byte %llu, bit %d\n", buffer->byte_position, buffer->bit_position);
}

void testBitBufferSimple() {
    /*
     * Teste básico de ida e volta das funções write_bits e read_bits
     * Verifica se padrões de bits escritos podem ser lidos corretamente
     */
    printf("\n*************** Teste BitBuffer Basico ***************\n");
    
    // Cria um buffer de bits com capacidade inicial
    BitBuffer* buffer = init_bit_buffer(16);
    if (!buffer) {
        printf("Falha ao criar buffer!\n");
        return;
    }
    
    // Padrões de teste - pares (valor, número de bits)
    struct TestPattern {
        int value;
        int bits;
    } patterns[] = {
        {5, 3},      // 101 (3 bits)
        {10, 4},     // 1010 (4 bits)
        {15, 4},     // 1111 (4 bits)
        {0, 3},      // 000 (3 bits)
        {255, 8}     // 11111111 (8 bits)
    };
    
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    printf("Escrevendo %d padroes no buffer...\n", num_patterns);
    
    // Escreve os padrões no buffer
    for (int i = 0; i < num_patterns; i++) {
        if (!write_bits(buffer, patterns[i].value, patterns[i].bits)) {
            printf("ERRO: Falha ao escrever padrao %d!\n", i);
            free_bit_buffer(buffer);
            return;
        }
        printf("Escrito: valor=%d, bits=%d\n", patterns[i].value, patterns[i].bits);
    }
    
    // Mostra o conteúdo do buffer para debugging
    printBitsInBuffer(buffer);
    
    // Reseta a posição do buffer para leitura
    buffer->byte_position = 0;
    buffer->bit_position = 0;
    
    // Lê os padrões do buffer
    printf("\nLendo padroes do buffer...\n");
    for (int i = 0; i < num_patterns; i++) {
        int read_value = read_bits(buffer, patterns[i].bits);
        printf("Esperado: %d, Lido: %d - %s\n", 
               patterns[i].value, read_value, 
               (patterns[i].value == read_value) ? "OK" : "ERRO");
        
        if (patterns[i].value != read_value) {
            printf("ERRO: Falha na leitura do padrao %d!\n", i);
        }
    }
    
    free_bit_buffer(buffer);
    printf("********************************************\n\n");
}

void testBitBufferExtensive() {
    /*
     * Teste extensivo de ida e volta das funções write_bits e read_bits
     * Testa vários padrões e limites do buffer
     */
    printf("\n*************** Teste BitBuffer Extensivo ***************\n");
    
    BitBuffer* buffer = init_bit_buffer(8);  // Começa com um buffer pequeno
    if (!buffer) {
        printf("Falha ao criar buffer!\n");
        return;
    }
    
    int errors = 0;
    int total_tests = 0;
    
    // Teste 1: Escrever e ler bits individuais
    printf("Teste 1: Bits individuais\n");
    int bit_pattern[] = {1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 0, 0};
    int num_bits = sizeof(bit_pattern) / sizeof(bit_pattern[0]);
    
    // Escreve bits individuais
    for (int i = 0; i < num_bits; i++) {
        if (!write_bits(buffer, bit_pattern[i], 1)) {
            printf("ERRO: Falha ao escrever bit individual %d!\n", i);
            errors++;
        }
        total_tests++;
    }
    
    // Reseta o buffer para leitura
    buffer->byte_position = 0;
    buffer->bit_position = 0;
    
    // Lê bits individuais
    for (int i = 0; i < num_bits; i++) {
        int bit = read_bits(buffer, 1);
        if (bit != bit_pattern[i]) {
            printf("ERRO: Bit individual %d nao corresponde! Esperado: %d, Lido: %d\n", 
                   i, bit_pattern[i], bit);
            errors++;
        }
        total_tests++;
    }
    
    // Reseta o buffer
    free_bit_buffer(buffer);
    buffer = init_bit_buffer(8);
    
    // Teste 2: Escrever e ler valores com diferentes tamanhos
    printf("\nTeste 2: Valores com diferentes tamanhos\n");
    struct TestCase {
        int value;
        int bits;
    } test_cases[] = {
        {0, 1},       // 0 (1 bit)
        {1, 1},       // 1 (1 bit)
        {3, 2},       // 11 (2 bits)
        {7, 3},       // 111 (3 bits)
        {12, 4},      // 1100 (4 bits)
        {21, 5},      // 10101 (5 bits)
        {42, 6},      // 101010 (6 bits)
        {85, 7},      // 1010101 (7 bits)
        {170, 8},     // 10101010 (8 bits)
        {511, 9},     // 111111111 (9 bits)
        {1023, 10}    // 1111111111 (10 bits)
    };
    
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Escreve os valores
    for (int i = 0; i < num_cases; i++) {
        if (!write_bits(buffer, test_cases[i].value, test_cases[i].bits)) {
            printf("ERRO: Falha ao escrever caso %d!\n", i);
            errors++;
        }
        total_tests++;
    }
    
    // Mostra o conteúdo do buffer para debugging
    printBitsInBuffer(buffer);
    
    // Reseta para leitura
    buffer->byte_position = 0;
    buffer->bit_position = 0;
    
    // Lê os valores
    for (int i = 0; i < num_cases; i++) {
        int read_value = read_bits(buffer, test_cases[i].bits);
        if (read_value != test_cases[i].value) {
            printf("ERRO: Caso %d nao corresponde! Esperado: %d, Lido: %d\n", 
                   i, test_cases[i].value, read_value);
            errors++;
        }
        total_tests++;
    }
    
    // Teste 3: Escrever e ler valores que forçam o buffer a crescer
    printf("\nTeste 3: Forcar o crescimento do buffer\n");
    free_bit_buffer(buffer);
    buffer = init_bit_buffer(1);  // Começa com apenas 1 byte
    
    // Escreve uma sequência que ocupa vários bytes
    int big_value = 0xFFFFFF;  // 24 bits ligados
    if (!write_bits(buffer, big_value, 24)) {
        printf("ERRO: Falha ao escrever valor grande!\n");
        errors++;
    }
    total_tests++;
    
    // Verifica a capacidade do buffer após o crescimento
    printf("Capacidade do buffer apos crescimento: %llu bytes\n", buffer->capacity);
    
    // Reseta para leitura
    buffer->byte_position = 0;
    buffer->bit_position = 0;
    
    // Lê o valor grande
    int read_big_value = read_bits(buffer, 24);
    if (read_big_value != big_value) {
        printf("ERRO: Valor grande nao corresponde! Esperado: %d, Lido: %d\n", 
               big_value, read_big_value);
        errors++;
    }
    total_tests++;
    
    // Resultado final
    printf("\nResultado do teste BitBuffer:\n");
    printf("Total de testes: %d\n", total_tests);
    printf("Erros encontrados: %d\n", errors);
    
    if (errors == 0) {
        printf("SUCESSO: Todos os testes passaram!\n");
    } else {
        printf("FALHA: %d testes falharam!\n", errors);
    }
    
    free_bit_buffer(buffer);
    printf("********************************************\n\n");
}

void testHuffmanRoundtrip() {
    /*
     * Testa o processo completo de codificação e decodificação Huffman
     * Codifica e decodifica alguns valores de teste e verifica se são iguais
     */
    printf("\n*************** Teste Huffman ida e volta ***************\n");
    
    // Cria um buffer para o teste
    BitBuffer* buffer = init_bit_buffer(64);
    if (!buffer) {
        printf("Falha ao criar buffer!\n");
        return;
    }
    
    // 1. Teste de coeficientes DC
    int dc_diffs[] = {0, 1, -1, 15, -15, 64, -64, 127, -127, 255, -255}; 
    int num_dc_values = sizeof(dc_diffs) / sizeof(dc_diffs[0]);
    int errors_dc = 0;
    
    printf("Testando coeficientes DC (diferencas)...\n");
    
    for (int i = 0; i < num_dc_values; i++) {
        // Reset do buffer para cada teste
        buffer->byte_position = 0;
        buffer->bit_position = 0;
        memset(buffer->data, 0, buffer->capacity);
        
        // Codifica a diferença DC
        if (!write_dc_coefficient(buffer, dc_diffs[i])) {
            printf("ERRO: Falha ao codificar diferenca DC %d!\n", dc_diffs[i]);
            errors_dc++;
            continue;
        }
        
        // Reinicia posição para leitura
        buffer->byte_position = 0;
        buffer->bit_position = 0;
        
        // Decodifica a diferença DC usando a nova assinatura
        int decoded_dc;
        int success = decode_dc_coefficient(&decoded_dc, buffer);
        
        // Verifica se a decodificação foi bem-sucedida e se o valor é igual ao original
        if (!success || decoded_dc != dc_diffs[i]) {
            printf("ERRO DC: Original=%d, Decodificado=%d\n", dc_diffs[i], decoded_dc);
            errors_dc++;
        }
    }
    
    // 2. Teste de coeficientes AC
    struct ACTestCase {
        int run;
        int value;
    } ac_cases[] = {
        {0, 0},      // EOB
        {0, 1},      // (0,1)
        {0, -1},     // (0,-1)
        {1, 2},      // (1,2)
        {2, -2},     // (2,-2)
        {0, 7},      // (0,7)
        {3, -7},     // (3,-7)
        {15, 0},     // ZRL
        {10, 5},     // (10,5)
        {14, -10}    // (14,-10)
    };
    int num_ac_cases = sizeof(ac_cases) / sizeof(ac_cases[0]);
    int errors_ac = 0;
    
    printf("\nTestando coeficientes AC...\n");
    
    for (int i = 0; i < num_ac_cases; i++) {
        // Reset do buffer para cada teste
        buffer->byte_position = 0;
        buffer->bit_position = 0;
        memset(buffer->data, 0, buffer->capacity);
        
        // Codifica o par AC
        if (!write_ac_coefficient(buffer, ac_cases[i].run, ac_cases[i].value)) {
            printf("ERRO: Falha ao codificar AC (%d,%d)!\n", 
                   ac_cases[i].run, ac_cases[i].value);
            errors_ac++;
            continue;
        }
        
        // Reinicia posição para leitura
        buffer->byte_position = 0;
        buffer->bit_position = 0;
        
        // Decodifica o par AC
        int run_length, value;
        int result = decode_ac_coefficient(buffer, &run_length, &value);
        
        // Verifica se a decodificação foi bem-sucedida
        if (result <= 0) {
            printf("ERRO: Falha ao decodificar AC (%d,%d)!\n", 
                   ac_cases[i].run, ac_cases[i].value);
            errors_ac++;
            continue;
        }
        
        // Verifica casos especiais: EOB e ZRL
        if (ac_cases[i].run == 0 && ac_cases[i].value == 0) {
            // Deve ser EOB (result = 2)
            if (result != 2) {
                printf("ERRO AC EOB: Nao reconhecido como EOB!\n");
                errors_ac++;
            }
        } else if (ac_cases[i].run == 15 && ac_cases[i].value == 0) {
            // Deve ser ZRL (result = 3)
            if (result != 3) {
                printf("ERRO AC ZRL: Não reconhecido como ZRL!\n");
                errors_ac++;
            }
        } else {
            // Caso normal, verifique run e value
            if (run_length != ac_cases[i].run || value != ac_cases[i].value) {
                printf("ERRO AC: Original=(%d,%d), Decodificado=(%d,%d)\n", 
                       ac_cases[i].run, ac_cases[i].value, run_length, value);
                errors_ac++;
            }
        }
    }
    
    // Resultados finais
    printf("\nResultado do teste Huffman Roundtrip:\n");
    printf("Testes DC: %d erros de %d testes\n", errors_dc, num_dc_values);
    printf("Testes AC: %d erros de %d testes\n", errors_ac, num_ac_cases);
    
    if (errors_dc == 0 && errors_ac == 0) {
        printf("SUCESSO: Todos os testes Huffman passaram!\n");
    } else {
        printf("FALHA: Encontrados erros nos testes Huffman!\n");
    }
    
    free_bit_buffer(buffer);
    printf("********************************************\n\n");
}

long fsize(const char *filename)
{
    /*
     * Retorna o tamanho do arquivo em bytes.
     * 
     * Parâmetros:
     * filename: caminho do arquivo
     */
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return -1;  // Retorna -1 em caso de erro
    }
    long size;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    return size;
}
