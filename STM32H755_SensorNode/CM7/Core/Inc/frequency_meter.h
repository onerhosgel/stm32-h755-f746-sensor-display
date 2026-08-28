#ifndef FREQUENCY_METER_H
#define FREQUENCY_METER_H

#include "stm32h7xx_hal.h"

void FrequencyMeter_Init(TIM_HandleTypeDef *htim);
float FrequencyMeter_GetHz(void);
uint8_t FrequencyMeter_IsValid(void);

#endif
