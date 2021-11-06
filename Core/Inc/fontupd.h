#ifndef __FONTUPD_H__
#define __FONTUPD_H__
#include <stm32f4xx.h>

extern u32 FONTINFOADDR;
typedef struct __attribute__((packed))
{
    u8 fontok;
    u32 ugbkaddr;
    u32 ugbksize;
    u32 f12addr;
    u32 gbk12size;
    u32 f16addr;
    u32 gbk16size;
    u32 f24addr;
    u32 gkb24size;
} _font_info;

extern _font_info ftinfo;

u32 fupd_prog(u16 x, u16 y, u8 size, u32 fsize, u32 pos);
u8 updata_fontx(u16 x, u16 y, u8 size, u8 *fxpath, u8 fx);
u8 update_font(u16 x, u16 y, u8 size, u8 *src);
u8 font_init(void);
#endif
