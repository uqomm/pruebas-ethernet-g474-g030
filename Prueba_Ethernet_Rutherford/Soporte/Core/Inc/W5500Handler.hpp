/*
 * W5500Handler.hpp
 *
 *  Created on: Jan 24, 2025
 *      Author: Alan
 */

#ifndef INC_W5500HANDLER_HPP_
#define INC_W5500HANDLER_HPP_

#include "main.h"
#include "Gpio.hpp"
#include "GpioHandler.hpp"
#include "string.h"

// Estados del Socket
#define SOCK_INIT   0x13
#define SOCK_ESTABLISHED 0x17

// Commando del Socket
#define Sn_CR_OPEN  0x01  // Inicializar o abrir socket
#define Sn_CR_LISTEN  0x02  // Socket Escuchando
#define Sn_CR_CONNECT 0x04 // Enviar solicitud de conexión al servidor
#define Sn_CR_DISCON  0x08  // Desconectar
#define Sn_CR_CLOSE   0x10  // Cerrar el socket
#define Sn_CR_SEND    0x20  // Enviar datos
#define Sn_CR_RECV    0x40  // Recibir datos

// STATUS DEL SOCKET
#define SOCK_INIT 0x13
#define SOCK_LISTEN 0x14
#define SOCK_ESTABLISHED 0x17
#define SOCK_CLOSE_WAIT 0x1C
#define SOCK_CLOSED 0x00

//MANEJO DE MEMORIA DE n SOCKET
#define Sn_RXBUF_SIZE 0x1E //Socket n Receive Buffer Size
#define Sn_TXBUF_SIZE 0x1F //Socket n Transmit Buffer Size
#define Sn_TX_FSR 0x20 // [0x0020-0x0021] Socket n - Tamaño libre de TX
#define Sn_TX_RD 0x22 // [0x0022-0x0023] Socket n Lectura Posicion de TX Pointer
#define Sn_TX_WR 0x24	//[0x0024-0x0025]Socket n - Escritura de Puntero TX
#define Sn_RX_RSR 0x26 //[0x0026-0x0027]  Socket n - Tamaño de data recibida
#define Sn_RX_RD 0x28 //[0x0028-0x0029] Socket n - Lectura Posicion Puntero RX


#define COMMON_REGISTER_BLOCK 0x00
#define SOCKET0 0x01  // Selecciona el Socket 0 Register Block
#define SOCKET1 0x05  // Selecciona el Socket 1 Register Block
#define SOCKET2 0x09  // Selecciona el Socket 2 Register Block
#define SOCKET3 0x0D  // Selecciona el Socket 3 Register Block
#define SOCKET4 0x11  // Selecciona el Socket 4 Register Block
#define SOCKET5 0x15  // Selecciona el Socket 5 Register Block
#define SOCKET6 0x19  // Selecciona el Socket 6 Register Block
#define SOCKET7 0x1D  // Selecciona el Socket 7 Register Block

#define Socket0_TX_Buffer 0x02	//Selecciona el TX_Buffer del Socket 0
#define Socket0_RX_Buffer 0x03	//Selecciona el RX_Buffer del Socket 0


#define MR  0x0000    // Registro de Modo
#define IMR 0x0016    // Registro de Máscara de Interrupción
#define RTR 0x0019    // [0x0019 – 0x001A] Registro de Tiempo de Reintento
#define RCR 0x001B    // [0x001B] Registro de Conteo de Reintentos
#define SHAR 0x0009  // [0x0009 – 0x000E] Dirección MAC de Origen
#define GAR 0x0001  // [0x0001 – 0x0004] Dirección de Puerta de Enlace
#define SUBR 0x0005  // [0x0005 – 0x0008] Máscara de Subred
#define SIPR 0x000F  // [0x000F – 0x0012]  Dirección IP de Origen
#define PHYCFGR 0x002E // Registro de configuración PHY
#define VERSIONR 0x0039
#define Sn_MR(n) (0x0400 + (n*0x20) + 0x00) // Socket n - Registro de Modo
#define Sn_PORT 0x0004 //[0x0004-0x0005]   Socket n - Puerto de Origen
#define Sn_CR 0x0001// Socket n - Registro de Comando
#define Sn_SR 0x0003 // Socket n - Registro de Estado
#define Sn_IR 0x0002//  [0x0002] Socket n - Registro de Interrupción
#define Sn_IMR 0x002C// HABILITACION DE INTERRUPCIONES
#define Sn_DIPR 0x000C  // [0x000C-0x000F]Socket n - Dirección IP Destino
#define Sn_DPORT 0x0010 //  [0x0010-0x0011] Socket n - Puerto Destino

#define Sn_IMR_MASK 0x1F //
#define Sn_IR_CON 0x01  //
#define Sn_IR_DISCON 0x02 //
#define Sn_IR_RECV 0x04 //
#define Sn_IR_TIMEOUT 0x08 //
#define Sn_IR_SENDOK 0x10 //





//Tamaño de los buffers de datos
#define DATA_BUFFER_SIZE 2048
// Variables globales

class W5500Handler {
public:
	W5500Handler(SPI_HandleTypeDef *_spi, Gpio _rst, Gpio _nss);
	virtual ~W5500Handler();

	void w5500_reset();
	uint8_t w5500_check_connection();
	void w5500_configure_basic_registers();
	void TCP_server(uint8_t socket_n, uint16_t source_port);
	void start_tcp_client(uint8_t socket_n, uint16_t dest_port, uint8_t *dest_ip);
	void tcp_client_send(uint8_t* data, uint16_t size, uint8_t socket);

private:
	void W5500_Init();
	HAL_StatusTypeDef w5500_write_register(uint16_t address, uint8_t block, uint8_t data);
	uint8_t w5500_read_register(uint16_t address, uint8_t register_block);
	void w5500_write_tx_buffer(uint16_t address, uint8_t offset, uint8_t *data, uint16_t length);
	void W5500_read_buffer(uint32_t addr, uint8_t *buf, uint16_t len);

	uint16_t get_tx_wr_ptr(uint8_t sn);
	uint16_t get_RX_RD(uint8_t sn);
	uint16_t rx_size_recive(uint8_t sn);
	uint16_t tx_free_size(uint8_t sn);
	void set_tx_WR_ptr(uint8_t sn, uint16_t txwr);
	void set_RX_RD(uint8_t sn, uint16_t rxrd);

	SPI_HandleTypeDef *spi;
	Gpio nss;
	Gpio rst;
	Gpio interr;
	uint8_t g_rx_buffer[DATA_BUFFER_SIZE];
	uint8_t g_tx_buffer[DATA_BUFFER_SIZE];

	uint8_t _PHYCFGR_RST = 0x1D;
	uint8_t _PHYCFGR_NRST = 0X5D;
	uint8_t irq_flags = 0;
	uint8_t status_socket = 0;

};

#endif /* INC_W5500HANDLER_HPP_ */
