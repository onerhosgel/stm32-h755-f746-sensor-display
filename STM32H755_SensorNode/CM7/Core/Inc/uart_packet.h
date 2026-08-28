#ifndef UART_PACKET_H
#define UART_PACKET_H

#include "stm32h7xx_hal.h"
#include "sensor_manager.h"

HAL_StatusTypeDef UARTPacket_Send(UART_HandleTypeDef *huart,
                                  const SensorData_t *data);

uint8_t UARTPacket_IsBusy(void);

#endif
