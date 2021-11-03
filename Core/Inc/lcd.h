
#ifndef __LCD_H
#define __LCD_H
#include "stdlib.h"
#include "sys.h"

// LCD��Ҫ������
typedef struct
{
    u16 LCD_W;   // LCD�̶�����(�����޸�)
    u16 LCD_H;   // LCD�̶��߶�(�����޸�)
    u16 width;   // LCDʵ�ʿ���(���ݺ������任)
    u16 height;  // LCDʵ�ʸ߶�(���ݺ������任)
    u16 id;      // LCD ID
    u8 dir;      //���������������ƣ�0��������1��������
    u16 wramcmd; //��ʼдgramָ��
    u16 rramcmd; //��ʼ��gramָ��
    u16 setxcmd; //����x����ָ��
    u16 setycmd; //����y����ָ��
    u16 setdircmd; //������ʾ����ָ��
} _lcd_dev;

// LCD����
extern _lcd_dev lcddev; //����LCD��Ҫ����
/////////////////////////////////////�û�������///////////////////////////////////
#define USE_HORIZONTAL 0 //����Һ����˳ʱ����ת���� 	0-0����ת��1-90����ת��2-180����ת��3-270����ת

// TFTLCD������Ҫ���õĺ���
extern u16 POINT_COLOR; //Ĭ�Ϻ�ɫ
extern u16 BACK_COLOR;  //������ɫ.Ĭ��Ϊ��ɫ

////////////////////////////////////////////////////////////////////
//-----------------LCD�˿ڶ���----------------
#define LED 15 //�����������              PB15
#define RST 13 //��λ����                 PD13

// QDtechȫϵ��ģ������������ܿ��Ʊ��������û�Ҳ���Խ�PWM���ڱ�������
#define LCD_LED PBout(LED) // LCD����
#define LCD_RST PDout(RST) //��λ����              PD13

// LCD��ַ�ṹ��
typedef struct
{
    vu16 LCD_REG;
    vu16 LCD_RAM;
} LCD_TypeDef;

//ʹ��NOR/SRAM�� Bank1.sector1,��ַλHADDR[27,26]=00 A16��Ϊ��������������
//ʹ��16λģʽʱ��ע������ʱSTM32�ڲ�������һλ����! 111 1110=0X7E
#define LCD_BASE ((u32)(0x60000000 | 0x0001FFFE))
#define LCD ((LCD_TypeDef *)LCD_BASE)

//������ɫ
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40 //��ɫ
#define BRRED 0XFC07 //�غ�ɫ
#define GRAY 0X8430  //��ɫ
// GUI��ɫ

#define DARKBLUE 0X01CF  //����ɫ
#define LIGHTBLUE 0X7D7C //ǳ��ɫ
#define GRAYBLUE 0X5458  //����ɫ
//������ɫΪPANEL����ɫ

#define LIGHTGREEN 0X841F //ǳ��ɫ
#define LIGHTGRAY 0XEF5B  //ǳ��ɫ(PANNEL)
#define LGRAY 0XC618      //ǳ��ɫ(PANNEL),���屳��ɫ
#define DARKGRAY 0X8410   //���ɫ

#define LGRAYBLUE 0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE 0X2B12    //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

void LCD_Init(void);
u16 LCD_read(void);
void LCD_Clear(u16 Color);
void LCD_SetCursor(u16 Xpos, u16 Ypos);
void LCD_DrawPoint(u16 x, u16 y); //����
u16 LCD_ReadPoint(u16 x, u16 y);  //����
void LCD_SetWindows(u16 xStar, u16 yStar, u16 xEnd, u16 yEnd);
u16 LCD_RD_DATA(void); //��ȡLCD����
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

/*****0x1963��ʼ������********/
// LCD�ֱ�������
#define SSD_HOR_RESOLUTION 800 // LCDˮƽ�ֱ���
#define SSD_VER_RESOLUTION 480 // LCD��ֱ�ֱ���
// LCD������������
#define SSD_HOR_PULSE_WIDTH 1   //ˮƽ����
#define SSD_HOR_BACK_PORCH 46   //ˮƽǰ��
#define SSD_HOR_FRONT_PORCH 210 //ˮƽ����
#define SSD_VER_PULSE_WIDTH 1   //��ֱ����
#define SSD_VER_BACK_PORCH 23   //��ֱǰ��
#define SSD_VER_FRONT_PORCH 22  //��ֱǰ��
//���¼����������Զ�����
#define SSD_HT (SSD_HOR_RESOLUTION + SSD_HOR_BACK_PORCH + SSD_HOR_FRONT_PORCH)
#define SSD_HPS (SSD_HOR_BACK_PORCH)
#define SSD_VT (SSD_VER_RESOLUTION + SSD_VER_BACK_PORCH + SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif