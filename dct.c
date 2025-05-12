#include "dct.h"
#include <math.h>
#define M_PI 3.14159265358979323846

void precomputeCosines(float cosTable[8][8]) {
    const double fac = M_PI / 16.0;

    for(int u = 0; u < 8; u++){        // u = i or j
        for(int z = 0; z < 8; z++){    // z = x or y
            cosTable[u][z] = cos( (2*z + 1) * u * fac );
        }
    }
}

void forwardDCT(float block[8][8], float Dctfrequencies[8][8]) {
    float C[8] = {1.0/sqrt(2),1.0,1.0,1.0,1.0,1.0,1.0,1.0};

    double sum;
    float cosTable[8][8];
    precomputeCosines(cosTable);

    for(int i = 0;i<8;i++){ // i,j = F[i][j]
        for(int j = 0;j<8;j++){
            sum = 0.0;
             for(int x = 0;x<8;x++){ // x,y = ∑[x] & ∑[y] 
                for(int y = 0;y<8;y++){
                    sum += block[x][y]*cosTable[i][x]*cosTable[j][y];
                }
            }
            Dctfrequencies[i][j] = C[i]*C[j]*sum/4.0;
        }
    }
}

void inverseDCT(float Dctfrequencies[8][8], float block[8][8]) {
    float C[8] = {1.0/sqrt(2),1.0,1.0,1.0,1.0,1.0,1.0,1.0};

    double sum;
    float cosTable[8][8];
    precomputeCosines(cosTable);

    for(int x = 0;x<8;x++){ // x,y = P[x][y]
        for(int y = 0;y<8;y++){
            sum = 0.0;
             for(int i = 0;i<8;i++){ // i,j = ∑[i] & ∑[j] 
                for(int j = 0;j<8;j++){
                    sum += C[i]*C[j]*Dctfrequencies[i][j]*cosTable[i][x]*cosTable[j][y];
                }
            }
            block[x][y] = sum/4.0;
        }
    }
}