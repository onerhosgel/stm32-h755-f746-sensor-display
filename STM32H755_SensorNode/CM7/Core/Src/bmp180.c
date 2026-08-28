#include "bmp180.h"

#define BMP180_REG_CALIB       0xAAU
#define BMP180_REG_CHIP_ID     0xD0U
#define BMP180_REG_CONTROL     0xF4U
#define BMP180_REG_DATA        0xF6U

#define BMP180_CMD_TEMP        0x2EU
#define BMP180_OSS             0U

typedef struct
{
    int16_t  AC1;
    int16_t  AC2;
    int16_t  AC3;
    uint16_t AC4;
    uint16_t AC5;
    uint16_t AC6;
    int16_t  B1;
    int16_t  B2;
    int16_t  MB;
    int16_t  MC;
    int16_t  MD;
} BMP180_Calib_t;

static BMP180_Calib_t calib;

static uint16_t U16_BE(const uint8_t *data)
{
    return ((uint16_t)data[0] << 8) |
           (uint16_t)data[1];
}

static int16_t S16_BE(const uint8_t *data)
{
    return (int16_t)U16_BE(data);
}

HAL_StatusTypeDef BMP180_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t chipId;
    uint8_t data[22];

    HAL_StatusTypeDef status;

    status = HAL_I2C_IsDeviceReady(hi2c,
                                   BMP180_I2C_ADDR,
                                   3,
                                   100);

    if (status != HAL_OK)
    {
        return status;
    }

    status = HAL_I2C_Mem_Read(hi2c,
                              BMP180_I2C_ADDR,
                              BMP180_REG_CHIP_ID,
                              I2C_MEMADD_SIZE_8BIT,
                              &chipId,
                              1,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    if (chipId != 0x55U)
    {
        return HAL_ERROR;
    }

    status = HAL_I2C_Mem_Read(hi2c,
                              BMP180_I2C_ADDR,
                              BMP180_REG_CALIB,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              sizeof(data),
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    calib.AC1 = S16_BE(&data[0]);
    calib.AC2 = S16_BE(&data[2]);
    calib.AC3 = S16_BE(&data[4]);
    calib.AC4 = U16_BE(&data[6]);
    calib.AC5 = U16_BE(&data[8]);
    calib.AC6 = U16_BE(&data[10]);

    calib.B1 = S16_BE(&data[12]);
    calib.B2 = S16_BE(&data[14]);
    calib.MB = S16_BE(&data[16]);
    calib.MC = S16_BE(&data[18]);
    calib.MD = S16_BE(&data[20]);

    return HAL_OK;
}

HAL_StatusTypeDef BMP180_ReadPressure(I2C_HandleTypeDef *hi2c,
                                      float *pressure_hPa)
{
    uint8_t command;
    uint8_t data[3];

    HAL_StatusTypeDef status;

    int32_t UT;
    int32_t UP;

    int32_t X1;
    int32_t X2;
    int32_t X3;

    int32_t B3;
    int32_t B5;
    int32_t B6;

    uint32_t B4;
    uint32_t B7;

    int32_t pressure;

    command = BMP180_CMD_TEMP;

    status = HAL_I2C_Mem_Write(hi2c,
                               BMP180_I2C_ADDR,
                               BMP180_REG_CONTROL,
                               I2C_MEMADD_SIZE_8BIT,
                               &command,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(5);

    status = HAL_I2C_Mem_Read(hi2c,
                              BMP180_I2C_ADDR,
                              BMP180_REG_DATA,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              2,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    UT = ((int32_t)data[0] << 8) |
         (int32_t)data[1];

    X1 = ((UT - (int32_t)calib.AC6) *
          (int32_t)calib.AC5) >> 15;

    if ((X1 + calib.MD) == 0)
    {
        return HAL_ERROR;
    }

    X2 = ((int32_t)calib.MC << 11) /
         (X1 + calib.MD);

    B5 = X1 + X2;

    command = (uint8_t)(0x34U + (BMP180_OSS << 6));

    status = HAL_I2C_Mem_Write(hi2c,
                               BMP180_I2C_ADDR,
                               BMP180_REG_CONTROL,
                               I2C_MEMADD_SIZE_8BIT,
                               &command,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(5);

    status = HAL_I2C_Mem_Read(hi2c,
                              BMP180_I2C_ADDR,
                              BMP180_REG_DATA,
                              I2C_MEMADD_SIZE_8BIT,
                              data,
                              3,
                              100);

    if (status != HAL_OK)
    {
        return status;
    }

    UP = ((((int32_t)data[0] << 16) |
           ((int32_t)data[1] << 8) |
           (int32_t)data[2]) >>
          (8 - BMP180_OSS));

    B6 = B5 - 4000;

    X1 = ((int32_t)calib.B2 *
          ((B6 * B6) >> 12)) >> 11;

    X2 = ((int32_t)calib.AC2 *
          B6) >> 11;

    X3 = X1 + X2;

    B3 = (((((int32_t)calib.AC1 * 4 + X3)
            << BMP180_OSS) + 2) >> 2);

    X1 = ((int32_t)calib.AC3 *
          B6) >> 13;

    X2 = ((int32_t)calib.B1 *
          ((B6 * B6) >> 12)) >> 16;

    X3 = (X1 + X2 + 2) >> 2;

    B4 = ((uint32_t)calib.AC4 *
          (uint32_t)(X3 + 32768)) >> 15;

    if (B4 == 0U)
    {
        return HAL_ERROR;
    }

    B7 = ((uint32_t)(UP - B3)) *
         (50000U >> BMP180_OSS);

    if (B7 < 0x80000000U)
    {
        pressure = (int32_t)((B7 << 1) / B4);
    }
    else
    {
        pressure = (int32_t)((B7 / B4) << 1);
    }

    X1 = (pressure >> 8) *
         (pressure >> 8);

    X1 = (X1 * 3038) >> 16;

    X2 = (-7357 * pressure) >> 16;

    pressure = pressure +
               ((X1 + X2 + 3791) >> 4);

    *pressure_hPa =
        (float)pressure / 100.0f;

    return HAL_OK;
}
