#include "th09c.h"

HAL_StatusTypeDef TH09C_Read(I2C_HandleTypeDef *hi2c,
                             float *temperature,
                             float *humidity)
{
    uint8_t command = 0x03U;
    uint8_t data[6];

    uint16_t rawTemperature;
    uint16_t rawHumidity;

    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Write(hi2c,
                               TH09C_I2C_ADDR,
                               0x22U,
                               I2C_MEMADD_SIZE_8BIT,
                               &command,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(150);

    status = HAL_I2C_Mem_Read(hi2c,
                              TH09C_I2C_ADDR,
                              0x30U,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              6,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    if (((data[2] & 0x01U) == 0U) ||
        ((data[5] & 0x01U) == 0U))
    {
        return HAL_ERROR;
    }

    rawTemperature = ((uint16_t)data[1] << 8) |
                     (uint16_t)data[0];

    rawHumidity = ((uint16_t)data[4] << 8) |
                  (uint16_t)data[3];

    *temperature = ((float)rawTemperature / 64.0f) - 273.15f;
    *humidity    = (float)rawHumidity / 512.0f;

    return HAL_OK;
}
