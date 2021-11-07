#ifndef __AHT10_H
#define __AHT10_H
#include "delay.h"
/* PB10 SCL PB11 SDA */

#define AHTX0_I2CADDR_DEFAULT (0x38 << 1) ///< AHT default i2c address
#define AHTX0_CMD_CALIBRATE 0xE1          ///< Calibration command
#define AHTX0_CMD_TRIGGER 0xAC            ///< Trigger reading command
#define AHTX0_CMD_SOFTRESET 0xBA          ///< Soft reset command
#define AHTX0_STATUS_BUSY 0x80            ///< Status bit for busy
#define AHTX0_STATUS_CALIBRATED 0x08      ///< Status bit for calibrated

#define AHT_SDA_IN()                                                                                                   \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 0 << 11 * 2;                                                                                   \
    }
#define AHT_SDA_OUT()                                                                                                  \
    {                                                                                                                  \
        GPIOB->MODER &= ~(3 << (11 * 2));                                                                              \
        GPIOB->MODER |= 1 << 11 * 2;                                                                                   \
    }

#define AHT_IIC_SCL PBout(10) // SCL
#define AHT_IIC_SDA PBout(11) // SDA
#define AHT_SDA_READ PBin(11)

void AHT_Init(void);
void AHT_Measure(void);
void ATH_Read(int32_t *temperature, int32_t *humidity);
void AHT_Reset(void);

#endif
