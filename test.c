#include "bitmap.h"

void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int tam) {
    
    int cont = 0;
    int diff = 2;
    int biggestR = 0;
    int biggestG = 0;
    int biggestB = 0;

    for (int i = 0; i < tam; i++) {
        int dR = (int)recon[i].R - (int)orig[i].R;
        int dG = (int)recon[i].G - (int)orig[i].G;
        int dB = (int)recon[i].B - (int)orig[i].B;

        if(dR > diff || dR < -diff || dG > diff || dG < -diff || dB > diff || dG < -diff) {
            cont++;
            biggestR = (dR > biggestR) ? dR : biggestR;
            biggestG = (dG > biggestG) ? dG : biggestG;
            biggestB = (dB > biggestB) ? dB : biggestB;
        }
    }
    if (cont == 0){
        printf("*************** Test Info ***************\n");
        printf("No differences found.\n");
    } else {
        printf("*************** Test Info ***************\n");
        printf("Differences found: %d\n", cont);
        printf("Biggest differences is R: %d G: %d B: %d \n", biggestR, biggestG, biggestB);
    }
}
