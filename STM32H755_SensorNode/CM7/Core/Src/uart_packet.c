#include "uart_packet.h"

#include <stdio.h>
#include <stdint.h>

static uint8_t txBuffer[128] __attribute__((aligned(32)));

static volatile uint8_t txBusy = 0U;
static UART_HandleTypeDef *activeUart = NULL;


static int32_t ToTenths(float value)
{
    if (value >= 0.0f)
    {
        return (int32_t)((value * 10.0f) + 0.5f);
    }

    return (int32_t)((value * 10.0f) - 0.5f);
}


static void FormatTenths(char *buffer,
                         uint32_t bufferSize,
                         float value)
{
    int32_t scaled;
    uint32_t absoluteValue;

    scaled = ToTenths(value);

    if (scaled < 0)
    {
        absoluteValue = (uint32_t)(-scaled);

        snprintf(buffer,
                 bufferSize,
                 "-%lu.%lu",
                 (unsigned long)(absoluteValue / 10U),
                 (unsigned long)(absoluteValue % 10U));
    }
    else
    {
        absoluteValue = (uint32_t)scaled;

        snprintf(buffer,
                 bufferSize,
                 "%lu.%lu",
                 (unsigned long)(absoluteValue / 10U),
                 (unsigned long)(absoluteValue % 10U));
    }
}


HAL_StatusTypeDef UARTPacket_Send(UART_HandleTypeDef *huart,
                                  const SensorData_t *data)
{
    char temperatureText[16];
    char humidityText[16];
    char pressureText[16];

    int length;
    HAL_StatusTypeDef status;

    if ((huart == NULL) || (data == NULL))
    {
        return HAL_ERROR;
    }

    if (txBusy != 0U)
    {
        return HAL_BUSY;
    }

    FormatTenths(temperatureText,
                 sizeof(temperatureText),
                 data->temperature);

    FormatTenths(humidityText,
                 sizeof(humidityText),
                 data->humidity);

    FormatTenths(pressureText,
                 sizeof(pressureText),
                 data->pressure);

    length = snprintf((char *)txBuffer,
                      sizeof(txBuffer),
                      "$%s,%s,%s,%u,%u,%u,%u*\r\n",
                      temperatureText,
                      humidityText,
                      pressureText,
                      (unsigned int)data->red,
                      (unsigned int)data->green,
                      (unsigned int)data->blue,
                      (unsigned int)data->clear);

    if ((length <= 0) ||
        (length >= (int)sizeof(txBuffer)))
    {
        return HAL_ERROR;
    }

    /*
     * Cortex-M7 data cache kullaniliyorsa DMA'nin
     * guncel buffer'i gorebilmesi icin cache temizlenir.
     */
#if (__DCACHE_PRESENT == 1U)
if ((SCB->CCR & SCB_CCR_DC_Msk) != 0U)
{
    SCB_CleanDCache_by_Addr((uint32_t *)txBuffer,
                            sizeof(txBuffer));
}
#endif

    txBusy = 1U;
    activeUart = huart;

    status = HAL_UART_Transmit_DMA(huart,
                                   txBuffer,
                                   (uint16_t)length);

    if (status != HAL_OK)
    {
        txBusy = 0U;
        activeUart = NULL;
    }

    return status;
}


uint8_t UARTPacket_IsBusy(void)
{
    return txBusy;
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == activeUart)
    {
        txBusy = 0U;
        activeUart = NULL;
    }
}


void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == activeUart)
    {
        txBusy = 0U;
        activeUart = NULL;
    }
}
