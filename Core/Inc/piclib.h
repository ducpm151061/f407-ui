#ifndef __PICLIB_H
#define __PICLIB_H
#include "bmp.h"
#include "custom_malloc.h"
#include "exfuns.h"
#include "ff.h"
#include "gif.h"
#include "lcd.h"
#include "sys.h"
#include "tjpgd.h"

#define PIC_FORMAT_ERR 0x27
#define PIC_SIZE_ERR 0x28
#define PIC_WINDOW_ERR 0x29
#define PIC_MEM_ERR 0x11
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

typedef struct
{
    u16 (*read_point)(u16, u16);                  // u16 read_point(u16 x,u16 y)
    void (*draw_point)(u16, u16, u16);            // void draw_point(u16 x,u16 y,u16 color)
    void (*fill)(u16, u16, u16, u16, u16);        /// void fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
    void (*draw_hline)(u16, u16, u16, u16);       // void draw_hline(u16 x0,u16 y0,u16 len,u16 color)
    void (*fillcolor)(u16, u16, u16, u16, u16 *); // void piclib_fill_color(u16 x,u16 y,u16 width,u16 height,u16 *color)
} _pic_phy;

extern _pic_phy pic_phy;

typedef struct
{
    u16 lcdwidth;
    u16 lcdheight;
    u32 ImgWidth;
    u32 ImgHeight;

    u32 Div_Fac;

    u32 S_Height;
    u32 S_Width;

    u32 S_XOFF;
    u32 S_YOFF;

    u32 staticx;
    u32 staticy;
} _pic_info;
extern _pic_info picinfo;
void piclib_init(void);
u16 piclib_alpha_blend(u16 src, u16 dst, u8 alpha);
void ai_draw_init(void);
u8 is_element_ok(u16 x, u16 y, u8 chg);
u8 ai_load_picfile(const u8 *filename, u16 x, u16 y, u16 width, u16 height, u8 fast);
void *pic_memalloc(u32 size);
void pic_memfree(void *mf);
#endif /* __PICLIB_H */
