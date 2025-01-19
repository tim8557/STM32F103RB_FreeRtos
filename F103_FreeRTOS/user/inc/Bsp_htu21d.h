
#ifndef _BSP_HTU21D__
#define _BSP_HTU21D__

#include "stm32f10x.h"
#include "stdint.h"

#define BSP_HTU21D_DEVICE_ADDRESS            0x80
#define BSP_HTU21D_TRIGGER_TEMP_MEASURE      0xE3
#define BSP_HTU21D_TRIGGER_HUMIDITY_MEASURE  0xE5

#define BSP_HTU21D_BUFFER_SIZE          0x3

typedef struct 
{
    uint8_t res_buff[BSP_HTU21D_BUFFER_SIZE];
    float temp;
    uint8_t crc;
}Htu21d_Temp_Type;

typedef struct 
{
    uint8_t res_buff[BSP_HTU21D_BUFFER_SIZE];
    uint32_t humidity;
    uint8_t crc;
}Htu21d_Humidity_Type;


void Htu21d_Gettemperature(I2C_TypeDef* I2Cx, Htu21d_Temp_Type* res);
void Htu21d_Gethumidity(I2C_TypeDef* I2Cx, Htu21d_Humidity_Type* res);
void Htu21d_Init(void);

#endif