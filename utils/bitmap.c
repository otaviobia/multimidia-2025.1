/* Esse arquivo é responsável por ler e escrever cabeçalhos de arquivos BMP,
 * ler pixels do arquivo BMP e armazená-los em uma estrutura de pixels.
 * Também imprime os cabeçalhos lidos.
 */
#include <stdlib.h>
#include "bitmap.h"

void loadBMPHeaders (FILE *fp, BITMAPFILEHEADER *FileHeader, BITMAPINFOHEADER *InfoHeader) {
    /*
     * Lê os cabeçalhos do arquivo BMP, armazena as informações nas estruturas
     * FileHeader e InfoHeader.
     */
    readHeader(fp, FileHeader);
    readInfoHeader(fp, InfoHeader);

    // Para a execução se o arquivo BMP já estiver comprimido
    if (InfoHeader->Compression != 0) {
        printf("This is a compressed BMP!!!");
        fclose(fp);
        return;
    }
}

void readHeader(FILE *F, BITMAPFILEHEADER *H) {
    /*
     * Usa a função fread para ler o cabeçalho do arquivo BMP e armazena os campos
     * na estrutura H.
     */
    fread(&H->Type,sizeof (unsigned short int),1,F);
    fread(&H->Size,sizeof (unsigned int),1,F);
    fread(&H->Reserved1,sizeof (unsigned short int),1,F);
    fread(&H->Reserved2,sizeof (unsigned short int),1,F);
    fread(&H->OffBits,sizeof (unsigned int),1,F);
}

void readInfoHeader(FILE *F, BITMAPINFOHEADER *INFO_H) {
    /*
     * Usa a função fread para ler o cabeçalho de informações do arquivo BMP e armazena
     * os campos na estrutura INFO_H.
     */
    fread(&INFO_H->Size,sizeof (unsigned int),1,F);
    fread(&INFO_H->Width,sizeof (int),1,F);
    fread(&INFO_H->Height,sizeof (int),1,F);
    fread(&INFO_H->Planes,sizeof (unsigned short int),1,F);
    fread(&INFO_H->BitCount,sizeof (unsigned short int),1,F);
    fread(&INFO_H->Compression,sizeof (unsigned int),1,F);
    fread(&INFO_H->SizeImage,sizeof (unsigned int),1,F);
    fread(&INFO_H->XResolution,sizeof (int),1,F);
    fread(&INFO_H->YResolution,sizeof (int),1,F);
    fread(&INFO_H->NColours,sizeof (unsigned int),1,F);
    fread(&INFO_H->ImportantColours,sizeof (unsigned int),1,F);        
}

void printHeaders (BITMAPFILEHEADER *FileHeader,  BITMAPINFOHEADER *InfoHeader) {
    /*
     * Imprime os campos das estruturas FileHeader e InfoHeader.
     */
    printf("*************** File Header ***************\n\n");
    
    printf("Magic number for file: %x\n", FileHeader->Type);   
    printf("File's size: %d\n",FileHeader->Size);           
    printf("Offset to bitmap data: %d\n", FileHeader->OffBits);
    
    printf("\n\n");
    printf("*************** Info Header ***************\n\n");
    printf("Info header's size: %d\n", InfoHeader->Size);
    printf("Width: %d\n", InfoHeader->Width);          
    printf("Height: %d\n",InfoHeader->Height);
    printf("Color planes: %d\n", InfoHeader->Planes);
    printf("Bits per pixel: %d\n", InfoHeader->BitCount);
    printf("Compression type (0 = no compression): %d\n", InfoHeader->Compression);
    printf("Image's data size: %d\n", InfoHeader->SizeImage);
    printf("X Pixels per meter: %d\n", InfoHeader->XResolution);
    printf("Y Pixels per meter: %d\n", InfoHeader->YResolution);
    printf("Number of colors: %d\n", InfoHeader->NColours);
    printf("Number of important colors: %d\n", InfoHeader->ImportantColours); 
}

void readPixels(FILE *input, BITMAPINFOHEADER InfoHeader, BITMAPFILEHEADER FileHeader, PIXELRGB *Image) {
    /* 
     * Lê os pixels do arquivo BMP e armazena no vetor de pixels Image.
     * A imagem é lida em ordem BGR (azul, verde, vermelho).
     */    
    fseek(input, FileHeader.OffBits, SEEK_SET); // pular o header
    
    int tam = InfoHeader.Height * InfoHeader.Width;
    for (int i = 0; i < tam; i++) {
        Image[i].B = fgetc(input);
        Image[i].G = fgetc(input);
        Image[i].R = fgetc(input);
    }
}

void writeHeaders(FILE *output, BITMAPFILEHEADER FileHeader, BITMAPINFOHEADER InfoHeader) {
    /*
     * Escreve os cabeçalhos do arquivo BMP no arquivo de saída.
     */
    fseek(output, 0, SEEK_SET);
    
    // Escreve FileHeader
    fwrite(&FileHeader.Type, sizeof (unsigned short int), 1, output);
    fwrite(&FileHeader.Size, sizeof (unsigned int), 1, output);
    fwrite(&FileHeader.Reserved1, sizeof (unsigned short int), 1, output);
    fwrite(&FileHeader.Reserved2, sizeof (unsigned short int), 1, output);
    fwrite(&FileHeader.OffBits, sizeof (unsigned int), 1, output);
    
    // Escreve InfoHeader
    fwrite(&InfoHeader.Size, sizeof (unsigned int), 1, output);
    fwrite(&InfoHeader.Width, sizeof (int), 1, output);
    fwrite(&InfoHeader.Height, sizeof (int), 1, output);
    fwrite(&InfoHeader.Planes, sizeof (unsigned short int), 1, output);
    fwrite(&InfoHeader.BitCount, sizeof (unsigned short int), 1, output);
    fwrite(&InfoHeader.Compression, sizeof (unsigned int), 1, output);
    fwrite(&InfoHeader.SizeImage, sizeof (unsigned int), 1, output);
    fwrite(&InfoHeader.XResolution, sizeof (int), 1, output);
    fwrite(&InfoHeader.YResolution, sizeof (int), 1, output);
    fwrite(&InfoHeader.NColours, sizeof (unsigned int), 1, output);
    fwrite(&InfoHeader.ImportantColours, sizeof (unsigned int), 1, output);
}

void writeBMP(FILE *output, BITMAPFILEHEADER FileHeader, BITMAPINFOHEADER InfoHeader, PIXELRGB *Image) {
    // Escreve os cabeçalhos
    writeHeaders(output, FileHeader, InfoHeader);

    // Escreve os pixels
    for (int i = 0; i < (InfoHeader.Height * InfoHeader.Width); i++) {
        fputc(Image[i].B, output);
        fputc(Image[i].G, output);
        fputc(Image[i].R, output);
    }
}

unsigned char clampFloatToByte(float value) {
    /*
     * Clampa um valor float para o intervalo [0, 255] e converte para unsigned char.
     * Se o valor for menor que 0, retorna 0. Se for maior que 255, retorna 255.
     */
    if (value < 0.0f) return 0;
    if (value > 255.0f) return 255;
    return (unsigned char)(value + 0.5f); // arredonda
}

void convertToYCBCR(PIXELRGB *Image, PIXELYCBCR *ImageYCbCr, int tam) {
    /* 
     * Converte pixels de uma imagem RGB para YCbCr.
     */
    for (int i = 0; i < tam; i++) {
        float R = Image[i].R;
        float G = Image[i].G;
        float B = Image[i].B;

        float Y  =  0.299f * R + 0.587f * G + 0.114f * B;
        float Cb = -0.1687f * R - 0.3313f * G + 0.5f * B + 128.0f;
        float Cr =  0.5f * R - 0.4187f * G - 0.0813f * B + 128.0f;

        ImageYCbCr[i].Y  = clampFloatToByte(Y);
        ImageYCbCr[i].Cb = clampFloatToByte(Cb);
        ImageYCbCr[i].Cr = clampFloatToByte(Cr);
    }
}

void convertToRGB(PIXELYCBCR *ImageYCbCr, PIXELRGB *Image, int tam) {
    /* 
     * Converte pixels de uma imagem YCbCr para RGB.
     */
    for (int i = 0; i < tam; i++) {
        float Y  = ImageYCbCr[i].Y;
        float Cb = ImageYCbCr[i].Cb - 128.0f;
        float Cr = ImageYCbCr[i].Cr - 128.0f;

        float R = Y + 1.402f * Cr;
        float G = Y - 0.344136f * Cb - 0.714136f * Cr;
        float B = Y + 1.772f * Cb;

        Image[i].R = clampFloatToByte(R);
        Image[i].G = clampFloatToByte(G);
        Image[i].B = clampFloatToByte(B);
    }
}
