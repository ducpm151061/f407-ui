#ifndef __GT911_DRIVER_H
#define __GT911_DRIVER_H

#include "sys.h"

#define RST_OUT()                                                                                                      \
    {                                                                                                                  \
        GPIOC->MODER &= ~(3 << (13 * 2));                                                                              \
        GPIOC->MODER |= 1 << 13 * 2;                                                                                   \
    } // set RSSET pin to output
#define INT_OUT()                                                                                                      \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (1 * 2));                                                                               \
        GPIOB->MODER |= 1 << 1 * 2;                                                                                    \
    } // set RSSET pin to output
#define INT_IN()                                                                                                       \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (1 * 2));                                                                               \
        GPIOB->MODER |= 0 << 1 * 2;                                                                                    \
    } // set RSSET pin to output

// RST--PC13
// INT--PB1
#define RST_CTRL PCout(13) // GT911 RESET pin out high or low
#define INT_CTRL PBout(1)  // GT911 INT pin out high or low
#define INT_GET PBin(1)    // Get GT911 INT pin status

#define GT9XX_IIC_RADDR 0xBB // IIC read address
#define GT9XX_IIC_WADDR 0xBA // IIC write address

#define GT9XX_READ_ADDR 0x814E // touch point information
#define GT9XX_ID_ADDR 0x8140   // ID of touch IC

void GT911_int_sync(u16 ms);
void GT911_reset_guitar(u8 addr);
void GT911_gpio_init(void);
u8 GT9XX_WriteHandle(u16 addr);
u8 GT9XX_WriteData(u16 addr, u8 value);
u8 GT9XX_ReadData(u16 addr, u8 cnt, u8 *value);
u8 GT911_Init(void);
u8 Touch_Get_Count(void);
u8 GT911_Scan(void);
void GT911_Position(int16_t *Touch_X_Position, int16_t *Touch_Y_Position);

#endif
