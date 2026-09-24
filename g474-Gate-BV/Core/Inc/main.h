/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define SCK_LORA_Pin GPIO_PIN_2
#define SCK_LORA_GPIO_Port GPIOE
#define NSS_LORA1_Pin GPIO_PIN_3
#define NSS_LORA1_GPIO_Port GPIOE
#define RST_LORA1_Pin GPIO_PIN_4
#define RST_LORA1_GPIO_Port GPIOE
#define MISO_LORA_Pin GPIO_PIN_5
#define MISO_LORA_GPIO_Port GPIOE
#define MOSI_LORA_Pin GPIO_PIN_6
#define MOSI_LORA_GPIO_Port GPIOE
#define CS_BLE_Pin GPIO_PIN_5
#define CS_BLE_GPIO_Port GPIOF
#define RDY_BLE_Pin GPIO_PIN_7
#define RDY_BLE_GPIO_Port GPIOF
#define EN_BLE_Pin GPIO_PIN_8
#define EN_BLE_GPIO_Port GPIOF
#define SCK_BLE_Pin GPIO_PIN_9
#define SCK_BLE_GPIO_Port GPIOF
#define BOOT_BLE_Pin GPIO_PIN_10
#define BOOT_BLE_GPIO_Port GPIOF
#define OUT_MOSFET1_Pin GPIO_PIN_0
#define OUT_MOSFET1_GPIO_Port GPIOC
#define OUT_MOSFET2_Pin GPIO_PIN_1
#define OUT_MOSFET2_GPIO_Port GPIOC
#define NSS_LORA2_Pin GPIO_PIN_2
#define NSS_LORA2_GPIO_Port GPIOC
#define RST_LORA2_Pin GPIO_PIN_3
#define RST_LORA2_GPIO_Port GPIOC
#define TX_UART_AUX_Pin GPIO_PIN_2
#define TX_UART_AUX_GPIO_Port GPIOA
#define RX_UART_AUX_Pin GPIO_PIN_3
#define RX_UART_AUX_GPIO_Port GPIOA
#define SCN_W5500_Pin GPIO_PIN_4
#define SCN_W5500_GPIO_Port GPIOA
#define SCLK_W5500_Pin GPIO_PIN_3
#define SCLK_W5500_GPIO_Port GPIOB
#define MISO_W5500_Pin GPIO_PIN_4
#define MISO_W5500_GPIO_Port GPIOB
#define MOSI_W5500_Pin GPIO_PIN_5
#define MOSI_W5500_GPIO_Port GPIOB
#define INTN_W5500_Pin GPIO_PIN_4
#define INTN_W5500_GPIO_Port GPIOC
#define NRST_W5500_Pin GPIO_PIN_6
#define NRST_W5500_GPIO_Port GPIOB
#define V2C_Pin GPIO_PIN_1
#define V2C_GPIO_Port GPIOB
#define V1C_Pin GPIO_PIN_2
#define V1C_GPIO_Port GPIOB
#define DC_meas1_Pin GPIO_PIN_7
#define DC_meas1_GPIO_Port GPIOE
#define DC_meas2_Pin GPIO_PIN_8
#define DC_meas2_GPIO_Port GPIOE
#define DC_meas3_Pin GPIO_PIN_9
#define DC_meas3_GPIO_Port GPIOE
#define DC_meas4_Pin GPIO_PIN_10
#define DC_meas4_GPIO_Port GPIOE
#define DC_meas5_Pin GPIO_PIN_11
#define DC_meas5_GPIO_Port GPIOE
#define DC_meas6_Pin GPIO_PIN_12
#define DC_meas6_GPIO_Port GPIOE
#define DC_meas7_Pin GPIO_PIN_13
#define DC_meas7_GPIO_Port GPIOE
#define DC_meas8_Pin GPIO_PIN_14
#define DC_meas8_GPIO_Port GPIOE
#define MISO_BLE_Pin GPIO_PIN_14
#define MISO_BLE_GPIO_Port GPIOB
#define MOSI_BLE_Pin GPIO_PIN_15
#define MOSI_BLE_GPIO_Port GPIOB
#define RFOU_CTA1_Pin GPIO_PIN_6
#define RFOU_CTA1_GPIO_Port GPIOC
#define RFOU_CTB1_Pin GPIO_PIN_7
#define RFOU_CTB1_GPIO_Port GPIOC
#define RFOU_CTC1_Pin GPIO_PIN_0
#define RFOU_CTC1_GPIO_Port GPIOG
#define RFOU_CTAb1_Pin GPIO_PIN_1
#define RFOU_CTAb1_GPIO_Port GPIOG
#define RFOU_CTBb1_Pin GPIO_PIN_2
#define RFOU_CTBb1_GPIO_Port GPIOG
#define RFOU_CTCb1_Pin GPIO_PIN_3
#define RFOU_CTCb1_GPIO_Port GPIOG
#define RFOU_CTAb2_Pin GPIO_PIN_4
#define RFOU_CTAb2_GPIO_Port GPIOG
#define RFOU_CTBb2_Pin GPIO_PIN_8
#define RFOU_CTBb2_GPIO_Port GPIOC
#define RFOU_CTCb2_Pin GPIO_PIN_9
#define RFOU_CTCb2_GPIO_Port GPIOC
#define RFOU_CTA2_Pin GPIO_PIN_8
#define RFOU_CTA2_GPIO_Port GPIOA
#define RFOU_CTB2_Pin GPIO_PIN_9
#define RFOU_CTB2_GPIO_Port GPIOA
#define RFOU_CTC2_Pin GPIO_PIN_10
#define RFOU_CTC2_GPIO_Port GPIOA
#define SCL_EP_Pin GPIO_PIN_15
#define SCL_EP_GPIO_Port GPIOA
#define TX_232_Pin GPIO_PIN_12
#define TX_232_GPIO_Port GPIOC
#define RFIN_CTA3_Pin GPIO_PIN_6
#define RFIN_CTA3_GPIO_Port GPIOG
#define RFIN_CTB3_Pin GPIO_PIN_7
#define RFIN_CTB3_GPIO_Port GPIOG
#define RFIN_CTC3_Pin GPIO_PIN_8
#define RFIN_CTC3_GPIO_Port GPIOG
#define RFIN_CTA2_Pin GPIO_PIN_9
#define RFIN_CTA2_GPIO_Port GPIOG
#define RFIN_CTB2_Pin GPIO_PIN_0
#define RFIN_CTB2_GPIO_Port GPIOD
#define RFIN_CTC2_Pin GPIO_PIN_1
#define RFIN_CTC2_GPIO_Port GPIOD
#define RX_232_Pin GPIO_PIN_2
#define RX_232_GPIO_Port GPIOD
#define SDA_EP_Pin GPIO_PIN_7
#define SDA_EP_GPIO_Port GPIOB
#define RFIN_CTA1_Pin GPIO_PIN_9
#define RFIN_CTA1_GPIO_Port GPIOB
#define RFIN_CTB1_Pin GPIO_PIN_0
#define RFIN_CTB1_GPIO_Port GPIOE
#define RFIN_CTC1_Pin GPIO_PIN_1
#define RFIN_CTC1_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
