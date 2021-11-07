#ifndef __FT5426_H
#define __FT5426_H
#include "delay.h"

#define FT_RST PCout(13)

#define FT_CMD_WR 0X70
#define FT_CMD_RD 0X71

#define FT_DEVIDE_MODE 0x00
#define FT_REG_NUM_FINGER 0x02

#define FT_TP1_REG 0X03
#define FT_TP2_REG 0X09
#define FT_TP3_REG 0X0F
#define FT_TP4_REG 0X15
#define FT_TP5_REG 0X1B

#define FT_ID_G_LIB_VERSION 0xA1
#define FT_ID_G_MODE 0xA4
#define FT_ID_G_THGROUP 0x80
#define FT_ID_G_PERIODACTIVE 0x88

u8 FT5426_WR_Reg(u16 reg, u8 *buf, u8 len);
void FT5426_RD_Reg(u16 reg, u8 *buf, u8 len);
u8 FT5426_Init(void);
u8 FT5426_Scan(void);

#endif
