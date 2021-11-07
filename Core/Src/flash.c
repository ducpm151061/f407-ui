#include "flash.h"
#include "delay.h"
#include "usart.h"

/*****************************************************************************
 * @name       :u32 FLASH_ReadWord(u32 faddr)
 * @date       :2020-05-08
 * @function   :Read half word (16 bit data) of the specified address
 * @parameters :faddr: the address of read
 * @retvalue   :Data read
 ******************************************************************************/
u32 FLASH_ReadWord(u32 faddr)
{
    return *(vu32 *)faddr;
}

/*****************************************************************************
 * @name       :uint16_t FLASH_GetFlashSector(u32 addr)
 * @date       :2020-05-08
 * @function   :Get the flash sector of an address
 * @parameters :addr: flash address
 * @retvalue   :0~11, which is the sector where addr is located
 ******************************************************************************/
uint16_t FLASH_GetFlashSector(u32 addr)
{
    if (addr < ADDR_FLASH_SECTOR_1)
        return FLASH_Sector_0;
    else if (addr < ADDR_FLASH_SECTOR_2)
        return FLASH_Sector_1;
    else if (addr < ADDR_FLASH_SECTOR_3)
        return FLASH_Sector_2;
    else if (addr < ADDR_FLASH_SECTOR_4)
        return FLASH_Sector_3;
    else if (addr < ADDR_FLASH_SECTOR_5)
        return FLASH_Sector_4;
    else if (addr < ADDR_FLASH_SECTOR_6)
        return FLASH_Sector_5;
    else if (addr < ADDR_FLASH_SECTOR_7)
        return FLASH_Sector_6;
    else if (addr < ADDR_FLASH_SECTOR_8)
        return FLASH_Sector_7;
    else if (addr < ADDR_FLASH_SECTOR_9)
        return FLASH_Sector_8;
    else if (addr < ADDR_FLASH_SECTOR_10)
        return FLASH_Sector_9;
    else if (addr < ADDR_FLASH_SECTOR_11)
        return FLASH_Sector_10;
    return FLASH_Sector_11;
}

/*****************************************************************************
 * @name       :void FLASH_Write(u32 WriteAddr,u32 *pBuffer,u32 NumToWrite)
 * @date       :2020-03-02
 * @function   :Write data of the specified length from the specified address
                                This function is also valid for OTP area! Can be used to write OTP area!
                                OTP regional address range: 0x1fff7800 ~ 0x1fff7a0f
 * @parameters :WriteAddr:the start address to be written of flash
                                pBuffer:the point of written data
                                NumToWrite:the number of word to be written
 * @retvalue   :None
******************************************************************************/
void FLASH_Write(u32 WriteAddr, u32 *pBuffer, u32 NumToWrite)
{
    FLASH_Status status = FLASH_COMPLETE;
    u32 addrx = 0;
    u32 endaddr = 0;
    if (WriteAddr < INNER_FLASH_BASE || WriteAddr % 4)
        return;
    FLASH_Unlock();
    FLASH_DataCacheCmd(DISABLE);

    addrx = WriteAddr;
    endaddr = WriteAddr + NumToWrite * 4;
    if (addrx < 0X1FFF0000)
    {
        while (addrx < endaddr)
        {
            if (FLASH_ReadWord(addrx) != 0XFFFFFFFF)
            {
                status = FLASH_EraseSector(FLASH_GetFlashSector(addrx), VoltageRange_3);
                if (status != FLASH_COMPLETE)
                    break;
            }
            else
                addrx += 4;
        }
    }
    if (status == FLASH_COMPLETE)
    {
        while (WriteAddr < endaddr)
        {
            if (FLASH_ProgramWord(WriteAddr, *pBuffer) != FLASH_COMPLETE)
            {
                break;
            }
            WriteAddr += 4;
            pBuffer++;
        }
    }
    FLASH_DataCacheCmd(ENABLE);
    FLASH_Lock();
}

/*****************************************************************************
 * @name       :void FLASH_Read(u32 ReadAddr,u32 *pBuffer,u32 NumToRead)
 * @date       :2020-03-02
 * @function   :Read data of the specified length from the specified address
 * @parameters :ReadAddr:the start address to be read of flash
                                pBuffer:the point of read data
                                NumToRead:the number of word to be read
 * @retvalue   :None
******************************************************************************/
void FLASH_Read(u32 ReadAddr, u32 *pBuffer, u32 NumToRead)
{
    u32 i;
    for (i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = FLASH_ReadWord(ReadAddr);
        ReadAddr += 4;
    }
}
