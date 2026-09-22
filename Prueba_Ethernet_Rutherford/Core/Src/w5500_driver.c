/**
 * Minimal W5500 driver
 */
#include "w5500_driver.h"
#include "main.h"
#include "gpio.h"

void W5500_HardwareReset(void)
{
    HAL_GPIO_WritePin(NRST_W5500_GPIO_Port, NRST_W5500_Pin, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(NRST_W5500_GPIO_Port, NRST_W5500_Pin, GPIO_PIN_SET);
}

void debug_print(const char* m)
{
    /* no-op for now */
    (void)m;
}

uint8_t debug_is_ready(void) { return 0; }
void W5500_SetNetwork(uint8_t* a, uint8_t* b, uint8_t* c, uint8_t* d)
{ (void)a; (void)b; (void)c; (void)d; }
int8_t W5500_Init(void) { return 0; }
