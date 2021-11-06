#include "tcs34725.h"

static void TCS34725_I2C_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    TCS_IIC_SCL = 1;
    TCS_IIC_SDA = 1;
}
static void TCS34725_I2C_Start()
{
    TCS_SDA_OUT();
    TCS_IIC_SDA = 1;
    TCS_IIC_SCL = 1;
    delay_us(4);
    TCS_IIC_SDA = 0;
    delay_us(4);
    TCS_IIC_SCL = 0;
}
static void TCS34725_I2C_Stop()
{
    TCS_SDA_OUT();
    TCS_IIC_SCL = 0;
    TCS_IIC_SDA = 0;
    delay_us(4);
    TCS_IIC_SCL = 1;
    TCS_IIC_SDA = 1;
    delay_us(4);
}

static u8 TCS34725_I2C_Wait_ACK()
{
    u32 t = 0;

    TCS_SDA_IN();
    TCS_IIC_SDA = 1;
    delay_us(1);
    TCS_IIC_SCL = 1;
    delay_us(1);
    while (TCS_SDA_READ)
    {
        t++;
        if (t > 250)
        {
            TCS34725_I2C_Stop();
            return 1;
        }
    }
    TCS_IIC_SCL = 0;
    return 0;
}
static void TCS34725_I2C_ACK()
{
    TCS_IIC_SCL = 0;
    TCS_SDA_OUT();
    TCS_IIC_SDA = 0;
    delay_us(2);
    TCS_IIC_SCL = 1;
    delay_us(2);
    TCS_IIC_SCL = 0;
}
static void TCS34725_I2C_NACK()
{
    TCS_IIC_SCL = 0;
    TCS_SDA_OUT();
    TCS_IIC_SDA = 1;
    delay_us(2);
    TCS_IIC_SCL = 1;
    delay_us(2);
    ;
    TCS_IIC_SCL = 0;
}
static void TCS34725_I2C_Send_Byte(u8 byte)
{
    u8 i;

    TCS_SDA_OUT();
    TCS_IIC_SCL = 0;
    for (i = 0; i < 8; i++)
    {
        if (((byte & 0x80) >> 7) == 1)
            TCS_IIC_SDA = 1;
        else
            TCS_IIC_SDA = 0;
        byte <<= 1;

        delay_us(2);
        TCS_IIC_SCL = 1;
        delay_us(2);
        TCS_IIC_SCL = 0;
        delay_us(2);
    }
}
static u8 TCS34725_I2C_Read_Byte(u8 ack)
{
    u8 i, receive = 0;

    TCS_SDA_IN();
    for (i = 0; i < 8; i++)
    {
        TCS_IIC_SCL = 0;
        delay_us(2);
        TCS_IIC_SCL = 1;
        receive <<= 1;
        if (TCS_SDA_READ)
            receive++;
        delay_us(1);
    }
    if (!ack)
        TCS34725_I2C_NACK();
    else
        TCS34725_I2C_ACK();
    return receive;
}
/*********************************************/
/*******************************************************************************
 * @brief Writes data to a slave device.
 *
 * @param slaveAddress - Adress of the slave device.
 * @param dataBuffer - Pointer to a buffer storing the transmission data.
 * @param bytesNumber - Number of bytes to write.
 * @param stopBit - Stop condition control.
 *                  Example: 0 - A stop condition will not be sent;
 *                           1 - A stop condition will be sent.
 *******************************************************************************/
static void TCS34725_I2C_Write(u8 slaveAddress, u8 *dataBuffer, u8 bytesNumber, u8 stopBit)
{
    u8 i = 0;

    TCS34725_I2C_Start();
    TCS34725_I2C_Send_Byte((slaveAddress << 1) | 0x00);
    TCS34725_I2C_Wait_ACK();
    for (i = 0; i < bytesNumber; i++)
    {
        TCS34725_I2C_Send_Byte(*(dataBuffer + i));
        TCS34725_I2C_Wait_ACK();
    }
    if (stopBit == 1)
        TCS34725_I2C_Stop();
}
/*******************************************************************************
 * @brief Reads data from a slave device.
 *
 * @param slaveAddress - Adress of the slave device.
 * @param dataBuffer - Pointer to a buffer that will store the received data.
 * @param bytesNumber - Number of bytes to read.
 * @param stopBit - Stop condition control.
 *                  Example: 0 - A stop condition will not be sent;
 *                           1 - A stop condition will be sent.
 *******************************************************************************/
static void TCS34725_I2C_Read(u8 slaveAddress, u8 *dataBuffer, u8 bytesNumber, u8 stopBit)
{
    u8 i = 0;

    TCS34725_I2C_Start();
    TCS34725_I2C_Send_Byte((slaveAddress << 1) | 0x01);
    TCS34725_I2C_Wait_ACK();
    for (i = 0; i < bytesNumber; i++)
    {
        if (i == bytesNumber - 1)
        {
            *(dataBuffer + i) = TCS34725_I2C_Read_Byte(0);
        }
        else
        {
            *(dataBuffer + i) = TCS34725_I2C_Read_Byte(1);
        }
    }
    if (stopBit == 1)
        TCS34725_I2C_Stop();
}
/*******************************************************************************
 * @brief Writes data into TCS34725 registers, starting from the selected
 *        register address pointer.
 *
 * @param subAddr - The selected register address pointer.
 * @param dataBuffer - Pointer to a buffer storing the transmission data.
 * @param bytesNumber - Number of bytes that will be sent.
 *
 * @return None.
 *******************************************************************************/
static void TCS34725_Write(u8 subAddr, u8 *dataBuffer, u8 bytesNumber)
{
    u8 sendBuffer[10] = {
        0,
    };
    u8 byte = 0;

    sendBuffer[0] = subAddr | TCS34725_COMMAND_BIT;
    for (byte = 1; byte <= bytesNumber; byte++)
    {
        sendBuffer[byte] = dataBuffer[byte - 1];
    }
    TCS34725_I2C_Write(TCS34725_ADDRESS, sendBuffer, bytesNumber + 1, 1);
}
/*******************************************************************************
 * @brief Reads data from TCS34725 registers, starting from the selected
 *        register address pointer.
 *
 * @param subAddr - The selected register address pointer.
 * @param dataBuffer - Pointer to a buffer that will store the received data.
 * @param bytesNumber - Number of bytes that will be read.
 *
 * @return None.
 *******************************************************************************/
static void TCS34725_Read(u8 subAddr, u8 *dataBuffer, u8 bytesNumber)
{
    subAddr |= TCS34725_COMMAND_BIT;

    TCS34725_I2C_Write(TCS34725_ADDRESS, (u8 *)&subAddr, 1, 0);
    TCS34725_I2C_Read(TCS34725_ADDRESS, dataBuffer, bytesNumber, 1);
}
/*******************************************************************************
 * @brief TCS34725 Read
 *
 * @return None
 *******************************************************************************/
static void TCS34725_SetIntegrationTime(u8 time)
{
    TCS34725_Write(TCS34725_ATIME, &time, 1);
}
/*******************************************************************************
 * @brief TCS34725 Enable
 *
 * @return None
 *******************************************************************************/
static void TCS34725_SetGain(u8 gain)
{
    TCS34725_Write(TCS34725_CONTROL, &gain, 1);
}
/*******************************************************************************
 * @brief TCS34725 Set Gain
 *
 * @return None
 *******************************************************************************/
static void TCS34725_Enable(void)
{
    u8 cmd = TCS34725_ENABLE_PON;

    TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
    cmd = TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN;
    TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
    /* delay_ms(3); */
}
/*******************************************************************************
 * @brief TCS34725 Disable
 *
 * @return None
 *******************************************************************************/
static void TCS34725_Disable(void)
{
    u8 cmd = 0;

    TCS34725_Read(TCS34725_ENABLE, &cmd, 1);
    cmd = cmd & ~(TCS34725_ENABLE_PON | TCS34725_ENABLE_AEN);
    TCS34725_Write(TCS34725_ENABLE, &cmd, 1);
}
/*******************************************************************************
 * @brief TCS34725 Init
 *
 * @return ID - ID TCS3472X
 *******************************************************************************/
u8 TCS34725_Init(void)
{
    u8 id = 0;

    TCS34725_I2C_Init();
    TCS34725_Read(TCS34725_ID, &id, 1);
    if (id == 0x4D | id == 0x44)
    {
        TCS34725_SetIntegrationTime(TCS34725_INTEGRATIONTIME_50MS);
        TCS34725_SetGain(TCS34725_GAIN_1X);
        TCS34725_Enable();
        return 1;
    }
    return 0;
}
/*******************************************************************************
 * @brief TCS34725 Channel Data
 *
 * @return data - channel data
 *******************************************************************************/
u16 TCS34725_GetChannelData(u8 reg)
{
    u8 tmp[2] = {0, 0};
    u16 data;

    TCS34725_Read(reg, tmp, 2);
    data = (tmp[1] << 8) | tmp[0];

    return data;
}
/*******************************************************************************
 * @brief TCS34725 Raw Data
 *
 * @return 1 - Valid data
 *   	   0 - Invalid data
 *******************************************************************************/
u8 TCS34725_GetRawData(COLOR_RGBC *rgbc)
{
    u8 status = TCS34725_STATUS_AVALID;

    TCS34725_Read(TCS34725_STATUS, &status, 1);

    if (status & TCS34725_STATUS_AVALID)
    {
        rgbc->c = TCS34725_GetChannelData(TCS34725_CDATAL);
        rgbc->r = TCS34725_GetChannelData(TCS34725_RDATAL);
        rgbc->g = TCS34725_GetChannelData(TCS34725_GDATAL);
        rgbc->b = TCS34725_GetChannelData(TCS34725_BDATAL);
        return 1;
    }
    return 0;
}
/******************************************************************************/
// RGB to HSL
void RGBtoHSL(COLOR_RGBC *Rgb, COLOR_HSL *Hsl)
{
    u8 maxVal, minVal, difVal;
    u8 r = Rgb->r * 100 / Rgb->c; //[0-100]
    u8 g = Rgb->g * 100 / Rgb->c;
    u8 b = Rgb->b * 100 / Rgb->c;

    maxVal = max3v(r, g, b);
    minVal = min3v(r, g, b);
    difVal = maxVal - minVal;

    Hsl->l = (maxVal + minVal) / 2; //[0-100]

    if (maxVal == minVal)
    {
        Hsl->h = 0;
        Hsl->s = 0;
    }
    else
    {
        if (maxVal == r)
        {
            if (g >= b)
                Hsl->h = 60 * (g - b) / difVal;
            else
                Hsl->h = 60 * (g - b) / difVal + 360;
        }
        else
        {
            if (maxVal == g)
                Hsl->h = 60 * (b - r) / difVal + 120;
            else if (maxVal == b)
                Hsl->h = 60 * (r - g) / difVal + 240;
        }

        if (Hsl->l <= 50)
            Hsl->s = difVal * 100 / (maxVal + minVal); //[0-100]
        else
            Hsl->s = difVal * 100 / (200 - (maxVal + minVal));
    }
}
/******************************************************************************/
