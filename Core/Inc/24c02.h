#ifndef __24C02_H
#define __24C02_H
#include "iic.h"

#define AT24C01 127
#define AT24C02 255
#define AT24C04 511
#define AT24C08 1023
#define AT24C16 2047
#define AT24C32 4095
#define AT24C64 8191
#define AT24C128 16383
#define AT24C256 32767

#define EE_TYPE AT24C02

u8 AT24C02_ReadOneByte(u16 ReadAddr);
void AT24C02_WriteOneByte(u16 WriteAddr, u8 DataToWrite);
void AT24C02_WriteLenByte(u16 WriteAddr, u32 DataToWrite, u8 Len);
u32 AT24C02_ReadLenByte(u16 ReadAddr, u8 Len);
void AT24C02_Write(u16 WriteAddr, u8 *pBuffer, u16 NumToWrite);
void AT24C02_Read(u16 ReadAddr, u8 *pBuffer, u16 NumToRead);

u8 AT24C02_Check(void);
void AT24C02_Init(void);
#endif
