#include "rtc.h"
#include "delay.h"
#include "gui.h"
#include "lcd.h"
#include "led.h"

u8 rtc_show_flag = 0;
NVIC_InitTypeDef NVIC_InitStructure;

/*****************************************************************************
 * @name       :ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
 * @date       :2018-08-09
 * @function   :set the RTC time
 * @parameters :hour:the value of hour(0~12 or 0~23)
                                min:the value of minute(0~59)
                                sec:the value of second(0~59)
                                ampa:RTC_AM_PM_Definitions(RTC_H12_AM/RTC_H12_PM)
 * @retvalue   :1-successful
                0-failed
******************************************************************************/
ErrorStatus RTC_Set_Time(u8 hour, u8 min, u8 sec, u8 ampm)
{
    RTC_TimeTypeDef RTC_TimeTypeInitStructure;
    RTC_TimeTypeInitStructure.RTC_Hours = hour;
    RTC_TimeTypeInitStructure.RTC_Minutes = min;
    RTC_TimeTypeInitStructure.RTC_Seconds = sec;
    RTC_TimeTypeInitStructure.RTC_H12 = ampm;
    return RTC_SetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure);
}

/*****************************************************************************
 * @name       :ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
 * @date       :2018-08-09
 * @function   :set the RTC date
 * @parameters :year:the value of year(0~99)
                                month:the value of month(1~12)
                                day:the value of day(1~31)
                                week:the value of week(1~7)
 * @retvalue   :1-successful
                0-failed
******************************************************************************/
ErrorStatus RTC_Set_Date(u8 year, u8 month, u8 day, u8 week)
{
    RTC_DateTypeDef RTC_DateTypeInitStructure;
    RTC_DateTypeInitStructure.RTC_Date = day;
    RTC_DateTypeInitStructure.RTC_Month = month;
    RTC_DateTypeInitStructure.RTC_WeekDay = week;
    RTC_DateTypeInitStructure.RTC_Year = year;
    return RTC_SetDate(RTC_Format_BIN, &RTC_DateTypeInitStructure);
}

/*****************************************************************************
 * @name       :u8 RTC_Init(void)
 * @date       :2018-08-09
 * @function   :Initialize RTC
 * @parameters :None
 * @retvalue   :0-successful
                                1-LSE open failed
                                2-Failed to enter initialization mode
******************************************************************************/
u8 RTC_Set_Init(void)
{
    RTC_InitTypeDef RTC_InitStructure;
    u16 retry = 0X1FFF;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); //ʹ��PWRʱ��
    PWR_BackupAccessCmd(ENABLE);                        //ʹ�ܺ󱸼Ĵ�������
    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)  //�Ƿ��һ������?
    {
        RCC_LSEConfig(RCC_LSE_ON); // LSE ����
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) //���ָ����RCC��־λ�������,�ȴ����پ������
        {
            retry++;
            delay_ms(10);
        }
        if (retry == 0)
        {
            return 1; // LSE ����ʧ��.
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);    //����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��
        RCC_RTCCLKCmd(ENABLE);                     //ʹ��RTCʱ��
        RTC_InitStructure.RTC_AsynchPrediv = 0x7F; // RTC�첽��Ƶϵ��(1~0X7F)
        RTC_InitStructure.RTC_SynchPrediv = 0xFF;  // RTCͬ����Ƶϵ��(0~7FFF)
        RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24; // RTC����Ϊ,24Сʱ��ʽ
        if (!RTC_Init(&RTC_InitStructure))
        {
            return 2; //�����ʼ��ģʽʧ��
        }
        RTC_Set_Time(9, 30, 00, RTC_H12_AM);          //����ʱ��
        RTC_Set_Date(20, 7, 3, 5);                    //��������
        RTC_WriteBackupRegister(RTC_BKP_DR0, 0x5050); //����Ѿ���ʼ������
    }
    return 0;
}

/*****************************************************************************
 * @name       :void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec)
 * @date       :2018-08-09
 * @function   :Set alarm time (by week alarm, 24 hours)
 * @parameters :week:the value of Alarm week(1~7)
                                hour:the value of Alarm hour(0~23)
                                min:the value of Alarm minute(0~59)
                                sec:the value of Alarm second(0~59)
 * @retvalue   :None
******************************************************************************/
void RTC_Set_AlarmA(u8 week, u8 hour, u8 min, u8 sec)
{
    RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
    RTC_TimeTypeDef RTC_TimeTypeInitStructure;
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);          //�ر�����A
    RTC_TimeTypeInitStructure.RTC_Hours = hour;  //Сʱ
    RTC_TimeTypeInitStructure.RTC_Minutes = min; //����
    RTC_TimeTypeInitStructure.RTC_Seconds = sec; //��
    RTC_TimeTypeInitStructure.RTC_H12 = RTC_H12_AM;

    RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay = week;                               //����
    RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay; //��������
    RTC_AlarmTypeInitStructure.RTC_AlarmMask = RTC_AlarmMask_None; //��ȷƥ�����ڣ�ʱ����
    RTC_AlarmTypeInitStructure.RTC_AlarmTime = RTC_TimeTypeInitStructure;
    RTC_SetAlarm(RTC_Format_BIN, RTC_Alarm_A, &RTC_AlarmTypeInitStructure);
    RTC_AlarmCmd(RTC_Alarm_A, ENABLE); //��������A
}

/*****************************************************************************
 * @name       :void RTC_Set_AlarmA_IRQ(void)
 * @date       :2018-08-09
 * @function   :Set alarmA IRQ
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void RTC_Set_AlarmA_IRQ(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    RTC_ClearITPendingBit(RTC_IT_ALRA);  //���RTC����A�ı�־
    EXTI_ClearITPendingBit(EXTI_Line17); //���LINE17�ϵ��жϱ�־λ

    RTC_ITConfig(RTC_IT_ALRA, ENABLE);                     //��������A�ж�
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;            // LINE17
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //�ж��¼�
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //ʹ��LINE17
    EXTI_Init(&EXTI_InitStructure);                        //����

    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        //�����ȼ�2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);                              //����
}

/*****************************************************************************
 * @name       :void RTC_Set_WakeUp_IRQ(u32 wksel,u16 cnt)
 * @date       :2018-08-09
 * @function   :Periodic wake-up timer settings
 * @parameters :wksel:RTC wakeup timer definitions
                                #define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
                                #define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
                                #define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
                                #define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
                                #define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
                                #define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
                                cnt:Automatic reload load value. Reduce to 0, generate interrupt
 * @retvalue   :None
******************************************************************************/
void RTC_Set_WakeUp_IRQ(u32 wksel, u16 cnt)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    RTC_WakeUpCmd(DISABLE);                                //�ر�WAKE UP
    RTC_WakeUpClockConfig(wksel);                          //����ʱ��ѡ��
    RTC_SetWakeUpCounter(cnt);                             //����WAKE UP�Զ���װ�ؼĴ���
    RTC_ClearITPendingBit(RTC_IT_WUT);                     //���RTC WAKE UP�ı�־
    EXTI_ClearITPendingBit(EXTI_Line22);                   //���LINE22�ϵ��жϱ�־λ
    RTC_ITConfig(RTC_IT_WUT, ENABLE);                      //����WAKE UP ��ʱ���ж�
    RTC_WakeUpCmd(ENABLE);                                 //����WAKE UP ��ʱ����
    EXTI_InitStructure.EXTI_Line = EXTI_Line22;            // LINE22
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;    //�ж��¼�
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش���
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;              //ʹ��LINE22
    EXTI_Init(&EXTI_InitStructure);                        //����

    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;        //�����ȼ�2
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;              //ʹ���ⲿ�ж�ͨ��
    NVIC_Init(&NVIC_InitStructure);                              //����
}

/*****************************************************************************
 * @name       :void RTC_Alarm_IRQHandler(void)
 * @date       :2018-08-09
 * @function   :RTC alarm interrupt service function
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void RTC_Alarm_IRQHandler(void)
{
    if (RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET) // ALARM A�ж�?
    {
        RTC_ClearFlag(RTC_FLAG_ALRAF); //����жϱ�־
        LED1 = 0;
        Show_Str(lcddev.width / 2 - 88, 220, RED, WHITE, "It's the alarm time!!!", 16, 0);
    }
    EXTI_ClearITPendingBit(EXTI_Line17); //����ж���17���жϱ�־
}

/*****************************************************************************
 * @name       :void RTC_WKUP_IRQHandler(void)
 * @date       :2018-08-09
 * @function   :RTC wake up interrupt service function
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void RTC_WKUP_IRQHandler(void)
{
    if (RTC_GetFlagStatus(RTC_FLAG_WUTF) == SET) // WK_UP�ж�?
    {
        RTC_ClearFlag(RTC_FLAG_WUTF); //����жϱ�־
        LED0 = !LED0;
        rtc_show_flag = 1;
    }
    EXTI_ClearITPendingBit(EXTI_Line22); //����ж���22���жϱ�־
}
