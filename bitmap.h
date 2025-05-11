#ifndef _BITMAP_H_
    #define _BITMAP_H_
    #include <stdio.h>
    typedef struct {                     /**** BMP file header structure ****/ 
        unsigned short Type;             /* Magic number for file */
        unsigned int   Size;             /* Size of file */
        unsigned short Reserved1;        /* Reserved */
        unsigned short Reserved2;        /* ... */
        unsigned int   OffBits;          /* Offset to bitmap data */
    } BITMAPFILEHEADER;

    #define BF_TYPE 0x4D42               /* "MB" */

    typedef struct {                     /**** BMP file info structure ****/
        unsigned int   Size;             /* Size of info header */
        int            Width;            /* Width of image */
        int            Height;           /* Height of image */
        unsigned short Planes;           /* Number of color planes */
        unsigned short BitCount;         /* Number of bits per pixel */
        unsigned int   Compression;      /* Type of compression to use */
        unsigned int   SizeImage;        /* Size of image data */
        int            XResolution;      /* X pixels per meter */
        int            YResolution;      /* Y pixels per meter */
        unsigned int   NColours;         /* Number of colors used */
        unsigned int   ImportantColours; /* Number of important colors */
    } BITMAPINFOHEADER;

    typedef struct {                     /**** Pixel structure in RGB ****/
        unsigned char R;                 /* Red */
        unsigned char G;                 /* Green */
        unsigned char B;                 /* Blue */
    } PIXELRGB;

    typedef struct {                     /**** Pixel structure in YCbCr ****/
        unsigned char Y;                 /* Luma */
        unsigned char Cb;                /* Chroma Blue */
        unsigned char Cr;                /* Chroma Red */
    } PIXELYCBCR;
            
    void loadBMPHeaders (FILE *fp, BITMAPFILEHEADER *FileHeader, BITMAPINFOHEADER *InfoHeader);
    void readInfoHeader(FILE *F, BITMAPINFOHEADER *INFO_H);
    void readHeader(FILE *F, BITMAPFILEHEADER *H);
    void readPixels(FILE *input, BITMAPINFOHEADER InfoHeader, PIXELRGB *Image);
    void printHeaders (BITMAPFILEHEADER *FileHeader,  BITMAPINFOHEADER *InfoHeader);
    void writeBMP(FILE *output, BITMAPFILEHEADER FileHeader, BITMAPINFOHEADER InfoHeader, PIXELRGB *Image);
    void convertToYCBCR(PIXELRGB *Image, PIXELYCBCR *ImageYCbCr, int tam);
    void convertToRGB(PIXELYCBCR *ImageYCbCr, PIXELRGB *Image, int tam);
#endif