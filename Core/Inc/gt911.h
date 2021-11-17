#ifndef __GT911_H
#define __GT911_H

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
// Product ID (LSB 4 bytes, GT9110: 0x06 0x00 0x00 0x09)
#define GT9XX_ID_ADDR 0x8140 // ID of touch IC

#define GT9XX_REG_COMMAND 0x8040
#define GT9XX_REG_ESD_CHECK 0x8041
#define GT9XX_REG_PROXIMITY_EN 0x8042

// The version number of the configuration file
#define GT9XX_REG_CONFIG_DATA 0x8047
// X output maximum value (LSB 2 bytes)
#define GT9XX_REG_MAX_X 0x8048
// Y output maximum value (LSB 2 bytes)
#define GT9XX_REG_MAX_Y 0x804A
// Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
#define GT9XX_REG_MAX_TOUCH 0x804C

// Module switch 1
// 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode */
#define GT9XX_REG_MOD_SW1 0x804D
// Module switch 2
// 7:1 Reserved, 0 Touch key */
#define GT9XX_REG_MOD_SW2 0x804E

// Number of debuffs fingers press/release
#define GT9XX_REG_SHAKE_CNT 0x804F

// X threshold
#define GT9XX_REG_X_THRESHOLD 0x8057

// Configuration update fresh
#define GT9XX_REG_CONFIG_FRESH 0x8100

// ReadOnly registers (device and coordinates info)
// Firmware version (LSB 2 bytes)
#define GT9XX_REG_FW_VER 0x8144

// Current output X resolution (LSB 2 bytes)
#define GT9XX_READ_X_RES 0x8146
// Current output Y resolution (LSB 2 bytes)
#define GT9XX_READ_Y_RES 0x8148
// Module vendor ID
#define GT9XX_READ_VENDOR_ID 0x814A

#define GT9XX_POINT1_X_ADDR 0x8150
#define GT9XX_POINT1_Y_ADDR 0x8152

/* Commands for REG_COMMAND */
// 0: read coordinate state
#define GT9XX_CMD_READ 0x00
// 1: difference value original value
#define GT9XX_CMD_DIFFVAL 0x01
// 2: software reset
#define GT9XX_CMD_SOFTRESET 0x02
// 3: Baseline update
#define GT9XX_CMD_BASEUPDATE 0x03
// 4: Benchmark calibration
#define GT9XX_CMD_CALIBRATE 0x04
// 5: Off screen (send other invalid)
#define GT9XX_CMD_SCREEN_OFF 0x05

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

#endif /* __GT911_H */
