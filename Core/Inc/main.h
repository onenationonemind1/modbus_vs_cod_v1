/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define __USE_MODBUS_PARSER_            1
#define __USE_MODE_RS232_               0
#define __USE_MODE_RS485_               1
#define __USE_UART_4_DMA_               1

#define INIT_DATA_1                     0x2
#define INIT_DATA_2                     0x3

#define VERSION_MAJOR                   1
#define VERSION_MINOR                   0
#define VERSION_REVISION                2

#define RS485_ID                        1
#define RS485_FUNCTION_CODE             3

#define PERIOD_SEC_01                   1
#define PERIOD_SEC_02                   2
#define PERIOD_SEC_05                   5
#define PERIOD_SEC_10                   10
#define PERIOD_MIN_01                   60
#define PERIOD_MIN_02                   120
#define PERIOD_MIN_05                   300
#define PERIOD_MIN_60                   3600

#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sensor.h"
#include "sensor_am1002.h"
#include "function.h"
#include "timer.h"
#include "modbus.h"
#include "parser.h"
#include "flash.h"
#include "uart.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
