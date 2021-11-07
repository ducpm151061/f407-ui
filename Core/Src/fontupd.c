#include "fontupd.h"
#include "custom_malloc.h"
#include "delay.h"
#include "ff.h"
#include "gui.h"
#include "lcd.h"
#include "string.h"
#include "usart.h"
#include "w25q64.h"

#define FONTSECSIZE 791
#define FONTINFOADDR 1024 * 1024 * 12

_font_info ftinfo;

char *const GBK24_PATH = "/FONT/GBK24.FON";
char *const GBK16_PATH = "/FONT/GBK16.FON";
char *const GBK12_PATH = "/FONT/GBK12.FON";
char *const UNIGBK_PATH = "/FONT/UNIGBK.BIN";

u32 fupd_prog(u16 x, u16 y, u8 size, u32 fsize, u32 pos)
{
    float prog;
    u8 t = 0XFF;
    prog = (float)pos / fsize;
    prog *= 100;
    if (t != prog)
    {
        LCD_ShowString(x + 3 * size / 2, y, size, (char *)"%", 1);
        t = prog;
        if (t > 100)
            t = 100;
        LCD_ShowNum(x, y, t, 3, size, 0);
    }
    return 0;
}

u8 updata_fontx(u16 x, u16 y, u8 size, u8 *fxpath, u8 fx)
{
    u32 flashaddr = 0;
    FIL *fftemp;
    u8 *tempbuf;
    u8 res;
    u16 bread;
    u32 offx = 0;
    u8 rval = 0;
    fftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    if (fftemp == NULL)
        rval = 1;
    tempbuf = mymalloc(SRAMIN, 4096);
    if (tempbuf == NULL)
        rval = 1;
    res = f_open(fftemp, (const TCHAR *)fxpath, FA_READ);
    if (res)
        rval = 2;
    if (rval == 0)
    {
        switch (fx)
        {
        case 0:
            ftinfo.ugbkaddr = FONTINFOADDR + sizeof(ftinfo);
            ftinfo.ugbksize = fftemp->fsize;
            flashaddr = ftinfo.ugbkaddr;
            break;
        case 1:
            ftinfo.f12addr = ftinfo.ugbkaddr + ftinfo.ugbksize;
            ftinfo.gbk12size = fftemp->fsize;
            flashaddr = ftinfo.f12addr;
            break;
        case 2:
            ftinfo.f16addr = ftinfo.f12addr + ftinfo.gbk12size;
            ftinfo.gbk16size = fftemp->fsize;
            flashaddr = ftinfo.f16addr;
            break;
        case 3:
            ftinfo.f24addr = ftinfo.f16addr + ftinfo.gbk16size;
            ftinfo.gkb24size = fftemp->fsize;
            flashaddr = ftinfo.f24addr;
            break;
        }

        while (res == FR_OK)
        {
            res = f_read(fftemp, tempbuf, 4096, (UINT *)&bread);
            if (res != FR_OK)
                break;
            W25Q64_Write(tempbuf, offx + flashaddr, 4096);
            offx += bread;
            fupd_prog(x, y, size, fftemp->fsize, offx);
            if (bread != 4096)
                break;
        }
        f_close(fftemp);
    }
    myfree(SRAMIN, fftemp);
    myfree(SRAMIN, tempbuf);
    return res;
}
u8 update_font(u16 x, u16 y, u8 size, u8 *src)
{
    u8 *pname;
    u32 *buf;
    u8 res = 0;
    u16 i, j;
    FIL *fftemp;
    u8 rval = 0;
    res = 0XFF;
    ftinfo.fontok = 0XFF;
    pname = mymalloc(SRAMIN, 100);
    buf = mymalloc(SRAMIN, 4096);
    fftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    if (buf == NULL || pname == NULL || fftemp == NULL)
    {
        myfree(SRAMIN, fftemp);
        myfree(SRAMIN, pname);
        myfree(SRAMIN, buf);
        return 5;
    }
    strcpy((char *)pname, (char *)src);
    strcat((char *)pname, (char *)UNIGBK_PATH);
    res = f_open(fftemp, (const TCHAR *)pname, FA_READ);
    if (res)
        strcpy((char *)pname, (char *)src);
    strcat((char *)pname, (char *)GBK12_PATH);
    res = f_open(fftemp, (const TCHAR *)pname, FA_READ);
    if (res)
        strcpy((char *)pname, (char *)src);
    strcat((char *)pname, (char *)GBK16_PATH);
    res = f_open(fftemp, (const TCHAR *)pname, FA_READ);
    if (res)
        strcpy((char *)pname, (char *)src);
    strcat((char *)pname, (char *)GBK24_PATH);
    res = f_open(fftemp, (const TCHAR *)pname, FA_READ);
    if (res)
        rval |= 1 << 7;
    myfree(SRAMIN, fftemp);
    if (rval == 0)
    {
        LCD_ShowString(x, y, size, "Erasing sectors... ", 0);
        for (i = 0; i < FONTSECSIZE; i++)
        {
            fupd_prog(x + 20 * size / 2, y, size, FONTSECSIZE, i);
            W25Q64_Read((u8 *)buf, ((FONTINFOADDR / 4096) + i) * 4096, 4096);
            for (j = 0; j < 1024; j++)
            {
                if (buf[j] != 0XFFFFFFFF)
                    break;
            }
            if (j != 1024)
                W25Q64_Erase_Sector((FONTINFOADDR / 4096) + i);
        }
        myfree(SRAMIN, buf);
        strcpy((char *)pname, (char *)src);
        strcat((char *)pname, (char *)UNIGBK_PATH);
        res = updata_fontx(x + 20 * size / 2, y, size, pname, 0);
        if (res)
        {
            myfree(SRAMIN, pname);
            return 1;
        }
        strcpy((char *)pname, (char *)src);
        strcat((char *)pname, (char *)GBK12_PATH);
        res = updata_fontx(x + 20 * size / 2, y, size, pname, 1);
        if (res)
        {
            myfree(SRAMIN, pname);
            return 2;
        }
        strcpy((char *)pname, (char *)src);
        strcat((char *)pname, (char *)GBK16_PATH);
        res = updata_fontx(x + 20 * size / 2, y, size, pname, 2);
        if (res)
        {
            myfree(SRAMIN, pname);
            return 3;
        }
        strcpy((char *)pname, (char *)src);
        strcat((char *)pname, (char *)GBK24_PATH);
        res = updata_fontx(x + 20 * size / 2, y, size, pname, 3);
        if (res)
        {
            myfree(SRAMIN, pname);
            return 4;
        }
        ftinfo.fontok = 0XAA;
        W25Q64_Write((u8 *)&ftinfo, FONTINFOADDR, sizeof(ftinfo));
    }
    myfree(SRAMIN, pname);
    myfree(SRAMIN, buf);
    return rval;
}
u8 font_init(void)
{
    u8 t = 0;
    W25Q64_Init();
    while (t < 10)
    {
        t++;
        W25Q64_Read((u8 *)&ftinfo, FONTINFOADDR, sizeof(ftinfo));
        if (ftinfo.fontok == 0XAA)
            break;
        delay_ms(20);
    }
    if (ftinfo.fontok != 0XAA)
        return 1;
    return 0;
}
