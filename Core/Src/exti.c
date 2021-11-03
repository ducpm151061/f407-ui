
#include "exti.h"
#include "sys.h"

u8 touch_flag = 0;

/*****************************************************************************
 * @name       :void Touch_EXTI_Init(void)
 * @date       :2020-05-13
 * @function   :DInitialization  PC1 exti
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void Touch_EXTI_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    touch_flag = 0;
}

/*****************************************************************************
 * @name       :void EXTI1_IRQHandler(void)
 * @date       :2020-05-13
 * @function   :PC1 Interrupt Processing Function
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void EXTI1_IRQHandler(void)
{
    touch_flag = 1;
    EXTI_ClearITPendingBit(EXTI_Line1);
}
