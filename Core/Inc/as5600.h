#ifndef __AS5600_H
#define __AS5600_H
/* PB10 SCL PB11 SDA */
#include "stdbool.h"
#include "stm32f4xx.h"
#include "sys.h"

#define AS_IN()                                                                                                        \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 0 << 11 * 2;                                                                                   \
    }
#define AS_OUT()                                                                                                       \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 1 << (11 * 2);                                                                                 \
    }

#define AS_SCL_LOW (GPIOB->ODR &= ~(GPIO_Pin_10))
#define AS_SCL_HIGH (GPIOB->ODR |= (GPIO_Pin_10))
#define AS_SDA_LOW (GPIOB->ODR &= ~(GPIO_Pin_11))
#define AS_SDA_HIGH (GPIOB->ODR |= (GPIO_Pin_11))

#define AS_SDA_STATE (GPIOB->IDR &= (GPIO_Pin_11))

#define AS_READY 0x00
#define AS_BUS_BUSY 0x01
#define AS_BUS_ERROR 0x02

#define AS_NACK 0x00
#define AS_ACK 0x01

#define AS_DIR_TRANS 0x00
#define AS_DIR_RECV 0x01

#define AS_ADDRESS 0x36

typedef enum
{
    _ams5600_Address = 0x36,
    _zmco = 0x00,
    _zpos_hi = 0x01,
    _zpos_lo = 0x02,
    _mpos_hi = 0x03,
    _mpos_lo = 0x04,
    _mang_hi = 0x05,
    _mang_lo = 0x06,
    _conf_hi = 0x07,
    _conf_lo = 0x08,
    _raw_ang_hi = 0x0c,
    _raw_ang_lo = 0x0d,
    _ang_hi = 0x0e,
    _ang_lo = 0x0f,
    _stat = 0x0b,
    _agc = 0x1a,
    _mag_hi = 0x1b,
    _mag_lo = 0x1c,
    _burn = 0xff
} AMS5600Registers_t;

void AS_Init(void);
int16_t get_max_angle(void);
int16_t get_start_pos(void);
int16_t gen_end_pos(void);
int16_t get_scaled_angle(void);
int16_t get_agc();
int16_t get_magnitude();
int16_t get_burn_count();
int16_t get_raw_angle(void);
int16_t setEndPosition(int16_t endAngle);
int16_t setStartPosition(int16_t startAngle);
int16_t set_max_angle(int16_t newMaxAngle);
bool detect_magnet(void);
u8 get_magnet_strength(void);
u8 burn_angle();
u8 burn_max_angle_and_config();
int16_t get_angle(void);


#endif
