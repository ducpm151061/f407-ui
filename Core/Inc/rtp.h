#ifndef __RTP_H__
#define __RTP_H__
#include "delay.h"

#define PEN PBin(1)   // T_PEN
#define DOUT PBin(2)  // T_MISO
#define TDIN PCout(4) // T_MOSI
#define TCLK PBout(0) // T_SCK
#define TCS PCout(13) // T_CS

void RTP_Write_Byte(u8 num);
u16 RTP_Read_AD(u8 CMD);
u16 RTP_Read_XOY(u8 xy);
u8 RTP_Read_XY(u16 *x, u16 *y);
u8 RTP_Read_XY2(u16 *x, u16 *y);
void RTP_Drow_Touch_Point(u16 x, u16 y, u16 color);
void RTP_Draw_Big_Point(u16 x, u16 y, u16 color);
u8 RTP_Scan(u8 tp);
void RTP_Save_Adjdata(void);
u8 RTP_Get_Adjdata(void);
void RTP_Adjust(void);
u8 RTP_Init(void);
void RTP_Adj_Info_Show(u16 x0, u16 y0, u16 x1, u16 y1, u16 x2, u16 y2, u16 x3, u16 y3, u16 fac);

#endif
