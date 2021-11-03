#ifndef _AT45DBXX_H
#define _AT45DBXX_H
#include "ws_spi.h"

#define BUF1_WRITE 0x84
#define BUF2_WRITE 0x87
#define BUF1_READ 0xD4
#define BUF2_READ 0xD6
#define BBUF1_TO_MM_PAGE_PROG_WITH_ERASE 0x83
#define BBUF2_TO_MM_PAGE_PROG_WITH_ERASE 0x86
#define MM_PAGE_TO_B1_XFER 0x53
#define MM_PAGE_TO_B2_XFER 0x55
#define PAGE_ERASE 0x81   // 512/528 bytes per page
#define SECTOR_ERASE 0x7C // 128k bytes per sector
#define READ_STATE_REGISTER 0xD7
#define Read_ID 0x9F

#define AT45DBXX_Enable GPIO_ResetBits(GPIOC, GPIO_Pin_1);
#define AT45DBXX_Disable GPIO_SetBits(GPIOC, GPIO_Pin_1);

void AT45DBXX_Init(void);
void AT45DBXX_Read_ID(u8 *Data);

#endif /*_AT45DBXX_H*/
