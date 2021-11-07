#ifndef __BH1750_H
#define __BH1750_H

#include "delay.h"

#define BH_SDA_IN()                                                                                                    \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 0 << 11 * 2;                                                                                   \
    }
#define BH_SDA_OUT()                                                                                                   \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 1 << 11 * 2;                                                                                   \
    }

#define BH_IIC_SCL PBout(10) // SCL
#define BH_IIC_SDA PBout(11) // SDA
#define BH_SDA_READ PBin(11)

// Address of BH1750
#define BH1750_Addr (0x23 << 1)

// BH1750 command code
#define POWER_OFF 0x00
#define POWER_ON 0x01
#define MODULE_RESET 0x07
#define CONTINUE_H_MODE 0x10
#define CONTINUE_H_MODE2 0x11
#define CONTINUE_L_MODE 0x13
#define ONE_TIME_H_MODE 0x20
#define ONE_TIME_H_MODE2 0x21
#define ONE_TIME_L_MODE 0x23

// Measurement mode
#define Measure_Mode CONTINUE_H_MODE

// Resolution light intensity (unit: lx) = (high byte + low byte) / 1.2 * measurement accuracy
#if ((Measure_Mode == CONTINUE_H_MODE2) | (Measure_Mode == ONE_TIME_H_MODE2))
#define Resolution 0.5
#elif ((Measure_Mode == CONTINUE_H_MODE) | (Measure_Mode == ONE_TIME_H_MODE))
#define Resolution 1
#elif ((Measure_Mode == CONTINUE_L_MODE) | (Measure_Mode == ONE_TIME_L_MODE))
#define Resolution 4
#endif

void BH1750_Init(void);
uint16_t BH1750_Read_Measure(void);
float Light_Intensity(void); // Reading the value of light intensity

#endif /* __BH1750_H */