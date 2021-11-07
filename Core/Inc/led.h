
#ifndef __LED_H
#define __LED_H
#include "delay.h"
#define LED_IO_DEF 0

#if LED_IO_DEF == 0
#define LED0 PAout(1)
#define LED1 PCout(5)
#define LED0_SET LED0 = 1
#define LED0_CLR LED0 = 0
#define LED1_SET LED1 = 1
#define LED1_CLR LED1 = 0
#elif LED_IO_DEF == 1
#define LED0 GPIO_Pin_1 // PA1
#define LED1 GPIO_Pin_5 // PC5
#define LED0_SET GPIO_SetBits(GPIOA, LED0)
#define LED0_CLR GPIO_ResetBits(GPIOA, LED0)
#define LED1_SET GPIO_SetBits(GPIOC, LED1)
#define LED1_CLR GPIO_ResetBits(GPIOC, LED1)
#else
#define LED0 GPIO_Pin_1
#define LED1 GPIO_Pin_5
#define LED0_SET GPIOA->BSRRL = LED0
#define LED0_CLR GPIOA->BSRRH = LED0
#define LED1_SET GPIOC->BSRRL = LED1
#define LED1_CLR GPIOC->BSRRH = LED1
#endif

void LED_Init(void);
void LED_Toggle(void);
#endif
