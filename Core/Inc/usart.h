#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"
#define USE_MICROLIB 0 

#define USART_REC_LEN 256 

extern u8 USART_RX_BUF[USART_REC_LEN]; 
extern u16 USART_RX_STA;               

void Debug_Init(u32 bound);
void Debug_Send(const char *str);
void Debug_Printf(char *Data, ...);

#define PRINT(...) Debug_Printf(__VA_ARGS__)
#endif