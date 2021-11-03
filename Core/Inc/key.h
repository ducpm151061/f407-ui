#ifndef __KEY_H
#define __KEY_H
#include "sys.h"

// KEY�˿ڶ�ȡ��ʽ�л�
#define KEY_IO_DEF 0 // 0-λ������1-�⺯��������2-�Ĵ�������

#if KEY_IO_DEF == 0                                           //λ������ʽ��ȡ�����˿�ֵ
#define KEY0_VALUE PEin(4)                                    // PE4
#define KEY_UP_VALUE PAin(0)                                  // PA0
#elif KEY_IO_DEF == 1                                         //�⺯����ʽ��ȡ�����˿�ֵ
#define KEY0_VALUE GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)   // PE4
#define KEY_UP_VALUE GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) // PA0
#else                                                         //�Ĵ�����ʽ��ȡ�����˿�ֵ
#define KEY0_VALUE ((GPIOE->IDR & GPIO_Pin_4) ? 1 : 0)
#define KEY_UP_VALUE ((GPIOA->IDR & GPIO_Pin_0) ? 1 : 0)
#endif

#define KEY0_PRES 1   // KEY0����
#define KEY_UP_PRES 2 // KEY_UP����

void KEY_Init(void); // IO��ʼ��
u8 KEY_Scan(u8);     //����ɨ�躯��

#endif
