#include "aht10.h"

static void AHT_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    AHT_IIC_SCL = 1;
    AHT_IIC_SDA = 1;
}

static void AHT_IIC_Start(void)
{
    AHT_SDA_OUT();
    AHT_IIC_SDA = 1;
    AHT_IIC_SCL = 1;
    delay_us(4);
    AHT_IIC_SDA = 0;
    delay_us(4);
    AHT_IIC_SCL = 0;
}

static void AHT_IIC_Stop(void)
{
    AHT_SDA_OUT();
    AHT_IIC_SCL = 0;
    AHT_IIC_SDA = 0;
    delay_us(4);
    AHT_IIC_SCL = 1;
    AHT_IIC_SDA = 1;
    delay_us(4);
}

static u8 AHT_IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    AHT_SDA_IN();
    AHT_IIC_SDA = 1;
    delay_us(1);
    AHT_IIC_SCL = 1;
    delay_us(1);
    while (AHT_SDA_READ)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            AHT_IIC_Stop();
            return 1;
        }
    }
    AHT_IIC_SCL = 0;
    return 0;
}

static void ATH_IIC_Ack(void)
{
    AHT_IIC_SCL = 0;
    AHT_SDA_OUT();
    AHT_IIC_SDA = 0;
    delay_us(2);
    AHT_IIC_SCL = 1;
    delay_us(2);
    AHT_IIC_SCL = 0;
}

static void AHT_IIC_NAck(void)
{
    AHT_IIC_SCL = 0;
    AHT_SDA_OUT();
    AHT_IIC_SDA = 1;
    delay_us(2);
    AHT_IIC_SCL = 1;
    delay_us(2);
    AHT_IIC_SCL = 0;
}

static void AHT_IIC_Send_Byte(u8 txd)
{
    u8 t;
    AHT_SDA_OUT();
    AHT_IIC_SCL = 0;
    for (t = 0; t < 8; t++)
    {
        AHT_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2);
        AHT_IIC_SCL = 1;
        delay_us(2);
        AHT_IIC_SCL = 0;
        delay_us(2);
    }
}

static u8 AHT_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    AHT_SDA_IN();
    for (i = 0; i < 8; i++)
    {
        AHT_IIC_SCL = 0;
        delay_us(2);
        AHT_IIC_SCL = 1;
        receive <<= 1;
        if (AHT_SDA_READ)
        {
            receive++;
        }
        delay_us(1);
    }
    if (!ack)
    {
        AHT_IIC_NAck();
    }
    else
    {
        ATH_IIC_Ack();
    }
    return receive;
}

void AHT_Init(void)
{
    AHT_IIC_Init();
    AHT_IIC_Start();
    AHT_IIC_Send_Byte(AHTX0_I2CADDR_DEFAULT);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(AHTX0_CMD_CALIBRATE);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(AHTX0_STATUS_CALIBRATED);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(0x00);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Stop();
}

void AHT_Measure(void)
{
    AHT_IIC_Start();
    AHT_IIC_Send_Byte(AHTX0_I2CADDR_DEFAULT);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(AHTX0_CMD_TRIGGER);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(0x33);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Send_Byte(0x00);
    AHT_IIC_Wait_Ack();
    AHT_IIC_Stop();
}

void ATH_Read(int32_t *temperature, int32_t *humidity)
{
    u8 i;
    u32 temp = 0, hum = 0;
    u8 value[5];
    AHT_IIC_Start();
    AHT_IIC_Send_Byte(0x71);
    AHT_IIC_Wait_Ack();
    for (i = 0; i < 6; i++)
    {
        value[i] = AHT_IIC_Read_Byte(i == (5) ? 0 : 1);
    }
    AHT_IIC_Stop();

    if ((value[0] & 0x68) == 0x08)
    {
        hum = value[1];
        hum = (hum << 8) | value[2];
        hum = (hum << 4) | value[3];
        hum = hum >> 4;

        hum = (hum * 1000) / 1024 / 1024;

        temp = value[3];
        temp = temp & 0x0000000F;
        temp = (temp << 8) | value[4];
        temp = (temp << 8) | value[5];

        temp = (temp * 200) / 1024 / 1024 - 50;
    }
    *temperature = temp;
    *humidity = hum;
}

void AHT_Reset(void)
{
    AHT_IIC_Start();
    AHT_IIC_Send_Byte(AHTX0_I2CADDR_DEFAULT);
    AHT_IIC_Wait_Ack();

    AHT_IIC_Send_Byte(AHTX0_CMD_SOFTRESET);
    AHT_IIC_Wait_Ack();

    I2C_Stop();
}
