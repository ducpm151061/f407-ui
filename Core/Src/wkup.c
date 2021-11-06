#include "wkup.h"
#include "delay.h"
#include "led.h"
#include "usart.h"

/*****************************************************************************
 * @name       :void KEY_Init(void)
 * @date       :2020-05-08
 * @function   :The system enters standby mode
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void Sys_Enter_Standby(void)
{
    //	while(WKUP_KD);
    RCC_AHB1PeriphResetCmd(0X04FF, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    RTC_ITConfig(RTC_IT_TS | RTC_IT_WUT | RTC_IT_ALRB | RTC_IT_ALRA, DISABLE);
    RTC_ClearITPendingBit(RTC_IT_TS | RTC_IT_WUT | RTC_IT_ALRB | RTC_IT_ALRA);
    PWR_ClearFlag(PWR_FLAG_WU);
    PWR_WakeUpPinCmd(ENABLE);
    PWR_EnterSTANDBYMode();
}

/*****************************************************************************
 * @name       :void KEY_Init(void)
 * @date       :2020-05-08
 * @function   :Detect the signal of wkup pin
 * @parameters :None
 * @retvalue   :1-Press continuously for more than 3S
                                0-Error triggering
******************************************************************************/
u8 Check_WKUP(void)
{
    u8 t = 0;
    u8 tx = 0;
    LED0 = 0;
    while (1)
    {
        tx++;
        if (WKUP_KD)
        {
            tx = 0;
        }
        else
        {
            if (tx > 3)
            {
                LED0 = 1;
                return 0;
            }
        }
        delay_ms(30);
        if (t >= 100)
        {
            LED0 = 0;
            return 1;
        }
    }
}

/*****************************************************************************
 * @name       :void EXTI0_IRQHandler(void)
 * @date       :2020-05-08
 * @function   :EXTI0 Interrupt service function
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void EXTI0_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line0);
    if (Check_WKUP())
    {
        Sys_Enter_Standby();
    }
}

/*****************************************************************************
 * @name       :void WKUP_Init(void)
 * @date       :2020-05-08
 * @function   :wake up initialization
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void WKUP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    if (Check_WKUP() == 0)
    {
        Sys_Enter_Standby();
    }
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
