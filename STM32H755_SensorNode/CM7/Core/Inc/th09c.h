#ifndef TH09C_H
#define TH09C_H

#include "stm32h7xx_hal.h"

#define TH09C_I2C_ADDR    (0x43U << 1)

HAL_StatusTypeDef TH09C_Read(I2C_HandleTypeDef *hi2c,
                             float *temperature,
                             float *humidity);

#endif
