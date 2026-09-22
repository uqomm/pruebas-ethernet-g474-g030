/*
 * W5500Handler.cpp
 *
 *  Created on: Jan 24, 2025
 *      Author: Alan
 */

#include <W5500Handler.hpp>
#include "vector"

W5500Handler::W5500Handler(SPI_HandleTypeDef *_spi, Gpio _rst, Gpio _nss) {
	// TODO Auto-generated constructor stub
	spi = _spi;
	nss = _nss;
	rst = _rst;

	HAL_GPIO_WritePin(rst.get_port(), rst.get_pin(), GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(rst.get_port(), rst.get_pin(), GPIO_PIN_SET);
	HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_SET);
	W5500_Init();
}

W5500Handler::~W5500Handler() {
	// TODO Auto-generated destructor stub
}

void W5500Handler::W5500_Init() {
	w5500_reset();

	if (w5500_check_connection()) {		//VERFICA CORRECTA COMUNICACION SPI CON CHIP W5500
		uint8_t phy_cfg;
		phy_cfg = w5500_read_register(PHYCFGR, COMMON_REGISTER_BLOCK); // Lee PHYCFGR
		if (!(phy_cfg & 0x01)) {									// Verifica PHY LINK
			while (1); //error: PHY Link no está activo
		}
		w5500_configure_basic_registers();	//CONFIGURACIONES BASICAS GENERALES W5500
	} else {
		while (1); // W5500 no responde
	}
}

void W5500Handler::w5500_configure_basic_registers() {
	// Configura el modo de operación (por ejemplo, modo full-duplex)
	w5500_write_register(MR, COMMON_REGISTER_BLOCK, 0x00); // MR = Mode Register, bit 7 = 0 (sin reset), otros bits según necesidad

	//TIEMPO DE ESPERA PARA RETRANSMISION
	uint16_t time_retransmision = 1388;  //500ms
	uint8_t time_array[2];
	time_array[0] = (time_retransmision >> 8) & 0xFF;
	time_array[1] = time_retransmision & 0xFF;;
	for(int i = 0; i <=1; i++){
		w5500_write_register((RTR + i), COMMON_REGISTER_BLOCK, time_array[i]);
	}

	//NUMERO DE RETRANSMISIONES
	uint8_t num_retran = 0x8;   //num_retran + 1 = 8
	w5500_write_register(RCR, COMMON_REGISTER_BLOCK, num_retran);

	// Configura la dirección MAC (por ejemplo, 00:08:DC:12:34:56)
	uint8_t mac[6] = { 0x00, 0x08, 0xDC, 0x12, 0x34, 0x56 };
	for (int i = 0; i < 6; i++) {
		w5500_write_register(SHAR + i, COMMON_REGISTER_BLOCK, mac[i]);
	}

	// Configura la dirección IP, máscara de subred y puerta de enlace (opcional, puede hacerse más tarde)
	uint8_t ip[4] = { 192, 168, 60, 104 };
	for (int i = 0; i < 4; i++) {
		w5500_write_register(SIPR + i,COMMON_REGISTER_BLOCK, ip[i]);
	}

	uint8_t subnet[4] = { 255, 255, 255, 0 };
	for (int i = 0; i < 4; i++) {
		w5500_write_register(SUBR + i, COMMON_REGISTER_BLOCK, subnet[i]);
	}

	uint8_t gateway[4] = { 192, 168, 60, 1 };
	for (int i = 0; i < 4; i++) {
		w5500_write_register(GAR + i, COMMON_REGISTER_BLOCK, gateway[i]);
	}
}


void W5500Handler::start_tcp_client(uint8_t socket_n, uint16_t dest_port,
		uint8_t *dest_ip) {
	// Establecer el modo TCP en el registro Sn_MR(socket_n)
	w5500_write_register(MR, socket_n, 0x01); // Modo TCP
	// Establecer el puerto de origen en Sn_PORT(socket_n)
	uint16_t valor = 0x1389;  // puerto 5000
	uint8_t array[2];

	array[0] = (valor >> 8) & 0xFF;
	array[1] = valor & 0xFF;

	for (int i = 0; i <= 1; i++) {
		w5500_write_register((Sn_PORT + i), socket_n, array[i]); // Puerto cliente
	}

	// Abrir el socket usando el comando OPEN
	w5500_write_register(Sn_CR, socket_n, Sn_CR_OPEN);

	// Esperar a que el socket esté inicializado (SOCK_INIT)
	while (w5500_read_register(Sn_SR, socket_n) != SOCK_INIT)
		;

	// Establecer la dirección IP del servidor en el registro Sn_DIPR(socket_n)
	for (int j = 0; j <= 3; j++) {
		w5500_write_register((Sn_DIPR + j), socket_n, dest_ip[j]); // Puerto cliente
	}

	// Establecer el puerto del servidor en Sn_DPORT(socket_n)
	uint16_t _dest_port = dest_port;  // puerto 5000
	uint8_t _dest_port_array[2];

	_dest_port_array[0] = (_dest_port >> 8) & 0xFF;
	_dest_port_array[1] = _dest_port & 0xFF;

	for (int l = 0; l <= 1; l++) {
		w5500_write_register((Sn_DPORT + l), socket_n, _dest_port_array[l]); // Puerto cliente
	}
	// HABILITACION DE TODAS LAS INTERRUPCIONES
	w5500_write_register(Sn_IMR, socket_n, Sn_IMR_MASK);
	// Enviar una solicitud de conexión al servidor usando el comando CONNECT
	w5500_write_register(Sn_CR, socket_n, Sn_CR_CONNECT);
	// Esperar hasta que la conexión sea establecida (SOCK_ESTABLISHED)
	while (w5500_read_register(Sn_IR, socket_n) != Sn_IR_CON);
	irq_flags = w5500_read_register(Sn_IR, socket_n);
	status_socket = w5500_read_register(Sn_SR, socket_n);
	if ((status_socket == SOCK_ESTABLISHED) && (irq_flags == Sn_IR_CON)) { //VERIFICACION DE CONEXION ESTABLECIDA
		// Enviar datos

		uint16_t rx_size = rx_size_recive(socket_n); // Leer el tamaño de los datos recibidos (Dirección de ejemplo)
		if (rx_size > 0) {
			W5500_read_buffer(0x8000, g_rx_buffer, rx_size); // Leer los datos recibidos (Dirección de ejemplo)
			// Procesar los datos recibidos en g_rx_buffer
			w5500_write_register(Sn_CR, socket_n, Sn_CR_RECV); // Enviar comando de recepción
		}
		w5500_write_register(Sn_IR, socket_n, Sn_IMR_MASK);
	}
}

void W5500Handler::tcp_client_send(uint8_t* data, uint16_t size, uint8_t socket_n) {

	uint16_t free_size = tx_free_size(socket_n);
	if (free_size >= size) {
		uint16_t current_TX_WR = get_tx_wr_ptr(socket_n);

		w5500_write_tx_buffer(current_TX_WR, Socket0_TX_Buffer, data,
				size);

		current_TX_WR = (current_TX_WR + size);
		set_tx_WR_ptr(socket_n, current_TX_WR);
		current_TX_WR = get_tx_wr_ptr(socket_n);
		w5500_write_register(Sn_CR, socket_n, Sn_CR_SEND);
//		HAL_Delay(1000);
	}
}




void W5500Handler::TCP_server(uint8_t socket_n, uint16_t source_port) {
	uint8_t status;
	uint16_t len;
	uint16_t rx_rd;
	uint16_t tx_wr;
	uint8_t array[2];

	// 1. Configura el socket para modo TCP
	w5500_write_register(MR, socket_n, 0x01);
	// 2. Configura el puerto de origen
	array[0] = (source_port >> 8) & 0xFF;
	array[1] = source_port & 0xFF;

	for (int i = 0; i <= 1; i++) {
		w5500_write_register((Sn_PORT + i), socket_n, array[i]); //
	}
	// 3. Abre el socket
	w5500_write_register(Sn_CR, socket_n, Sn_CR_OPEN);
	// 4. Espera a que el socket esté inicializado
	while (w5500_read_register(Sn_SR, socket_n) != SOCK_INIT) {
		HAL_Delay(1);
	}
	// 5. Pon el socket a escuchar
	w5500_write_register(Sn_CR, socket_n, Sn_CR_LISTEN);
	// 6. Espera a que el socket esté en modo LISTEN
	while (w5500_read_register(Sn_SR, socket_n) != SOCK_LISTEN) {
		HAL_Delay(1);
	}
	// 7. Espera a que se establezca una conexión
	while (w5500_read_register(Sn_SR, socket_n) != SOCK_ESTABLISHED);

	uint32_t ip_client = w5500_read_register(Sn_DIPR, socket_n);
	uint32_t port_client = w5500_read_register(Sn_DPORT, socket_n);





//	if (status_Sn == SOCK_LISTEN) {
//	} else if (status_Sn == SOCK_ESTABLISHED) {
//		uint32_t ip_client = w5500_read_register(Sn_DIPR, socket_n);
//		uint32_t port_client = w5500_read_register(Sn_DPORT, socket_n);
//
//		//todo envios y recepciones de mensajes
//	} else if (status_Sn == SOCK_CLOSE_WAIT) {
//	}
//
//	else if (status_Sn == SOCK_CLOSED) {
//		w5500_write_register(Sn_CR, socket_n, Sn_CR_CLOSE);
//	}


	// 8. El socket ahora está en modo SOCK_ESTABLISHED
//	while (1) {
//		status = w5500_read_register(Sn_SR, socket_n);
//		if (status == SOCK_ESTABLISHED) {
//			//Comprobar si hay datos en el buffer de recepción
//			len = getSn_RX_RSR(socket_n);
//			if (len > 0) {
//				if (len > DATA_BUFFER_SIZE)
//					len = DATA_BUFFER_SIZE;
//				// 1. Obtener el puntero de lectura RX
//				rx_rd = getSn_RX_RD(socket_n);
//				// 2. Leer los datos desde el buffer RX
//				W5500_read_buffer((rx_rd << 8) + (0x03 + 4 * socket_n) << 3,
//						g_rx_buffer, len);
//				// 3. Actualizar el puntero de lectura RX
//				setSn_RX_RD(socket_n, rx_rd + len);
//				// 4. Enviar el comando RECV
//				w5500_write_register(Sn_CR,socket_n, Sn_CR_RECV);
//
//				// 5. Procesar los datos recibidos (ejemplo: eco)
//				memcpy(g_tx_buffer, g_rx_buffer, len);
//				// Comprueba que haya espacio en el buffer de transmisión
//				while (getSn_TX_FSR(socket_n) < len) {
//					HAL_Delay(1);
//				}
//				//Obtener el puntero de escritura de TX
//				tx_wr = getSn_TX_WR(socket_n);
//				//Escribir en el buffer de transmisión
//				W5500_write_buffer((tx_wr << 8) + (0x02 + 4 * socket_n) << 3,
//						g_tx_buffer, len);
//				//Actualizar el puntero de escritura
//				setSn_TX_WR(socket_n, tx_wr + len);
//				//Enviar el comando SEND
//				w5500_write_register(Sn_CR, socket_n, Sn_CR_SEND);
//				while (w5500_read_register(Sn_CR,socket_n) != 0x00) {
//					HAL_Delay(1);
//				} //Espera a que se complete el comando
//			}
//		} else if (status == SOCK_CLOSE_WAIT) {
//			w5500_write_register(Sn_CR,socket_n, Sn_CR_DISCON);
//			while (w5500_read_register(Sn_SR,socket_n) != SOCK_CLOSED) {
//				HAL_Delay(1);
//			}
//			w5500_write_register(Sn_CR,socket_n, Sn_CR_OPEN);
//			while (w5500_read_register(Sn_SR,socket_n) != SOCK_INIT) {
//				HAL_Delay(1);
//			}
//			w5500_write_register(Sn_CR, socket_n, Sn_CR_LISTEN);
//			while (w5500_read_register(Sn_SR, socket_n) != SOCK_LISTEN) {
//				HAL_Delay(1);
//			}
		}

void W5500Handler::w5500_reset() {
	w5500_write_register(MR, COMMON_REGISTER_BLOCK, 0x80);  // MR = Mode Register, bit 7 = RST (Reset)
	HAL_Delay(100);  // Espera un momento para que el reset se complete
}


uint8_t W5500Handler::w5500_check_connection() {
	uint8_t version = 0;
	version = w5500_read_register(VERSIONR, COMMON_REGISTER_BLOCK);
	if (version == 0x04) {  // El W5500 tiene un valor predeterminado de versión de 0x04
		return 1;  // Conexión exitosa
	} else {
		return 0;  // Error de conexión
	}
}


HAL_StatusTypeDef W5500Handler::w5500_write_register(uint16_t address, uint8_t block, uint8_t data) {
    HAL_StatusTypeDef status;
    uint8_t cmd[4]; // 16-bits offset address/8bit-BLOCK/8bit-data
    // Fase de dirección (2 bytes)
    cmd[0]= (address >> 8) & 0xFF;  // Byte alto de la dirección
    cmd[1] = address & 0xFF;         // Byte bajo de la dirección
    // Fase de control (1 byte)
    // BSB[4:0]: Block Select Bits (especificar el bloque de registro)
    // RWB: Read/Write Access Mode Bit (1 para escritura)
    // OM[1:0]: SPI Operation Mode Bits (00 para VDM)
    cmd[2] = (block << 3) | (1 << 2) | 0x00;
    // Fase de datos (1 byte)
    cmd[3] = data;                   // Dato a escribir

    HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_RESET); // Activa el pin CS (bajo)
    status = HAL_SPI_Transmit(spi, cmd, 4, HAL_MAX_DELAY); // Transmite los datos
    HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_SET); // Desactiva el pin CS (alto)

    return status;
}


uint8_t W5500Handler::w5500_read_register(uint16_t address, uint8_t register_block) {
    uint8_t cmd[4];
    uint8_t data = 0;
    HAL_StatusTypeDef status1;
    HAL_StatusTypeDef status2;
    // Fase de Dirección: Byte alto y byte bajo de la dirección
    cmd[0] = (address >> 8) & 0xFF;
    cmd[1] = address & 0xFF;
    // Fase de Control:  Bloque de registro, modo de lectura (RWB = 0) y modo SPI (OM[1:0] = 00)
    cmd[2] = ((register_block << 3) & 0xF8)| (0 << 2) | 0x00; // Desplaza el bloque de registro a los bits 7-3
    cmd[3] = 0;                       // Asegura que el bit RWB (Read/Write) sea 0 (lectura)
    HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_RESET); // Activa el pin CS (bajo)
    // Transmitir la dirección y el byte de control
    status1 = HAL_SPI_Transmit(spi, cmd, 3, 2000);
    // Recibir el dato
    status2 = HAL_SPI_Receive(spi, &cmd[3], 1, 2000);
    HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_SET); // Desactiva el pin CS (alto)
    if (status1 == HAL_OK && status2 == HAL_OK) {
    	data = cmd[3];
        return data;
    } else {
        return HAL_ERROR;
    }
}


void W5500Handler::w5500_write_tx_buffer(uint16_t address,
		uint8_t offset, uint8_t *data, uint16_t length) {

	HAL_StatusTypeDef status;
	uint8_t cmd[3] = { 0 };
	cmd[0] = (address >> 8) & 0xFF;  // Byte alto de la dirección
	cmd[1] = address & 0xFF;         // Byte bajo de la dirección
	// Fase de control (1 byte)
	// BSB[4:0]: Block Select Bits (especificar el bloque de registro)
	// RWB: Read/Write Access Mode Bit (1 para escritura)
	// OM[1:0]: SPI Operation Mode Bits (00 para VDM)
	cmd[2] = (offset << 3) | (1 << 2) | 0x00;

	HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_RESET); // Activa el pin CS (bajo)
	 status = HAL_SPI_Transmit(spi, cmd, 3, HAL_MAX_DELAY); // Transmite los datos
	HAL_SPI_Transmit(spi, data, length, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(nss.get_port(), nss.get_pin(), GPIO_PIN_SET); // Activa el pin CS (bajo)
}



void W5500Handler::W5500_read_buffer(uint32_t addr, uint8_t *buf,
		uint16_t len) {
	uint8_t header[3];
	header[0] = (addr >> 16) & 0xFF;
	header[1] = (addr >> 8) & 0xFF;
	header[2] = addr & 0xFF;
	HAL_SPI_Transmit(spi, header, 3, HAL_MAX_DELAY);
	HAL_SPI_Receive(spi, buf, len, HAL_MAX_DELAY);

}

//LECTURA de Puntero TX
uint16_t W5500Handler::get_tx_wr_ptr(uint8_t sn) {
	uint16_t val;
	val = w5500_read_register(Sn_TX_WR, sn) << 8;
	val |= w5500_read_register(Sn_TX_WR + 1, sn);
	return val;
}
//LECTURA Posicion Puntero RX
uint16_t W5500Handler::get_RX_RD(uint8_t sn) {
	uint16_t val;
	val = w5500_read_register(Sn_RX_RD, sn) << 8;
	val |= w5500_read_register(Sn_RX_RD + 1, sn);
	return val;
}
//Tamaño de data recibida
uint16_t W5500Handler::rx_size_recive(uint8_t sn) {
	uint16_t val;
	val = w5500_read_register(Sn_RX_RSR, sn) << 8;
	val |= w5500_read_register(Sn_RX_RSR + 1, sn);
	return val;
}
//ESPACIO LIBRE del Socket n TX
uint16_t W5500Handler::tx_free_size(uint8_t sn) {
	uint16_t val;
	val = w5500_read_register(Sn_TX_FSR, sn) << 8;
	val |= w5500_read_register(Sn_TX_FSR + 1, sn);
	return val;
}
//Escritura de Puntero TX
void W5500Handler::set_tx_WR_ptr(uint8_t sn, uint16_t txwr) {
	w5500_write_register(Sn_TX_WR, sn, (txwr >> 8) & 0xFF);
	w5500_write_register(Sn_TX_WR + 1, sn, txwr & 0xFF);
}
//Escritura Posicion Puntero RX
void W5500Handler::set_RX_RD(uint8_t sn, uint16_t rxrd) {
	w5500_write_register(Sn_RX_RD, sn, (rxrd >> 8) & 0xFF);
	w5500_write_register(Sn_RX_RD + 1, sn, rxrd & 0xFF);
}



