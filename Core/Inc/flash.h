#ifndef __FLASH_H__
#define __FLASH_H__
#include "sys.h"

#define INNER_FLASH_BASE 0x08000000

#define ADDR_FLASH_SECTOR_0 ((u32)0x08000000)  // 16 Kbytes
#define ADDR_FLASH_SECTOR_1 ((u32)0x08004000)  // 16 Kbytes
#define ADDR_FLASH_SECTOR_2 ((u32)0x08008000)  // 16 Kbytes
#define ADDR_FLASH_SECTOR_3 ((u32)0x0800C000)  // 16 Kbytes
#define ADDR_FLASH_SECTOR_4 ((u32)0x08010000)  // 64 Kbytes
#define ADDR_FLASH_SECTOR_5 ((u32)0x08020000)  // 128 Kbytes
#define ADDR_FLASH_SECTOR_6 ((u32)0x08040000)  // 128 Kbytes
#define ADDR_FLASH_SECTOR_7 ((u32)0x08060000)  // 128 Kbytes
#define ADDR_FLASH_SECTOR_8 ((u32)0x08080000)  // 128 Kbytes
#define ADDR_FLASH_SECTOR_9 ((u32)0x080A0000)  // 128 Kbytes
#define ADDR_FLASH_SECTOR_10 ((u32)0x080C0000) // 128 Kbytes
#define ADDR_FLASH_SECTOR_11 ((u32)0x080E0000) // 128 Kbytes

u32 FLASH_ReadWord(u32 faddr);
void FLASH_Write(u32 WriteAddr, u32 *pBuffer, u32 NumToWrite);
void FLASH_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead);

#endif /* __FLASH_H__ */
