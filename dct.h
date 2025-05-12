#ifndef _DCT_H_
    #define _DCT_H_

    void precomputeCosines(float cosTable[8][8]);
    void forwardDCT(float block[8][8], float Dctfrequencies[8][8]);
    void inverseDCT(float Dctfrequencies[8][8], float block[8][8]);
    void precomputeTransformation(float C[8][8]);
    void forwardDCTMatrix(float block[8][8], float Dctfrequencies[8][8]);
    void inverseDCTMatrix(float Dctfrequencies[8][8], float block[8][8]);
    void MatrixMulFirstTransp(float A[8][8], float B[8][8], float Dest[8][8]);
    void MatrixMulSecTransp(float A[8][8], float B[8][8], float Dest[8][8]);





#endif