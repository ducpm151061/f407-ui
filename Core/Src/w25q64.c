#include "w25q64.h"
#include "delay.h"
#include "spi.h"
#include "usart.h"

u16 W25QXX_TYPE = W25Q64; //Ĭ����W25Q64

// 4KbytesΪһ��Sector
// 16������Ϊ1��Block
// W25Q64
//����Ϊ8M�ֽ�,����64��Block,2048��Sector

/*****************************************************************************
 * @name       :void W25Q64_Init(void)
 * @date       :2020-05-08
 * @function   :Initialization of SPI flash IO port
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //ʹ��GPIOBʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE); //ʹ��GPIOGʱ��
                                                          // GPIOB14
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;            // PB14
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;         //���
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;    // 100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          //����
    GPIO_Init(GPIOB, &GPIO_InitStructure);                //��ʼ��
    W25Q64_CS = 1;                                        // SPI FLASH��ѡ��
    SPI1_Init();                                          //��ʼ��SPI
    SPI1_SetSpeed(SPI_BaudRatePrescaler_4);               //����Ϊ21Mʱ��,����ģʽ
    W25QXX_TYPE = W25QXX_ReadID();                        //��ȡFLASH ID.
}

/*****************************************************************************
 * @name       :u8 W25Q64_ReadSR(void)
 * @date       :2020-05-08
 * @function   :Read the status register of w25q64
                                BIT7  6   5   4   3   2   1   0
                              SPR   RV  TB BP2 BP1 BP0 WEL BUSY
                                SPR:Default 0, status register protection bit, used with WP
                                TB,BP2,BP1,BP0:Flash area write protection settings
                                WEL:Write enable lock
                                BUSY:Busy flag bit (1, busy; 0, idle)
                                Default:0x00
 * @parameters :None
 * @retvalue   :the value of the status register
******************************************************************************/
u8 W25Q64_ReadSR(void)
{
    u8 byte = 0;
    W25Q64_CS = 0;                          //ʹ������
    SPI1_ReadWriteByte(W25X_ReadStatusReg); //���Ͷ�ȡ״̬�Ĵ�������
    byte = SPI1_ReadWriteByte(0Xff);        //��ȡһ���ֽ�
    W25Q64_CS = 1;                          //ȡ��Ƭѡ
    return byte;
}

/*****************************************************************************
 * @name       :void W25Q64_Write_SR(u8 sr)
 * @date       :2020-05-08
 * @function   :Write the status register of w25q64
                                (Only SPR,TB,bp2,BP1,bp0(bit 7,5,4,3,2)can be written!!!)
                                BIT7  6   5   4   3   2   1   0
                              SPR   RV  TB BP2 BP1 BP0 WEL BUSY
                                SPR:Default 0, status register protection bit, used with WP
                                TB,BP2,BP1,BP0:Flash area write protection settings
                                WEL:Write enable lock
                                BUSY:Busy flag bit (1, busy; 0, idle)
                                Default:0x00
 * @parameters :sr:the value of the status register to be written
 * @retvalue   :the value of the status register
******************************************************************************/
void W25Q64_Write_SR(u8 sr)
{
    W25Q64_CS = 0;                           //ʹ������
    SPI1_ReadWriteByte(W25X_WriteStatusReg); //����дȡ״̬�Ĵ�������
    SPI1_ReadWriteByte(sr);                  //д��һ���ֽ�
    W25Q64_CS = 1;                           //ȡ��Ƭѡ
}

/*****************************************************************************
 * @name       :void W25Q64_Write_Enable(void)
 * @date       :2020-05-08
 * @function   :W25q64 write enable(Set wel)
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Write_Enable(void)
{
    W25Q64_CS = 0;                        //ʹ������
    SPI1_ReadWriteByte(W25X_WriteEnable); //����дʹ��
    W25Q64_CS = 1;                        //ȡ��Ƭѡ
}

/*****************************************************************************
 * @name       :void W25Q64_Write_Disable(void)
 * @date       :2020-05-08
 * @function   :W25q64 write disable(Reset wel)
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Write_Disable(void)
{
    W25Q64_CS = 0;                         //ʹ������
    SPI1_ReadWriteByte(W25X_WriteDisable); //����д��ָֹ��
    W25Q64_CS = 1;                         //ȡ��Ƭѡ
}

/*****************************************************************************
 * @name       :u16 W25QXX_ReadID(void)
 * @date       :2020-05-08
 * @function   :Read chip ID
 * @parameters :None
 * @retvalue   :0XEF13:W25Q80
                                0XEF14:W25Q16
                                0XEF15:W25Q32
                                0XEF16:W25Q64
                                0XEF17:W25Q128
******************************************************************************/
u16 W25QXX_ReadID(void)
{
    u16 Temp = 0;
    W25Q64_CS = 0;
    SPI1_ReadWriteByte(0x90); //���Ͷ�ȡID����
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    SPI1_ReadWriteByte(0x00);
    Temp |= SPI1_ReadWriteByte(0xFF) << 8;
    Temp |= SPI1_ReadWriteByte(0xFF);
    W25Q64_CS = 1;
    return Temp;
}

/*****************************************************************************
 * @name       :void W25Q64_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)
 * @date       :2020-05-08
 * @function   :read spi flash
                                Start reading data of specified length at the specified address
 * @parameters :pBuffer:Data storage area
                                ReadAddr:Address to start reading (24bit)
                                NumByteToRead:Number of bytes to read (maximum 65535)
 * @retvalue   :None
******************************************************************************/
void W25Q64_Read(u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead)
{
    u16 i;
    W25Q64_CS = 0;                              //ʹ������
    SPI1_ReadWriteByte(W25X_ReadData);          //���Ͷ�ȡ����
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 16)); //����24bit��ַ
    SPI1_ReadWriteByte((u8)((ReadAddr) >> 8));
    SPI1_ReadWriteByte((u8)ReadAddr);
    for (i = 0; i < NumByteToRead; i++)
    {
        pBuffer[i] = SPI1_ReadWriteByte(0XFF); //ѭ������
    }
    W25Q64_CS = 1;
}

/*****************************************************************************
 * @name       :void W25Q64_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)
 * @date       :2020-05-08
 * @function   :Write less than 256 bytes of data in a page(0~65535)
                                Start writing data of specified length at the specified address
 * @parameters :pBuffer:Data storage area
                                WriteAddr:Address to start writing (24bit)
                                NumByteToWrite:Number of bytes to write (maximum 256),This number
                                              should not exceed the number of bytes left on this page!!!
 * @retvalue   :None
******************************************************************************/
void W25Q64_Write_Page(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 i;
    W25Q64_Write_Enable();                       // SET WEL
    W25Q64_CS = 0;                               //ʹ������
    SPI1_ReadWriteByte(W25X_PageProgram);        //����дҳ����
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 16)); //����24bit��ַ
    SPI1_ReadWriteByte((u8)((WriteAddr) >> 8));
    SPI1_ReadWriteByte((u8)WriteAddr);
    for (i = 0; i < NumByteToWrite; i++)
    {
        SPI1_ReadWriteByte(pBuffer[i]); //ѭ��д��
    }
    W25Q64_CS = 1;      //ȡ��Ƭѡ
    W25Q64_Wait_Busy(); //�ȴ�д�����
}

/*****************************************************************************
 * @name       :void W25Q64_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
 * @date       :2020-05-08
 * @function   :Write SPI flash without check
                                You must ensure that all the data in the address range written is 0xff,
                                otherwise the data written in non-0xff will fail!
                                With automatic page change function
                                Start to write the specified length data at the specified address,
                                but make sure that the address does not cross the boundary!
 * @parameters :pBuffer:Data storage area
                                WriteAddr:Address to start writing (24bit)
                                NumByteToWrite:Number of bytes to write (maximum 65535)
 * @retvalue   :None
******************************************************************************/
void W25Q64_Write_NoCheck(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u16 pageremain;
    pageremain = 256 - WriteAddr % 256; //��ҳʣ����ֽ���
    if (NumByteToWrite <= pageremain)
        pageremain = NumByteToWrite; //������256���ֽ�
    while (1)
    {
        W25Q64_Write_Page(pBuffer, WriteAddr, pageremain);
        if (NumByteToWrite == pageremain)
        {
            break; //д�������
        }
        else // NumByteToWrite>pageremain
        {
            pBuffer += pageremain;
            WriteAddr += pageremain;

            NumByteToWrite -= pageremain; //��ȥ�Ѿ�д���˵��ֽ���
            if (NumByteToWrite > 256)
            {
                pageremain = 256; //һ�ο���д��256���ֽ�
            }
            else
            {
                pageremain = NumByteToWrite; //����256���ֽ���
            }
        }
    }
}

u8 W25Q64_BUFFER[4096];
/*****************************************************************************
 * @name       :void W25Q64_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
 * @date       :2020-05-08
 * @function   :Write SPI flash
                                Start to write the specified length data at the specified address
                                This function has erase operation!
 * @parameters :pBuffer:Data storage area
                                WriteAddr:Address to start writing (24bit)
                                NumByteToWrite:Number of bytes to write (maximum 65535)
 * @retvalue   :None
******************************************************************************/
void W25Q64_Write(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u32 secpos;
    u16 secoff;
    u16 secremain;
    u16 i;
    u8 *W25QXX_BUF;
    W25QXX_BUF = W25Q64_BUFFER;
    secpos = WriteAddr / 4096; //������ַ
    secoff = WriteAddr % 4096; //�������ڵ�ƫ��
    secremain = 4096 - secoff; //����ʣ��ռ��С
    // printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);//������
    if (NumByteToWrite <= secremain)
        secremain = NumByteToWrite; //������4096���ֽ�
    while (1)
    {
        W25Q64_Read(W25QXX_BUF, secpos * 4096, 4096); //������������������
        for (i = 0; i < secremain; i++)               //У������
        {
            if (W25QXX_BUF[secoff + i] != 0XFF)
            {
                break; //��Ҫ����
            }
        }
        if (i < secremain) //��Ҫ����
        {
            W25Q64_Erase_Sector(secpos);    //�����������
            for (i = 0; i < secremain; i++) //����
            {
                W25QXX_BUF[i + secoff] = pBuffer[i];
            }
            W25Q64_Write_NoCheck(W25QXX_BUF, secpos * 4096, 4096); //д����������
        }
        else
        {
            W25Q64_Write_NoCheck(pBuffer, WriteAddr, secremain); //д�Ѿ������˵�,ֱ��д������ʣ������.
        }
        if (NumByteToWrite == secremain)
        {
            break; //д�������
        }
        else //д��δ����
        {
            secpos++;                    //������ַ��1
            secoff = 0;                  //ƫ��λ��Ϊ0
            pBuffer += secremain;        //ָ��ƫ��
            WriteAddr += secremain;      //д��ַƫ��
            NumByteToWrite -= secremain; //�ֽ����ݼ�
            if (NumByteToWrite > 4096)
            {
                secremain = 4096; //��һ����������д����
            }
            else
            {
                secremain = NumByteToWrite; //��һ����������д����
            }
        }
    }
}

/*****************************************************************************
 * @name       :void W25Q64_Erase_Chip(void)
 * @date       :2020-05-08
 * @function   :Erase whole chip(Waiting time is too long)
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Erase_Chip(void)
{
    W25Q64_Write_Enable(); // SET WEL
    W25Q64_Wait_Busy();
    W25Q64_CS = 0;                      //ʹ������
    SPI1_ReadWriteByte(W25X_ChipErase); //����Ƭ��������
    W25Q64_CS = 1;                      //ȡ��Ƭѡ
    W25Q64_Wait_Busy();                 //�ȴ�оƬ��������
}

/*****************************************************************************
 * @name       :void W25Q64_Erase_Sector(u32 Dst_Addr)
 * @date       :2020-05-08
 * @function   :Erase a sector(Minimum time: 150ms)
 * @parameters :Dst_Addr:The sector address(Set according to actual capacity)
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Erase_Sector(u32 Dst_Addr)
{
    //	printf("fe:%x\r\n",Dst_Addr);	//����falsh�������,������
    Dst_Addr *= 4096;
    W25Q64_Write_Enable(); // SET WEL
    W25Q64_Wait_Busy();
    W25Q64_CS = 0;                              //ʹ������
    SPI1_ReadWriteByte(W25X_SectorErase);       //������������ָ��
    SPI1_ReadWriteByte((u8)((Dst_Addr) >> 16)); //����24bit��ַ
    SPI1_ReadWriteByte((u8)((Dst_Addr) >> 8));
    SPI1_ReadWriteByte((u8)Dst_Addr);
    W25Q64_CS = 1;      //ȡ��Ƭѡ
    W25Q64_Wait_Busy(); //�ȴ��������
}

/*****************************************************************************
 * @name       :void W25Q64_Wait_Busy(void)
 * @date       :2020-05-08
 * @function   :Waiting for idle
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_Wait_Busy(void)
{
    while ((W25Q64_ReadSR() & 0x01) == 0x01)
        ; // �ȴ�BUSYλ���
}

/*****************************************************************************
 * @name       :void W25Q64_PowerDown(void)
 * @date       :2020-05-08
 * @function   :Enter power down mode
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_PowerDown(void)
{
    W25Q64_CS = 0;                      //ʹ������
    SPI1_ReadWriteByte(W25X_PowerDown); //���͵�������
    W25Q64_CS = 1;                      //ȡ��Ƭѡ
    delay_us(3);                        //�ȴ�TPD
}

/*****************************************************************************
 * @name       :void W25Q64_WAKEUP(void)
 * @date       :2020-05-08
 * @function   :Enter wake up mode
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void W25Q64_WAKEUP(void)
{
    W25Q64_CS = 0;                             //ʹ������
    SPI1_ReadWriteByte(W25X_ReleasePowerDown); //  send W25X_PowerDown command 0xAB
    W25Q64_CS = 1;                             //ȡ��Ƭѡ
    delay_us(3);                               //�ȴ�TRES1
}
