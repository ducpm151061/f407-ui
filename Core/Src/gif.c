#include "gif.h"
#include "delay.h"
#include "ff.h"
#include "piclib.h"

const u16 _aMaskTbl[16] = {
    0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
    0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff,
};
const u8 _aInterlaceOffset[] = {8, 8, 4, 2};
const u8 _aInterlaceYPos[] = {0, 4, 2, 1};

u8 gifdecoding = 0;

#if GIF_USE_MALLOC == 0
gif89a tgif89a;
FIL f_gfile;
LZW_INFO tlzw;
#endif

u8 gif_check_head(FIL *file)
{
    u8 gifversion[6];
    u32 readed;
    u8 res;
    res = f_read(file, gifversion, 6, (UINT *)&readed);
    if (res)
        return 1;
    if ((gifversion[0] != 'G') || (gifversion[1] != 'I') || (gifversion[2] != 'F') || (gifversion[3] != '8') ||
        ((gifversion[4] != '7') && (gifversion[4] != '9')) || (gifversion[5] != 'a'))
        return 2;
    else
        return 0;
}
u16 gif_getrgb565(u8 *ctb)
{
    u16 r, g, b;
    r = (ctb[0] >> 3) & 0X1F;
    g = (ctb[1] >> 2) & 0X3F;
    b = (ctb[2] >> 3) & 0X1F;
    return b + (g << 5) + (r << 11);
}
u8 gif_readcolortbl(FIL *file, gif89a *gif, u16 num)
{
    u8 rgb[3];
    u16 t;
    u8 res;
    u32 readed;
    for (t = 0; t < num; t++)
    {
        res = f_read(file, rgb, 3, (UINT *)&readed);
        if (res)
            return 1;
        gif->colortbl[t] = gif_getrgb565(rgb);
    }
    return 0;
}
u8 gif_getinfo(FIL *file, gif89a *gif)
{
    u32 readed;
    u8 res;
    res = f_read(file, (u8 *)&gif->gifLSD, 7, (UINT *)&readed);
    if (res)
        return 1;
    if (gif->gifLSD.flag & 0x80)
    {
        gif->numcolors = 2 << (gif->gifLSD.flag & 0x07);
        if (gif_readcolortbl(file, gif, gif->numcolors))
            return 1;
    }
    return 0;
}
void gif_savegctbl(gif89a *gif)
{
    u16 i = 0;
    for (i = 0; i < 256; i++)
        gif->bkpcolortbl[i] = gif->colortbl[i];
}
void gif_recovergctbl(gif89a *gif)
{
    u16 i = 0;
    for (i = 0; i < 256; i++)
        gif->colortbl[i] = gif->bkpcolortbl[i];
}

void gif_initlzw(gif89a *gif, u8 codesize)
{
    mymemset((u8 *)gif->lzw, 0, sizeof(LZW_INFO));
    gif->lzw->SetCodeSize = codesize;
    gif->lzw->CodeSize = codesize + 1;
    gif->lzw->ClearCode = (1 << codesize);
    gif->lzw->EndCode = (1 << codesize) + 1;
    gif->lzw->MaxCode = (1 << codesize) + 2;
    gif->lzw->MaxCodeSize = (1 << codesize) << 1;
    gif->lzw->ReturnClear = 1;
    gif->lzw->LastByte = 2;
    gif->lzw->sp = gif->lzw->aDecompBuffer;
}

u16 gif_getdatablock(FIL *gfile, u8 *buf, u16 maxnum)
{
    u8 cnt;
    u32 readed;
    u32 fpos;
    f_read(gfile, &cnt, 1, (UINT *)&readed);
    if (cnt)
    {
        if (buf)
        {
            if (cnt > maxnum)
            {
                fpos = f_tell(gfile);
                f_lseek(gfile, fpos + cnt);
                return cnt;
            }
            f_read(gfile, buf, cnt, (UINT *)&readed);
        }
        else
        {
            fpos = f_tell(gfile);
            f_lseek(gfile, fpos + cnt);
        }
    }
    return cnt;
}
// ReadExtension
// Purpose:
// Reads an extension block. One extension block can consist of several data blocks.
// If an unknown extension block occures, the routine failes.
u8 gif_readextension(FIL *gfile, gif89a *gif, int *pTransIndex, u8 *pDisposal)
{
    u8 temp;
    u32 readed;
    u8 buf[4];
    f_read(gfile, &temp, 1, (UINT *)&readed);
    switch (temp)
    {
    case GIF_PLAINTEXT:
    case GIF_APPLICATION:
    case GIF_COMMENT:
        while (gif_getdatablock(gfile, 0, 256) > 0)
            ;
        return 0;
    case GIF_GRAPHICCTL:
        if (gif_getdatablock(gfile, buf, 4) != 4)
            return 1;
        gif->delay = (buf[2] << 8) | buf[1];
        *pDisposal = (buf[0] >> 2) & 0x7;
        if ((buf[0] & 0x1) != 0)
            *pTransIndex = buf[3];
        f_read(gfile, &temp, 1, (UINT *)&readed);
        if (temp != 0)
            return 1;
        return 0;
    }
    return 1;
}

int gif_getnextcode(FIL *gfile, gif89a *gif)
{
    int i, j, End;
    long Result;
    if (gif->lzw->ReturnClear)
    {
        // The first code should be a clearcode.
        gif->lzw->ReturnClear = 0;
        return gif->lzw->ClearCode;
    }
    End = gif->lzw->CurBit + gif->lzw->CodeSize;
    if (End >= gif->lzw->LastBit)
    {
        int Count;
        if (gif->lzw->GetDone)
            return -1; // Error
        gif->lzw->aBuffer[0] = gif->lzw->aBuffer[gif->lzw->LastByte - 2];
        gif->lzw->aBuffer[1] = gif->lzw->aBuffer[gif->lzw->LastByte - 1];
        if ((Count = gif_getdatablock(gfile, &gif->lzw->aBuffer[2], 300)) == 0)
            gif->lzw->GetDone = 1;
        if (Count < 0)
            return -1; // Error
        gif->lzw->LastByte = 2 + Count;
        gif->lzw->CurBit = (gif->lzw->CurBit - gif->lzw->LastBit) + 16;
        gif->lzw->LastBit = (2 + Count) * 8;
        End = gif->lzw->CurBit + gif->lzw->CodeSize;
    }
    j = End >> 3;
    i = gif->lzw->CurBit >> 3;
    if (i == j)
        Result = (long)gif->lzw->aBuffer[i];
    else if (i + 1 == j)
        Result = (long)gif->lzw->aBuffer[i] | ((long)gif->lzw->aBuffer[i + 1] << 8);
    else
        Result =
            (long)gif->lzw->aBuffer[i] | ((long)gif->lzw->aBuffer[i + 1] << 8) | ((long)gif->lzw->aBuffer[i + 2] << 16);
    Result = (Result >> (gif->lzw->CurBit & 0x7)) & _aMaskTbl[gif->lzw->CodeSize];
    gif->lzw->CurBit += gif->lzw->CodeSize;
    return (int)Result;
}
int gif_getnextbyte(FIL *gfile, gif89a *gif)
{
    int i, Code, Incode;
    while ((Code = gif_getnextcode(gfile, gif)) >= 0)
    {
        if (Code == gif->lzw->ClearCode)
        {
            // Corrupt GIFs can make this happen
            if (gif->lzw->ClearCode >= (1 << MAX_NUM_LWZ_BITS))
                return -1; // Error
            // Clear the tables
            mymemset((u8 *)gif->lzw->aCode, 0, sizeof(gif->lzw->aCode));
            for (i = 0; i < gif->lzw->ClearCode; ++i)
                gif->lzw->aPrefix[i] = i;
            // Calculate the'special codes' independence of the initial code size
            // and initialize the stack pointer
            gif->lzw->CodeSize = gif->lzw->SetCodeSize + 1;
            gif->lzw->MaxCodeSize = gif->lzw->ClearCode << 1;
            gif->lzw->MaxCode = gif->lzw->ClearCode + 2;
            gif->lzw->sp = gif->lzw->aDecompBuffer;
            // Read the first code from the stack after clear ingand initializing*/
            do
            {
                gif->lzw->FirstCode = gif_getnextcode(gfile, gif);
            } while (gif->lzw->FirstCode == gif->lzw->ClearCode);
            gif->lzw->OldCode = gif->lzw->FirstCode;
            return gif->lzw->FirstCode;
        }
        if (Code == gif->lzw->EndCode)
            return -2; // End code
        Incode = Code;
        if (Code >= gif->lzw->MaxCode)
        {
            *(gif->lzw->sp)++ = gif->lzw->FirstCode;
            Code = gif->lzw->OldCode;
        }
        while (Code >= gif->lzw->ClearCode)
        {
            *(gif->lzw->sp)++ = gif->lzw->aPrefix[Code];
            if (Code == gif->lzw->aCode[Code])
                return Code;
            if ((gif->lzw->sp - gif->lzw->aDecompBuffer) >= sizeof(gif->lzw->aDecompBuffer))
                return Code;
            Code = gif->lzw->aCode[Code];
        }
        *(gif->lzw->sp)++ = gif->lzw->FirstCode = gif->lzw->aPrefix[Code];
        if ((Code = gif->lzw->MaxCode) < (1 << MAX_NUM_LWZ_BITS))
        {
            gif->lzw->aCode[Code] = gif->lzw->OldCode;
            gif->lzw->aPrefix[Code] = gif->lzw->FirstCode;
            ++gif->lzw->MaxCode;
            if ((gif->lzw->MaxCode >= gif->lzw->MaxCodeSize) && (gif->lzw->MaxCodeSize < (1 << MAX_NUM_LWZ_BITS)))
            {
                gif->lzw->MaxCodeSize <<= 1;
                ++gif->lzw->CodeSize;
            }
        }
        gif->lzw->OldCode = Incode;
        if (gif->lzw->sp > gif->lzw->aDecompBuffer)
            return *--(gif->lzw->sp);
    }
    return Code;
}
// DispGIFImage
// Purpose:
//    This routine draws a GIF image from the current pointer which should point to a
//    valid GIF data block. The size of the desired image is given in the image descriptor.
// Return value:
//   0 if succeed
//   1 if not succeed
// Parameters:
//   pDescriptor  - Points to a IMAGE_DESCRIPTOR structure, which contains infos about size, colors and interlacing.
//   x0, y0       - Obvious.
//   Transparency - Color index which should be treated as transparent.
//   Disposal     - Contains the disposal method of the previous image. If Disposal == 2, the transparent pixels
//                  of the image are rendered with the background color.
u8 gif_dispimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0, int Transparency, u8 Disposal)
{
    u32 readed;
    u8 lzwlen;
    int Index, OldIndex, XPos, YPos, YCnt, Pass, Interlace, XEnd;
    int Width, Height, Cnt, ColorIndex;
    u16 bkcolor;
    u16 *pTrans;

    Width = gif->gifISD.width;
    Height = gif->gifISD.height;
    XEnd = Width + x0 - 1;
    bkcolor = gif->colortbl[gif->gifLSD.bkcindex];
    pTrans = (u16 *)gif->colortbl;
    f_read(gfile, &lzwlen, 1, (UINT *)&readed);
    gif_initlzw(gif, lzwlen); // Initialize the LZW stack with the LZW code size
    Interlace = gif->gifISD.flag & 0x40;
    for (YCnt = 0, YPos = y0, Pass = 0; YCnt < Height; YCnt++)
    {
        Cnt = 0;
        OldIndex = -1;
        for (XPos = x0; XPos <= XEnd; XPos++)
        {
            if (gif->lzw->sp > gif->lzw->aDecompBuffer)
                Index = *--(gif->lzw->sp);
            else
                Index = gif_getnextbyte(gfile, gif);
            if (Index == -2)
                return 0; // Endcode
            if ((Index < 0) || (Index >= gif->numcolors))
            {
                // IfIndex out of legal range stop decompressing
                return 1; // Error
            }
            // If current index equals old index increment counter
            if ((Index == OldIndex) && (XPos <= XEnd))
                Cnt++;
            else
            {
                if (Cnt)
                {
                    if (OldIndex != Transparency)
                    {
                        pic_phy.draw_hline(XPos - Cnt - 1, YPos, Cnt + 1, *(pTrans + OldIndex));
                    }
                    else if (Disposal == 2)
                    {
                        pic_phy.draw_hline(XPos - Cnt - 1, YPos, Cnt + 1, bkcolor);
                    }
                    Cnt = 0;
                }
                else
                {
                    if (OldIndex >= 0)
                    {
                        if (OldIndex != Transparency)
                            pic_phy.draw_point(XPos - 1, YPos, *(pTrans + OldIndex));
                        else if (Disposal == 2)
                            pic_phy.draw_point(XPos - 1, YPos, bkcolor);
                    }
                }
            }
            OldIndex = Index;
        }
        if ((OldIndex != Transparency) || (Disposal == 2))
        {
            if (OldIndex != Transparency)
                ColorIndex = *(pTrans + OldIndex);
            else
                ColorIndex = bkcolor;
            if (Cnt)
            {
                pic_phy.draw_hline(XPos - Cnt - 1, YPos, Cnt + 1, ColorIndex);
            }
            else
                pic_phy.draw_point(XEnd, YPos, ColorIndex);
        }
        // Adjust YPos if image is interlaced
        if (Interlace)
        {
            YPos += _aInterlaceOffset[Pass];
            if ((YPos - y0) >= Height)
            {
                ++Pass;
                YPos = _aInterlaceYPos[Pass] + y0;
            }
        }
        else
            YPos++;
    }
    return 0;
}
void gif_clear2bkcolor(u16 x, u16 y, gif89a *gif, ImageScreenDescriptor pimge)
{
    u16 x0, y0, x1, y1;
    u16 color = gif->colortbl[gif->gifLSD.bkcindex];
    if (pimge.width == 0 || pimge.height == 0)
        return;
    if (gif->gifISD.yoff > pimge.yoff)
    {
        x0 = x + pimge.xoff;
        y0 = y + pimge.yoff;
        x1 = x + pimge.xoff + pimge.width - 1;
        ;
        y1 = y + gif->gifISD.yoff - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
            pic_phy.fill(x0, y0, x1, y1, color);
    }
    if (gif->gifISD.xoff > pimge.xoff)
    {
        x0 = x + pimge.xoff;
        y0 = y + pimge.yoff;
        x1 = x + gif->gifISD.xoff - 1;
        ;
        y1 = y + pimge.yoff + pimge.height - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
            pic_phy.fill(x0, y0, x1, y1, color);
    }
    if ((gif->gifISD.yoff + gif->gifISD.height) < (pimge.yoff + pimge.height))
    {
        x0 = x + pimge.xoff;
        y0 = y + gif->gifISD.yoff + gif->gifISD.height - 1;
        x1 = x + pimge.xoff + pimge.width - 1;
        ;
        y1 = y + pimge.yoff + pimge.height - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
            pic_phy.fill(x0, y0, x1, y1, color);
    }
    if ((gif->gifISD.xoff + gif->gifISD.width) < (pimge.xoff + pimge.width))
    {
        x0 = x + gif->gifISD.xoff + gif->gifISD.width - 1;
        y0 = y + pimge.yoff;
        x1 = x + pimge.xoff + pimge.width - 1;
        ;
        y1 = y + pimge.yoff + pimge.height - 1;
        if (x0 < x1 && y0 < y1 && x1 < 320 && y1 < 320)
            pic_phy.fill(x0, y0, x1, y1, color);
    }
}

u8 gif_drawimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0)
{
    u32 readed;
    u8 res, temp;
    u16 numcolors;
    ImageScreenDescriptor previmg;

    u8 Disposal;
    int TransIndex;
    u8 Introducer;
    TransIndex = -1;
    do
    {
        res = f_read(gfile, &Introducer, 1, (UINT *)&readed);
        if (res)
            return 1;
        switch (Introducer)
        {
        case GIF_INTRO_IMAGE:
            previmg.xoff = gif->gifISD.xoff;
            previmg.yoff = gif->gifISD.yoff;
            previmg.width = gif->gifISD.width;
            previmg.height = gif->gifISD.height;

            res = f_read(gfile, (u8 *)&gif->gifISD, 9, (UINT *)&readed);
            if (res)
                return 1;
            if (gif->gifISD.flag & 0x80)
            {
                gif_savegctbl(gif);
                numcolors = 2 << (gif->gifISD.flag & 0X07);
                if (gif_readcolortbl(gfile, gif, numcolors))
                    return 1;
            }
            if (Disposal == 2)
                gif_clear2bkcolor(x0, y0, gif, previmg);
            gif_dispimage(gfile, gif, x0 + gif->gifISD.xoff, y0 + gif->gifISD.yoff, TransIndex, Disposal);
            while (1)
            {
                f_read(gfile, &temp, 1, (UINT *)&readed);
                if (temp == 0)
                    break;
                readed = f_tell(gfile);
                if (f_lseek(gfile, readed + temp))
                    break;
            }
            if (temp != 0)
                return 1; // Error
            return 0;
        case GIF_INTRO_TERMINATOR:
        case GIF_INTRO_EXTENSION:
            // Read image extension*/
            res = gif_readextension(gfile, gif, &TransIndex, &Disposal);
            if (res)
                return 1;
            break;
        default:
            return 1;
        }
    } while (Introducer != GIF_INTRO_TERMINATOR);
    return 0;
}

void gif_quit(void)
{
    gifdecoding = 0;
}

u8 gif_decode(const u8 *filename, u16 x, u16 y, u16 width, u16 height)
{
    u8 res = 0;
    u16 dtime = 0;
    gif89a *mygif89a;
    FIL *gfile;
#if GIF_USE_MALLOC == 1
    gfile = (FIL *)pic_memalloc(sizeof(FIL));
    if (gfile == NULL)
        res = PIC_MEM_ERR;
    mygif89a = (gif89a *)pic_memalloc(sizeof(gif89a));
    if (mygif89a == NULL)
        res = PIC_MEM_ERR;
    mygif89a->lzw = (LZW_INFO *)pic_memalloc(sizeof(LZW_INFO));
    if (mygif89a->lzw == NULL)
        res = PIC_MEM_ERR;
#else
    gfile = &f_gfile;
    mygif89a = &tgif89a;
    mygif89a->lzw = &tlzw;
#endif

    if (res == 0) // OK
    {
        res = f_open(gfile, (TCHAR *)filename, FA_READ);
        if (res == 0)
        {
            if (gif_check_head(gfile))
                res = PIC_FORMAT_ERR;
            if (gif_getinfo(gfile, mygif89a))
                res = PIC_FORMAT_ERR;
            if (mygif89a->gifLSD.width > width || mygif89a->gifLSD.height > height)
                res = PIC_SIZE_ERR;
            else
            {
                x = (width - mygif89a->gifLSD.width) / 2 + x;
                y = (height - mygif89a->gifLSD.height) / 2 + y;
            }
            gifdecoding = 1;
            while (gifdecoding && res == 0)
            {
                res = gif_drawimage(gfile, mygif89a, x, y);
                if (mygif89a->gifISD.flag & 0x80)
                    gif_recovergctbl(mygif89a);
                if (mygif89a->delay)
                    dtime = mygif89a->delay;
                else
                    dtime = 10;
                while (dtime-- && gifdecoding)
                    delay_ms(10);
                if (res == 2)
                {
                    res = 0;
                    break;
                }
            }
        }
        f_close(gfile);
    }
#if GIF_USE_MALLOC == 1
    pic_memfree(gfile);
    pic_memfree(mygif89a->lzw);
    pic_memfree(mygif89a);
#endif
    return res;
}
