#ifndef __WS_SPI_H
#define __WS_SPI_H
#include "stdint.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "usart.h"
void SPI2_Configuration(void);
void SPI2_Send_byte(u16 data);
u16 SPI2_Receive_byte(void);

#endif /*__WS_SPI_H*/
