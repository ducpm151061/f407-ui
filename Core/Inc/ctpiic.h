#ifndef __CTPIIC_H
#define __CTPIIC_H
#include "sys.h"

// IO��������
#define CTP_SDA_IN()                                                                                                   \
    {                                                                                                                  \
        GPIOC->MODER &= ~(3 << (4 * 2));                                                                               \
        GPIOC->MODER |= 0 << 4 * 2;                                                                                    \
    } // PC4����ģʽ
#define CTP_SDA_OUT()                                                                                                  \
    {                                                                                                                  \
        GPIOC->MODER &= ~(3 << (4 * 2));                                                                               \
        GPIOC->MODER |= 1 << 4 * 2;                                                                                    \
    } // PC4���ģʽ

// IO��������
#define CTP_IIC_SCL PBout(0) // SCL
#define CTP_IIC_SDA PCout(4) // SDA
#define CTP_READ_SDA PCin(4) //����SDA

// IIC���в�������
void CTP_IIC_Init(void);                 //��ʼ��IIC��IO��
void CTP_IIC_Start(void);                //����IIC��ʼ�ź�
void CTP_IIC_Stop(void);                 //����IICֹͣ�ź�
void CTP_IIC_Send_Byte(u8 txd);          // IIC����һ���ֽ�
u8 CTP_IIC_Read_Byte(unsigned char ack); // IIC��ȡһ���ֽ�
u8 CTP_IIC_Wait_Ack(void);               // IIC�ȴ�ACK�ź�
void CTP_IIC_Ack(void);                  // IIC����ACK�ź�
void CTP_IIC_NAck(void);                 // IIC������ACK�ź�

#endif
