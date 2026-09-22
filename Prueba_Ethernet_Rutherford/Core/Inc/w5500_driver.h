/**
  ******************************************************************************
  * @file    w5500_driver.h
  * @brief   W5500 SPI driver adaptation for STM32 HAL.
  *          Provides SPI callbacks, hardware reset, and chip initialization.
  ******************************************************************************
  */

#ifndef W5500_DRIVER_H
#define W5500_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief  Hardware reset of the W5500 via NRST pin (PC5).
 *         Pulses the reset line LOW for ~1ms, then waits 100ms for PLL lock.
 */
void W5500_HardwareReset(void);

/**
 * @brief  Initialize the W5500 chip.
 *         Registers SPI/CS/CRIS callbacks with the WIZnet ioLibrary,
 *         configures socket buffer sizes, and verifies the chip version (0x04).
 * @return 0 on success, -1 if chip not detected (version mismatch).
 */
int8_t W5500_Init(void);

/**
 * @brief  Configure static network parameters (MAC, IP, subnet, gateway).
 * @param  mac      6-byte MAC address
 * @param  ip       4-byte IP address
 * @param  subnet   4-byte subnet mask
 * @param  gateway  4-byte default gateway
 */
void W5500_SetNetwork(uint8_t* mac, uint8_t* ip, uint8_t* subnet, uint8_t* gateway);

/**
 * @brief  Simple debug print over USB CDC.
 *          Uses CDC_Transmit_FS() to send a null-terminated string.
 * @param  msg  Null-terminated string to transmit.
 */
void debug_print(const char* msg);

/**
 * @brief  Check if USB CDC is connected and ready.
 * @return 1 if ready, 0 if not connected yet.
 */
uint8_t debug_is_ready(void);

#ifdef __cplusplus
}
#endif

#endif /* W5500_DRIVER_H */
