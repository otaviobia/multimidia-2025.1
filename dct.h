#ifndef _DCT_H_
    #define _DCT_H_

    void precomputeCosines(float cosTable[8][8]);
    void forwardDCT(float block[8][8], float Dctfrequencies[8][8]);
    void inverseDCT(float Dctfrequencies[8][8], float block[8][8]);

#endif