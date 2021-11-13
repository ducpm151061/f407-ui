#include "lcd.h"
#include "delay.h"

_lcd_dev lcddev;

u16 POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;
u16 DeviceCode;

/*****************************************************************************
 * @name       :void LCD_WR_REG(u16 data)
 * @date       :2018-08-09
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :data:Command value to be written
 * @retvalue   :None
 ******************************************************************************/
void LCD_WR_REG(vu16 data)
{
    data = data;
    LCD->LCD_REG = data;
}

/*****************************************************************************
 * @name       :void LCD_WR_DATA(u16 data)
 * @date       :2018-08-09
 * @function   :Write an 16-bit data to the LCD screen
 * @parameters :data:data value to be written
 * @retvalue   :None
 ******************************************************************************/
void LCD_WR_DATA(vu16 data)
{
    data = data;
    LCD->LCD_RAM = data;
}

/*****************************************************************************
 * @name       :u16 LCD_RD_DATA(void)
 * @date       :2018-11-13
 * @function   :Read an 16-bit value from the LCD screen
 * @parameters :None
 * @retvalue   :read value
 ******************************************************************************/
u16 LCD_RD_DATA(void)
{
    vu16 data;
    data = LCD->LCD_RAM;
    return data;
}

/*****************************************************************************
 * @name       :void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
 * @date       :2018-08-09
 * @function   :Write data into registers
 * @parameters :LCD_Reg:Register address
                LCD_RegValue:Data to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{
    LCD->LCD_REG = LCD_Reg;
    LCD->LCD_RAM = LCD_RegValue;
}

/*****************************************************************************
 * @name       :u16 LCD_ReadReg(u16 LCD_Reg)
 * @date       :2018-11-13
 * @function   :read value from specially registers
 * @parameters :LCD_Reg:Register address
 * @retvalue   :read value
 ******************************************************************************/
void LCD_ReadReg(u16 LCD_Reg, u16 *Rval, int n)
{
    LCD_WR_REG(LCD_Reg);
    while (n--)
    {
        *(Rval++) = LCD_RD_DATA();
        delay_us(300);
    }
}

/*****************************************************************************
 * @name       :void LCD_WriteRAM_Prepare(void)
 * @date       :2018-08-09
 * @function   :Write GRAM
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.wramcmd);
}

/*****************************************************************************
 * @name       :void LCD_ReadRAM_Prepare(void)
 * @date       :2018-11-13
 * @function   :Read GRAM
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_ReadRAM_Prepare(void)
{
    LCD_WR_REG(lcddev.rramcmd);
}

/*****************************************************************************
 * @name       :void Lcd_WriteData_16Bit(u16 Data)
 * @date       :2018-08-09
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :Data:Data to be written
 * @retvalue   :None
 ******************************************************************************/
void Lcd_WriteData_16Bit(u16 Data)
{
    LCD->LCD_RAM = Data;
}

u16 Color_To_565(u8 r, u8 g, u8 b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}

/*****************************************************************************
 * @name       :u16 Lcd_ReadData_16Bit(void)
 * @date       :2018-11-13
 * @function   :Read an 16-bit value from the LCD screen
 * @parameters :None
 * @retvalue   :read value
 ******************************************************************************/
u16 Lcd_ReadData_16Bit(void)
{
    u16 r, g;
    // dummy data
    r = LCD_RD_DATA();
    if (lcddev.id == 0x1963)
    {
        return r;
    }
    delay_us(1);
    // real color
    r = LCD_RD_DATA();
    if ((lcddev.id == 0xB509) || (lcddev.id == 0x7793) || (lcddev.id == 0x9486) || (lcddev.id == 0x7796))
    {
        return r;
    }
    delay_us(1);
    g = LCD_RD_DATA();
    if ((lcddev.id == 0x9341) || (lcddev.id == 0x9488) || (lcddev.id == 0x5310) || (lcddev.id == 0x5510))
    {
        return Color_To_565(r >> 8, r & 0xFF, g >> 8);
    }
    return 0;
}

/*****************************************************************************
 * @name       :void LCD_DrawPoint(u16 x,u16 y)
 * @date       :2018-08-09
 * @function   :Write a pixel data at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y)
{
    LCD_SetCursor(x, y);
    Lcd_WriteData_16Bit(POINT_COLOR);
}

/*****************************************************************************
 * @name       :u16 LCD_ReadPoint(u16 x,u16 y)
 * @date       :2018-11-13
 * @function   :Read a pixel color value at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :the read color value
******************************************************************************/
u16 LCD_ReadPoint(u16 x, u16 y)
{
    u16 color;
    if (x >= lcddev.width || y >= lcddev.height)
    {
        return 0;
    }
    LCD_SetCursor(x, y);
    LCD_ReadRAM_Prepare();
    color = Lcd_ReadData_16Bit();
    return color;
}

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
 ******************************************************************************/
void LCD_Clear(u16 Color)
{
    unsigned int i;
    u32 total_point = lcddev.width * lcddev.height;
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
    for (i = 0; i < total_point; i++)
    {
        LCD->LCD_RAM = Color;
    }
}

/*****************************************************************************
 * @name       :void LCD_PWM_BackLightSet(u8 pwm)
 * @date       :2020-05-13
 * @function   :Set the backlight brightness from PWM(only for 0x1963)
 * @parameters :pwm:pwm value 0~100, The greater the value, the brighter
 * @retvalue   :None
 ******************************************************************************/
void LCD_PWM_BackLightSet(u8 pwm)
{
    LCD_WR_REG(0xBE);
    LCD_WR_DATA(0x05);
    LCD_WR_DATA(pwm * 2.55);
    LCD_WR_DATA(0x01);
    LCD_WR_DATA(0xFF);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
}

/*****************************************************************************
 * @name       :void LCD_GPIOInit(void)
 * @date       :2018-08-09
 * @function   :Initialization LCD screen GPIO
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_GPIOInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef readWriteTiming;
    FSMC_NORSRAMTimingInitTypeDef writeTiming;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = (3 << 0) | (3 << 4) | (7 << 8) | (3 << 14);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = (0X1FF << 7);
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC); // PD7 NE1
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);

    readWriteTiming.FSMC_AddressSetupTime = 0XF;
    readWriteTiming.FSMC_AddressHoldTime = 0x00;
    readWriteTiming.FSMC_DataSetupTime = 60;
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;

    writeTiming.FSMC_AddressSetupTime = 9;
    writeTiming.FSMC_AddressHoldTime = 0x00;
    writeTiming.FSMC_DataSetupTime = 8;
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision = 0x00;
    writeTiming.FSMC_DataLatency = 0x00;
    writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

void LCD_Set_BWTR(void)
{
    FSMC_Bank1E->BWTR[0] &= ~(0XF << 0);
    FSMC_Bank1E->BWTR[0] &= ~(0XF << 8);
    if (lcddev.id == 0x9341)
    {
        FSMC_Bank1E->BWTR[0] |= 4 << 0;
        FSMC_Bank1E->BWTR[0] |= 3 << 8;
    }
    else if ((lcddev.id == 0x9486) || (lcddev.id == 0x9488) || (lcddev.id == 0x5310) || (lcddev.id == 0x7796) ||
             (lcddev.id == 0x1963))
    {
        FSMC_Bank1E->BWTR[0] |= 4 << 0;
        FSMC_Bank1E->BWTR[0] |= 4 << 8;
    }
    else if ((lcddev.id == 0xB509) || (lcddev.id == 0x7793))
    {
        FSMC_Bank1E->BWTR[0] |= 5 << 0;
        FSMC_Bank1E->BWTR[0] |= 4 << 8;
    }
    else if (lcddev.id == 0x5510)
    {
        FSMC_Bank1E->BWTR[0] |= 3 << 0;
        FSMC_Bank1E->BWTR[0] |= 2 << 8;
    }
}

/*****************************************************************************
 * @name       :void LCD_Init(void)
 * @date       :2018-08-09
 * @function   :Initialization LCD screen
 * @parameters :None
 * @retvalue   :None
 ******************************************************************************/
void LCD_Init(void)
{
    LCD_GPIOInit();
    lcddev.id = LCD_Read_ID();
    PRINT("LCD ID:%d\r\n", lcddev.id);
    LCD_Set_BWTR();
    delay_ms(100);
    // LCD_RESET();
    switch (lcddev.id)
    {
    case 0x9341: {
        //*************ILI9341**********//
        LCD_WR_REG(0xCF);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC9); // C1
        LCD_WR_DATA(0X30);
        LCD_WR_REG(0xED);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0X12);
        LCD_WR_DATA(0X81);
        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x85);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x7A);
        LCD_WR_REG(0xCB);
        LCD_WR_DATA(0x39);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xF7);
        LCD_WR_DATA(0x20);
        LCD_WR_REG(0xEA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC0);  // Power control
        LCD_WR_DATA(0x1B); // VRH[5:0]
        LCD_WR_REG(0xC1);  // Power control
        LCD_WR_DATA(0x00); // SAP[2:0];BT[3:0] 01
        LCD_WR_REG(0xC5);  // VCM control
        LCD_WR_DATA(0x30); // 3F
        LCD_WR_DATA(0x30); // 3C
        LCD_WR_REG(0xC7);  // VCM control2
        LCD_WR_DATA(0XB7);
        LCD_WR_REG(0x36); // Memory Access Control
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1A);
        LCD_WR_REG(0xB6); // Display Function Control
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0xA2);
        LCD_WR_REG(0xF2); // 3Gamma Function Disable
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x26); // Gamma curve selected
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xE0); // Set Gamma
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x2A);
        LCD_WR_DATA(0x28);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0XA9);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0XE1); // Set Gamma
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x15);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x2B);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x0F);
        LCD_WR_REG(0x2B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x3f);
        LCD_WR_REG(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xef);
        LCD_WR_REG(0x11); // Exit Sleep
        delay_ms(120);
        LCD_WR_REG(0x29); // display on
        lcddev.LCD_W = 240;
        lcddev.LCD_H = 320;
        break;
    }
    case 0x9486: {
        //*************ILI9486**********//
        LCD_WR_REG(0XF1);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0X0F);
        LCD_WR_DATA(0x8F);
        LCD_WR_REG(0XF2);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0xA3);
        LCD_WR_DATA(0x12);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0XB2);
        LCD_WR_DATA(0x12);
        LCD_WR_DATA(0xFF);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0XF8);
        LCD_WR_DATA(0x21);
        LCD_WR_DATA(0x04);
        LCD_WR_REG(0XF9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0x36);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0xB4);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC1);
        LCD_WR_DATA(0x47); // 0x41
        LCD_WR_REG(0xC5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xAF); // 0x91
        LCD_WR_DATA(0x80);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x1F);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x08);
        LCD_WR_DATA(0x48);
        LCD_WR_DATA(0x98);
        LCD_WR_DATA(0x37);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x13);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE1);
        LCD_WR_DATA(0x0F);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x2E);
        LCD_WR_DATA(0x0B);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x47);
        LCD_WR_DATA(0x75);
        LCD_WR_DATA(0x37);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0x11);
        LCD_WR_REG(0x36);
        LCD_WR_DATA(0x28);
        delay_ms(120);
        LCD_WR_REG(0x29);
        lcddev.LCD_W = 320;
        lcddev.LCD_H = 480;
        break;
    }
    case 0x9488: {
        //************* ILI9488**********//
        LCD_WR_REG(0XF7);
        LCD_WR_DATA(0xA9);
        LCD_WR_DATA(0x51);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x82);
        LCD_WR_REG(0xC0);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x09);
        LCD_WR_REG(0xC1);
        LCD_WR_DATA(0x41);
        LCD_WR_REG(0XC5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x80);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0xB0);
        LCD_WR_DATA(0x11);
        LCD_WR_REG(0xB4);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xB6);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x22);
        LCD_WR_REG(0xB7);
        LCD_WR_DATA(0xc6);
        LCD_WR_REG(0xBE);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_REG(0xE9);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x36);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x07);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x09);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x0B);
        LCD_WR_DATA(0x41);
        LCD_WR_DATA(0x89);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x1B);
        LCD_WR_DATA(0x0F);
        LCD_WR_REG(0XE1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x1A);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x0E);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x09);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x0F);
        LCD_WR_REG(0x11);
        delay_ms(120);
        LCD_WR_REG(0x29);
        lcddev.LCD_W = 320;
        lcddev.LCD_H = 480;
        break;
    }
    case 0x7796: {
        //************* ST7796S**********//
        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0xC3);
        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0x96);
        LCD_WR_REG(0x36);
        LCD_WR_DATA(0x68);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB0);
        LCD_WR_DATA(0x80);
        LCD_WR_REG(0xB6);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xB5);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x80);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xB4);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB7);
        LCD_WR_DATA(0xC6);
        LCD_WR_REG(0xC5);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xE4);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x40);
        LCD_WR_DATA(0x8A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x19);
        LCD_WR_DATA(0xA5);
        LCD_WR_DATA(0x33);
        LCD_WR_REG(0xC2);
        LCD_WR_REG(0xA7);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0xF0);
        LCD_WR_DATA(0x09);
        LCD_WR_DATA(0x13);
        LCD_WR_DATA(0x12);
        LCD_WR_DATA(0x12);
        LCD_WR_DATA(0x2B);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x1B);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x1D);
        LCD_WR_DATA(0x21);
        LCD_WR_REG(0XE1);
        LCD_WR_DATA(0xF0);
        LCD_WR_DATA(0x09);
        LCD_WR_DATA(0x13);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x27);
        LCD_WR_DATA(0x3B);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x4D);
        LCD_WR_DATA(0x0B);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x1D);
        LCD_WR_DATA(0x21);
        LCD_WR_REG(0X36);
        LCD_WR_DATA(0xEC);
        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0xC3);
        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0x69);
        LCD_WR_REG(0X13);
        LCD_WR_REG(0X11);
        LCD_WR_REG(0X29);
        lcddev.LCD_W = 320;
        lcddev.LCD_H = 480;
        break;
    }
    case 0x7793:
    case 0xB509: {
        //************* R61509V7793**********//
        LCD_WriteReg(0x0000, 0x0000);
        LCD_WriteReg(0x0000, 0x0000);
        LCD_WriteReg(0x0000, 0x0000);
        LCD_WriteReg(0x0000, 0x0000);
        delay_ms(15);
        LCD_WriteReg(0x0400, 0x6200);
        LCD_WriteReg(0x0008, 0x0808);
        // gamma
        LCD_WriteReg(0x0300, 0x0C00);
        LCD_WriteReg(0x0301, 0x5A0B);
        LCD_WriteReg(0x0302, 0x0906);
        LCD_WriteReg(0x0303, 0x1017);
        LCD_WriteReg(0x0304, 0x2300);
        LCD_WriteReg(0x0305, 0x1700);
        LCD_WriteReg(0x0306, 0x6309);
        LCD_WriteReg(0x0307, 0x0C09);
        LCD_WriteReg(0x0308, 0x100C);
        LCD_WriteReg(0x0309, 0x2232);
        LCD_WriteReg(0x0010, 0x0016); // 69.5Hz         0016
        LCD_WriteReg(0x0011, 0x0101);
        LCD_WriteReg(0x0012, 0x0000);
        LCD_WriteReg(0x0013, 0x0001);
        LCD_WriteReg(0x0100, 0x0330); // BT,AP
        LCD_WriteReg(0x0101, 0x0237); // DC0,DC1,VC
        LCD_WriteReg(0x0103, 0x0D00); // VDV
        LCD_WriteReg(0x0280, 0x6100); // VCM
        LCD_WriteReg(0x0102, 0xC1B0); // VRH,VCMR,PSON,PON
        delay_ms(50);
        LCD_WriteReg(0x0001, 0x0100);
        LCD_WriteReg(0x0002, 0x0100);
        LCD_WriteReg(0x0003, 0x1030); // 1030
        LCD_WriteReg(0x0009, 0x0001);
        LCD_WriteReg(0x000C, 0x0000);
        LCD_WriteReg(0x0090, 0x8000);
        LCD_WriteReg(0x000F, 0x0000);
        LCD_WriteReg(0x0210, 0x0000);
        LCD_WriteReg(0x0211, 0x00EF);
        LCD_WriteReg(0x0212, 0x0000);
        LCD_WriteReg(0x0213, 0x018F); // 432=01AF,400=018F
        LCD_WriteReg(0x0500, 0x0000);
        LCD_WriteReg(0x0501, 0x0000);
        LCD_WriteReg(0x0502, 0x005F);
        LCD_WriteReg(0x0401, 0x0001); // R401H Bit0
        LCD_WriteReg(0x0404, 0x0000);
        delay_ms(50);
        LCD_WriteReg(0x0007, 0x0100); // BASEE
        delay_ms(50);
        LCD_WriteReg(0x0200, 0x0000);
        LCD_WriteReg(0x0201, 0x0000);
        LCD_WR_REG(0x0202);
        lcddev.LCD_W = 240;
        lcddev.LCD_H = 400;
        break;
    }
    case 0x5310: {
        //************* NT35310**********//
        LCD_WR_REG(0xED);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0xFE);
        LCD_WR_REG(0xEE);
        LCD_WR_DATA(0xDE);
        LCD_WR_DATA(0x21);
        LCD_WR_REG(0xF1);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xDF);
        LCD_WR_DATA(0x10);
        // VCOMvoltage//
        LCD_WR_REG(0xC4);
        LCD_WR_DATA(0x8F); // 5f
        LCD_WR_REG(0xC6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE2);
        LCD_WR_DATA(0xE2);
        LCD_WR_DATA(0xE2);
        LCD_WR_REG(0xBF);
        LCD_WR_DATA(0xAA);
        LCD_WR_REG(0xB0);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x19);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x21);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB1);
        LCD_WR_DATA(0x80);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x96);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB4);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x96);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA1);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB5);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5E);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xAC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x70);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x90);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xEB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBA);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC1);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x54);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xFF);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC2);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x04);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC3);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x39);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x37);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x26);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x26);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x24);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC4);
        LCD_WR_DATA(0x62);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x18);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x95);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE6);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC5);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x65);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC6);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x17);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xC9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE0);
        LCD_WR_DATA(0x16);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x21);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x46);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x52);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x7A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE1);
        LCD_WR_DATA(0x16);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x22);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x36);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x52);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x64);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x7A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x8B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB9);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xE0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE2);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4F);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x61);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x79);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x97);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE3);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x0A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x1C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x33);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x62);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x78);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x97);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA6);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC7);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE4);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x2A);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x74);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x93);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBE);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE5);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x02);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x3C);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x4B);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x5D);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x74);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x84);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x93);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xA2);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xB3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBE);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xC4);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xCD);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xD3);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xDC);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE6);
        LCD_WR_DATA(0x11);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x34);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x43);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE7);
        LCD_WR_DATA(0x32);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x76);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x67);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x67);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x87);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x56);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x23);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x33);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x45);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE8);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x87);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x77);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x88);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xAA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0xBB);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x99);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x66);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x44);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x55);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xE9);
        LCD_WR_DATA(0xAA);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x00);
        LCD_WR_DATA(0xAA);
        LCD_WR_REG(0xCF);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF0);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x50);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF3);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF9);
        LCD_WR_DATA(0x06);
        LCD_WR_DATA(0x10);
        LCD_WR_DATA(0x29);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x3A);
        LCD_WR_DATA(0x55); // 66
        LCD_WR_REG(0x11);
        delay_ms(100);
        LCD_WR_REG(0x29);
        LCD_WR_REG(0x35);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x51);
        LCD_WR_DATA(0xFF);
        LCD_WR_REG(0x53);
        LCD_WR_DATA(0x2C);
        LCD_WR_REG(0x55);
        LCD_WR_DATA(0x82);
        LCD_WR_REG(0x2c);
        lcddev.LCD_W = 320;
        lcddev.LCD_H = 480;
        break;
    }
    case 0x5510: {
        //************* NT35510**********//
        LCD_WR_REG(0xF000);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xF001);
        LCD_WR_DATA(0xAA);
        LCD_WR_REG(0xF002);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xF003);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0xF004);
        LCD_WR_DATA(0x01);
        //# AVDD: manual); LCD_WR_DATA(
        LCD_WR_REG(0xB600);
        LCD_WR_DATA(0x34);
        LCD_WR_REG(0xB601);
        LCD_WR_DATA(0x34);
        LCD_WR_REG(0xB602);
        LCD_WR_DATA(0x34);
        LCD_WR_REG(0xB000);
        LCD_WR_DATA(0x0D); // 09
        LCD_WR_REG(0xB001);
        LCD_WR_DATA(0x0D);
        LCD_WR_REG(0xB002);
        LCD_WR_DATA(0x0D);
        //# AVEE: manual); LCD_WR_DATA( -6V
        LCD_WR_REG(0xB700);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB701);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB702);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB100);
        LCD_WR_DATA(0x0D);
        LCD_WR_REG(0xB101);
        LCD_WR_DATA(0x0D);
        LCD_WR_REG(0xB102);
        LCD_WR_DATA(0x0D);
        //#Power Control for
        // VCL
        LCD_WR_REG(0xB800);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB801);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB802);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB200);
        LCD_WR_DATA(0x00);
        //# VGH: Clamp Enable); LCD_WR_DATA(
        LCD_WR_REG(0xB900);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB901);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB902);
        LCD_WR_DATA(0x24);
        LCD_WR_REG(0xB300);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB301);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB302);
        LCD_WR_DATA(0x05);
        /// LCD_WR_REG(0xBF00); LCD_WR_DATA(0x01);
        //# VGL(LVGL):
        LCD_WR_REG(0xBA00);
        LCD_WR_DATA(0x34);
        LCD_WR_REG(0xBA01);
        LCD_WR_DATA(0x34);
        LCD_WR_REG(0xBA02);
        LCD_WR_DATA(0x34);
        //# VGL_REG(VGLO)
        LCD_WR_REG(0xB500);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xB501);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xB502);
        LCD_WR_DATA(0x0B);
        //# VGMP/VGSP:
        LCD_WR_REG(0xBC00);
        LCD_WR_DATA(0X00);
        LCD_WR_REG(0xBC01);
        LCD_WR_DATA(0xA3);
        LCD_WR_REG(0xBC02);
        LCD_WR_DATA(0X00);
        //# VGMN/VGSN
        LCD_WR_REG(0xBD00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBD01);
        LCD_WR_DATA(0xA3);
        LCD_WR_REG(0xBD02);
        LCD_WR_DATA(0x00);
        //# VCOM=-0.1
        LCD_WR_REG(0xBE00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBE01);
        LCD_WR_DATA(0x63); // 4f
        //  VCOMH+0x01;
        //#R+
        LCD_WR_REG(0xD100);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD101);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD102);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD103);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD104);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD105);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD106);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD107);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD108);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD109);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD10A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD10B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD10C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD10D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD10E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD10F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD110);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD111);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD112);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD113);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD114);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD115);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD116);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD117);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD118);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD119);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD11A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD11B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD11C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD11D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD11E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD11F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD120);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD121);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD122);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD123);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD124);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD125);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD126);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD127);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD128);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD129);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD12A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD12B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD12C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD12D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD12E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD12F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD130);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD131);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD132);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD133);
        LCD_WR_DATA(0xC1);
        //#G+
        LCD_WR_REG(0xD200);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD201);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD202);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD203);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD204);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD205);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD206);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD207);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD208);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD209);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD20A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD20B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD20C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD20D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD20E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD20F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD210);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD211);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD212);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD213);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD214);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD215);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD216);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD217);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD218);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD219);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD21A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD21B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD21C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD21D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD21E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD21F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD220);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD221);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD222);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD223);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD224);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD225);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD226);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD227);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD228);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD229);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD22A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD22B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD22C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD22D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD22E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD22F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD230);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD231);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD232);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD233);
        LCD_WR_DATA(0xC1);
        //#B+
        LCD_WR_REG(0xD300);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD301);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD302);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD303);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD304);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD305);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD306);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD307);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD308);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD309);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD30A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD30B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD30C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD30D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD30E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD30F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD310);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD311);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD312);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD313);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD314);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD315);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD316);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD317);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD318);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD319);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD31A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD31B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD31C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD31D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD31E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD31F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD320);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD321);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD322);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD323);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD324);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD325);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD326);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD327);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD328);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD329);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD32A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD32B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD32C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD32D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD32E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD32F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD330);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD331);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD332);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD333);
        LCD_WR_DATA(0xC1);
        //#R-///////////////////////////////////////////
        LCD_WR_REG(0xD400);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD401);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD402);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD403);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD404);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD405);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD406);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD407);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD408);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD409);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD40A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD40B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD40C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD40D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD40E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD40F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD410);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD411);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD412);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD413);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD414);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD415);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD416);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD417);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD418);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD419);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD41A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD41B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD41C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD41D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD41E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD41F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD420);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD421);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD422);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD423);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD424);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD425);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD426);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD427);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD428);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD429);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD42A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD42B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD42C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD42D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD42E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD42F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD430);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD431);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD432);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD433);
        LCD_WR_DATA(0xC1);
        //#G-//////////////////////////////////////////////
        LCD_WR_REG(0xD500);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD501);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD502);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD503);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD504);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD505);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD506);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD507);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD508);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD509);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD50A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD50B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD50C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD50D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD50E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD50F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD510);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD511);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD512);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD513);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD514);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD515);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD516);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD517);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD518);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD519);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD51A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD51B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD51C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD51D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD51E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD51F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD520);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD521);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD522);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD523);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD524);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD525);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD526);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD527);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD528);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD529);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD52A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD52B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD52C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD52D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD52E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD52F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD530);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD531);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD532);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD533);
        LCD_WR_DATA(0xC1);
        //#B-///////////////////////////////
        LCD_WR_REG(0xD600);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD601);
        LCD_WR_DATA(0x37);
        LCD_WR_REG(0xD602);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD603);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xD604);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD605);
        LCD_WR_DATA(0x7B);
        LCD_WR_REG(0xD606);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD607);
        LCD_WR_DATA(0x99);
        LCD_WR_REG(0xD608);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD609);
        LCD_WR_DATA(0xB1);
        LCD_WR_REG(0xD60A);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD60B);
        LCD_WR_DATA(0xD2);
        LCD_WR_REG(0xD60C);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xD60D);
        LCD_WR_DATA(0xF6);
        LCD_WR_REG(0xD60E);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD60F);
        LCD_WR_DATA(0x27);
        LCD_WR_REG(0xD610);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD611);
        LCD_WR_DATA(0x4E);
        LCD_WR_REG(0xD612);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD613);
        LCD_WR_DATA(0x8C);
        LCD_WR_REG(0xD614);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xD615);
        LCD_WR_DATA(0xBE);
        LCD_WR_REG(0xD616);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD617);
        LCD_WR_DATA(0x0B);
        LCD_WR_REG(0xD618);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD619);
        LCD_WR_DATA(0x48);
        LCD_WR_REG(0xD61A);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD61B);
        LCD_WR_DATA(0x4A);
        LCD_WR_REG(0xD61C);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD61D);
        LCD_WR_DATA(0x7E);
        LCD_WR_REG(0xD61E);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD61F);
        LCD_WR_DATA(0xBC);
        LCD_WR_REG(0xD620);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xD621);
        LCD_WR_DATA(0xE1);
        LCD_WR_REG(0xD622);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD623);
        LCD_WR_DATA(0x10);
        LCD_WR_REG(0xD624);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD625);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xD626);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD627);
        LCD_WR_DATA(0x5A);
        LCD_WR_REG(0xD628);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD629);
        LCD_WR_DATA(0x73);
        LCD_WR_REG(0xD62A);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD62B);
        LCD_WR_DATA(0x94);
        LCD_WR_REG(0xD62C);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD62D);
        LCD_WR_DATA(0x9F);
        LCD_WR_REG(0xD62E);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD62F);
        LCD_WR_DATA(0xB3);
        LCD_WR_REG(0xD630);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD631);
        LCD_WR_DATA(0xB9);
        LCD_WR_REG(0xD632);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xD633);
        LCD_WR_DATA(0xC1);
        //#Enable Page0
        LCD_WR_REG(0xF000);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xF001);
        LCD_WR_DATA(0xAA);
        LCD_WR_REG(0xF002);
        LCD_WR_DATA(0x52);
        LCD_WR_REG(0xF003);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0xF004);
        LCD_WR_DATA(0x00);
        //# RGB I/F Setting
        LCD_WR_REG(0xB000);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0xB001);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB002);
        LCD_WR_DATA(0x02);
        LCD_WR_REG(0xB003);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB004);
        LCD_WR_DATA(0x02);
        //## SDT:
        LCD_WR_REG(0xB600);
        LCD_WR_DATA(0x08);
        LCD_WR_REG(0xB500);
        LCD_WR_DATA(0x50); // 0x6b ???? 480x854       0x50 ???? 480x800
        //## Gate EQ:
        LCD_WR_REG(0xB700);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB701);
        LCD_WR_DATA(0x00);
        //## Source EQ:
        LCD_WR_REG(0xB800);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xB801);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB802);
        LCD_WR_DATA(0x05);
        LCD_WR_REG(0xB803);
        LCD_WR_DATA(0x05);
        //# Inversion: Column inversion (NVT)
        LCD_WR_REG(0xBC00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBC01);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBC02);
        LCD_WR_DATA(0x00);
        //# BOE's Setting(default)
        LCD_WR_REG(0xCC00);
        LCD_WR_DATA(0x03);
        LCD_WR_REG(0xCC01);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xCC02);
        LCD_WR_DATA(0x00);
        //# Display Timing:
        LCD_WR_REG(0xBD00);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xBD01);
        LCD_WR_DATA(0x84);
        LCD_WR_REG(0xBD02);
        LCD_WR_DATA(0x07);
        LCD_WR_REG(0xBD03);
        LCD_WR_DATA(0x31);
        LCD_WR_REG(0xBD04);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xBA00);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xFF00);
        LCD_WR_DATA(0xAA);
        LCD_WR_REG(0xFF01);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0xFF02);
        LCD_WR_DATA(0x25);
        LCD_WR_REG(0xFF03);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0x3500);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x3600);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0x3a00);
        LCD_WR_DATA(0x55);
        LCD_WR_REG(0x1100);
        delay_ms(120);
        LCD_WR_REG(0x2900);
        LCD_WR_REG(0x2c00);
        lcddev.LCD_W = 480;
        lcddev.LCD_H = 800;
        break;
    }
    case 0x1963: {
        //************* SSD1963**********//
        LCD_WR_REG(0xE2);
        // OSC*(N+1), VCO = 300MHz
        LCD_WR_DATA(0x1D);
        LCD_WR_DATA(0x02); // Divider M = 2, PLL = 300/(M+1) = 100MHz
        LCD_WR_DATA(0x04); // Validate M and N values
        delay_us(100);
        LCD_WR_REG(0xE0);  // Start PLL command
        LCD_WR_DATA(0x01); // enable PLL
        delay_ms(10);
        LCD_WR_REG(0xE0);  // Start PLL command again
        LCD_WR_DATA(0x03); // now, use PLL output as system clock
        delay_ms(12);
        LCD_WR_REG(0x01);
        delay_ms(10);
        LCD_WR_REG(0xE6); // 33Mhz
        LCD_WR_DATA(0x2F);
        LCD_WR_DATA(0xFF);
        LCD_WR_DATA(0xFF);
        LCD_WR_REG(0xB0);
        LCD_WR_DATA(0x20);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA((SSD_HOR_RESOLUTION - 1) >> 8);
        LCD_WR_DATA(SSD_HOR_RESOLUTION - 1);
        LCD_WR_DATA((SSD_VER_RESOLUTION - 1) >> 8);
        LCD_WR_DATA(SSD_VER_RESOLUTION - 1);
        LCD_WR_DATA(0x00); // RGB
        LCD_WR_REG(0xB4);  // Set horizontal period
        LCD_WR_DATA((SSD_HT - 1) >> 8);
        LCD_WR_DATA(SSD_HT - 1);
        LCD_WR_DATA(SSD_HPS >> 8);
        LCD_WR_DATA(SSD_HPS);
        LCD_WR_DATA(SSD_HOR_PULSE_WIDTH - 1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB6); // Set vertical period
        LCD_WR_DATA((SSD_VT - 1) >> 8);
        LCD_WR_DATA(SSD_VT - 1);
        LCD_WR_DATA(SSD_VPS >> 8);
        LCD_WR_DATA(SSD_VPS);
        LCD_WR_DATA(SSD_VER_FRONT_PORCH - 1);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xF0);  // SSD1963CPU16bit
        LCD_WR_DATA(0x03); // 16-bit(565 format) data for 16bpp
        LCD_WR_REG(0x29);
        LCD_WR_REG(0xD0);
        LCD_WR_DATA(0x00); // disable
        LCD_WR_REG(0xBE);
        LCD_WR_DATA(0x05);
        LCD_WR_DATA(0xFE);
        LCD_WR_DATA(0x01);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_DATA(0x00);
        LCD_WR_REG(0xB8);
        LCD_WR_DATA(0x03);
        LCD_WR_DATA(0x01);
        LCD_WR_REG(0xBA);
        LCD_WR_DATA(0X01); // GPIO[1:0]=01,
        LCD_PWM_BackLightSet(100);
        lcddev.LCD_W = 480;
        lcddev.LCD_H = 800;
        break;
    }
    default: {
        PRINT("unknow LCD ID\r\n");
        break;
    }
    }
    LCD_direction(USE_HORIZONTAL);
    LCD_LED = 1;
    LCD_Clear(WHITE);
}

/*****************************************************************************
 * @name       :void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
 * @date       :2018-08-09
 * @function   :Setting LCD display window
 * @parameters :xStar:the bebinning x coordinate of the LCD display window
                                yStar:the bebinning y coordinate of the LCD display window
                                xEnd:the endning x coordinate of the LCD display window
                                yEnd:the endning y coordinate of the LCD display window
 * @retvalue   :None
******************************************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar, u16 xEnd, u16 yEnd)
{
    u16 x = 0, y = 0, tmp = 0;
    if ((lcddev.id == 0xB509) || (lcddev.id == 0x7793))
    {
        switch (lcddev.dir)
        {
        case 0:
            x = xStar;
            y = yStar;
            break;
        case 1:
            tmp = yStar;
            yStar = xStar;
            xStar = lcddev.height - 1 - yEnd;
            yEnd = xEnd;
            xEnd = lcddev.height - 1 - tmp;
            x = xEnd;
            y = yStar;
            break;
        case 2:
            tmp = xStar;
            xStar = lcddev.width - 1 - xEnd;
            xEnd = lcddev.width - 1 - tmp;
            tmp = yStar;
            yStar = lcddev.height - 1 - yEnd;
            yEnd = lcddev.height - 1 - tmp;
            x = xEnd;
            y = yEnd;
            break;
        case 3:
            tmp = xStar;
            xStar = yStar;
            yStar = lcddev.width - 1 - xEnd;
            xEnd = yEnd;
            yEnd = lcddev.width - 1 - tmp;
            x = xStar;
            y = yEnd;
            break;
        default:
            break;
        }
        LCD_WriteReg(0x210, xStar);
        LCD_WriteReg(0x212, yStar);
        LCD_WriteReg(0x211, xEnd);
        LCD_WriteReg(0x213, yEnd);
        LCD_WriteReg(lcddev.setxcmd, x);
        LCD_WriteReg(lcddev.setycmd, y);
        LCD_WriteRAM_Prepare();
        return;
    }
    else if (lcddev.id == 0x5510)
    {
        LCD_WR_REG(lcddev.setxcmd);
        LCD_WR_DATA(xStar >> 8);
        LCD_WR_REG(lcddev.setxcmd + 1);
        LCD_WR_DATA(xStar & 0XFF);
        LCD_WR_REG(lcddev.setxcmd + 2);
        LCD_WR_DATA(xEnd >> 8);
        LCD_WR_REG(lcddev.setxcmd + 3);
        LCD_WR_DATA(xEnd & 0XFF);
        LCD_WR_REG(lcddev.setycmd);
        LCD_WR_DATA(yStar >> 8);
        LCD_WR_REG(lcddev.setycmd + 1);
        LCD_WR_DATA(yStar & 0XFF);
        LCD_WR_REG(lcddev.setycmd + 2);
        LCD_WR_DATA(yEnd >> 8);
        LCD_WR_REG(lcddev.setycmd + 3);
        LCD_WR_DATA(yEnd & 0XFF);
        LCD_WriteRAM_Prepare();
        return;
    }
    else if (lcddev.id == 0x1963)
    {
        switch (lcddev.dir)
        {
        case 0:
            xStar = xStar + xEnd;
            xEnd = xStar - xEnd;
            xStar = xStar - xEnd;
            xStar = lcddev.width - xStar - 1;
            xEnd = lcddev.width - xEnd - 1;
            break;
        case 2:
            yStar = yStar + yEnd;
            yEnd = yStar - yEnd;
            yStar = yStar - yEnd;
            yStar = lcddev.height - yStar - 1;
            yEnd = lcddev.height - yEnd - 1;
            break;
        case 3:
            xStar = xStar + xEnd;
            xEnd = xStar - xEnd;
            xStar = xStar - xEnd;
            xStar = lcddev.width - xStar - 1;
            xEnd = lcddev.width - xEnd - 1;
            yStar = yStar + yEnd;
            yEnd = yStar - yEnd;
            yStar = yStar - yEnd;
            yStar = lcddev.height - yStar - 1;
            yEnd = lcddev.height - yEnd - 1;
            break;
        default:
            break;
        }
    }
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(xStar >> 8);
    LCD_WR_DATA(0x00FF & xStar);
    LCD_WR_DATA(xEnd >> 8);
    LCD_WR_DATA(0x00FF & xEnd);

    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(yStar >> 8);
    LCD_WR_DATA(0x00FF & yStar);
    LCD_WR_DATA(yEnd >> 8);
    LCD_WR_DATA(0x00FF & yEnd);

    LCD_WriteRAM_Prepare();
}

/*****************************************************************************
 * @name       :void LCD_SetCursor(u16 Xpos, u16 Ypos)
 * @date       :2018-08-09
 * @function   :Set coordinate value
 * @parameters :Xpos:the  x coordinate of the pixel
                                Ypos:the  y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{
    LCD_SetWindows(Xpos, Ypos, Xpos, Ypos);
}

/*****************************************************************************
 * @name       :void LCD_direction(u8 direction)
 * @date       :2018-08-09
 * @function   :Setting the display direction of LCD screen
 * @parameters :direction:0-0 degree
                          1-90 degree
                                                    2-180 degree
                                                    3-270 degree
 * @retvalue   :None
******************************************************************************/
void LCD_direction(u8 direction)
{
    u16 dir_value[4] = {0};
    lcddev.dir = direction % 4;
    if (lcddev.dir % 2)
    {
        lcddev.width = lcddev.LCD_H;
        lcddev.height = lcddev.LCD_W;
    }
    else
    {
        lcddev.width = lcddev.LCD_W;
        lcddev.height = lcddev.LCD_H;
    }
    if ((lcddev.id == 0x9341) || (lcddev.id == 0x9486) || (lcddev.id == 0x9488) || (lcddev.id == 0x5310) ||
        (lcddev.id == 0x7796))
    {
        lcddev.setxcmd = 0x2A;
        lcddev.setycmd = 0x2B;
        lcddev.wramcmd = 0x2C;
        lcddev.rramcmd = 0x2E;
        lcddev.setdircmd = 0x36;
        if (lcddev.id == 0x9341)
        {
            dir_value[0] = (1 << 3);
            dir_value[1] = (1 << 3) | (1 << 5) | (1 << 6);
            dir_value[2] = (1 << 3) | (1 << 7) | (1 << 6);
            dir_value[3] = (1 << 3) | (1 << 7) | (1 << 5);
        }
        else if (lcddev.id == 0x9486)
        {
            dir_value[0] = (1 << 6) | (1 << 3);
            dir_value[1] = (1 << 3) | (1 << 4) | (1 << 5);
            dir_value[2] = (1 << 3) | (1 << 7);
            dir_value[3] = (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7);
        }
        else if (lcddev.id == 0x9488)
        {
            dir_value[0] = (1 << 3);
            dir_value[1] = (1 << 3) | (1 << 6) | (1 << 5);
            dir_value[2] = (1 << 3) | (1 << 6) | (1 << 7);
            dir_value[3] = (1 << 3) | (1 << 5) | (1 << 7);
        }
        else if (lcddev.id == 0x5310)
        {
            dir_value[0] = (0 << 3) | (0 << 7) | (0 << 6) | (0 << 5);
            dir_value[1] = (0 << 3) | (0 << 7) | (1 << 6) | (1 << 5);
            dir_value[2] = (0 << 3) | (1 << 7) | (1 << 6) | (0 << 5);
            dir_value[3] = (0 << 3) | (1 << 7) | (0 << 6) | (1 << 5);
        }
        else if (lcddev.id == 0x7796)
        {
            dir_value[0] = (1 << 3) | (1 << 6);
            dir_value[1] = (1 << 3) | (1 << 5);
            dir_value[2] = (1 << 3) | (1 << 7) | (1 << 4);
            dir_value[3] = (1 << 3) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4);
        }
    }
    else if (lcddev.id == 0x5510)
    {
        lcddev.setxcmd = 0x2A00;
        lcddev.setycmd = 0x2B00;
        lcddev.wramcmd = 0x2C00;
        lcddev.rramcmd = 0x2E00;
        lcddev.setdircmd = 0x3600;
        dir_value[0] = 0x00;
        dir_value[1] = (1 << 5) | (1 << 6);
        dir_value[2] = (1 << 7) | (1 << 6);
        dir_value[3] = (1 << 7) | (1 << 5);
    }
    else if ((lcddev.id == 0xB509) || (lcddev.id == 0x7793))
    {
        lcddev.setxcmd = 0x200;
        lcddev.setycmd = 0x201;
        lcddev.wramcmd = 0x202;
        lcddev.rramcmd = 0x202;
        lcddev.setdircmd = 0x0003;
        dir_value[0] = 0x1030;
        dir_value[1] = 0x1028;
        dir_value[2] = 0x1000;
        dir_value[3] = 0x1018;
    }
    else if (lcddev.id == 0x1963)
    {
        if (lcddev.dir % 2)
        {
            lcddev.setxcmd = 0x2A;
            lcddev.setycmd = 0x2B;
        }
        else
        {
            lcddev.setxcmd = 0x2B;
            lcddev.setycmd = 0x2A;
        }
        lcddev.wramcmd = 0x2C;
        lcddev.rramcmd = 0x2E;
        lcddev.setdircmd = 0x36;
        dir_value[0] = (0 << 3) | (1 << 7) | (0 << 6) | (1 << 5);
        dir_value[1] = (0 << 3) | (0 << 7) | (0 << 6) | (0 << 5);
        dir_value[2] = (0 << 3) | (0 << 7) | (1 << 6) | (1 << 5);
        dir_value[3] = (0 << 3) | (1 << 7) | (1 << 6) | (0 << 5);
    }
    switch (lcddev.dir)
    {
    case 0:
        LCD_WriteReg(lcddev.setdircmd, dir_value[0]);
        break;
    case 1:
        LCD_WriteReg(lcddev.setdircmd, dir_value[1]);
        break;
    case 2:
        LCD_WriteReg(lcddev.setdircmd, dir_value[2]);
        break;
    case 3:
        LCD_WriteReg(lcddev.setdircmd, dir_value[3]);
        break;
    default:
        break;
    }
}

/*****************************************************************************
 * @name       :u16 LCD_Read_ID(void)
 * @date       :2018-11-13
 * @function   :Read ID
 * @parameters :None
 * @retvalue   :ID value
 ******************************************************************************/
u16 LCD_Read_ID(void)
{
    u16 val[4];
    LCD_ReadReg(0x0000, val, 1);
    if ((val[0] == 0xB509) || (val[0] == 0x7793))
    {
        return val[0];
    }
    LCD_ReadReg(0xD3, val, 4);
    val[0] = (val[2] << 8) | val[3];
    if ((val[0] == 0x9341) || (val[0] == 0x9486) || (val[0] == 0x9488) || (val[0] == 0x7796))
    {
        return val[0];
    }
    LCD_ReadReg(0xD4, val, 4);
    val[0] = (val[2] << 8) | val[3];
    if (val[0] == 0x5310)
    {
        return 0x5310;
    }
    LCD_ReadReg(0xA1, val, 3);
    val[0] = (val[1] << 8) | val[2];
    if (val[0] == 0x5761)
    {
        return 0x1963;
    }
    LCD_WR_REG(0xF000);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xF001);
    LCD_WR_DATA(0xAA);
    LCD_WR_REG(0xF002);
    LCD_WR_DATA(0x52);
    LCD_WR_REG(0xF003);
    LCD_WR_DATA(0x08);
    LCD_WR_REG(0xF004);
    LCD_WR_DATA(0x01);

    LCD_ReadReg(0xC500, &val[1], 1);
    LCD_ReadReg(0xC501, &val[2], 1);
    val[0] = (val[1] << 8) | val[2];
    if (val[0] == 0x5510)
    {
        return 0x5510;
    }
    return 0;
}
