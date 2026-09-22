He ampliado el documento anterior para incluir la **versión más detallada y completa** de todas las tablas presentes en el datasheet del W5500, incluyendo el desglose de bits de los registros, estados temporales y especificaciones de temporización avanzadas.

---

# Informe Técnico Integral de Tablas: Controlador Ethernet W5500

### 1. Asignación de Pines y Configuración de Hardware

**Tabla 1: Notación de tipos de pines**

| Tipo | Descripción |
| :--- | :--- |
| I | Entrada (Input) |
| O | Salida (Output) |
| I/O | Entrada / Salida |
| A | Analógico |
| PWR | Alimentación 3.3V |
| GND | Tierra |

**Tabla 2: Descripción detallada de pines**

| Pin | Símbolo | Tipo | Descripción Técnica |
| :--- | :--- | :--- | :--- |
| 1, 2 | TXN, TXP | AO | Par diferencial de transmisión hacia el medio. |
| 5, 6 | RXN, RXP | AI | Par diferencial de recepción desde el medio. |
| 10 | EXRES1 | AI/O | Resistencia externa de 12.4KΩ (1%) para sesgo interno. |
| 18 | VBG | AO | Voltaje Band Gap (1.2V a 25℃). Debe dejarse flotando. |
| 20 | TOCAP | AO | Capacitor de referencia externa de 4.7uF. |
| 22 | 1V2O | AO | Salida del regulador interno de 1.2V (Capacitor 10nF). |
| 24 | SPDLED | O | Low: 100Mbps; High: 10Mbps. |
| 25 | LINKLED | O | Low: Enlace establecido; High: Sin enlace. |
| 26 | DUPLED | O | Low: Full-duplex; High: Half-duplex. |
| 27 | ACTLED | O | Low: Actividad (CRS activo); High: Sin actividad. |
| 32 | SCSn | I | Chip Select SPI (Activo bajo). Tiene Pull-up. |
| 36 | INTn | O | Salida de interrupción (Activo bajo). |
| 37 | RSTn | I | Reset de hardware (Mínimo 500 us). Tiene Pull-up. |
| 43-45| PMODE[2:0]| I | Configuración de modo de la PHY (Pull-ups internos). |

**Tabla 3: Modos de operación de la PHY (PMODE)**

| PMODE2 | PMODE1 | PMODE0 | Descripción del Modo |
| :---: | :---: | :---: | :--- |
| 0 | 0 | 0 | 10BT Half-duplex, Auto-negociación desactivada |
| 0 | 0 | 1 | 10BT Full-duplex, Auto-negociación desactivada |
| 0 | 1 | 0 | 100BT Half-duplex, Auto-negociación desactivada |
| 0 | 1 | 1 | 100BT Full-duplex, Auto-negociación desactivada |
| 1 | 0 | 0 | 100BT Half-duplex, Auto-negociación activada |
| 1 | 1 | 1 | Todo capaz, Auto-negociación activada |

### 2. Estructura de Control SPI y Memoria

**Tabla 4: Selección de Bloques (BSB [4:0])**

| BSB [4:0] | Bloque Seleccionado | BSB [4:0] | Bloque Seleccionado |
| :--- | :--- | :--- | :--- |
| 00000 | Registro Común | 01101 | Registros Socket 3 |
| 00001 | Registros Socket 0 | 01110 | Buffer TX Socket 3 |
| 00010 | Buffer TX Socket 0 | 01111 | Buffer RX Socket 3 |
| 00011 | Buffer RX Socket 0 | 10001 | Registros Socket 4 |
| 00101 | Registros Socket 1 | 11101 | Registros Socket 7 |
| 00110 | Buffer TX Socket 1 | 11110 | Buffer TX Socket 7 |
| 00111 | Buffer RX Socket 1 | 11111 | Buffer RX Socket 7 |

**Tabla 5: Modos de Operación SPI (OM [1:0])**

| OM [1:0] | Significado | Longitud de Fase de Datos |
| :--- | :--- | :--- |
| 00 | Modo Variable (VDM) | N-Bytes (controlado por SCSn) |
| 01 | Modo Fijo (FDM) | 1 Byte |
| 10 | Modo Fijo (FDM) | 2 Bytes |
| 11 | Modo Fijo (FDM) | 4 Bytes |

### 3. Mapa Detallado de Registros (Offsets)

**Tabla 6: Registros Comunes (Common Register Block)**

| Offset | Símbolo | Descripción | Offset | Símbolo | Descripción |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 0x0000 | MR | Modo | 0x0017 | SIR | Interrupción Socket |
| 0x0001 | GAR | Gateway IP | 0x0018 | SIMR | Máscara Int. Socket |
| 0x0005 | SUBR | Máscara Subred | 0x0019 | RTR | Tiempo de Reintento |
| 0x0009 | SHAR | Dirección MAC | 0x001B | RCR | Cuenta de Reintento |
| 0x000F | SIPR | Dirección IP | 0x002E | PHYCFGR | Configuración PHY |
| 0x0015 | IR | Interrupción | 0x0039 | VERSIONR | Versión del Chip |

**Tabla 7: Registros de Socket (Socket n Register Block)**

| Offset | Símbolo | Descripción | Offset | Símbolo | Descripción |
| :--- | :--- | :--- | :--- | :--- | :--- |
| 0x0000 | Sn_MR | Modo Socket | 0x001E | Sn_RXBUF_SIZE | Tamaño Buffer RX |
| 0x0001 | Sn_CR | Comando | 0x001F | Sn_TXBUF_SIZE | Tamaño Buffer TX |
| 0x0002 | Sn_IR | Interrupción | 0x0020 | Sn_TX_FSR | Tamaño Libre TX |
| 0x0003 | Sn_SR | Estado Socket | 0x0022 | Sn_TX_RD | Puntero Lectura TX |
| 0x0004 | Sn_PORT | Puerto Origen | 0x0024 | Sn_TX_WR | Puntero Escritura TX |
| 0x000C | Sn_DIPR | IP Destino | 0x0026 | Sn_RX_RSR | Tamaño Recibido RX |
| 0x0010 | Sn_DPORT | Puerto Destino | 0x0028 | Sn_RX_RD | Puntero Lectura RX |

### 4. Desglose de Bits de Registros Críticos

**Tabla 8: Registro de Modo (MR)**

| Bit | Símbolo | Valor | Descripción |
| :---: | :--- | :---: | :--- |
| 7 | RST | 1 | Reset de software (auto-limpiable) |
| 5 | WOL | 1 | Habilita Wake on LAN |
| 4 | PB | 1 | Habilita bloqueo de Ping |
| 3 | PPPoE | 1 | Habilita modo PPPoE (para ADSL) |
| 1 | FARP | 1 | Habilita Force ARP (envía ARP en cada dato) |

**Tabla 9: Registro de Configuración PHY (PHYCFGR)**

| Bit | Símbolo | Valor | Descripción |
| :---: | :--- | :---: | :--- |
| 7 | RST | 0 | Reset de la PHY interna |
| 6 | OPMD | 1 | Configura PHY vía bits OPMDC (1) o pines PMODE (0) |
| 5-3 | OPMDC | 110 | Modo Power Down (Ejemplo) |
| 2 | DPX | 1/0 | Estado Duplex (Solo lectura): Full (1), Half (0) |
| 1 | SPD | 1/0 | Estado Velocidad (Solo lectura): 100M (1), 10M (0) |
| 0 | LNK | 1/0 | Estado Enlace (Solo lectura): Up (1), Down (0) |

**Tabla 10: Registro de Modo de Socket (Sn_MR)**

| Bit | Símbolo | Valor | Descripción |
| :---: | :--- | :---: | :--- |
| 7 | MULTI | 1 | Habilita Multicasting (solo UDP) |
| 6 | BCASTB | 1 | Bloquea paquetes Broadcast |
| 5 | ND / MC | 1 | No Delayed ACK (TCP) / Versión IGMP (UDP) |
| 3-0 | P[3:0] | 0001 | Protocolo: TCP (0001), UDP (0010), MACRAW (0100) |

**Tabla 11: Códigos de Comando de Socket (Sn_CR)**

| Valor | Comando | Descripción Técnica |
| :---: | :--- | :--- |
| 0x01 | OPEN | Inicializa el socket según Sn_MR. |
| 0x02 | LISTEN | Modo servidor TCP; espera SYN. |
| 0x04 | CONNECT | Modo cliente TCP; envía SYN. |
| 0x08 | DISCON | Cierre activo (envía FIN) o pasivo. |
| 0x10 | CLOSE | Cierra el socket inmediatamente sin proceso FIN. |
| 0x20 | SEND | Transmite datos del buffer TX. |
| 0x40 | RECV | Notifica el fin del procesamiento de datos en RX. |

### 5. Estados de Socket y Memoria

**Tabla 12: Registro de Estado del Socket (Sn_SR)**

| Valor | Estado | Significado |
| :---: | :--- | :--- |
| 0x00 | SOCK_CLOSED | Socket liberado o cerrado. |
| 0x13 | SOCK_INIT | Socket abierto en modo TCP. |
| 0x14 | SOCK_LISTEN | Servidor TCP esperando conexión. |
| 0x17 | SOCK_ESTABLISHED| Conexión TCP establecida; transferencia permitida. |
| 0x1C | SOCK_CLOSE_WAIT | Recibido FIN del par; cierre pendiente. |
| 0x22 | SOCK_UDP | Socket abierto en modo UDP. |
| 0x42 | SOCK_MACRAW | Socket 0 abierto en modo MACRAW. |

**Tabla 13: Estados Temporales de Socket**

| Valor | Estado | Situación |
| :---: | :--- | :--- |
| 0x15 | SOCK_SYNSENT | SYN enviado, esperando SYN/ACK. |
| 0x16 | SOCK_SYNRECV | SYN recibido, enviando SYN/ACK. |
| 0x18 | SOCK_FIN_WAIT | Proceso de desconexión en curso. |
| 0x1D | SOCK_LAST_ACK | Esperando respuesta final al pedido de desconexión. |

**Tabla 14: Configuración de Búferes RX/TX**

| Valor (dec) | Tamaño de Buffer |
| :---: | :---: |
| 0 | 0 KB |
| 1 | 1 KB |
| 2 | 2 KB (Valor por defecto) |
| 4 | 4 KB |
| 8 | 8 KB |
| 16 | 16 KB |

### 6. Especificaciones Eléctricas y Temporización

**Tabla 15: Características DC (Ta = -40 a 85°C)**

| Símbolo | Parámetro | Mín | Máx | Unidad |
| :--- | :--- | :---: | :---: | :---: |
| VDD | Voltaje de alimentación | 2.97 | 3.63 | V |
| VIH | Voltaje entrada nivel alto | 2.0 | 5.5 | V |
| VIL | Voltaje entrada nivel bajo | -0.3 | 0.8 | V |
| RPU | Resistencia Pull-up (SCSn, RSTn) | 50 | 112 | Kohm|
| VOL | Voltaje salida nivel bajo (8mA) | | 0.4 | V |
| VOH | Voltaje salida nivel alto (8mA) | 2.4 | | V |

**Tabla 16: Disipación de Potencia**

| Condición | Corriente (Typ) | Unidad |
| :--- | :---: | :---: |
| 100M Link | 128 | mA |
| 100M Transmitiendo | 132 | mA |
| 10M Transmitiendo | 79 | mA |
| Modo Power Down | 13 | mA |

**Tabla 17: Temporización SPI (Frecuencia máx 80/33.3 MHz)**

| Símbolo | Descripción | Mín | Unidad |
| :--- | :--- | :---: | :---: |
| TWH | Tiempo de nivel alto SCLK | 6 | ns |
| TWL | Tiempo de nivel bajo SCLK | 6 | ns |
| TCSS | Tiempo de Setup SCSn | 5 | ns |
| TCSH | Tiempo de Hold SCSn | 5 | ns |
| TDS | Tiempo de Setup Datos Entrada | 3 | ns |
| TDH | Tiempo de Hold Datos Entrada | 3 | ns |

