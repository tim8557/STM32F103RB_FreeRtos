
#ifndef _BSP_HTU21D__
#define _BSP_HTU21D__

#include "stm32f10x.h"
#include "stdint.h"

#define BSP_HTU21D_DEVICE_ADDRESS            0x80
#define BSP_HTU21D_TRIGGER_TEMP_MEASURE      0xE3
#define BSP_HTU21D_TRIGGER_HUMIDITY_MEASURE  0xE3

#define BSP_HTU21D_TEMP_BUFFER_SIZE          0x3

void Htu21d_Gettemperature(I2C_TypeDef* I2Cx, uint8_t* res_buff);
void Htu21d_Init(void);

#endif