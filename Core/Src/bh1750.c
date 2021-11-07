#include "bh1750.h"

static void BH_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    BH_IIC_SCL = 1;
    BH_IIC_SDA = 1;
}

static void BH_IIC_Start(void)
{
    BH_SDA_OUT();
    BH_IIC_SDA = 1;
    BH_IIC_SCL = 1;
    delay_us(4);
    BH_IIC_SDA = 0;
    delay_us(4);
    BH_IIC_SCL = 0;
}

static void BH_IIC_Stop(void)
{
    BH_SDA_OUT();
    BH_IIC_SCL = 0;
    BH_IIC_SDA = 0;
    delay_us(4);
    BH_IIC_SCL = 1;
    BH_IIC_SDA = 1;
    delay_us(4);
}

static u8 BH_IIC_Wait_Ack(void)
{
    u8 ucErrTime = 0;
    BH_SDA_IN();
    BH_IIC_SDA = 1;
    delay_us(1);
    BH_IIC_SCL = 1;
    delay_us(1);
    while (BH_SDA_READ)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            BH_IIC_Stop();
            return 1;
        }
    }
    BH_IIC_SCL = 0;
    return 0;
}

static void BH_IIC_Ack(void)
{
    BH_IIC_SCL = 0;
    BH_SDA_OUT();
    BH_IIC_SDA = 0;
    delay_us(2);
    BH_IIC_SCL = 1;
    delay_us(2);
    BH_IIC_SCL = 0;
}

static void BH_IIC_NAck(void)
{
    BH_IIC_SCL = 0;
    BH_SDA_OUT();
    BH_IIC_SDA = 1;
    delay_us(2);
    BH_IIC_SCL = 1;
    delay_us(2);
    BH_IIC_SCL = 0;
}

static void BH_IIC_Send_Byte(u8 txd)
{
    u8 t;
    BH_SDA_OUT();
    BH_IIC_SCL = 0;
    for (t = 0; t < 8; t++)
    {
        BH_IIC_SDA = (txd & 0x80) >> 7;
        txd <<= 1;
        delay_us(2);
        BH_IIC_SCL = 1;
        delay_us(2);
        BH_IIC_SCL = 0;
        delay_us(2);
    }
}

static u8 BH_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i, receive = 0;
    BH_SDA_IN();
    for (i = 0; i < 8; i++)
    {
        BH_IIC_SCL = 0;
        delay_us(2);
        BH_IIC_SCL = 1;
        receive <<= 1;
        if (BH_SDA_READ)
        {
            receive++;
        }
        delay_us(1);
    }
    if (!ack)
    {
        BH_IIC_NAck();
    }
    else
    {
        BH_IIC_Ack();
    }
    return receive;
}

// BH1750 write a byte
static void BH1750_Byte_Write(uint8_t data)
{
    BH_IIC_Start();
    // Send write address
    BH_IIC_Send_Byte(BH1750_Addr | 0);
    BH_IIC_Wait_Ack();
    // Send control command
    BH_IIC_Send_Byte(data);
    BH_IIC_Wait_Ack();
    BH_IIC_Stop();
}

// BH1750s power on
static void BH1750_Power_ON(void)
{
    BH1750_Byte_Write(POWER_ON);
}

// BH1750s power off
static void BH1750_Power_OFF(void)
{
    BH1750_Byte_Write(POWER_OFF);
}

// BH1750 reset only valid when powered on
static void BH1750_Reset(void)
{
    BH1750_Byte_Write(MODULE_RESET);
}

// BH1750 initialization
void BH1750_Init(void)
{
    BH_IIC_Init();
    BH1750_Power_ON(); // BH1750s power on
    BH1750_Reset();    // BH1750 reset
    BH1750_Byte_Write(Measure_Mode);
    delay_ms(120);
}

// BH1750 reading measurement data
// Return value successful: return light intensity failed: return 0
uint16_t BH1750_Read_Measure(void)
{
    uint16_t receive_data = 0;
    BH_IIC_Start();
    // Send read address
    BH_IIC_Send_Byte(BH1750_Addr | 1);
    BH_IIC_Wait_Ack();

    // Read high octet
    receive_data = BH_IIC_Read_Byte(1);
    // Read low octet
    receive_data = (receive_data << 8) + BH_IIC_Read_Byte(0);
    BH_IIC_Stop();
    return receive_data; // Return read data
}

// Obtaining light intensity
float Light_Intensity(void)
{
    return (float)(BH1750_Read_Measure() / 1.2f * Resolution);
}
