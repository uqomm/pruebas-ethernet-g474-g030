/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body - STM32G030K8T6 + W5500 (Ethernet UDP)
  *                   SPI1: PA5=SCK, PA6=MISO, PA7=MOSI, SCN=PB0, NRST=PA8
  *                   Logica portada del proyecto Prueba_Ethernet_Rutherford:
  *                   - Sn_MR=0x02 (UDP, broadcast habilitado, BCSTB=0)
  *                   - PHY forzada a 100M full (PHYCFGR=0xD8) porque PMODE fija 10M half
  *                   - Socket UDP puerto 5000, IP 192.168.1.50
  *                   - Escucha + eco + TX periodico "HELLO from W5500"
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* Debug globals (lectura por SWD, direcciones en el .map) */
volatile uint8_t  g_ver = 0xFF;
volatile uint8_t  g_phy = 0xFF;
volatile uint8_t  g_socket = 0xFF;
volatile uint8_t  g_snmr = 0xFF;
volatile uint8_t  g_diag[8] = {0};
volatile uint8_t  g_phy_live = 0;
volatile uint16_t g_rsr = 0;
volatile uint16_t g_tx_count = 0;
volatile uint16_t g_rx_count = 0;

/* W5500: bloques y offsets */
#define COMMON_BLOCK  0x00
#define MR            0x0000
#define SHAR          0x0009
#define GAR           0x0001
#define SUBR          0x0005
#define SIPR          0x000F
#define PHYCFGR       0x002E
#define VERSIONR      0x0039

#define SOCK0_BLOCK   0x01
#define SOCK0_TXBUF   0x02
#define SOCK0_RXBUF   0x03

#define Sn_MR         0x0000
#define Sn_CR         0x0001
#define Sn_SR         0x0003
#define Sn_PORT       0x0004
#define Sn_DIPR       0x000C
#define Sn_DPORT      0x0010
#define Sn_RXBUF_SIZE 0x001E
#define Sn_TXBUF_SIZE 0x001F
#define Sn_TX_FSR     0x0020
#define Sn_TX_WR      0x0024
#define Sn_RX_RSR     0x0026
#define Sn_RX_RD      0x0028

#define SOCK_UDP      0x22
#define CR_OPEN       0x01
#define CR_LISTEN     0x02
#define CR_CLOSE      0x10
#define CR_SEND       0x20
#define CR_RECV       0x40

#define SOCK_CLOSED       0x00
#define SOCK_INIT         0x13
#define SOCK_LISTEN       0x14
#define SOCK_ESTABLISHED  0x17
#define SOCK_CLOSE_WAIT   0x1C

#define W5500_UDP_PORT 5000

/* Prototipos */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void w5500_hw_reset(void);
static void w5500_write_reg(uint16_t addr, uint8_t block, uint8_t data);
static uint8_t w5500_read_reg(uint16_t addr, uint8_t block);
static void w5500_write_reg16(uint16_t addr, uint8_t block, uint16_t val);
static uint16_t w5500_read_reg16(uint16_t addr, uint8_t block);
static void w5500_write_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len);
static void w5500_read_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len);

/**
  * @brief  The application entry point.
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_SPI1_Init();

  /* 1. HW reset W5500 */
  w5500_hw_reset();

  /* 2. SW reset: MR=0x80 */
  w5500_write_reg(MR, COMMON_BLOCK, 0x80);
  HAL_Delay(100);

  /* 2b. DIAG: leer VERSIONR 8 veces */
  {
      uint8_t i;
      for (i = 0; i < 8; i++) {
          g_diag[i] = w5500_read_reg(VERSIONR, COMMON_BLOCK);
      }
  }

  /* 3. Verificar VERSIONR */
  g_ver = w5500_read_reg(VERSIONR, COMMON_BLOCK);
  if (g_ver != 0x04) {
      while (1) { HAL_Delay(1000); }   /* W5500 no detectado */
  }

  /* 4. Common registers */
  w5500_write_reg(MR, COMMON_BLOCK, 0x00);  /* normal mode */
  {
      uint8_t mac[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x01};
      uint8_t ip[4]  = {192, 168, 1, 50};
      uint8_t sub[4] = {255, 255, 255, 0};
      uint8_t gw[4]  = {192, 168, 1, 1};
      int i;
      for (i = 0; i < 6; i++) w5500_write_reg(SHAR + i, COMMON_BLOCK, mac[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(SIPR + i, COMMON_BLOCK, ip[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(SUBR + i, COMMON_BLOCK, sub[i]);
      for (i = 0; i < 4; i++) w5500_write_reg(GAR  + i, COMMON_BLOCK, gw[i]);
  }

  /* 4b. Auto-negociacion (OPMD=1, OPMDC=111). Empareja duplex con el switch. */
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0xF8);
  HAL_Delay(1);
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0x78);  /* RST=0 -> reset PHY */
  HAL_Delay(1);
  w5500_write_reg(PHYCFGR, COMMON_BLOCK, 0xF8);  /* RST=1 -> liberar */
  /* Esperar link real (auto-neg puede tardar >200ms). Max ~3s. */
  {
      uint32_t link_wait = 0;
      while (link_wait < 3000)
      {
          if (w5500_read_reg(PHYCFGR, COMMON_BLOCK) & 0x01) break;  /* LNK=1 */
          HAL_Delay(10);
          link_wait += 10;
      }
  }

  /* 5. PHY link status */
  g_phy = w5500_read_reg(PHYCFGR, COMMON_BLOCK);

  /* 6. Socket 0: 2KB RX + 2KB TX */
  w5500_write_reg(Sn_RXBUF_SIZE, SOCK0_BLOCK, 2);
  w5500_write_reg(Sn_TXBUF_SIZE, SOCK0_BLOCK, 2);

  /* 7. Socket 0 = UDP, puerto 5000 (test RX broadcast -> NO requiere ARP) */
  w5500_write_reg(Sn_MR, SOCK0_BLOCK, 0x02);   /* UDP, BCSTB=0 */
  w5500_write_reg16(Sn_PORT, SOCK0_BLOCK, W5500_UDP_PORT);
  w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_OPEN);
  HAL_Delay(10);
  g_snmr = w5500_read_reg(Sn_MR, SOCK0_BLOCK);
  g_socket = w5500_read_reg(Sn_SR, SOCK0_BLOCK);  /* SOCK_UDP 0x22 */

  /* 8. Main loop: recibir UDP (broadcast/unicast) y hacer eco */
  {
      uint8_t rx[512];

      while (1)
      {
          g_phy_live = w5500_read_reg(PHYCFGR, COMMON_BLOCK);
          g_socket  = w5500_read_reg(Sn_SR, SOCK0_BLOCK);
          g_rsr     = w5500_read_reg16(Sn_RX_RSR, SOCK0_BLOCK);

          if (g_rsr >= 1)
          {
              uint16_t rxrd = w5500_read_reg16(Sn_RX_RD, SOCK0_BLOCK);
              uint16_t n = g_rsr;
              if (n > sizeof(rx)) n = sizeof(rx);
              w5500_read_buf(SOCK0_RXBUF, rxrd, rx, n);
              w5500_write_reg16(Sn_RX_RD, SOCK0_BLOCK, rxrd + n);
              w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_RECV);
              g_rx_count++;

              /* eco: Sn_DIPR/Sn_DPORT actualizados por el paquete recibido */
              uint16_t txwr;
              while (w5500_read_reg16(Sn_TX_FSR, SOCK0_BLOCK) < n) HAL_Delay(1);
              txwr = w5500_read_reg16(Sn_TX_WR, SOCK0_BLOCK);
              w5500_write_buf(SOCK0_TXBUF, txwr, rx, n);
              w5500_write_reg16(Sn_TX_WR, SOCK0_BLOCK, txwr + n);
              w5500_write_reg(Sn_CR, SOCK0_BLOCK, CR_SEND);
              g_tx_count++;
          }

          HAL_Delay(5);
      }
  }
}

/* ==================== Funciones W5500 (SPI) ==================== */

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
    cmd[2] = (block << 3) | (1 << 2) | 0x00;   /* write, VDM */
    cmd[3] = data;
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

static uint8_t w5500_read_reg(uint16_t addr, uint8_t block)
{
    uint8_t cmd[4];
    cmd[0] = (addr >> 8) & 0xFF;
    cmd[1] = addr & 0xFF;
    cmd[2] = ((block << 3) & 0xF8) | (0 << 2) | 0x00;  /* read, VDM */
    cmd[3] = 0;
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, cmd, 3, 2000);
    HAL_SPI_Receive(&hspi1, &cmd[3], 1, 2000);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
    return cmd[3];
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
    HAL_SPI_Transmit(&hspi1, hdr, 3, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

static void w5500_read_buf(uint8_t block, uint16_t addr, uint8_t* data, uint16_t len)
{
    uint8_t hdr[3];
    hdr[0] = (addr >> 8) & 0xFF;
    hdr[1] = addr & 0xFF;
    hdr[2] = ((block << 3) & 0xF8) | (0 << 2) | 0x00;  /* read, VDM */
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, hdr, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
}

/* ==================== Inicializacion ==================== */

static void MX_SPI1_Init(void)
{
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;  /* 8 MHz @ 16 MHz HSI */
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
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* SCN (PB0) y NRST (PA8) como salida, estado inicial alto */
  HAL_GPIO_WritePin(SCN_W5500_GPIO_Port, SCN_W5500_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(NRST_W5500_GPIO_Port, NRST_W5500_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = SCN_W5500_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SCN_W5500_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = NRST_W5500_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(NRST_W5500_GPIO_Port, &GPIO_InitStruct);
}

static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
