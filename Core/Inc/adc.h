#ifndef __ADC_H
#define __ADC_H
#include "sys.h"

void ADC1_Init(void);                      
u16 Get_ADC_Value(u8 ch);                  
u16 Get_ADC_Average_Value(u8 ch, u8 times);
#endif
