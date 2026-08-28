/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : H755 Sensor UART Transmitter
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "sensor_manager.h"
#include <stdio.h>
#include <stdint.h>
/* USER CODE END Includes */

/* USER CODE BEGIN PD */

#define DUAL_CORE_BOOT_SYNC_SEQUENCE

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U)
#endif

#endif

/* USER CODE END PD */

/* USER CODE BEGIN PV */

static SensorData_t sensorData;

static char uartTxBuffer[128];

volatile uint32_t uartTxOkCount = 0U;
volatile uint32_t uartTxErrorCount = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

static int32_t FloatToX10(float value);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/*
 * Float degeri 10 ile carpip en yakin tam sayiya yuvarlar.
 *
 * 25.44  -> 254
 * 52.18  -> 522
 * 1003.0 -> 10030
 */
static int32_t FloatToX10(float value)
{
    float scaled = value * 10.0f;

    if (scaled >= 0.0f)
    {
        scaled += 0.5f;
    }
    else
    {
        scaled -= 0.5f;
    }

    return (int32_t)scaled;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN Boot_Mode_Sequence_0 */

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)

    int32_t timeout;

#endif

    /* USER CODE END Boot_Mode_Sequence_0 */


    /* USER CODE BEGIN Boot_Mode_Sequence_1 */

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)

    timeout = 0xFFFF;

    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) &&
           (timeout-- > 0))
    {
    }

    if (timeout < 0)
    {
        Error_Handler();
    }

#endif

    /* USER CODE END Boot_Mode_Sequence_1 */


    /* MCU Configuration--------------------------------------------------------*/

    HAL_Init();

    SystemClock_Config();


    /* USER CODE BEGIN Boot_Mode_Sequence_2 */

#if defined(DUAL_CORE_BOOT_SYNC_SEQUENCE)

    __HAL_RCC_HSEM_CLK_ENABLE();

    HAL_HSEM_FastTake(HSEM_ID_0);

    HAL_HSEM_Release(HSEM_ID_0, 0);

    timeout = 0xFFFF;

    while ((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) &&
           (timeout-- > 0))
    {
    }

    if (timeout < 0)
    {
        Error_Handler();
    }

#endif

    /* USER CODE END Boot_Mode_Sequence_2 */


    /* Initialize all configured peripherals */

    MX_GPIO_Init();

    MX_DMA_Init();

    MX_I2C1_Init();

    MX_TIM2_Init();

    MX_TIM4_Init();

    /*
     * USART1
     * TX = PB6
     * Baud = 115200
     */
    MX_USART1_UART_Init();


    /* USER CODE BEGIN 2 */

    HAL_Delay(500);

    /*
     * TH09C + BMP180 + TCS34725
     * daha once calisan SensorManager
     */
    SensorManager_Init(&hi2c1);

    HAL_Delay(500);

    /* USER CODE END 2 */


    /* Infinite loop */
    /* USER CODE BEGIN WHILE */

    while (1)
    {
        int32_t temperature10;
        int32_t humidity10;
        int32_t pressure10;

        int length;

        /*
         * Tum sensorleri oku.
         *
         * sensorData:
         * temperature
         * humidity
         * pressure
         * red
         * green
         * blue
         * clear
         */
        SensorManager_ReadAll(&hi2c1, &sensorData);


        temperature10 = FloatToX10(sensorData.temperature);
        humidity10    = FloatToX10(sensorData.humidity);
        pressure10    = FloatToX10(sensorData.pressure);


        /*
         * UART paket formati:
         *
         * $TEMP,HUM,PRESS,R,G,B,CLEAR*\r\n
         *
         * Ornek:
         *
         * $254,522,10030,31,41,32,101*
         */
        length = snprintf(
            uartTxBuffer,
            sizeof(uartTxBuffer),
            "$%ld,%ld,%ld,%u,%u,%u,%u*\r\n",

            (long)temperature10,
            (long)humidity10,
            (long)pressure10,

            (unsigned int)sensorData.red,
            (unsigned int)sensorData.green,
            (unsigned int)sensorData.blue,
            (unsigned int)sensorData.clear
        );


        /*
         * Paketi H755 PB6 / USART1_TX uzerinden F746'ya gonder.
         */
        if ((length > 0) &&
            (length < (int)sizeof(uartTxBuffer)))
        {
            HAL_StatusTypeDef status;

            status = HAL_UART_Transmit(
                &huart1,
                (uint8_t *)uartTxBuffer,
                (uint16_t)length,
                100U
            );

            if (status == HAL_OK)
            {
                uartTxOkCount++;
            }
            else
            {
                uartTxErrorCount++;
            }
        }
        else
        {
            uartTxErrorCount++;
        }


        /*
         * 500 ms'de bir yeni sensor paketi
         */
        HAL_Delay(500);
    }

    /* USER CODE END WHILE */
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
    {
    }


    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_DIV1;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;


    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2 |
        RCC_CLOCKTYPE_D3PCLK1 |
        RCC_CLOCKTYPE_D1PCLK1;


    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.SYSCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB3CLKDivider =
        RCC_APB3_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_APB1_DIV2;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_APB2_DIV2;

    RCC_ClkInitStruct.APB4CLKDivider =
        RCC_APB4_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_1) != HAL_OK)
    {
        Error_Handler();
    }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}


#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
}

#endif /* USE_FULL_ASSERT */
