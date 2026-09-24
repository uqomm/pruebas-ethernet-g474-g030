/**
  ******************************************************************************
  * @file    w5500_network.c
  * @brief   UDP network layer implementation for W5500.
  ******************************************************************************
  */

#include "w5500_network.h"
#include "w5500_driver.h"
#include "socket.h"

/* ------------------------------------------------------------------------- */
/* W5500_UDP_Init                                                            */
/* ------------------------------------------------------------------------- */
int8_t W5500_UDP_Init(uint8_t sock, uint16_t port)
{
    int8_t ret;

    ret = WIZCHIP_socket(sock, Sn_MR_UDP, port, 0);
    if (ret != SOCK_OK)
    {
        debug_print("UDP_Init: ERROR opening socket!\r\n");
        return ret;
    }

    debug_print("UDP_Init: Socket ready, listening on port...\r\n");
    return 0;
}

/* ------------------------------------------------------------------------- */
/* W5500_UDP_Send                                                            */
/* ------------------------------------------------------------------------- */
int32_t W5500_UDP_Send(uint8_t sock, uint8_t* dest_ip, uint16_t dest_port,
                        uint8_t* data, uint16_t len)
{
    int32_t ret;

    ret = WIZCHIP_sendto(sock, data, len, dest_ip, dest_port);
    return ret;
}

/* ------------------------------------------------------------------------- */
/* W5500_UDP_Receive                                                         */
/* ------------------------------------------------------------------------- */
int32_t W5500_UDP_Receive(uint8_t sock, uint8_t* buf, uint16_t* len,
                           uint8_t* src_ip, uint16_t* src_port)
{
    int32_t ret;

    ret = WIZCHIP_recvfrom(sock, buf, len, src_ip, src_port);
    return ret;
}
