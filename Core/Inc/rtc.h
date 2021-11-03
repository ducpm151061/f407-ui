#ifndef __RTC_H
#define __RTC_H
#include "sys.h"

u8 RTC_Set_Init(void);                                         // RTC��ʼ��
void RTC_Set_AlarmA_IRQ(void);                                 // RTC���ӳ�ʼ��
ErrorStatus RTC_Set_Time(u8 hour, u8 min, u8 sec, u8 ampm);    // RTCʱ������
ErrorStatus RTC_Set_Date(u8 year, u8 month, u8 date, u8 week); // RTC��������
void RTC_Set_AlarmA(u8 week, u8 hour, u8 min, u8 sec); //��������ʱ��(����������,24Сʱ��)
void RTC_Set_WakeUp_IRQ(u32 wksel, u16 cnt); //�����Ի��Ѷ�ʱ������

#endif
