#ifndef __TEST_H__
#define __TEST_H__
#include "24c02.h"
#include "adc.h"
#include "ctpiic.h"
#include "dac.h"
#include "delay.h"
#include "dma.h"
#include "exti.h"
#include "flash.h"
#include "stm32f4xx.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
// #include "font.h"
#include "ft5426.h"
#include "gt911.h"
#include "gui.h"
#include "iic.h"
#include "iwdg.h"
#include "key.h"
#include "lcd.h"
#include "led.h"
// #include "pic.h"
#include "pwm.h"
#include "random.h"
#include "rtc.h"
#include "rtp.h"
#include "spi.h"
#include "timer.h"
#include "touch.h"
#include "usart.h"
#include "w25q64.h"
#include "wkup.h"

#include "aht10.h"
#include "as5600.h"
#include "bh1750.h"
#include "custom_malloc.h"
#include "exfuns.h"
#include "ff.h"
#include "fontupd.h"
#include "piclib.h"
#include "sdio_sdcard.h"
#include "tcs34725.h"
#include "text.h"
#include "ws_AT45DBXX.h"

#include "usb_vcp.h"

void Ctp_Test(void);
void W25Q64_Test(void);
void AT24C02_Test(void);
void WS_AT45DBXX_Test(void);
void AS5600_Test(void);
void AHT10_Test(void);
void BH1750_Test(void);
void TCS34725_Test(void);
void Check_SDCard(void);
void Check_Font(void);
void Show_Picture(void);

#endif
