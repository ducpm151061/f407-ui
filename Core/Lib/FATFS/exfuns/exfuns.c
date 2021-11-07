#include "exfuns.h"
#include "custom_malloc.h"
#include "fattester.h"
#include "string.h"
#include "usart.h"

const char *FILE_TYPE_TBL[6][13] = {
    {"BIN"},
    {"LRC"},
    {"NES"},
    {"TXT", "C", "H"},
    {"MP1", "MP2", "MP3", "MP4", "M4A", "3GP", "3G2", "OGG", "ACC", "WMA", "WAV", "MID", "FLAC"},
    {"BMP", "JPG", "JPEG", "GIF"},
};
FATFS *fs[2];
FIL *file;
FIL *ftemp;
UINT br, bw;
FILINFO fileinfo;
DIR dir;

u8 *fatbuf;
u8 exfuns_init(void)
{
    fs[0] = (FATFS *)mymalloc(SRAMIN, sizeof(FATFS));
    fs[1] = (FATFS *)mymalloc(SRAMIN, sizeof(FATFS));
    file = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    ftemp = (FIL *)mymalloc(SRAMIN, sizeof(FIL));
    fatbuf = (u8 *)mymalloc(SRAMIN, 512);
    if (fs[0] && fs[1] && file && ftemp && fatbuf)
        return 0;
    else
        return 1;
}

u8 char_upper(u8 c)
{
    if (c < 'A')
        return c;
    if (c >= 'a')
        return c - 0x20;
    else
        return c;
}
u8 f_typetell(u8 *fname)
{
    u8 tbuf[5];
    u8 *attr = '\0';
    u8 i = 0, j;
    while (i < 250)
    {
        i++;
        if (*fname == '\0')
            break;
        fname++;
    }
    if (i == 250)
        return 0XFF;
    for (i = 0; i < 5; i++)
    {
        fname--;
        if (*fname == '.')
        {
            fname++;
            attr = fname;
            break;
        }
    }
    strcpy((char *)tbuf, (const char *)attr); // copy
    for (i = 0; i < 4; i++)
        tbuf[i] = char_upper(tbuf[i]);
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 13; j++)
        {
            if (*FILE_TYPE_TBL[i][j] == 0)
                break;
            if (strcmp((const char *)FILE_TYPE_TBL[i][j], (const char *)tbuf) == 0)
            {
                return (i << 4) | j;
            }
        }
    }
    return 0XFF;
}

u8 exf_getfree(u8 *drv, u32 *total, u32 *free)
{
    FATFS *fs1;
    u8 res;
    u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
    res = (u32)f_getfree((const TCHAR *)drv, (DWORD *)&fre_clust, &fs1);
    if (res == 0)
    {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;
        fre_sect = fre_clust * fs1->csize;
#if _MAX_SS != 512
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#endif
        *total = tot_sect >> 1;
        *free = fre_sect >> 1;
    }
    return res;
}
