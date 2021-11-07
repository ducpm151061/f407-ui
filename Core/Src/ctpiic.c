#include "ctpiic.h"

/*****************************************************************************
 * @name       :void CTP_Delay(void)
 * @date       :2020-05-13
 * @function   :Delay in controlling IIC speed
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_Delay(void)
{
    delay_us(1);
}

/*****************************************************************************
 * @name       :void CTP_IIC_Init(void)
 * @date       :2020-05-13
 * @function   :Initialize IIC
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);      

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;     
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
    GPIO_Init(GPIOC, &GPIO_InitStructure);        

    CTP_IIC_SCL = 1;
    CTP_IIC_SDA = 1;
}

/*****************************************************************************
 * @name       :void CTP_IIC_Start(void)
 * @date       :2020-05-13
 * @function   :Generating IIC starting signal
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_Start(void)
{
    CTP_SDA_OUT(); 
    CTP_IIC_SDA = 1;
    CTP_IIC_SCL = 1;
    CTP_Delay();
    CTP_IIC_SDA = 0; // START:when CLK is high,DATA change form high to low
    CTP_Delay();
    CTP_IIC_SCL = 0; 
}

/*****************************************************************************
 * @name       :void CTP_IIC_Stop(void)
 * @date       :2020-05-13
 * @function   :Generating IIC stop signal
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_Stop(void)
{
    CTP_SDA_OUT(); 
    CTP_IIC_SCL = 0;
    CTP_IIC_SDA = 0;
    CTP_Delay();
    CTP_IIC_SCL = 1;
    CTP_Delay();
    CTP_IIC_SDA = 1; // STOP:when CLK is high DATA change form low to high
}

/*****************************************************************************
 * @name       :u8 CTP_IIC_Wait_Ack(void)
 * @date       :2020-05-13
 * @function   :Wait for the response signal
 * @parameters :None
 * @retvalue   :0-receive response signal successfully
                                1-receive response signal unsuccessfully
******************************************************************************/
u8 CTP_IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    CTP_SDA_IN(); 
    CTP_IIC_SDA = 1;
    CTP_IIC_SCL = 1;
    CTP_Delay();
    while (CTP_READ_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            CTP_IIC_Stop();
            return 1;
        }
    }
    CTP_IIC_SCL = 0; 
    return 0;
}

/*****************************************************************************
 * @name       :void CTP_IIC_Ack(void)
 * @date       :2020-05-13
 * @function   :Generate ACK response signal
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_Ack(void)
{
    CTP_IIC_SCL = 0;
    CTP_SDA_OUT();
    CTP_IIC_SDA = 0;
    CTP_Delay();
    CTP_IIC_SCL = 1;
    CTP_Delay();
    CTP_IIC_SCL = 0;
}

/*****************************************************************************
 * @name       :void CTP_IIC_NAck(void)
 * @date       :2020-05-13
 * @function   :Don't generate ACK response signal
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_NAck(void)
{
    CTP_IIC_SCL = 0;
    CTP_SDA_OUT();
    CTP_IIC_SDA = 1;
    CTP_Delay();
    CTP_IIC_SCL = 1;
    CTP_Delay();
    CTP_IIC_SCL = 0;
}

/*****************************************************************************
 * @name       :void CTP_IIC_Send_Byte(u8 txd)
 * @date       :2020-05-13
 * @function   :send a byte data by IIC bus
 * @parameters :txd:Data to be sent
 * @retvalue   :None
 ******************************************************************************/
void CTP_IIC_Send_Byte(u8 txd)
{
    u8 t;
    CTP_SDA_OUT();
    CTP_IIC_SCL = 0; 
    for (t = 0; t < 8; t++)
    {
        CTP_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        CTP_IIC_SCL = 1;
        CTP_Delay();
        CTP_IIC_SCL = 0;
        CTP_Delay();
    }
}

/*****************************************************************************
 * @name       :u8 CTP_IIC_Read_Byte(unsigned char ack)
 * @date       :2020-05-13
 * @function   :read a byte data by IIC bus
 * @parameters :ack:0-send nACK
                                      1-send ACK
 * @retvalue   :Data to be read
******************************************************************************/
u8 CTP_IIC_Read_Byte(unsigned char ack)
{
    u8 i, receive = 0;
    CTP_SDA_IN(); 
    for (i = 0; i < 8; i++)
    {
        CTP_IIC_SCL = 0;
        CTP_Delay();
        CTP_IIC_SCL = 1;
        receive <<= 1;
        if (CTP_READ_SDA)
            receive++;
    }
    if (!ack)
        CTP_IIC_NAck(); 
    else
        CTP_IIC_Ack(); 
    return receive;
}
