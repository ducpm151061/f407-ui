/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* FatFs lower layer API */
#include "custom_malloc.h"
#include "sdio_sdcard.h"
#include "w25q64.h"

#define SD_CARD 0  
#define EX_FLASH 1

#define FLASH_SECTOR_SIZE 512
u16 FLASH_SECTOR_COUNT = 2048 * 10; 
#define FLASH_BLOCK_SIZE 8          

DSTATUS disk_initialize(BYTE pdrv /* Physical drive nmuber (0..) */
)
{
    u8 res = 0;
    switch (pdrv)
    {
    case SD_CARD:       
        res = SD_Init(); // SD_Init()
        break;
    case EX_FLASH:
        W25Q64_Init();
        FLASH_SECTOR_COUNT = 2048 * 10; 
        break;
    default:
        res = 1;
    }
    if (res)
        return STA_NOINIT;
    else
        return 0; 
}

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber (0..) */
)
{
    return 0;
}

DRESULT disk_read(BYTE pdrv,    /* Physical drive nmuber (0..) */
                  BYTE *buff,   /* Data buffer to store read data */
                  DWORD sector, /* Sector address (LBA) */
                  UINT count    /* Number of sectors to read (1..128) */
)
{
    u8 res = 0;
    if (!count)
        return RES_PARERR; 
    switch (pdrv)
    {
    case SD_CARD: 
        res = SD_ReadDisk(buff, sector, count);
        break;
    case EX_FLASH: 
        for (; count > 0; count--)
        {
            W25Q64_Read(buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
            sector++;
            buff += FLASH_SECTOR_SIZE;
        }
        res = 0;
        break;
    default:
        res = 1;
    }
    if (res == 0x00)
        return RES_OK;
    else
        return RES_ERROR;
}
#if _USE_WRITE
DRESULT disk_write(BYTE pdrv,        /* Physical drive nmuber (0..) */
                   const BYTE *buff, /* Data to be written */
                   DWORD sector,     /* Sector address (LBA) */
                   UINT count        /* Number of sectors to write (1..128) */
)
{
    u8 res = 0;
    if (!count)
        return RES_PARERR; 
    switch (pdrv)
    {
    case SD_CARD: 
        res = SD_WriteDisk((u8 *)buff, sector, count);
        break;
    case EX_FLASH: 
        for (; count > 0; count--)
        {
            W25Q64_Write((u8 *)buff, sector * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
            sector++;
            buff += FLASH_SECTOR_SIZE;
        }
        res = 0;
        break;
    default:
        res = 1;
    }
    if (res == 0x00)
        return RES_OK;
    else
        return RES_ERROR;
}
#endif

#if _USE_IOCTL
DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
)
{
    DRESULT res;
    if (pdrv == SD_CARD) 
    {
        switch (cmd)
        {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(DWORD *)buff = 512;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD *)buff = SDCardInfo.CardBlockSize;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = SDCardInfo.CardCapacity / 512;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    }
    else if (pdrv == EX_FLASH) 
    {
        switch (cmd)
        {
        case CTRL_SYNC:
            res = RES_OK;
            break;
        case GET_SECTOR_SIZE:
            *(WORD *)buff = FLASH_SECTOR_SIZE;
            res = RES_OK;
            break;
        case GET_BLOCK_SIZE:
            *(WORD *)buff = FLASH_BLOCK_SIZE;
            res = RES_OK;
            break;
        case GET_SECTOR_COUNT:
            *(DWORD *)buff = FLASH_SECTOR_COUNT;
            res = RES_OK;
            break;
        default:
            res = RES_PARERR;
            break;
        }
    }
    else
        res = RES_ERROR; 
    return res;
}
#endif
// User defined function to give a current time to fatfs module      */
// 31-25: Year(0-127 org.1980), 24-21: Month(1-12), 20-16: Day(1-31) */
// 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */
DWORD get_fattime(void)
{
    return 0;
}
void *ff_memalloc(UINT size)
{
    return (void *)mymalloc(SRAMIN, size);
}
void ff_memfree(void *mf)
{
    myfree(SRAMIN, mf);
}
