/* Esse arquivo é responsável por implementar a Transformada Discreta de Cosseno (DCT) e sua inversa (IDCT).
 * A DCT é usada para transformar blocos de pixels do domínio espacial para o domínio de frequência.
 */ 
#include "dct.h"
#define M_PI 3.14159265358979323846

void precomputeCosines(float cosTable[8][8]) {
    /*
     * Pré-calcula os valores de cosseno necessários para a DCT.
     *
     * Parâmetros:
     * cosTable: matriz 8x8 onde serão armazenados os valores de cosseno
     */
    const double fac = M_PI / 16.0;

    for(int u = 0; u < 8; u++){        // u = i or j
        for(int z = 0; z < 8; z++){    // z = x or y
            cosTable[u][z] = cos( (2*z + 1) * u * fac );
        }
    }
}

void precomputeTransformation(float C[8][8]) {
    /*
     * Pré-calcula a matriz de transformação C para uso na DCT.
     * Aplica os fatores de normalização adequados aos valores de cosseno.
     *
     * Parâmetros:
     * C: matriz 8x8 de transformação a ser preenchida
     */
    float cosTable[8][8];
    precomputeCosines(cosTable);

    const float sqrt1 = 1.0f / sqrt(8);
    static const float sqrt2 = 1.0f / 2.0f;

    for (int i = 0; i < 8; i++) {
        float alpha = (i == 0) ? sqrt1 : sqrt2;
        for (int j = 0; j < 8; j++) {
            C[i][j] = alpha * cosTable[i][j];
        }
    }
}

void forwardDCT(float block[8][8], float Dctfrequencies[8][8]) {
    /*
     * Implementação direta da DCT (Transformada Discreta de Cosseno) usando a fórmula matemática.
     * Transforma um bloco do domínio espacial para o domínio de frequência.
     *
     * Parâmetros:
     * block: bloco 8x8 no domínio espacial (entrada)
     * Dctfrequencies: bloco 8x8 no domínio de frequência (saída)
     */
    // Inicializa a matriz de destino com zeros
    memset(Dctfrequencies, 0, sizeof(float) * 64);
    
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
            Dctfrequencies[i][j] = (float)round(C[i]*C[j]*sum/4.0);
        }
    }
}

void inverseDCT(float Dctfrequencies[8][8], float block[8][8]) {
    /*
     * Implementação direta da IDCT (Transformada Discreta de Cosseno Inversa) usando a fórmula matemática.
     * Transforma um bloco do domínio de frequência de volta para o domínio espacial.
     *
     * Parâmetros:
     * Dctfrequencies: bloco 8x8 no domínio de frequência (entrada)
     * block: bloco 8x8 no domínio espacial (saída)
     */
    // Inicializa a matriz de destino com zeros
    memset(block, 0, sizeof(float) * 64);
    
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
            block[x][y] = (float)round(sum/4.0);
        }
    }
}
void MatrixMul(float A[8][8], float B[8][8], float Dest[8][8]) {
    /*
     * Multiplica duas matrizes 8x8 (A * B) e armazena o resultado em Dest.
     *
     * Parâmetros:
     * A: primeira matriz 8x8
     * B: segunda matriz 8x8
     * Dest: matriz 8x8 de destino para o resultado
     */
    memset(Dest, 0, sizeof(float) * 64);
    for (int i = 0; i < 8; i++) { // loop over rows of A
        for (int j = 0; j < 8; j++) { // loop over columns of B
            Dest[i][j] = 0;
            for(int k = 0; k < 8; k++) { // Loop over columns of A and rows of B
                Dest[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void MatrixMulSecTransp(float A[8][8], float B[8][8], float Dest[8][8]) {
    /*
     * Multiplica matriz A pela transposta da matriz B (A * B^T) e armazena o resultado em Dest.
     *
     * Parâmetros:
     * A: primeira matriz 8x8
     * B: segunda matriz 8x8 (será transposta)
     * Dest: matriz 8x8 de destino para o resultado
     */
    memset(Dest, 0, sizeof(float) * 64);
    for (int i = 0; i < 8; i++) { // loop over rows of A	
        for (int j = 0; j < 8; j++) { // loop over rows of B
            for(int k = 0; k < 8; k++) { // Loop over columns of A and B^T
                Dest[i][j] += A[i][k] * B[j][k]; // Inverted B to simulate transpose
            }
        }
    }
}

void MatrixMulFirstTransp(float A[8][8], float B[8][8], float Dest[8][8]) {
    /*
     * Multiplica a transposta da matriz A pela matriz B (A^T * B) e armazena o resultado em Dest.
     *
     * Parâmetros:
     * A: primeira matriz 8x8 (será transposta)
     * B: segunda matriz 8x8
     * Dest: matriz 8x8 de destino para o resultado
     */
    memset(Dest, 0, sizeof(float) * 64);
    for (int i = 0; i < 8; i++) { // loop over rows of A	
        for (int j = 0; j < 8; j++) { // loop over rows of B
            for(int k = 0; k < 8; k++) { // Loop over columns of A^t and B
                Dest[i][j] += A[k][i] * B[k][j]; // Inverted A to simulate transpose
            }
        }
    }
}

void forwardDCTMatrix(float block[8][8], float Dctfrequencies[8][8]) {
    /*
     * Aplica a Transformada Discreta de Cosseno (DCT) em um bloco 8x8 usando multiplicação de matrizes.
     *
     * Parâmetros:
     * block: bloco 8x8 no domínio espacial (entrada)
     * Dctfrequencies: bloco 8x8 no domínio de frequência (saída)
     */
    memset(Dctfrequencies, 0, sizeof(float) * 64);
    float C[8][8];
    precomputeTransformation(C);
    float temp[8][8] = {0};
    MatrixMul(C, block, temp);
    MatrixMulSecTransp(temp, C, Dctfrequencies);
    // Dct = C * block * C^T 
    // temp = C * B => Dct = temp * C^T 
}

void inverseDCTMatrix(float Dctfrequencies[8][8], float block[8][8]) {
    /*
     * Aplica a Transformada Discreta de Cosseno Inversa (IDCT) em um bloco 8x8 usando multiplicação de matrizes.
     *
     * Parâmetros:
     * Dctfrequencies: bloco 8x8 no domínio de frequência (entrada)
     * block: bloco 8x8 no domínio espacial (saída)
     */
    memset(block, 0, sizeof(float) * 64);	
    float C[8][8];
    precomputeTransformation(C);
    float temp[8][8] = {0};

    MatrixMulFirstTransp(C, Dctfrequencies, temp);
    MatrixMul(temp, C, block);
    // IDct = C^T * Dctf * C 
    // temp = C^T * Dctf => IDct = temp * C 

}