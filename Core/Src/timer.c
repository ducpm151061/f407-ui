#include "timer.h"
#include "led.h"

/*****************************************************************************
 * @name       :void TIM3_Int_Init(u16 arr,u16 psc)
 * @date       :2020-05-08
 * @function   :General timer 3 interrupt initialization
 * @parameters :arr:the value of auto reload
                psc:clock presplitting frequency
                                Timer overflow time calculation method:
                                tout=((arr+1)*(PSC+1))/Ft(us)
                                Ft = timer operating frequency (MHz),this is 84MHZ

 * @retvalue   :None
******************************************************************************/
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); ///ʹ��TIM3ʱ��

    TIM_TimeBaseInitStructure.TIM_Period = arr;                     //�Զ���װ��ֵ
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;                  //��ʱ����Ƶ
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);          //��ʼ��TIM3
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                   //������ʱ��3�����ж�
    TIM_Cmd(TIM3, ENABLE);                                       //ʹ�ܶ�ʱ��3
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;              //��ʱ��3�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; //��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;        //�����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*****************************************************************************
 * @name       :void TIM3_IRQHandler(void)
 * @date       :2020-05-08
 * @function   :Timer 3 interrupt service function
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) //����ж�
    {
        LED1 = !LED1; // LED1��ת
    }
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update); //����жϱ�־λ
}
