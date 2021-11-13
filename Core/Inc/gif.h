#ifndef __GIF_H__
#define __GIF_H__
#include "ff.h"
#include "sys.h"

#define GIF_USE_MALLOC 1

#define LCD_MAX_LOG_COLORS 256
#define MAX_NUM_LWZ_BITS 12

#define GIF_INTRO_TERMINATOR ';' // 0X3B
#define GIF_INTRO_EXTENSION '!'  // 0X21
#define GIF_INTRO_IMAGE ','      // 0X2C

#define GIF_COMMENT 0xFE
#define GIF_APPLICATION 0xFF
#define GIF_PLAINTEXT 0x01
#define GIF_GRAPHICCTL 0xF9

typedef struct
{
    u8 aBuffer[258];                      // Input buffer for data block
    short aCode[(1 << MAX_NUM_LWZ_BITS)]; // This array stores the LZW codes for the compressed strings
    u8 aPrefix[(1 << MAX_NUM_LWZ_BITS)];  // Prefix character of the LZW code.
    u8 aDecompBuffer[3000]; // Decompression buffer. The higher the compression, the more bytes are needed in the
                            // buffer.
    u8 *sp;                 // Pointer into the decompression buffer
    int CurBit;
    int LastBit;
    int GetDone;
    int LastByte;
    int ReturnClear;
    int CodeSize;
    int SetCodeSize;
    int MaxCode;
    int MaxCodeSize;
    int ClearCode;
    int EndCode;
    int FirstCode;
    int OldCode;
} LZW_INFO;

typedef struct __attribute__((packed))
{
    u16 width;
    u16 height;
    u8 flag;
    u8 bkcindex;
    u8 pixratio;
} LogicalScreenDescriptor;

typedef struct __attribute__((packed))
{
    u16 xoff;
    u16 yoff;
    u16 width;
    u16 height;
    u8 flag;
} ImageScreenDescriptor;

typedef struct __attribute__((packed))
{
    LogicalScreenDescriptor gifLSD;
    ImageScreenDescriptor gifISD;
    u16 colortbl[256];
    u16 bkpcolortbl[256];
    u16 numcolors;
    u16 delay;
    LZW_INFO *lzw;
} gif89a;

extern u8 gifdecoding;
u8 gif_check_head(FIL *file);
u16 gif_getrgb565(u8 *ctb);
u8 gif_readcolortbl(FIL *file, gif89a *gif, u16 num);
u8 gif_getinfo(FIL *file, gif89a *gif);
void gif_savegctbl(gif89a *gif);
void gif_recovergctbl(gif89a *gif);
void gif_initlzw(gif89a *gif, u8 codesize);
u16 gif_getdatablock(FIL *gfile, u8 *buf, u16 maxnum);
u8 gif_readextension(FIL *gfile, gif89a *gif, int *pTransIndex, u8 *pDisposal);
int gif_getnextcode(FIL *gfile, gif89a *gif);
int gif_getnextbyte(FIL *gfile, gif89a *gif);
u8 gif_dispimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0, int Transparency, u8 Disposal);
void gif_clear2bkcolor(u16 x, u16 y, gif89a *gif, ImageScreenDescriptor pimge);
u8 gif_drawimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0);

u8 gif_decode(const u8 *filename, u16 x, u16 y, u16 width, u16 height);
void gif_quit(void);
#endif /* __GIF_H__ */
