#include "pwm.h"
#include "led.h"

/*****************************************************************************
 * @name       :void TIM14_PWM_Init(u32 arr,u32 psc)
 * @date       :2020-05-08
 * @function   :Tim5 PWM partial initialization
 * @parameters :arr:the value of auto reload
                psc:clock presplitting frequency
                                Timer overflow time calculation method:
                                tout=((arr+1)*(PSC+1))/Ft(us)
                                Ft = timer operating frequency (MHz),this is 84MHZ
 * @retvalue   :None
******************************************************************************/
void TIM5_PWM_Init(u32 arr, u32 psc)
{
    //�˲������ֶ��޸�IO������
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);        // TIM5ʱ��ʹ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);       //ʹ��PORTAʱ��
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM5);     // GPIOA1����Ϊ��ʱ��5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;                   // GPIOA1
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                //���ù���
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;          //�ٶ�100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;              //���츴�����
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;                //����
    GPIO_Init(GPIOA, &GPIO_InitStructure);                      //��ʼ��PA1
    TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //��ʱ����Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //���ϼ���ģʽ
    TIM_TimeBaseStructure.TIM_Period = arr;                     //�Զ���װ��ֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //��ʼ����ʱ��5

    //��ʼ��TIM5 Channel2 PWMģʽ
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM������ȵ���ģʽ2
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;      //�������:TIM����Ƚϼ��Ե�
    TIM_OC2Init(TIM5, &TIM_OCInitStructure); //����Tָ���Ĳ�����ʼ������TIM5
    TIM_OC2PreloadConfig(TIM5, TIM_OCPreload_Enable); //ʹ��TIM5��CCR1�ϵ�Ԥװ�ؼĴ���
    TIM_ARRPreloadConfig(TIM5, ENABLE);               // ARPEʹ��
    TIM_Cmd(TIM5, ENABLE);                            //ʹ��TIM5
}
