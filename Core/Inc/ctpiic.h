#ifndef __CTPIIC_H
#define __CTPIIC_H
#include "sys.h"

#define CTP_SDA_IN()                                                                                                   \
    {                                                                                                                  \
        GPIOC->MODER &= ~(3 << (4 * 2));                                                                               \
        GPIOC->MODER |= 0 << 4 * 2;                                                                                    \
    }
#define CTP_SDA_OUT()                                                                                                  \
    {                                                                                                                  \
        GPIOC->MODER &= ~(3 << (4 * 2));                                                                               \
        GPIOC->MODER |= 1 << 4 * 2;                                                                                    \
    }

#define CTP_IIC_SCL PBout(0) // SCL
#define CTP_IIC_SDA PCout(4) // SDA
#define CTP_READ_SDA PCin(4)

void CTP_IIC_Init(void);
void CTP_IIC_Start(void);
void CTP_IIC_Stop(void);
void CTP_IIC_Send_Byte(u8 txd);
u8 CTP_IIC_Read_Byte(unsigned char ack);
u8 CTP_IIC_Wait_Ack(void);
void CTP_IIC_Ack(void);
void CTP_IIC_NAck(void);

#endif
