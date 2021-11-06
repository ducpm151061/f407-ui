#include "random.h"

// initialization
u8 RNG_Init(void)
{

    u16 retry = 0;

    // Can make the clock
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);

    // Can make RNG
    RNG_Cmd(ENABLE);

    // Wait for the random number to be ready
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET && retry < 10000)
    {

        retry++;

        delay_us(100);
    }

    if (retry >= 10000)
    {

        return 1;
    }

    RNG_GetRandomNumber();

    return 0;
}

// Get random numbers
u32 RNG_Get_RandomNum(void)
{

    // Wait for the random number to be ready
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET)
        /*Do Nothing*/;

    return RNG_GetRandomNumber();
}

// Get random numbers in the range
u32 RNG_Get_RandomRange(int min, int max)
{

    return RNG_Get_RandomNum() % (max - min + 1) + min;
}