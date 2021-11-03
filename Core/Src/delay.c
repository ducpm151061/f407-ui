#include "delay.h"
#include "sys.h"

static u8 fac_us = 0;
static u16 fac_ms = 0;

/*****************************************************************************
 * @name       :void delay_init(u8 SYSCLK)
 * @date       :2020-05-08
 * @function   :Initialization delay function
 * @parameters :SYSCLK:System clock frequency
 * @retvalue   :None
 ******************************************************************************/
void delay_init(u8 SYSCLK)
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    fac_us = SYSCLK / 8;
    fac_ms = (u16)fac_us * 1000;
}

/*****************************************************************************
 * @name       :void delay_us(u32 nus)
 * @date       :2020-05-08
 * @function   :Microsecond delay function
 * @parameters :nus:Microseconds to delay(No more than 798915us)
                                        the maximum value is 2^24/fac_ us@fac_ us=21
 * @retvalue   :None
******************************************************************************/
void delay_us(u32 nus)
{
    u32 temp;
    SysTick->LOAD = nus * fac_us;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
}

/*****************************************************************************
 * @name       :void delay_xms(u16 nms)
 * @date       :2020-05-08
 * @function   :millisecond delay setting
 * @parameters :mus:milliseconds to delay(No more than 798915us)
                                        the Systick > load is a 24 bit register, so the maximum delay is:
                                        nms<=0xffffff*8*1000/SYSCLK
                                        Sysclk in Hz and NMS in MS
                                        At 168MHz, NMS < = 798ms
 * @retvalue   :None
******************************************************************************/
void delay_xms(u16 nms)
{
    u32 temp;
    SysTick->LOAD = (u32)nms * fac_ms;
    SysTick->VAL = 0x00;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL = 0X00;
}

/*****************************************************************************
 * @name       :void delay_ms(u16 nms)
 * @date       :2020-05-08
 * @function   :millisecond delay function
 * @parameters :nus:milliseconds to delay(0~65535)
 * @retvalue   :None
 ******************************************************************************/
void delay_ms(u16 nms)
{
    u8 repeat = nms / 540;
    u16 remain = nms % 540;
    while (repeat)
    {
        delay_xms(540);
        repeat--;
    }
    if (remain)
        delay_xms(remain);
}
