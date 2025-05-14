#include "test.h"

void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int tam) {
    
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
        printf("*************** Rgb Comparison Info ***************\n");
        printf("No differences found.\n");
    } else {
        printf("*************** Rgb Comparison Info ***************\n");
        printf("Differences found: %d\n", cont);
        printf("Biggest differences is R: %d G: %d B: %d \n", biggestR, biggestG, biggestB);
    }
}

void compareBlock(const float Block[8][8], const float RecBlock[8][8]) {
    int diffCont = 0; 
    float maxDiff = 0.0f; 
    float diff = 0.01f; 

    printf("*************** Block Comparison Info ***************\n");

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
    int diffcont= 0; // count of diff in dct
    int diffcontRec = 0; // Count of diff in idct
    float maxDiffDCT = 0.0f; // Max diff in dct
    float maxDiffRec = 0.0f; // Max diff in idct
    float diff = 0.01f; // Diff analyzed

    printf("*************** DCT Comparison Info ***************\n");

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

void compareYBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height) {
    int cont = 0;
    int diff = 2;
    int biggestDiff = 0;
    
    printf("*************** Y Block Comparison Info ***************\n");

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
        printf("No differences found.\n");
    } else {
        printf("Differences found: %d\n", cont);
        printf("Biggest Y difference: %d\n", biggestDiff);
    }
}

void compareCbCrBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height) {
    int contCb = 0, contCr = 0;
    int diff = 2;
    int biggestCb = 0, biggestCr = 0;
    
    printf("*************** CbCr Block Comparison Info ***************\n");

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
        //printf("No differences found in CbCr block.\n");
    } else {
        if (contCb > 0) {
            printf("Differences found in Cb: %d at startx %d & starty %d\n", contCb,start_x,start_y);
            //printf("Biggest Cb difference: %d\n", biggestCb);
        }
        if (contCr > 0) {
            printf("Differences found in Cr: %d at startx %d & starty %d\n", contCr,start_x,start_y);
            //printf("Biggest Cr difference: %d\n", biggestCr);
        }
    }
}

void testImageSubsampling(PIXELYCBCR *image, int width, int height) {
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
                //compareYBlock(image, recon, ox, oy, width, height);
            }

            // Test CbCr (16x16 block)
            float cb_temp[8][8], cr_temp[8][8];
            
            // Extract and immediately reconstruct Cb and Cr
            extract_block_chroma420(image, cb_temp, bx, by, width, height, 'B');
            extract_block_chroma420(image, cr_temp, bx, by, width, height, 'R');
            
            reconstructBlock8x8_CbCr420(recon, cb_temp, bx, by, width, height, 'B');
            reconstructBlock8x8_CbCr420(recon, cr_temp, bx, by, width, height, 'R');
            
            // Compare the block
            compareCbCrBlock(image, recon, bx, by, width, height);
        }
    }

    printf("\nFull Image Statistics:\n");
    printf("Image dimensions: %dx%d\n", width, height);
    printf("Number of 16x16 macroblocks: %dx%d\n", 
           (width + 15) / 16, (height + 15) / 16);

    free(recon);
}