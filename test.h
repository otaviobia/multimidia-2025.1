#ifndef TEST_H
    #define TEST_H

    #include "bitmap.h"

    void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int count);
    void compareBlock(const float Block[8][8], const float RecBlock[8][8]);
    void DCTBenchComparison(const float Dctfrequencies0[8][8], const float Dctfrequencies1[8][8], const float reconstructedBlock0[8][8], const float reconstructedBlock1[8][8]);

#endif