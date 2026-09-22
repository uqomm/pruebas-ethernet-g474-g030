/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Gpio.hpp"
#include "GpioHandler.hpp"
#include "Memory.hpp"
#include "UartHandler.hpp"
#include "CommandMessage.hpp"
#include "AdcHandler.hpp"

#include "w5500.h"
#include "socket.h"
#include "dhcp.h"
#include "W5500Handler.hpp"
#include "vector"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
uint32_t CURR_BAT = 0;
uint32_t CURR_UP = 0;
uint32_t CURR_DOWN = 0;
uint32_t VOLT_BAT = 0;

uint16_t VOLT_BAT_TX = 0;
uint16_t CURR_BAT_TX = 0;
uint16_t CURR_UP_TX = 0;
uint16_t CURR_DOWN_TX = 0;
uint8_t state = 0;

std::vector<uint8_t> data_vector;
uint8_t data[9] = { 0 };

bool AC_ACTIV;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void save_data_in_array(uint16_t valor1, uint16_t valor2, uint16_t valor3,
		uint16_t valor4, bool valor5, uint8_t *array) {

	array[0] = (valor1 >> 8) & 0xFF; // Byte más significativo
	array[1] = valor1 & 0xFF;       // Byte menos significativo
	array[2] = (valor2 >> 8) & 0xFF; // Byte más significativo
	array[3] = valor2 & 0xFF;		// Byte menos significativo
	array[4] = (valor3 >> 8) & 0xFF; // Byte más significativo
	array[5] = valor3 & 0xFF;		// Byte menos significativo
	array[6] = (valor4 >> 8) & 0xFF; // Byte más significativo
	array[7] = valor4 & 0xFF;		// Byte menos significativo
	array[8] = (valor5); //
}

void W5500_Init() {
    // Configuración de la dirección MAC
    uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56};
    setSHAR(mac);
    // Configuración de la dirección IP (puede ser 0.0.0.0 para DHCP)
    uint8_t ip[4] = {0, 0, 0, 0};
    setSIPR(ip);
}

void DHCP_Init() {
    // Inicialización del cliente DHCP
    DHCP_init(0, NULL);
}

void DHCP_Process() {
    uint8_t dhcp_state;
    do {
        dhcp_state = DHCP_run();
        // Espera a que el DHCP asigne una dirección IP
    } while (dhcp_state == DHCP_RUNNING);

    if (dhcp_state == DHCP_IP_ASSIGN) {
        // La dirección IP ha sido asignada
        uint8_t ip[4];
        getSIPR(ip);
        printf("IP asignada: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    } else {
        // Error en la asignación de IP
        printf("Error en la asignación de IP\n");
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_ADC1_Init();
	MX_USART1_UART_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();

	/* USER CODE BEGIN 2 */

    // Inicialización del W5500
    W5500_Init();

    // Inicialización de DHCP
    DHCP_Init();

    // Proceso de DHCP
    DHCP_Process();



	Gpio ac_activ = Gpio(AC_ACTIV_GPIO_Port, AC_ACTIV_Pin);
	Gpio alive_led = Gpio(ALIVE_LED_GPIO_Port, ALIVE_LED_Pin);
	Gpio rs485_ctrl = Gpio(CTRL_485_GPIO_Port, CTRL_485_Pin);

	Gpio rst_w5500 = Gpio(NRST_W5500_GPIO_Port, NRST_W5500_Pin);
	Gpio nss_w5500 = Gpio(NSS_W5500_GPIO_Port, NSS_W5500_Pin);

	GpioHandler gpio_handler = GpioHandler();
	UartHandler uart = UartHandler(&huart1);
	AdcHandler adc = AdcHandler(&hadc1);

	W5500Handler w5500 = W5500Handler(&hspi1, rst_w5500, nss_w5500);

	uint8_t server_ip[4] = { 192, 168, 60, 73 }; // IP del servidor
	uint16_t server_port = 5000; 				//Puerto del servidor
	//	uint16_t source_port = 3000;			//Puerto de origen (para modo tcp server)

	w5500.start_tcp_client(SOCKET0, server_port, server_ip);

	CommandMessage cmd = CommandMessage(
			static_cast<uint8_t>(MODULE_FUNCTION::DR_FUENTE), 0x00);

	uint32_t tiempo_anterior = 0;
	uint32_t intervalo = 5000;					//intervalo tiempo para envio de data

	/* USER CODE END 2 */

	/* Infinite loop */
	while (1) {

		uint32_t tiempo_actual = HAL_GetTick();
		if (tiempo_actual - tiempo_anterior >= intervalo) {
			tiempo_anterior = tiempo_actual;

			CURR_BAT = adc.read(ADC_CHANNEL_0);
			CURR_UP = adc.read(ADC_CHANNEL_1);
			CURR_DOWN = adc.read(ADC_CHANNEL_3);
			VOLT_BAT = adc.read(ADC_CHANNEL_4);
			AC_ACTIV = gpio_handler.state(ac_activ);

			VOLT_BAT_TX = adc.maping(VOLT_BAT, 427, 2080, 5, 26.58);
			CURR_BAT_TX = adc.maping(CURR_BAT, 3160, 2653, 0, 1);
			CURR_UP_TX = adc.maping(CURR_UP, 3160, 2653, 0, 1);
			CURR_DOWN_TX = adc.maping(CURR_DOWN, 3160, 2653, 0, 1);

			save_data_in_array(VOLT_BAT_TX, CURR_BAT_TX, CURR_UP_TX,
					CURR_DOWN_TX, AC_ACTIV, data);

			for (int i = 0; i < 9; i++)
				data_vector.push_back(data[i]);

			cmd.composeMessage(&data_vector);
			data_vector.clear();
			data_vector = cmd.get_composed_message();

			state = uart.transmitMessage485Ctrl(data_vector.data(),
					data_vector.size(), rs485_ctrl);

			w5500.tcp_client_send(data_vector.data(), data_vector.size(),
			SOCKET0);

			data_vector.clear();
			memset(data, 0, sizeof(data));
//			w5500.TCP_server(SOCKET0_REGISTER_BLOCK, source_port);

		}
		gpio_handler.blink(alive_led, 50, 1000);

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */




void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00300617;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, NSS_W5500_Pin|CTRL_485_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, NRST_W5500_Pin|ALIVE_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : AC_ACTIV_Pin PA9 */
  GPIO_InitStruct.Pin = AC_ACTIV_Pin|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : NSS_W5500_Pin CTRL_485_Pin */
  GPIO_InitStruct.Pin = NSS_W5500_Pin|CTRL_485_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : NRST_W5500_Pin ALIVE_LED_Pin */
  GPIO_InitStruct.Pin = NRST_W5500_Pin|ALIVE_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
