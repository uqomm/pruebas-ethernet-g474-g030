/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>   /* strlen() para los mensajes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* W5500 Network Configuration */
#define W5500_SOCKET        0

/* MAC Address: 02:00:00:00:00:01 */
#define W5500_MAC0          0x02
#define W5500_MAC1          0x00
#define W5500_MAC2          0x00
#define W5500_MAC3          0x00
#define W5500_MAC4          0x00
#define W5500_MAC5          0x01

/* Static IP: 192.168.1.50 */
#define W5500_IP0           192
#define W5500_IP1           168
#define W5500_IP2           1
#define W5500_IP3           50

/* Subnet Mask: 255.255.255.0 */
#define W5500_SN0           255
#define W5500_SN1           255
#define W5500_SN2           255
#define W5500_SN3           0

/* Gateway: 192.168.1.1 */
#define W5500_GW0           192
#define W5500_GW1           168
#define W5500_GW2           1
#define W5500_GW3           1

/* PC / Destination IP: 192.168.1.10 */
#define PC_IP0              192
#define PC_IP1              168
#define PC_IP2              1
#define PC_IP3              10

#define UDP_PORT            5000
#define TX_INTERVAL_MS      10000   /* envio de "KA" cada 10 segundos */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Variables de estado (se leen por SWD con STM32_Programmer para depurar) */
volatile uint8_t  g_ver      = 0;   /* VERSIONR: debe ser 0x04 */
volatile uint8_t  g_phy      = 0;   /* PHYCFGR: link / velocidad / duplex */
volatile uint8_t  g_socket   = 0;   /* estado del socket (0x22 = UDP) */
volatile uint8_t  g_snmr     = 0;   /* protocolo del socket (0x02 = UDP) */
volatile uint8_t  g_phy_live = 0;   /* PHYCFGR leido en el bucle */
volatile uint16_t g_rsr      = 0;   /* bytes recibidos pendientes */
volatile uint16_t g_tx_count = 0;   /* mensajes enviados */
volatile uint16_t g_rx_count = 0;   /* mensajes recibidos */

/* Resultados del escaneo de pines (diagnostico) */
volatile uint8_t  g_scan_found = 0;      /* 1 si encontro el W5500 */
volatile uint8_t  g_scan_port   = 0xFF;  /* 0=GPIOA, 1=GPIOB, 2=GPIOC */
volatile uint8_t  g_scan_pin    = 0xFF;  /* numero de pin (0..15) */
volatile uint8_t  g_scan_ver    = 0;     /* VERSIONR leido en el ultimo intento */
volatile uint8_t  g_scan_nrst_found = 0; /* 1 si encontro el NRST */
volatile uint8_t  g_scan_nrst_port   = 0xFF;
volatile uint8_t  g_scan_nrst_pin    = 0xFF;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ------------------------------------------------------------------
 * W5500: bloques de registros
 * ------------------------------------------------------------------ */
#define COMMON_BLOCK   0x00      /* registros comunes (MAC, IP, PHY...) */
#define SOCK0_BLOCK    0x01      /* registros del socket 0 */
#define SOCK0_TXBUF    0x02      /* buffer de transmision del socket 0 */
#define SOCK0_RXBUF    0x03      /* buffer de recepcion del socket 0 */

/* Registros comunes (bloque 0) */
#define MR             0x0000    /* modo */
#define SHAR           0x0009    /* direccion MAC (6 bytes) */
#define GAR            0x0001    /* gateway (4 bytes) */
#define SUBR           0x0005    /* mascara de subred (4 bytes) */
#define SIPR           0x000F    /* IP local (4 bytes) */
#define PHYCFGR        0x002E    /* configuracion del PHY */
#define VERSIONR       0x0039    /* version del chip (debe leer 0x04) */

/* Registros del socket 0 */
#define Sn_MR          0x0000    /* protocolo (0x02 = UDP) */
#define Sn_CR          0x0001    /* comando */
#define Sn_SR          0x0003    /* estado */
#define Sn_PORT        0x0004    /* puerto local (2 bytes) */
#define Sn_DIPR        0x000C    /* IP destino (4 bytes) */
#define Sn_DPORT       0x0010    /* puerto destino (2 bytes) */
#define Sn_RXBUF_SIZE  0x001E    /* tamanio del buffer RX (en KB) */
#define Sn_TXBUF_SIZE  0x001F    /* tamanio del buffer TX (en KB) */
#define Sn_TX_FSR      0x0020    /* espacio libre en TX */
#define Sn_TX_WR       0x0024    /* puntero de escritura de TX */
#define Sn_RX_RSR      0x0026    /* bytes recibidos pendientes */
#define Sn_RX_RD       0x0028    /* puntero de lectura de RX */

/* Comandos (Sn_CR) */
#define CR_OPEN        0x01      /* abrir socket */
#define CR_SEND        0x20      /* enviar */
#define CR_RECV        0x40      /* confirmar recepcion */

/* Estado del socket */
#define SOCK_UDP       0x22      /* socket abierto en modo UDP */

/* ------- Funciones de bajo nivel (SPI) ------- */
static void     w5500_hw_reset(void);
static void     w5500_write_reg(uint16_t addr, uint8_t block, uint8_t data);
static uint8_t  w5500_read_reg(uint16_t addr, uint8_t block);
static void     w5500_write_reg16(uint16_t addr, uint8_t block, uint16_t val);
static uint16_t w5500_read_reg16(uint16_t addr, uint8_t block);
static void     w5500_write_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len);
static void     w5500_read_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len);

/* ------- Funciones de aplicacion ------- */
static void udp_send(const char* msg);   /* envia un mensaje UDP al destino actual */
static void diag_scan(void);             /* diagnostico: escanea SCN del W5500 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_SPI3_Init();
  /* USB CDC deshabilitado durante debug (colgaba en ISR USB_LP). Se usa SWD. */
  /* MX_USB_Device_Init(); */
  /* USER CODE BEGIN 2 */

  /* --- DIAGNOSTICO (deshabilitado): ya se encontraron SCN=PA4, NRST=PB6 --- */
  /* diag_scan(); */

  /* ================================================================
   * 1. Reset del W5500 y verificacion del chip
   * ================================================================ */
  w5500_hw_reset();                          /* reset por el pin NRST */
  w5500_write_reg(MR, COMMON_BLOCK, 0x80);   /* reset por software */
  HAL_Delay(100);
  w5500_write_reg(MR, COMMON_BLOCK, 0x00);   /* modo normal */

  g_ver = w5500_read_reg(VERSIONR, COMMON_BLOCK);
  if (g_ver != 0x04) {                       /* el chip debe leer 0x04 */
      while (1) { HAL_Delay(1000); }         /* W5500 no responde */
  }

  /* ================================================================
   * 2. Configuracion de red (MAC, IP, mascara, gateway)
   * ================================================================ */
  {
      uint8_t mac[6] = {W5500_MAC0, W5500_MAC1, W5500_MAC2,
                        W5500_MAC3, W5500_MAC4, W5500_MAC5};
      uint8_t ip[4]  = {W5500_IP0, W5500_IP1, W5500_IP2, W5500_IP3};
      uint8_t sn[4]  = {W5500_SN0, W5500_SN1, W5500_SN2, W5500_SN3};
      uint8_t gw[4]  = {W5500_GW0, W5500_GW1, W5500_GW2, W5500_GW3};
      int i;
      for (i = 0; i < 6; i++) w5500_write_reg(SHAR + i, COMMON_BLOCK, mac[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(SIPR + i, COMMON_BLOCK, ip[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(SUBR + i, COMMON_BLOCK, sn[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(GAR  + i, COMMON_BLOCK, gw[i]);
  }

  /* ================================================================
   * 3. PHY: auto-negociacion y espera del link
   * ================================================================ */
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0xF8);   /* RST=1, auto-neg, todo capaz */
  HAL_Delay(1);
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0x78);   /* RST=0 -> reset del PHY */
  HAL_Delay(1);
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0xF8);   /* RST=1 -> liberar reset */

  {
      uint32_t t = 0;
      while (t < 3000) {                          /* esperar link (max ~3 s) */
          if (w5500_read_reg(PHYCFGR, COMMON_BLOCK) & 0x01) break;  /* LNK=1 */
          HAL_Delay(10);
          t += 10;
      }
  }
  g_phy = w5500_read_reg(PHYCFGR, COMMON_BLOCK);

  /* ================================================================
   * 4. Socket 0 en modo UDP, puerto 5000
   * ================================================================ */
  w5500_write_reg(Sn_RXBUF_SIZE, SOCK0_BLOCK, 2);   /* 2 KB de RX */
  w5500_write_reg(Sn_TXBUF_SIZE, SOCK0_BLOCK, 2);   /* 2 KB de TX */
  w5500_write_reg(Sn_MR, SOCK0_BLOCK, 0x02);        /* 0x02 = UDP */
  w5500_write_reg16(Sn_PORT, SOCK0_BLOCK, UDP_PORT);
  w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_OPEN);     /* abrir el socket */
  HAL_Delay(10);
  g_snmr   = w5500_read_reg(Sn_MR, SOCK0_BLOCK);    /* debe leer 0x02 */
  g_socket = w5500_read_reg(Sn_SR, SOCK0_BLOCK);    /* debe leer 0x22 (UDP) */

  /* ================================================================
   * 5. Bucle principal
   *      (a) Si llega un mensaje UDP -> responde "RECIBIDO".
   *      (b) Cada 10 s -> envia "KA" al PC.
   * ================================================================ */
  {
      uint8_t  rx[128];
      uint32_t last_tick = HAL_GetTick();

      while (1)
      {
          /* --- (a) Recepcion --- */
          g_rsr = w5500_read_reg16(Sn_RX_RSR, SOCK0_BLOCK);
          if (g_rsr >= 1)
          {
              uint16_t n    = g_rsr;
              uint16_t rxrd = w5500_read_reg16(Sn_RX_RD, SOCK0_BLOCK);
              if (n > sizeof(rx)) n = sizeof(rx);

              w5500_read_buf(SOCK0_RXBUF, rxrd, rx, n);
              w5500_write_reg16(Sn_RX_RD, SOCK0_BLOCK, rxrd + n);
              w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_RECV);
              g_rx_count++;

              /* El W5500 ya guardo en Sn_DIPR/Sn_DPORT la IP y puerto
               * del emisor, por eso udp_send() responde al emisor. */
              udp_send("RECIBIDO");
          }

          /* --- (b) Keep-alive cada 10 s --- */
          if (HAL_GetTick() - last_tick >= TX_INTERVAL_MS)
          {
              last_tick = HAL_GetTick();

              /* destino = broadcast (255.255.255.255 : 5000).
               * Se usa broadcast (no unicast al PC) porque el unicast
               * necesita ARP para resolver la MAC del PC, y el RX del
               * W5500 esta fallando: nunca recibe la respuesta ARP, por
               * lo que el paquete UDP jamas se envia. El broadcast no
               * necesita ARP y sale directo por TX. */
              w5500_write_reg(Sn_DIPR + 0, SOCK0_BLOCK, 255);
              w5500_write_reg(Sn_DIPR + 1, SOCK0_BLOCK, 255);
              w5500_write_reg(Sn_DIPR + 2, SOCK0_BLOCK, 255);
              w5500_write_reg(Sn_DIPR + 3, SOCK0_BLOCK, 255);
              w5500_write_reg16(Sn_DPORT, SOCK0_BLOCK, UDP_PORT);

              udp_send("KA");
          }

          /* estado vivo para debug por SWD */
          g_phy_live = w5500_read_reg(PHYCFGR, COMMON_BLOCK);
          g_socket   = w5500_read_reg(Sn_SR, SOCK0_BLOCK);

          HAL_Delay(5);
      }
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* ================================================================
 * DIAGNOSTICO: escanea pines para encontrar el SCN del W5500.
 *   - Libera NRST (todos los candidatos en ALTO).
 *   - Prueba cada candidato como chip-select leyendo VERSIONR (0x04).
 * Candidatos: GPIOA/B/C (excluye SWD PA13/PA14 y SPI3 PB3/PB4/PB5).
 * ================================================================ */
static void diag_scan(void)
{
    struct { GPIO_TypeDef* port; uint16_t pin; uint8_t port_id; } cand[] = {
        {GPIOA, GPIO_PIN_0, 0}, {GPIOA, GPIO_PIN_1, 0}, {GPIOA, GPIO_PIN_2, 0},
        {GPIOA, GPIO_PIN_3, 0}, {GPIOA, GPIO_PIN_4, 0}, {GPIOA, GPIO_PIN_5, 0},
        {GPIOA, GPIO_PIN_6, 0}, {GPIOA, GPIO_PIN_7, 0}, {GPIOA, GPIO_PIN_8, 0},
        {GPIOA, GPIO_PIN_9, 0}, {GPIOA, GPIO_PIN_10, 0}, {GPIOA, GPIO_PIN_11, 0},
        {GPIOA, GPIO_PIN_12, 0}, {GPIOA, GPIO_PIN_15, 0},
        {GPIOB, GPIO_PIN_0, 1}, {GPIOB, GPIO_PIN_1, 1}, {GPIOB, GPIO_PIN_2, 1},
        {GPIOB, GPIO_PIN_6, 1}, {GPIOB, GPIO_PIN_7, 1}, {GPIOB, GPIO_PIN_8, 1},
        {GPIOB, GPIO_PIN_9, 1}, {GPIOB, GPIO_PIN_10, 1}, {GPIOB, GPIO_PIN_11, 1},
        {GPIOB, GPIO_PIN_12, 1}, {GPIOB, GPIO_PIN_13, 1}, {GPIOB, GPIO_PIN_14, 1},
        {GPIOB, GPIO_PIN_15, 1},
        {GPIOC, GPIO_PIN_13, 2}, {GPIOC, GPIO_PIN_14, 2}, {GPIOC, GPIO_PIN_15, 2},
    };
    const int n = (int)(sizeof(cand) / sizeof(cand[0]));
    GPIO_InitTypeDef gi = {0};
    uint8_t cmd[4];
    int i;

    __HAL_RCC_GPIOC_CLK_ENABLE();

    gi.Mode = GPIO_MODE_OUTPUT_PP;
    gi.Pull = GPIO_NOPULL;
    gi.Speed = GPIO_SPEED_FREQ_HIGH;

    /* Liberar NRST y deseleccionar: todos los candidatos en ALTO */
    for (i = 0; i < n; i++) {
        gi.Pin = cand[i].pin;
        HAL_GPIO_Init(cand[i].port, &gi);
        HAL_GPIO_WritePin(cand[i].port, cand[i].pin, GPIO_PIN_SET);
    }

    /* Probar cada candidato como chip-select */
    for (i = 0; i < n; i++) {
        cmd[0] = 0x00; cmd[1] = 0x39; cmd[2] = 0x00; cmd[3] = 0;  /* VERSIONR, read */
        HAL_GPIO_WritePin(cand[i].port, cand[i].pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi3, cmd, 3, 100);
        HAL_SPI_Receive(&hspi3, &cmd[3], 1, 100);
        HAL_GPIO_WritePin(cand[i].port, cand[i].pin, GPIO_PIN_SET);
        g_scan_ver = cmd[3];
        if (cmd[3] == 0x04) {
            g_scan_found = 1;
            g_scan_pin = cand[i].pin;
            g_scan_port = cand[i].port_id;
            break;
        }
    }

    /* Encontrar NRST: con SCN fijo (PA4, el encontrado), bajar cada candidato.
     * Si al bajarlo el VERSIONR deja de ser 0x04, ese pin es el reset. */
    if (g_scan_found) {
        GPIO_TypeDef* csPort = GPIOA;      /* SCN encontrado = PA4 */
        uint16_t csPin = GPIO_PIN_4;
        int j;
        for (j = 0; j < n; j++) {
            if (cand[j].port == csPort && cand[j].pin == csPin) continue;  /* saltar SCN */
            cmd[0] = 0x00; cmd[1] = 0x39; cmd[2] = 0x00; cmd[3] = 0;
            HAL_GPIO_WritePin(cand[j].port, cand[j].pin, GPIO_PIN_RESET);  /* reset candidate low */
            HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_RESET);              /* SCN low */
            HAL_SPI_Transmit(&hspi3, cmd, 3, 100);
            HAL_SPI_Receive(&hspi3, &cmd[3], 1, 100);
            HAL_GPIO_WritePin(csPort, csPin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(cand[j].port, cand[j].pin, GPIO_PIN_SET);
            if (cmd[3] != 0x04) {
                g_scan_nrst_found = 1;
                g_scan_nrst_pin = cand[j].pin;
                g_scan_nrst_port = cand[j].port_id;
                break;
            }
        }
    }
}

/* Envia un mensaje UDP por el socket 0.
 * ATENCION: el destino (Sn_DIPR / Sn_DPORT) debe estar seteado antes:
 *   - Para "KA"      -> se fija la IP del PC antes de llamar.
 *   - Para "RECIBIDO" -> el W5500 ya guardo el origen al recibir. */
static void udp_send(const char* msg)
{
    uint16_t len  = (uint16_t)strlen(msg);
    uint16_t txwr;

    /* esperar espacio libre en el buffer de transmision */
    while (w5500_read_reg16(Sn_TX_FSR, SOCK0_BLOCK) < len) {
        HAL_Delay(1);
    }

    /* escribir el mensaje y avanzar el puntero de escritura */
    txwr = w5500_read_reg16(Sn_TX_WR, SOCK0_BLOCK);
    w5500_write_buf(SOCK0_TXBUF, txwr, (uint8_t*)msg, len);
    w5500_write_reg16(Sn_TX_WR, SOCK0_BLOCK, txwr + len);

    /* ordenar el envio */
    w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_SEND);
    g_tx_count++;
}

static void w5500_hw_reset(void)
{
    HAL_GPIO_WritePin(NRST_W5500_GPIO_Port, NRST_W5500_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(NRST_W5500_GPIO_Port, NRST_W5500_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

static void w5500_write_reg(uint16_t addr, uint8_t block, uint8_t data)
{
    uint8_t cmd[4];
    cmd[0] = (addr >> 8) & 0xFF;
    cmd[1] = addr & 0xFF;
    cmd[2] = (block << 3) | (1 << 2) | 0x00;
    cmd[3] = data;
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi3, cmd, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

static uint8_t w5500_read_reg(uint16_t addr, uint8_t block)
{
    uint8_t cmd[4];
    cmd[0] = (addr >> 8) & 0xFF;
    cmd[1] = addr & 0xFF;
    cmd[2] = ((block << 3) & 0xF8) | (0 << 2) | 0x00;
    cmd[3] = 0;
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_StatusTypeDef st1 = HAL_SPI_Transmit(&hspi3, cmd, 3, 2000);
    HAL_StatusTypeDef st2 = HAL_SPI_Receive(&hspi3, &cmd[3], 1, 2000);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
    if (st1 == HAL_OK && st2 == HAL_OK) return cmd[3];
    return 0;
}

static void w5500_write_reg16(uint16_t addr, uint8_t block, uint16_t val)
{
    w5500_write_reg(addr,     block, (uint8_t)(val >> 8));
    w5500_write_reg(addr + 1, block, (uint8_t)(val & 0xFF));
}

static uint16_t w5500_read_reg16(uint16_t addr, uint8_t block)
{
    uint16_t hi = w5500_read_reg(addr,     block);
    uint16_t lo = w5500_read_reg(addr + 1, block);
    return (hi << 8) | lo;
}

static void w5500_write_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len)
{
    uint8_t hdr[3];
    hdr[0] = (addr >> 8) & 0xFF;
    hdr[1] = addr & 0xFF;
    hdr[2] = (block << 3) | (1 << 2) | 0x00;  /* write, VDM */
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi3, hdr, 3, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi3, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

static void w5500_read_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len)
{
    uint8_t hdr[3];
    hdr[0] = (addr >> 8) & 0xFF;
    hdr[1] = addr & 0xFF;
    hdr[2] = ((block << 3) & 0xF8) | (0 << 2) | 0x00;  /* read, VDM */
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi3, hdr, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi3, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

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
#ifdef USE_FULL_ASSERT
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
