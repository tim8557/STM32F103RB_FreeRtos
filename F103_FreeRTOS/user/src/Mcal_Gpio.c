#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define PIN8           (8u)
#define PIN9           (9u)

#define OUTPUT_HIGH    (Bit_SET)
#define OUTPUT_LOW     (Bit_RESET)


void McalPort_GpioAInit(void);

void McalPort_GpioAInit(void)
{
    GPIO_InitTypeDef GPIOB_InitStruct;

    GPIOB_InitStruct.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_8;
    GPIOB_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOB_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIOB_InitStruct);
}

void McalPort_GpioConfig(void)
{
    McalPort_GpioAInit();
}

void McalPort_SplashLedHigh(GPIO_TypeDef* port, uint16_t pin)
{
   GPIO_WriteBit(port, pin, Bit_RESET);
}

void McalPort_SplashLedLow(GPIO_TypeDef* port, uint16_t pin)
{
    GPIO_WriteBit(port, pin, Bit_SET);
}