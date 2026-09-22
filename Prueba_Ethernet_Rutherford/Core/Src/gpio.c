/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, NSS_LORA1_Pin|RST_LORA1_Pin|RFIN_CTB1_Pin|RFIN_CTC1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, CS_BLE_Pin|EN_BLE_Pin|BOOT_BLE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OUT_MOSFET1_Pin|OUT_MOSFET2_Pin|NSS_LORA2_Pin|RST_LORA2_Pin
                          |NRST_W5500_Pin|RFOU_CTA1_Pin|RFOU_CTB1_Pin|RFOU_CTBb2_Pin
                          |RFOU_CTCb2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SCN_W5500_Pin|RFOU_CTA2_Pin|RFOU_CTB2_Pin|RFOU_CTC2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, V2C_Pin|V1C_Pin|RFIN_CTA1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, RFOU_CTC1_Pin|RFOU_CTAb1_Pin|RFOU_CTBb1_Pin|RFOU_CTCb1_Pin
                          |RFOU_CTAb2_Pin|RFIN_CTA3_Pin|RFIN_CTB3_Pin|RFIN_CTC3_Pin
                          |RFIN_CTA2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, RFIN_CTB2_Pin|RFIN_CTC2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : NSS_LORA1_Pin RST_LORA1_Pin RFIN_CTB1_Pin RFIN_CTC1_Pin */
  GPIO_InitStruct.Pin = NSS_LORA1_Pin|RST_LORA1_Pin|RFIN_CTB1_Pin|RFIN_CTC1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_BLE_Pin EN_BLE_Pin BOOT_BLE_Pin */
  GPIO_InitStruct.Pin = CS_BLE_Pin|EN_BLE_Pin|BOOT_BLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : RDY_BLE_Pin */
  GPIO_InitStruct.Pin = RDY_BLE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RDY_BLE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_MOSFET1_Pin OUT_MOSFET2_Pin NSS_LORA2_Pin RST_LORA2_Pin
                           NRST_W5500_Pin RFOU_CTA1_Pin RFOU_CTB1_Pin RFOU_CTBb2_Pin
                           RFOU_CTCb2_Pin */
  GPIO_InitStruct.Pin = OUT_MOSFET1_Pin|OUT_MOSFET2_Pin|NSS_LORA2_Pin|RST_LORA2_Pin
                          |NRST_W5500_Pin|RFOU_CTA1_Pin|RFOU_CTB1_Pin|RFOU_CTBb2_Pin
                          |RFOU_CTCb2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : SCN_W5500_Pin RFOU_CTA2_Pin RFOU_CTB2_Pin RFOU_CTC2_Pin */
  GPIO_InitStruct.Pin = SCN_W5500_Pin|RFOU_CTA2_Pin|RFOU_CTB2_Pin|RFOU_CTC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : INTN_W5500_Pin */
  GPIO_InitStruct.Pin = INTN_W5500_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(INTN_W5500_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : V2C_Pin V1C_Pin RFIN_CTA1_Pin */
  GPIO_InitStruct.Pin = V2C_Pin|V1C_Pin|RFIN_CTA1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : RFOU_CTC1_Pin RFOU_CTAb1_Pin RFOU_CTBb1_Pin RFOU_CTCb1_Pin
                           RFOU_CTAb2_Pin RFIN_CTA3_Pin RFIN_CTB3_Pin RFIN_CTC3_Pin
                           RFIN_CTA2_Pin */
  GPIO_InitStruct.Pin = RFOU_CTC1_Pin|RFOU_CTAb1_Pin|RFOU_CTBb1_Pin|RFOU_CTCb1_Pin
                          |RFOU_CTAb2_Pin|RFIN_CTA3_Pin|RFIN_CTB3_Pin|RFIN_CTC3_Pin
                          |RFIN_CTA2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : RFIN_CTB2_Pin RFIN_CTC2_Pin */
  GPIO_InitStruct.Pin = RFIN_CTB2_Pin|RFIN_CTC2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
