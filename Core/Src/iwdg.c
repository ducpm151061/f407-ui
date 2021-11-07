#include "iwdg.h"

/*****************************************************************************
 * @name       :void IWDG_Init(u8 prer,u16 rlr)
 * @date       :2020-05-08
 * @function   :Initialize independent watchdog
 * @parameters :prer:Prer: frequency division:0~7(only the lower 3 bits are valid!)
                                rlr:auto reload load value,0~0xfff(The lower 11 bits are valid)
                                Time calculation (approximate):tout=((4*2^prer)*rlr)/32(ms)
 * @retvalue   :None
******************************************************************************/
void IWDG_Init(u8 prer, u16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prer);
    IWDG_SetReload(rlr);
    IWDG_ReloadCounter();
    IWDG_Enable();
}

/*****************************************************************************
 * @name       :void IWDG_Feed(void)
 * @date       :2020-05-08
 * @function   :Feeding independent watchdog
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void IWDG_Feed(void)
{
    IWDG_ReloadCounter(); // reload
}
