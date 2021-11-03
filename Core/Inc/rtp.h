#ifndef __RTP_H__
#define __RTP_H__
#include "sys.h"

//����败����оƬ��������
#define PEN PBin(1)   // T_PEN
#define DOUT PBin(2)  // T_MISO
#define TDIN PCout(4) // T_MOSI
#define TCLK PBout(0) // T_SCK
#define TCS PCout(13) // T_CS

void RTP_Write_Byte(u8 num);                        //�����оƬд��һ������
u16 RTP_Read_AD(u8 CMD);                            //��ȡADת��ֵ
u16 RTP_Read_XOY(u8 xy);                            //���˲��������ȡ(X/Y)
u8 RTP_Read_XY(u16 *x, u16 *y);                     //˫�����ȡ(X+Y)
u8 RTP_Read_XY2(u16 *x, u16 *y);                    //����ǿ�˲���˫���������ȡ
void RTP_Drow_Touch_Point(u16 x, u16 y, u16 color); //��һ������У׼��
void RTP_Draw_Big_Point(u16 x, u16 y, u16 color);   //��һ�����
u8 RTP_Scan(u8 tp);                                 //ɨ��
void RTP_Save_Adjdata(void);                        //����У׼����
u8 RTP_Get_Adjdata(void);                           //��ȡУ׼����
void RTP_Adjust(void);                              //������У׼
u8 RTP_Init(void);                                  //��ʼ��
void RTP_Adj_Info_Show(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 fac); //��ʾУ׼��Ϣ

#endif
