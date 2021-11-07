#include "text.h"
#include "fontupd.h"
#include "gui.h"
#include "lcd.h"
#include "string.h"
#include "sys.h"
#include "usart.h"
#include "w25q64.h"

void Get_HzMat(unsigned char *code, unsigned char *mat, u8 size)
{
    unsigned char qh, ql;
    unsigned char i;
    unsigned long foffset;
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);
    qh = *code;
    ql = *(++code);
    if (qh < 0x81 || ql < 0x40 || ql == 0xff || qh == 0xff)
    {
        for (i = 0; i < csize; i++)
            *mat++ = 0x00;
        return;
    }
    if (ql < 0x7f)
        ql -= 0x40;
    else
        ql -= 0x41;
    qh -= 0x81;
    foffset = ((unsigned long)190 * qh + ql) * csize;
    switch (size)
    {
    case 12:
        W25Q64_Read(mat, foffset + ftinfo.f12addr, csize);
        break;
    case 16:
        W25Q64_Read(mat, foffset + ftinfo.f16addr, csize);
        break;
    case 24:
        W25Q64_Read(mat, foffset + ftinfo.f24addr, csize);
        break;
    }
}
void Show_Font(u16 x, u16 y, char *font, u8 size, u8 mode)
{
    u8 temp, t, t1;
    u16 y0 = y;
    u8 dzk[72];
    u8 csize = (size / 8 + ((size % 8) ? 1 : 0)) * (size);
    if (size != 12 && size != 16 && size != 24)
        return;
    Get_HzMat(font, dzk, size);
    for (t = 0; t < csize; t++)
    {
        temp = dzk[t];
        for (t1 = 0; t1 < 8; t1++)
        {
            if (temp & 0x80)
                GUI_DrawPoint(x, y, POINT_COLOR);
            else if (mode == 0)
                GUI_DrawPoint(x, y, BACK_COLOR);
            temp <<= 1;
            y++;
            if ((y - y0) == size)
            {
                y = y0;
                x++;
                break;
            }
        }
    }
}

void Show_Str_Mid(u16 x, u16 y, char *str, u8 size, u8 len)
{
    u16 strlenth = 0;
    strlenth = strlen((const char *)str);
    strlenth *= size / 2;
    if (strlenth > len)
        Show_Str(x, y, lcddev.width, lcddev.height, str, size, 1);
    else
    {
        strlenth = (len - strlenth) / 2;
        Show_Str(strlenth + x, y, lcddev.width, lcddev.height, str, size, 1);
    }
}
