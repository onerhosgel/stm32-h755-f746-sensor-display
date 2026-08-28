#include "frequency_meter.h"

static TIM_HandleTypeDef *freqTimer = NULL;

static volatile uint32_t firstCapture = 0U;
static volatile uint32_t secondCapture = 0U;
static volatile uint32_t captureDifference = 0U;

static volatile uint8_t firstCaptured = 0U;
static volatile uint8_t frequencyValid = 0U;

static volatile float frequencyHz = 0.0f;


void FrequencyMeter_Init(TIM_HandleTypeDef *htim)
{
    freqTimer = htim;

    firstCapture = 0U;
    secondCapture = 0U;
    captureDifference = 0U;

    firstCaptured = 0U;
    frequencyValid = 0U;
    frequencyHz = 0.0f;

    HAL_TIM_IC_Start_IT(freqTimer, TIM_CHANNEL_1);
}


float FrequencyMeter_GetHz(void)
{
    return frequencyHz;
}


uint8_t FrequencyMeter_IsValid(void)
{
    return frequencyValid;
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ((freqTimer == NULL) ||
        (htim->Instance != freqTimer->Instance))
    {
        return;
    }

    if (htim->Channel != HAL_TIM_ACTIVE_CHANNEL_1)
    {
        return;
    }

    if (firstCaptured == 0U)
    {
        firstCapture =
            HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        firstCaptured = 1U;
    }
    else
    {
        secondCapture =
            HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

        if (secondCapture >= firstCapture)
        {
            captureDifference =
                secondCapture - firstCapture;
        }
        else
        {
            captureDifference =
                (0xFFFFFFFFU - firstCapture) +
                secondCapture +
                1U;
        }

        if (captureDifference > 0U)
        {
            /*
             * TIM2 counter 1 MHz:
             * 1 tick = 1 us
             */
            frequencyHz =
                1000000.0f /
                (float)captureDifference;

            frequencyValid = 1U;
        }

        firstCapture = secondCapture;
    }
}
