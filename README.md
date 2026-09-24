# Pruebas Ethernet W5500 — STM32G474QET6, STM32G030K8T6 y STM32G474CBT6

Pruebas de comunicación Ethernet (módulo **W5500** vía SPI) entre un microcontrolador STM32 y un PC.

## Proyectos

| Carpeta | MCU | Conexión W5500 (SPI) | Descripción |
|---|---|---|---|
| `Prueba_Ethernet_Rutherford/` | STM32G474QET6 | SPI1: PA5=SCK, PA6=MISO, PA7=MOSI, PA4=SCN, PC5=NRST | Keep-alive UDP `"KA"` cada 10 s + respuesta `"RECIBIDO"` |
| `Prueba_Ethernet_G030/` | STM32G030K8T6 | SPI1: PA5=SCK, PA6=MISO, PA7=MOSI, PB0=SCN, PA8=NRST | Test de recepción UDP por broadcast |
| `g474-Gate-BV/` | STM32G474CBT6 | SPI3: PB3=SCK, PB4=MISO, PB5=MOSI, PA4=SCN, PB6=NRST | Keep-alive UDP `"KA"` + `"RECIBIDO"` (versión SPI3) |

## Configuración de red (W5500)

- MAC: `02:00:00:00:00:01`
- IP local: `192.168.1.50`
- Máscara: `255.255.255.0`
- Gateway: `192.168.1.1`
- IP del PC destino: `192.168.1.10`
- Puerto UDP: `5000`

## Comportamiento (G474)

1. Cada **10 segundos** envía `"KA"` al PC (`192.168.1.10:5000`).
2. Si recibe un mensaje UDP, responde `"RECIBIDO"` al emisor.

Las constantes editables (IP del PC, puerto, intervalo) están al inicio de `Core/Src/main.c`.

## Notas de depuración

- Variables de estado leíbles por SWD con `STM32_Programmer_CLI` (`mode=HOTPLUG`):
  `g_ver` (VERSIONR, debe ser `0x04`), `g_phy`, `g_socket`, `g_rsr`, `g_tx_count`, `g_rx_count`.
  Las direcciones están en el archivo `.map` de cada compilación.
- El W5500 no responde `ping` ICMP (solo ARP + TCP/UDP).
