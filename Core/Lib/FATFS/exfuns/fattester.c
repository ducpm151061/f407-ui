#include "fattester.h"
#include "custom_malloc.h"
#include "exfuns.h"
#include "ff.h"
#include "sdio_sdcard.h"
#include "string.h"
#include "usart.h"

u8 mf_mount(u8 *path, u8 mt)
{
    return f_mount(fs[0], (const TCHAR *)path, mt);
}
u8 mf_open(u8 *path, u8 mode)
{
    u8 res;
    res = f_open(file, (const TCHAR *)path, mode);
    return res;
}
u8 mf_close(void)
{
    f_close(file);
    return 0;
}
u8 mf_read(u16 len)
{
    u16 i, t;
    u8 res = 0;
    u16 tlen = 0;
    printf("\r\nRead file data is:\r\n");
    for (i = 0; i < len / 512; i++)
    {
        res = f_read(file, fatbuf, 512, &br);
        if (res)
        {
            printf("Read Error:%d\r\n", res);
            break;
        }
        else
        {
            tlen += br;
            for (t = 0; t < br; t++)
                printf("%c", fatbuf[t]);
        }
    }
    if (len % 512)
    {
        res = f_read(file, fatbuf, len % 512, &br);
        if (res)
        {
            printf("\r\nRead Error:%d\r\n", res);
        }
        else
        {
            tlen += br;
            for (t = 0; t < br; t++)
                printf("%c", fatbuf[t]);
        }
    }
    if (tlen)
        printf("\r\nReaded data len:%d\r\n", tlen);
    printf("Read data over\r\n");
    return res;
}
u8 mf_write(u8 *dat, u16 len)
{
    u8 res;

    printf("\r\nBegin Write file...\r\n");
    printf("Write data len:%d\r\n", len);
    res = f_write(file, dat, len, &bw);
    if (res)
    {
        printf("Write Error:%d\r\n", res);
    }
    else
        printf("Writed data len:%d\r\n", bw);
    printf("Write data over.\r\n");
    return res;
}

u8 mf_opendir(u8 *path)
{
    return f_opendir(&dir, (const TCHAR *)path);
}
u8 mf_closedir(void)
{
    return f_closedir(&dir);
}
u8 mf_readdir(void)
{
    u8 res;
    char *fn;
#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(SRAMIN, fileinfo.lfsize);
#endif
    res = f_readdir(&dir, &fileinfo);
    if (res != FR_OK || fileinfo.fname[0] == 0)
    {
        myfree(SRAMIN, fileinfo.lfname);
        return res;
    }
#if _USE_LFN
    fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
    fn = fileinfo.fname;
    ;
#endif
    printf("\r\n DIR info:\r\n");

    printf("dir.id:%d\r\n", dir.id);
    printf("dir.index:%d\r\n", dir.index);
    printf("dir.sclust:%d\r\n", dir.sclust);
    printf("dir.clust:%d\r\n", dir.clust);
    printf("dir.sect:%d\r\n", dir.sect);

    printf("\r\n");
    printf("File Name is:%s\r\n", fn);
    printf("File Size is:%d\r\n", fileinfo.fsize);
    printf("File data is:%d\r\n", fileinfo.fdate);
    printf("File time is:%d\r\n", fileinfo.ftime);
    printf("File Attr is:%d\r\n", fileinfo.fattrib);
    printf("\r\n");
    myfree(SRAMIN, fileinfo.lfname);
    return 0;
}

u8 mf_scan_files(u8 *path)
{
    FRESULT res;
    char *fn; /* This function is assuming non-Unicode cfg. */
#if _USE_LFN
    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(SRAMIN, fileinfo.lfsize);
#endif

    res = f_opendir(&dir, (const TCHAR *)path);
    if (res == FR_OK)
    {
        printf("\r\n");
        while (1)
        {
            res = f_readdir(&dir, &fileinfo);
            if (res != FR_OK || fileinfo.fname[0] == 0)
                break;
                // if (fileinfo.fname[0] == '.') continue;
#if _USE_LFN
            fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else
            fn = fileinfo.fname;
#endif /* It is a file. */
            printf("%s/", path);
            printf("%s\r\n", fn);
        }
    }
    myfree(SRAMIN, fileinfo.lfname);
    return res;
}
u32 mf_showfree(u8 *drv)
{
    FATFS *fs1;
    u8 res;
    u32 fre_clust = 0, fre_sect = 0, tot_sect = 0;
    res = f_getfree((const TCHAR *)drv, (DWORD *)&fre_clust, &fs1);
    if (res == 0)
    {
        tot_sect = (fs1->n_fatent - 2) * fs1->csize;
        fre_sect = fre_clust * fs1->csize;
#if _MAX_SS != 512
        tot_sect *= fs1->ssize / 512;
        fre_sect *= fs1->ssize / 512;
#endif
        if (tot_sect < 20480)
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\nfree space:%d KB\r\n"
                   ":%d KB\r\n",
                   tot_sect >> 1, fre_sect >> 1);
        }
        else
        {
            /* Print free space in unit of KB (assuming 512 bytes/sector) */
            printf("\r\nfree space in unit:%d MB\r\n"
                   ":%d MB\r\n",
                   tot_sect >> 11, fre_sect >> 11);
        }
    }
    return fre_sect;
}
u8 mf_lseek(u32 offset)
{
    return f_lseek(file, offset);
}
u32 mf_tell(void)
{
    return f_tell(file);
}
u32 mf_size(void)
{
    return f_size(file);
}
u8 mf_mkdir(u8 *pname)
{
    return f_mkdir((const TCHAR *)pname);
}
u8 mf_fmkfs(u8 *path, u8 mode, u16 au)
{
    return f_mkfs((const TCHAR *)path, mode, au);
}
u8 mf_unlink(u8 *pname)
{
    return f_unlink((const TCHAR *)pname);
}

u8 mf_rename(u8 *oldname, u8 *newname)
{
    return f_rename((const TCHAR *)oldname, (const TCHAR *)newname);
}
void mf_getlabel(u8 *path)
{
    u8 buf[20];
    u32 sn = 0;
    u8 res;
    res = f_getlabel((const TCHAR *)path, (TCHAR *)buf, (DWORD *)&sn);
    if (res == FR_OK)
    {
        printf("\r\n%s :%s\r\n", path, buf);
        printf("%s :%X\r\n\r\n", path, sn);
    }
    else
        printf("\r\n:%X\r\n", res);
}
void mf_setlabel(u8 *path)
{
    u8 res;
    res = f_setlabel((const TCHAR *)path);
    if (res == FR_OK)
    {
        printf("\r\n:%s\r\n", path);
    }
    else
        printf("\r\n:%X\r\n", res);
}

void mf_gets(u16 size)
{
    TCHAR *rbuf;
    rbuf = f_gets((TCHAR *)fatbuf, size, file);
    if (*rbuf == 0)
        return;
    else
    {
        printf("\r\nThe String Readed Is:%s\r\n", rbuf);
    }
}
u8 mf_putc(u8 c)
{
    return f_putc((TCHAR)c, file);
}
u8 mf_puts(u8 *c)
{
    return f_puts((TCHAR *)c, file);
}
