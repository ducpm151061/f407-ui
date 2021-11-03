#ifndef __GIF_H__
#define __GIF_H__
#include "sys.h"

#define GIF_USE_MALLOC 1 //�����Ƿ�ʹ��malloc,��������ѡ��ʹ��malloc

#define LCD_MAX_LOG_COLORS 256
#define MAX_NUM_LWZ_BITS 12

#define GIF_INTRO_TERMINATOR ';' // 0X3B   GIF�ļ�������
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

__PACKED typedef struct
{
    u16 width;  // GIF����
    u16 height; // GIF�߶�
    u8 flag; //��ʶ��  1:3:1:3=ȫ����ɫ����־(1):��ɫ���(3):�����־(1):ȫ����ɫ����С(3)
    u8 bkcindex; //����ɫ��ȫ����ɫ���е�����(��������ȫ����ɫ��ʱ��Ч)
    u8 pixratio; //���ؿ��߱�
} LogicalScreenDescriptor;

__PACKED typedef struct
{
    u16 xoff;   // x����ƫ��
    u16 yoff;   // y����ƫ��
    u16 width;  //����
    u16 height; //�߶�
    u8 flag; //��ʶ��  1:1:1:2:3=�ֲ���ɫ����־(1):��֯��־(1):����(2):�ֲ���ɫ����С(3)
} ImageScreenDescriptor;

__PACKED typedef struct
{
    LogicalScreenDescriptor gifLSD; //�߼���Ļ������
    ImageScreenDescriptor gifISD;   //ͼ��������
    u16 colortbl[256];              //��ǰʹ����ɫ��
    u16 bkpcolortbl[256];           //������ɫ��.�����ھֲ���ɫ��ʱʹ��
    u16 numcolors;                  //��ɫ����С
    u16 delay;                      //�ӳ�ʱ��
    LZW_INFO *lzw;                  // LZW��Ϣ
} gif89a;

extern u8 gifdecoding;                                 // GIF���ڽ�����.
u8 gif_check_head(FIL *file);                          //���GIFͷ
u16 gif_getrgb565(u8 *ctb);                            //��RGB888תΪRGB565
u8 gif_readcolortbl(FIL *file, gif89a *gif, u16 num);  //��ȡ��ɫ��
u8 gif_getinfo(FIL *file, gif89a *gif);                //�õ��߼���Ļ����,ͼ��ߴ��
void gif_savegctbl(gif89a *gif);                       //����ȫ����ɫ��
void gif_recovergctbl(gif89a *gif);                    //�ָ�ȫ����ɫ��
void gif_initlzw(gif89a *gif, u8 codesize);            //��ʼ��LZW��ز���
u16 gif_getdatablock(FIL *gfile, u8 *buf, u16 maxnum); //��ȡһ�����ݿ�
u8 gif_readextension(FIL *gfile, gif89a *gif, int *pTransIndex, u8 *pDisposal); //��ȡ��չ����
int gif_getnextcode(FIL *gfile, gif89a *gif); //��LZW�����еõ���һ��LZW��,ÿ�������12λ
int gif_getnextbyte(FIL *gfile, gif89a *gif); //�õ�LZW����һ����
u8 gif_dispimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0, int Transparency, u8 Disposal); //��ʾͼƬ
void gif_clear2bkcolor(u16 x, u16 y, gif89a *gif, ImageScreenDescriptor pimge);           //�ָ��ɱ���ɫ
u8 gif_drawimage(FIL *gfile, gif89a *gif, u16 x0, u16 y0);                                //��GIFͼ���һ֡

u8 gif_decode(const u8 *filename, u16 x, u16 y, u16 width, u16 height); //��ָ���������һ��GIF�ļ�.
void gif_quit(void);                                                    //�˳���ǰ����.
#endif
