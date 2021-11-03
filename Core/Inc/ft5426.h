#ifndef __FT5426_H
#define __FT5426_H
#include "sys.h"

//����ݴ��������ӵ�оƬ����(δ����IIC����)
// IO��������
#define FT_RST PCout(13) // FT5426��λ����
#define FT_INT PBin(1)   // FT5426�ж�����

// I2C��д����
#define FT_CMD_WR 0X70 //д����
#define FT_CMD_RD 0X71 //������

// FT5426 ���ּĴ�������
#define FT_DEVIDE_MODE 0x00    // FT5206ģʽ���ƼĴ���
#define FT_REG_NUM_FINGER 0x02 //����״̬�Ĵ���

#define FT_TP1_REG 0X03 //��һ�����������ݵ�ַ
#define FT_TP2_REG 0X09 //�ڶ������������ݵ�ַ
#define FT_TP3_REG 0X0F //���������������ݵ�ַ
#define FT_TP4_REG 0X15 //���ĸ����������ݵ�ַ
#define FT_TP5_REG 0X1B //��������������ݵ�ַ

#define FT_ID_G_LIB_VERSION 0xA1  //�汾
#define FT_ID_G_MODE 0xA4         // FT5426�ж�ģʽ���ƼĴ���
#define FT_ID_G_THGROUP 0x80      //������Чֵ���üĴ���
#define FT_ID_G_PERIODACTIVE 0x88 //����״̬�������üĴ���

u8 FT5426_WR_Reg(u16 reg, u8 *buf, u8 len);
void FT5426_RD_Reg(u16 reg, u8 *buf, u8 len);
u8 FT5426_Init(void);
u8 FT5426_Scan(void);

#endif
