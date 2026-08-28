#include "tcs34725.h"

#define TCS34725_CMD             0x80U

#define TCS34725_REG_ENABLE      0x00U
#define TCS34725_REG_ATIME       0x01U
#define TCS34725_REG_CONTROL     0x0FU
#define TCS34725_REG_ID          0x12U
#define TCS34725_REG_STATUS      0x13U

#define TCS34725_ENABLE_PON      0x01U
#define TCS34725_ENABLE_AEN      0x02U


static HAL_StatusTypeDef TCS34725_ReadByte(I2C_HandleTypeDef *hi2c,
                                           uint8_t reg,
                                           uint8_t *data)
{
    return HAL_I2C_Mem_Read(hi2c,
                            TCS34725_I2C_ADDR,
                            TCS34725_CMD | reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            1,
                            20);
}


HAL_StatusTypeDef TCS34725_Init(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;

    uint8_t value;
    uint8_t id;

    status = HAL_I2C_IsDeviceReady(hi2c,
                                   TCS34725_I2C_ADDR,
                                   3,
                                   100);

    if (status != HAL_OK)
    {
        return status;
    }

    status = TCS34725_ReadByte(hi2c,
                               TCS34725_REG_ID,
                               &id);

    if (status != HAL_OK)
    {
        return status;
    }

    value = 0xF6U;

    status = HAL_I2C_Mem_Write(hi2c,
                               TCS34725_I2C_ADDR,
                               TCS34725_CMD | TCS34725_REG_ATIME,
                               I2C_MEMADD_SIZE_8BIT,
                               &value,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    value = 0x01U;

    status = HAL_I2C_Mem_Write(hi2c,
                               TCS34725_I2C_ADDR,
                               TCS34725_CMD | TCS34725_REG_CONTROL,
                               I2C_MEMADD_SIZE_8BIT,
                               &value,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    value = TCS34725_ENABLE_PON;

    status = HAL_I2C_Mem_Write(hi2c,
                               TCS34725_I2C_ADDR,
                               TCS34725_CMD | TCS34725_REG_ENABLE,
                               I2C_MEMADD_SIZE_8BIT,
                               &value,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(3);

    value = TCS34725_ENABLE_PON |
            TCS34725_ENABLE_AEN;

    status = HAL_I2C_Mem_Write(hi2c,
                               TCS34725_I2C_ADDR,
                               TCS34725_CMD | TCS34725_REG_ENABLE,
                               I2C_MEMADD_SIZE_8BIT,
                               &value,
                               1,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    HAL_Delay(30);

    return HAL_OK;
}


HAL_StatusTypeDef TCS34725_Read(I2C_HandleTypeDef *hi2c,
                                uint16_t *red,
                                uint16_t *green,
                                uint16_t *blue,
                                uint16_t *clear)
{
    uint8_t statusReg;

    uint8_t cLow;
    uint8_t cHigh;

    uint8_t rLow;
    uint8_t rHigh;

    uint8_t gLow;
    uint8_t gHigh;

    uint8_t bLow;
    uint8_t bHigh;

    uint32_t timeout = 0;

    do
    {
        if (TCS34725_ReadByte(hi2c,
                              TCS34725_REG_STATUS,
                              &statusReg) != HAL_OK)
        {
            return HAL_ERROR;
        }

        if ((statusReg & 0x01U) != 0U)
        {
            break;
        }

        HAL_Delay(5);

        timeout++;

    } while (timeout < 20U);

    if ((statusReg & 0x01U) == 0U)
    {
        return HAL_TIMEOUT;
    }

    if (TCS34725_ReadByte(hi2c, 0x14U, &cLow) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x15U, &cHigh) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x16U, &rLow) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x17U, &rHigh) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x18U, &gLow) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x19U, &gHigh) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x1AU, &bLow) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(2);

    if (TCS34725_ReadByte(hi2c, 0x1BU, &bHigh) != HAL_OK)
        return HAL_ERROR;

    *clear = ((uint16_t)cHigh << 8) |
             (uint16_t)cLow;

    *red = ((uint16_t)rHigh << 8) |
           (uint16_t)rLow;

    *green = ((uint16_t)gHigh << 8) |
             (uint16_t)gLow;

    *blue = ((uint16_t)bHigh << 8) |
            (uint16_t)bLow;

    return HAL_OK;
}
