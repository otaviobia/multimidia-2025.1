#ifndef TEST_H
    #define TEST_H

    #include "bitmap.h"
    #include "codec.h"
    #include "dct.h"
    #include <stdlib.h>
    #include <string.h>

    void compareRGB(const PIXELRGB *orig, const PIXELRGB *recon, int count);
    void compareBlock(const float Block[8][8], const float RecBlock[8][8]);
    void DCTBenchComparison(const float Dctfrequencies0[8][8], const float Dctfrequencies1[8][8], const float reconstructedBlock0[8][8], const float reconstructedBlock1[8][8]);
    void testImageSubsampling(PIXELYCBCR *image, int width, int height);
    void compareYBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height);
    void compareCbCrBlock(const PIXELYCBCR *orig, const PIXELYCBCR *recon, int start_x, int start_y, int width, int height);

#endif