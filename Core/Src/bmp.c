#include "bmp.h"
#include "piclib.h"
#include "string.h"

#if BMP_USE_MALLOC == 0
FIL f_bfile;
u8 bmpreadbuf[BMP_DBUF_SIZE];
#endif

u8 stdbmp_decode(const u8 *filename)
{
    FIL *f_bmp;
    u16 br;

    u16 count;
    u8 rgb, color_byte;
    u16 x, y, color;
    u16 countpix = 0;

    u16 realx = 0;
    u16 realy = 0;
    u8 yok = 1;
    u8 res;

    u8 *databuf;
    u16 readlen = BMP_DBUF_SIZE;

    u8 *bmpbuf;
    u8 biCompression = 0;

    u16 rowlen;
    BITMAPINFO *pbmp;

#if BMP_USE_MALLOC == 1
    databuf = (u8 *)pic_memalloc(readlen);
    if (databuf == NULL)
        return PIC_MEM_ERR;
    f_bmp = (FIL *)pic_memalloc(sizeof(FIL));
    if (f_bmp == NULL)
    {
        pic_memfree(databuf);
        return PIC_MEM_ERR;
    }
#else
    databuf = bmpreadbuf;
    f_bmp = &f_bfile;
#endif
    res = f_open(f_bmp, (const TCHAR *)filename, FA_READ);
    if (res == 0)
    {
        f_read(f_bmp, databuf, readlen, (UINT *)&br);
        pbmp = (BITMAPINFO *)databuf;
        count = pbmp->bmfHeader.bfOffBits;
        color_byte = pbmp->bmiHeader.biBitCount / 8;
        biCompression = pbmp->bmiHeader.biCompression;
        picinfo.ImgHeight = pbmp->bmiHeader.biHeight;
        picinfo.ImgWidth = pbmp->bmiHeader.biWidth;
        ai_draw_init();
        if ((picinfo.ImgWidth * color_byte) % 4)
            rowlen = ((picinfo.ImgWidth * color_byte) / 4 + 1) * 4;
        else
            rowlen = picinfo.ImgWidth * color_byte;
        color = 0;
        x = 0;
        y = picinfo.ImgHeight;
        rgb = 0;
        realy = (y * picinfo.Div_Fac) >> 13;
        bmpbuf = databuf;
        while (1)
        {
            while (count < readlen)
            {
                if (color_byte == 3)
                {
                    switch (rgb)
                    {
                    case 0:
                        color = bmpbuf[count] >> 3; // B
                        break;
                    case 1:
                        color += ((u16)bmpbuf[count] << 3) & 0X07E0; // G
                        break;
                    case 2:
                        color += ((u16)bmpbuf[count] << 8) & 0XF800; // R
                        break;
                    }
                }
                else if (color_byte == 2)
                {
                    switch (rgb)
                    {
                    case 0:
                        if (biCompression == BI_RGB) // RGB:5,5,5
                        {
                            color = ((u16)bmpbuf[count] & 0X1F);         // R
                            color += (((u16)bmpbuf[count]) & 0XE0) << 1; // G
                        }
                        else // RGB:5,6,5
                        {
                            color = bmpbuf[count]; // G,B
                        }
                        break;
                    case 1:
                        if (biCompression == BI_RGB) // RGB:5,5,5
                        {
                            color += (u16)bmpbuf[count] << 9; // R,G
                        }
                        else // RGB:5,6,5
                        {
                            color += (u16)bmpbuf[count] << 8; // R,G
                        }
                        break;
                    }
                }
                else if (color_byte == 4)
                {
                    switch (rgb)
                    {
                    case 0:
                        color = bmpbuf[count] >> 3; // B
                        break;
                    case 1:
                        color += ((u16)bmpbuf[count] << 3) & 0X07E0; // G
                        break;
                    case 2:
                        color += ((u16)bmpbuf[count] << 8) & 0XF800; // R
                        break;
                    case 3:
                        // alphabend=bmpbuf[count];
                        break;
                    }
                }
                else if (color_byte == 1)
                {
                }
                rgb++;
                count++;
                if (rgb == color_byte)
                {
                    if (x < picinfo.ImgWidth)
                    {
                        realx = (x * picinfo.Div_Fac) >> 13;
                        if (is_element_ok(realx, realy, 1) && yok)
                        {
                            pic_phy.draw_point(realx + picinfo.S_XOFF, realy + picinfo.S_YOFF - 1, color);
                            // POINT_COLOR=color;
                            // LCD_DrawPoint(realx+picinfo.S_XOFF,realy+picinfo.S_YOFF);
                            // SRAMLCD.Draw_Point(realx+picinfo.S_XOFF,realy+picinfo.S_YOFF,color);
                        }
                    }
                    x++;
                    color = 0x00;
                    rgb = 0;
                }
                countpix++;
                if (countpix >= rowlen)
                {
                    y--;
                    if (y == 0)
                        break;
                    realy = (y * picinfo.Div_Fac) >> 13;
                    if (is_element_ok(realx, realy, 0))
                        yok = 1;
                    else
                        yok = 0;
                    x = 0;
                    countpix = 0;
                    color = 0x00;
                    rgb = 0;
                }
            }
            res = f_read(f_bmp, databuf, readlen, (UINT *)&br);
            if (br != readlen)
                readlen = br;
            if (res || br == 0)
                break;
            bmpbuf = databuf;
            count = 0;
        }
        f_close(f_bmp);
    }
#if BMP_USE_MALLOC == 1
    pic_memfree(databuf);
    pic_memfree(f_bmp);
#endif
    return res;
}
u8 minibmp_decode(u8 *filename, u16 x, u16 y, u16 width, u16 height, u16 acolor, u8 mode)
{
    FIL *f_bmp;
    u16 br;
    u8 color_byte;
    u16 tx, ty, color;
    u8 res;
    u16 i, j;
    u8 *databuf;
    u16 readlen = BMP_DBUF_SIZE;

    u8 *bmpbuf;
    u8 biCompression = 0;

    u16 rowcnt;
    u16 rowlen;
    u16 rowpix = 0;
    u8 rowadd;

    u16 tmp_color;

    u8 alphabend = 0xff;
    u8 alphamode = mode >> 6;
    BITMAPINFO *pbmp;
    picinfo.S_Height = height;
    picinfo.S_Width = width;

#if BMP_USE_MALLOC == 1
    databuf = (u8 *)pic_memalloc(readlen);
    if (databuf == NULL)
        return PIC_MEM_ERR;
    f_bmp = (FIL *)pic_memalloc(sizeof(FIL));
    if (f_bmp == NULL)
    {
        pic_memfree(databuf);
        return PIC_MEM_ERR;
    }
#else
    databuf = bmpreadbuf;
    f_bmp = &f_bfile;
#endif
    res = f_open(f_bmp, (const TCHAR *)filename, FA_READ);
    if (res == 0)
    {
        f_read(f_bmp, databuf, sizeof(BITMAPINFO), (UINT *)&br);
        pbmp = (BITMAPINFO *)databuf;
        color_byte = pbmp->bmiHeader.biBitCount / 8;
        biCompression = pbmp->bmiHeader.biCompression;
        picinfo.ImgHeight = pbmp->bmiHeader.biHeight;
        picinfo.ImgWidth = pbmp->bmiHeader.biWidth;
        if ((picinfo.ImgWidth * color_byte) % 4)
            rowlen = ((picinfo.ImgWidth * color_byte) / 4 + 1) * 4;
        else
            rowlen = picinfo.ImgWidth * color_byte;
        rowadd = rowlen - picinfo.ImgWidth * color_byte;
        color = 0;
        tx = 0;
        ty = picinfo.ImgHeight - 1;
        if (picinfo.ImgWidth <= picinfo.S_Width && picinfo.ImgHeight <= picinfo.S_Height)
        {
            rowcnt = readlen / rowlen;
            readlen = rowcnt * rowlen;
            rowpix = picinfo.ImgWidth;
            f_lseek(f_bmp, pbmp->bmfHeader.bfOffBits);
            while (1)
            {
                res = f_read(f_bmp, databuf, readlen, (UINT *)&br);
                bmpbuf = databuf;
                if (br != readlen)
                    rowcnt = br / rowlen;
                if (color_byte == 3)
                {
                    for (j = 0; j < rowcnt; j++)
                    {
                        for (i = 0; i < rowpix; i++)
                        {
                            color = (*bmpbuf++) >> 3;                  // B
                            color += ((u16)(*bmpbuf++) << 3) & 0X07E0; // G
                            color += (((u16)*bmpbuf++) << 8) & 0XF800; // R
                            pic_phy.draw_point(x + tx, y + ty, color);
                            tx++;
                        }
                        bmpbuf += rowadd;
                        tx = 0;
                        ty--;
                    }
                }
                else if (color_byte == 2) // 16λBMPͼƬ
                {
                    for (j = 0; j < rowcnt; j++)
                    {
                        if (biCompression == BI_RGB) // RGB:5,5,5
                        {
                            for (i = 0; i < rowpix; i++)
                            {
                                color = ((u16)*bmpbuf & 0X1F);           // R
                                color += (((u16)*bmpbuf++) & 0XE0) << 1; // G
                                color += ((u16)*bmpbuf++) << 9;          // R,G
                                pic_phy.draw_point(x + tx, y + ty, color);
                                tx++;
                            }
                        }
                        else // RGB 565
                        {
                            for (i = 0; i < rowpix; i++)
                            {
                                color = *bmpbuf++;              // G,B
                                color += ((u16)*bmpbuf++) << 8; // R,G
                                pic_phy.draw_point(x + tx, y + ty, color);
                                tx++;
                            }
                        }
                        bmpbuf += rowadd;
                        tx = 0;
                        ty--;
                    }
                }
                else if (color_byte == 4)
                {
                    for (j = 0; j < rowcnt; j++)
                    {
                        for (i = 0; i < rowpix; i++)
                        {
                            color = (*bmpbuf++) >> 3;                  // B
                            color += ((u16)(*bmpbuf++) << 3) & 0X07E0; // G
                            color += (((u16)*bmpbuf++) << 8) & 0XF800; // R
                            alphabend = *bmpbuf++;
                            if (alphamode != 1)
                            {
                                tmp_color = pic_phy.read_point(x + tx, y + ty);
                                if (alphamode == 2)
                                {
                                    tmp_color = piclib_alpha_blend(tmp_color, acolor, mode & 0X1F);
                                }
                                color = piclib_alpha_blend(tmp_color, color, alphabend / 8);
                            }
                            else
                                tmp_color = piclib_alpha_blend(acolor, color, alphabend / 8);
                            pic_phy.draw_point(x + tx, y + ty, color);
                            tx++;
                        }
                        bmpbuf += rowadd;
                        tx = 0;
                        ty--;
                    }
                }
                if (br != readlen || res)
                    break;
            }
        }
        f_close(f_bmp);
    }
    else
        res = PIC_SIZE_ERR;
#if BMP_USE_MALLOC == 1
    pic_memfree(databuf);
    pic_memfree(f_bmp);
#endif
    return res;
}
u8 bmp_encode(u8 *filename, u16 x, u16 y, u16 width, u16 height, u8 mode)
{
    FIL *f_bmp;
    u16 bmpheadsize;
    BITMAPINFO hbmp;
    u8 res = 0;
    u16 tx, ty;
    u16 *databuf;
    u16 pixcnt;
    u16 bi4width;
    if (width == 0 || height == 0)
        return PIC_WINDOW_ERR;
    if ((x + width - 1) > lcddev.width)
        return PIC_WINDOW_ERR;
    if ((y + height - 1) > lcddev.height)
        return PIC_WINDOW_ERR;

#if BMP_USE_MALLOC == 1
    databuf = (u16 *)pic_memalloc(1024);
    if (databuf == NULL)
        return PIC_MEM_ERR;
    f_bmp = (FIL *)pic_memalloc(sizeof(FIL));
    if (f_bmp == NULL)
    {
        pic_memfree(databuf);
        return PIC_MEM_ERR;
    }
#else
    databuf = (u16 *)bmpreadbuf;
    f_bmp = &f_bfile;
#endif
    bmpheadsize = sizeof(hbmp);
    mymemset((u8 *)&hbmp, 0, sizeof(hbmp));
    hbmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    hbmp.bmiHeader.biWidth = width;
    hbmp.bmiHeader.biHeight = height;
    hbmp.bmiHeader.biPlanes = 1;
    hbmp.bmiHeader.biBitCount = 16;
    hbmp.bmiHeader.biCompression = BI_BITFIELDS;
    hbmp.bmiHeader.biSizeImage = hbmp.bmiHeader.biHeight * hbmp.bmiHeader.biWidth * hbmp.bmiHeader.biBitCount / 8;

    hbmp.bmfHeader.bfType = ((u16)'M' << 8) + 'B';
    hbmp.bmfHeader.bfSize = bmpheadsize + hbmp.bmiHeader.biSizeImage;
    hbmp.bmfHeader.bfOffBits = bmpheadsize;

    hbmp.RGB_MASK[0] = 0X00F800;
    hbmp.RGB_MASK[1] = 0X0007E0;
    hbmp.RGB_MASK[2] = 0X00001F;

    if (mode == 1)
        res = f_open(f_bmp, (const TCHAR *)filename, FA_READ | FA_WRITE);
    if (mode == 0 || res == 0x04)
        res = f_open(f_bmp, (const TCHAR *)filename, FA_WRITE | FA_CREATE_NEW);
    if ((hbmp.bmiHeader.biWidth * 2) % 4)
    {
        bi4width = ((hbmp.bmiHeader.biWidth * 2) / 4 + 1) * 4;
    }
    else
        bi4width = hbmp.bmiHeader.biWidth * 2;
    if (res == FR_OK)
    {
        res = f_write(f_bmp, (u8 *)&hbmp, bmpheadsize, &bw);
        for (ty = y + height - 1; hbmp.bmiHeader.biHeight; ty--)
        {
            pixcnt = 0;
            for (tx = x; pixcnt != (bi4width / 2);)
            {
                if (pixcnt < hbmp.bmiHeader.biWidth)
                    databuf[pixcnt] = LCD_ReadPoint(tx, ty);
                else
                    databuf[pixcnt] = 0Xffff;
                pixcnt++;
                tx++;
            }
            hbmp.bmiHeader.biHeight--;
            res = f_write(f_bmp, (u8 *)databuf, bi4width, &bw);
        }
        f_close(f_bmp);
    }
#if BMP_USE_MALLOC == 1
    pic_memfree(databuf);
    pic_memfree(f_bmp);
#endif
    return res;
}
