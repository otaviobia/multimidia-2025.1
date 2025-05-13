#include "test.h"
#include "dct.h"

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