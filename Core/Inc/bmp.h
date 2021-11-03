#ifndef __BMP_H__
#define __BMP_H__
#include "sys.h"

#define BMP_USE_MALLOC 1
#define BMP_DBUF_SIZE 2048

typedef __PACKED struct
{
    u32 biSize;
    long biWidth;
    long biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    long biXPelsPerMeter;
    long biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
} BITMAPINFOHEADER;

typedef __PACKED struct
{
    u16 bfType;
    u32 bfSize;
    u16 bfReserved1;
    u16 bfReserved2;
    u32 bfOffBits;
} BITMAPFILEHEADER;

typedef __PACKED struct
{
    u8 rgbBlue;
    u8 rgbGreen;
    u8 rgbRed;
    u8 rgbReserved;
} RGBQUAD;
typedef __PACKED struct
{
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bmiHeader;
    u32 RGB_MASK[3];
} BITMAPINFO;
typedef RGBQUAD *LPRGBQUAD;

#define BI_RGB 0
#define BI_RLE8 1
#define BI_RLE4 2
#define BI_BITFIELDS 3

u8 stdbmp_decode(const u8 *filename);
u8 minibmp_decode(u8 *filename, u16 x, u16 y, u16 width, u16 height, u16 acolor, u8 mode);
u8 bmp_encode(u8 *filename, u16 x, u16 y, u16 width, u16 height, u8 mode);
#endif
