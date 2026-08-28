#ifndef BMP180_H
#define BMP180_H

#include "stm32h7xx_hal.h"

#define BMP180_I2C_ADDR    (0x77U << 1)

HAL_StatusTypeDef BMP180_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef BMP180_ReadPressure(I2C_HandleTypeDef *hi2c,
                                      float *pressure_hPa);

#endif
