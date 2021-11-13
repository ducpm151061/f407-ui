#ifndef __LCD_H
#define __LCD_H
#include "stdlib.h"
#include "sys.h"
#include "usart.h"

typedef struct
{
    u16 LCD_W;
    u16 LCD_H;
    u16 width;
    u16 height;
    u16 id;
    u8 dir;
    u16 wramcmd;
    u16 rramcmd;
    u16 setxcmd;
    u16 setycmd;
    u16 setdircmd;
} _lcd_dev;

extern _lcd_dev lcddev;
#define USE_HORIZONTAL 0

extern u16 POINT_COLOR;
extern u16 BACK_COLOR;

#define LED 15 // PB15
#define RST 13 // PD13

#define LCD_LED PBout(LED)
#define LCD_RST PDout(RST) // PD13

typedef struct
{
    vu16 LCD_REG;
    vu16 LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE ((u32)(0x60000000 | 0x0001FFFE))
#define LCD ((LCD_TypeDef *)LCD_BASE)

#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0xF81F
#define GRED 0xFFE0
#define GBLUE 0x07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0xBC40
#define BRRED 0xFC07
#define GRAY 0x8430

#define DARKBLUE 0x01CF
#define LIGHTBLUE 0x7D7C
#define GRAYBLUE 0x5458

#define LIGHTGREEN 0x841F
#define LIGHTGRAY 0xEF5B
#define LGRAY 0xC618
#define DARKGRAY 0x8410

#define LGRAYBLUE 0xA651
#define LBBLUE 0x2B12

void LCD_Init(void);
u16 LCD_read(void);
void LCD_Clear(u16 Color);
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x, u16 y);
u16 LCD_ReadPoint(u16 x, u16 y);
void LCD_SetWindows(u16 xStar, u16 yStar, u16 xEnd, u16 yEnd);
u16 LCD_RD_DATA(void);
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue);
void LCD_WR_REG(u16 data);
void LCD_WR_DATA(u16 data);
void LCD_ReadReg(u16 LCD_Reg, u16 *Rval, int n);
void LCD_WriteRAM_Prepare(void);
void LCD_ReadRAM_Prepare(void);
void Lcd_WriteData_16Bit(u16 Data);
u16 Lcd_ReadData_16Bit(void);
void LCD_direction(u8 direction);
u16 LCD_Read_ID(void);
void LCD_PWM_BackLightSet(u8 pwm); // only for 0x1963

#define SSD_HOR_RESOLUTION 800
#define SSD_VER_RESOLUTION 480

#define SSD_HOR_PULSE_WIDTH 1
#define SSD_HOR_BACK_PORCH 46
#define SSD_HOR_FRONT_PORCH 210
#define SSD_VER_PULSE_WIDTH 1
#define SSD_VER_BACK_PORCH 23
#define SSD_VER_FRONT_PORCH 22

#define SSD_HT (SSD_HOR_RESOLUTION + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS (SSD_HOR_BACK_PORCH)
#define SSD_VT (SSD_VER_RESOLUTION + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif /* __LCD_H */
