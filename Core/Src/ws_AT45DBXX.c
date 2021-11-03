#include "ws_AT45DBXX.h"

void AT45DBXX_Init(void)
{
    SPI2_Configuration();
    PRINT("SPI is ready!\r\n");
    AT45DBXX_Disable;
}

static void AT45DBXX_BUSY(void)
{
    u8 AT45DBXXStruct;
    AT45DBXX_Enable;
    SPI2_Send_byte(READ_STATE_REGISTER);
    do
    {
        AT45DBXXStruct = SPI2_Receive_byte();
    } while (!(AT45DBXXStruct & 0x80));
    AT45DBXX_Disable;
}

void AT45DBXX_Read_ID(u8 *IData)
{
    u8 i;
    AT45DBXX_BUSY();
    AT45DBXX_Enable;
    SPI2_Send_byte(Read_ID); //ִ�ж�ȡid����
    for (i = 0; i < 4; i++)
    {
        IData[i] = SPI2_Receive_byte();
    }
    AT45DBXX_Disable;
}

static void write_buffer(u16 BufferOffset, u8 Data)
{
    AT45DBXX_Enable;
    SPI2_Send_byte(0x84);
    SPI2_Send_byte(0xff);
    SPI2_Send_byte((u8)BufferOffset >> 8);
    SPI2_Send_byte((u8)BufferOffset);
    SPI2_Send_byte(Data);
    AT45DBXX_Disable;
}

static u8 read_buffer(u16 BufferOffset)
{
    u8 temp;
    AT45DBXX_Enable;
    SPI2_Send_byte(0xd4);
    SPI2_Send_byte(0xff);
    SPI2_Send_byte((u8)BufferOffset >> 8);
    SPI2_Send_byte((u8)BufferOffset);
    temp = SPI2_Receive_byte();
    AT45DBXX_Disable;
    return temp;
}
