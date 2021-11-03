#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "sys.h"

// initialization
u8 RNG_Init(void);

// Get random numbers
u32 RNG_Get_RandomNum(void);

// Get random numbers in the range
u32 RNG_Get_RandomRange(int min, int max);

#endif /*__RANDOM_H__*/