#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "stm32h7xx_hal.h"
#include <stdint.h>

typedef struct
{
    float temperature;
    float humidity;
    float pressure;

    uint16_t red;
    uint16_t green;
    uint16_t blue;
    uint16_t clear;

    uint8_t temperatureHumidityOk;
    uint8_t pressureOk;
    uint8_t colorOk;

} SensorData_t;

void SensorManager_Init(I2C_HandleTypeDef *hi2c);

void SensorManager_ReadAll(I2C_HandleTypeDef *hi2c,
                           SensorData_t *data);

#endif
