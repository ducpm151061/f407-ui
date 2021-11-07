#ifndef __EXFUNS_H
#define __EXFUNS_H
#include "ff.h"
#include <stm32f4xx.h>

extern FATFS *fs[2];
extern FIL *file;
extern FIL *ftemp;
extern UINT br, bw;
extern FILINFO fileinfo;
extern DIR dir;
extern u8 *fatbuf;

#define T_BIN 0X00  // bin
#define T_LRC 0X10  // lrc
#define T_NES 0X20  // nes
#define T_TEXT 0X30 //.txt
#define T_C 0X31    //.c
#define T_H 0X32    //.h
#define T_FLAC 0X4C // flac
#define T_BMP 0X50  // bmp
#define T_JPG 0X51  // jpg
#define T_JPEG 0X52 // jpeg
#define T_GIF 0X53  // gif

u8 exfuns_init(void);
u8 f_typetell(u8 *fname);
u8 exf_getfree(u8 *drv, u32 *total, u32 *free);
u32 exf_fdsize(u8 *fdname);
#endif
