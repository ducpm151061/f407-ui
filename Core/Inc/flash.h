#ifndef __FLASH_H__
#define __FLASH_H__
#include "sys.h"

// FLASH��ʼ��ַ
#define INNER_FLASH_BASE 0x08000000 // STM32 FLASH����ʼ��ַ

// FLASH ��������ʼ��ַ
#define ADDR_FLASH_SECTOR_0 ((u32)0x08000000)  //����0��ʼ��ַ, 16 Kbytes
#define ADDR_FLASH_SECTOR_1 ((u32)0x08004000)  //����1��ʼ��ַ, 16 Kbytes
#define ADDR_FLASH_SECTOR_2 ((u32)0x08008000)  //����2��ʼ��ַ, 16 Kbytes
#define ADDR_FLASH_SECTOR_3 ((u32)0x0800C000)  //����3��ʼ��ַ, 16 Kbytes
#define ADDR_FLASH_SECTOR_4 ((u32)0x08010000)  //����4��ʼ��ַ, 64 Kbytes
#define ADDR_FLASH_SECTOR_5 ((u32)0x08020000)  //����5��ʼ��ַ, 128 Kbytes
#define ADDR_FLASH_SECTOR_6 ((u32)0x08040000)  //����6��ʼ��ַ, 128 Kbytes
#define ADDR_FLASH_SECTOR_7 ((u32)0x08060000)  //����7��ʼ��ַ, 128 Kbytes
#define ADDR_FLASH_SECTOR_8 ((u32)0x08080000)  //����8��ʼ��ַ, 128 Kbytes
#define ADDR_FLASH_SECTOR_9 ((u32)0x080A0000)  //����9��ʼ��ַ, 128 Kbytes
#define ADDR_FLASH_SECTOR_10 ((u32)0x080C0000) //����10��ʼ��ַ,128 Kbytes
#define ADDR_FLASH_SECTOR_11 ((u32)0x080E0000) //����11��ʼ��ַ,128 Kbytes

u32 FLASH_ReadWord(u32 faddr);                                 //������
void FLASH_Write(u32 WriteAddr, u32 *pBuffer, u32 NumToWrite); //��ָ����ַ��ʼд��ָ�����ȵ�����
void FLASH_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead); //��ָ����ַ��ʼ����ָ�����ȵ�����

#endif
