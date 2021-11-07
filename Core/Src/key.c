#include "key.h"

/*****************************************************************************
 * @name       :void KEY_Init(void)
 * @date       :2020-05-08
 * @function   :Initialize KEY GPIO
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*****************************************************************************
 * @name       :u8 KEY_Scan(u8 mode)
 * @date       :2020-05-08
 * @function   :Scan whether key is pressed or not(Key response priority:KEY0>KEY_UP)
 * @parameters :mode:0-Continuous pressing is not supported
                     1-Support continuous press
 * @retvalue   :0-no key is pressed
                1-KEY0 is pressed
                                2-KEY_UP is pressed
******************************************************************************/
u8 KEY_Scan(u8 mode)
{
    static u8 key_up = 1;
    if (mode)
        key_up = 1; 
    if (key_up && (KEY0_VALUE == 0 || KEY_UP_VALUE == 1))
    {
        delay_ms(10);
        key_up = 0;
        if (KEY0_VALUE == 0)
        {
            return KEY0_PRES;
        }
        else if (KEY_UP_VALUE == 1)
        {
            return KEY_UP_PRES;
        }
    }
    else if (KEY0_VALUE == 1 && KEY_UP_VALUE == 0)
    {
        key_up = 1;
    }
    return 0;
}
