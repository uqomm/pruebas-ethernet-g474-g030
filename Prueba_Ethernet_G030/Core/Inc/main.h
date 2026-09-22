/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   STM32G030K8T6 + W5500 (Ethernet) - Proyecto nuevo
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
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
/* W5500: SPI1 (PA5=SCK, PA6=MISO, PA7=MOSI), SCN=PB0, NRST=PA8 */
#define SCN_W5500_Pin GPIO_PIN_0
#define SCN_W5500_GPIO_Port GPIOB
#define NRST_W5500_Pin GPIO_PIN_8
#define NRST_W5500_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
