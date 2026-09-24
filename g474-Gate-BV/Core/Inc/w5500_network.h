/**
  ******************************************************************************
  * @file    w5500_network.h
  * @brief   UDP network layer for W5500.
  ******************************************************************************
  */

#ifndef W5500_NETWORK_H
#define W5500_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Initialize a UDP socket on the W5500.
 * @param  sock  Socket number (0..7)
 * @param  port  Local port to bind
 * @return 0 on success, negative on error
 */
int8_t W5500_UDP_Init(uint8_t sock, uint16_t port);

/**
 * @brief  Send a UDP datagram.
 * @param  sock       Socket number
 * @param  dest_ip    4-byte destination IP
 * @param  dest_port  Destination port
 * @param  data       Payload buffer
 * @param  len        Payload length
 * @return Number of bytes sent, or negative on error
 */
int32_t W5500_UDP_Send(uint8_t sock, uint8_t* dest_ip, uint16_t dest_port,
                        uint8_t* data, uint16_t len);

/**
 * @brief  Receive a UDP datagram (non-blocking).
 * @param  sock       Socket number
 * @param  buf        Receive buffer
 * @param  len        [in/out] Max size on entry, received size on exit
 * @param  src_ip     [out] Sender IP (4 bytes), can be NULL
 * @param  src_port   [out] Sender port, can be NULL
 * @return Received data length (>0), 0 if no data, negative on error
 */
int32_t W5500_UDP_Receive(uint8_t sock, uint8_t* buf, uint16_t* len,
                           uint8_t* src_ip, uint16_t* src_port);

#ifdef __cplusplus
}
#endif

#endif /* W5500_NETWORK_H */
