#include "sensor_manager.h"

#include "th09c.h"
#include "bmp180.h"
#include "tcs34725.h"

static uint8_t bmp180Ready = 0U;
static uint8_t tcs34725Ready = 0U;

void SensorManager_Init(I2C_HandleTypeDef *hi2c)
{
    if (BMP180_Init(hi2c) == HAL_OK)
    {
        bmp180Ready = 1U;
    }

    if (TCS34725_Init(hi2c) == HAL_OK)
    {
        tcs34725Ready = 1U;
    }
}

void SensorManager_ReadAll(I2C_HandleTypeDef *hi2c,
                           SensorData_t *data)
{
    data->temperatureHumidityOk = 0U;
    data->pressureOk = 0U;
    data->colorOk = 0U;


    if (TH09C_Read(hi2c,
                   &data->temperature,
                   &data->humidity) == HAL_OK)
    {
        data->temperatureHumidityOk = 1U;
    }
    else
    {
        data->temperature = 0.0f;
        data->humidity = 0.0f;
    }


    if (bmp180Ready == 0U)
    {
        if (BMP180_Init(hi2c) == HAL_OK)
        {
            bmp180Ready = 1U;
        }
    }

    if (bmp180Ready != 0U)
    {
        if (BMP180_ReadPressure(hi2c,
                                &data->pressure) == HAL_OK)
        {
            data->pressureOk = 1U;
        }
        else
        {
            data->pressure = 0.0f;
        }
    }
    else
    {
        data->pressure = 0.0f;
    }


    if (tcs34725Ready == 0U)
    {
        if (TCS34725_Init(hi2c) == HAL_OK)
        {
            tcs34725Ready = 1U;
        }
    }

    if (tcs34725Ready != 0U)
    {
        if (TCS34725_Read(hi2c,
                          &data->red,
                          &data->green,
                          &data->blue,
                          &data->clear) == HAL_OK)
        {
            data->colorOk = 1U;
        }
        else
        {
            data->red = 0U;
            data->green = 0U;
            data->blue = 0U;
            data->clear = 0U;
        }
    }
    else
    {
        data->red = 0U;
        data->green = 0U;
        data->blue = 0U;
        data->clear = 0U;
    }
}
