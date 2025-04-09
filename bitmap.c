#include <stdio.h>
#include <stdlib.h>

typedef struct /**** BMP file header structure ****/
{
    unsigned short bfType; /* Magic number for file = "MB" */
    unsigned int bfSize; /* Size of file */
    unsigned short bfReserved1; /* Reserved */
    unsigned short bfReserved2; /* ... */
    unsigned int bfOffBits; /* Offset to bitmap data */
} BMPFILEHEADER;

typedef struct /**** BMP file info structure ****/
{
    unsigned int biSize; /* Size of info header */
    int biWidth; /* Width of image */
    int biHeight; /* Height of image */
    unsigned short biPlanes; /* Number of color planes */
    unsigned short biBitCount; /* Number of bits per pixel */
    unsigned int biCompression; /* Type of compression to use */
    unsigned int biSizeImage; /* Size of image data */
    int biXPelsPerMeter; /* X pixels per meter */
    int biYPelsPerMeter; /* Y pixels per meter */
    unsigned int biClrUsed; /* Number of colors used */
    unsigned int biClrImportant; /* Number of important colors */
} BMPINFOHEADER;

typedef struct /**** Pixel info structure ****/
{
    unsigned int R; /* Red value of pixel (0-255)*/
    unsigned int G; /* Green value of pixel (0-255)*/
    unsigned int B; /* Blue value of pixel (0-255)*/
} PIXEL;

void leituraHeader(FILE *F, BMPFILEHEADER *H) {
    /*F é o arquivo Bitmap que deve ter sido “lido” do disco*/
    fread(&H->bfType,sizeof (unsigned short int),1,F);
    fread(&H->bfSize,sizeof (unsigned int),1,F);
    fread(&H->bfReserved1,sizeof (unsigned short int),1,F);
    fread(&H->bfReserved2,sizeof (unsigned short int),1,F);
    fread(&H->bfOffBits,sizeof (unsigned int),1,F);
}

void leituraInfo(FILE *F, BMPINFOHEADER *H) {
    /*F é o arquivo Bitmap que deve ter sido “lido” do disco*/
    fread(&H->biSize,sizeof (unsigned int),1,F);
    fread(&H->biWidth,sizeof (int),1,F);
    fread(&H->biHeight,sizeof (int),1,F);
    fread(&H->biPlanes,sizeof (unsigned short int),1,F);
    fread(&H->biBitCount,sizeof (unsigned short int),1,F);
    fread(&H->biCompression,sizeof (unsigned int),1,F);
    fread(&H->biSizeImage,sizeof (unsigned int),1,F);
    fread(&H->biXPelsPerMeter,sizeof (int),1,F);
    fread(&H->biYPelsPerMeter,sizeof (int),1,F);
    fread(&H->biClrUsed,sizeof (unsigned int),1,F);
    fread(&H->biClrImportant,sizeof (unsigned int),1,F);
}

int main(void) {
    FILE *f = fopen("mybmp.bmp", "r");
    BMPFILEHEADER *bmp = (BMPFILEHEADER*) malloc(sizeof(BMPFILEHEADER));
    BMPINFOHEADER *info = (BMPINFOHEADER*) malloc(sizeof(BMPINFOHEADER));
    leituraHeader(f, bmp);
    leituraInfo(f, info);
    printf("Magic Number: %x\nSize:%hu\nReserved 1: %hu\nReserved 2: %hu\nOffset to data: %hu\n", bmp->bfType, bmp->bfSize, bmp->bfReserved1, bmp->bfReserved2, bmp->bfOffBits);
    printf("%hu %hu %hu %hu %hu %hu %hu %hu %hu %hu %hu\n",
    info->biSize, info->biWidth, info->biHeight, info->biPlanes, info->biBitCount,
    info->biCompression, info->biSizeImage, info->biXPelsPerMeter, info->biYPelsPerMeter,
    info->biClrUsed, info->biClrImportant);
    PIXEL *pixel = (PIXEL *) malloc(sizeof(PIXEL));
    for (int x = 0; x < info->biWidth; x++) {
        for (int y = 0; y < info->biHeight; y++) {
            fread(&pixel->R,1,1,f);
            fread(&pixel->G,1,1,f);
            fread(&pixel->B,1,1,f);
            printf("(%d,%d): r%.2x g%.2x b%.2x\n", x, y, pixel->R, pixel->G, pixel->B);
        }
    }
    free(bmp);
    free(info);
    fclose(f);
    return(0);
}