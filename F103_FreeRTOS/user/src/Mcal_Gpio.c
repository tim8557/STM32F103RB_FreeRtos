#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#define PIN8           (8u)
#define PIN9           (9u)

#define OUTPUT_HIGH    (Bit_SET)
#define OUTPUT_LOW     (Bit_RESET)


void McalPort_GpioAInit(void);

void McalPort_GpioAInit(void)
{
    GPIO_InitTypeDef GPIOA_InitStruct;

    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIOA_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIOA_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIOA_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIOA_InitStruct);
}

void McalPort_GpioConfig(void)
{
    McalPort_GpioAInit();
}

void McalPort_SplashLedHigh(void)
{
   GPIO_WriteBit(GPIOB, GPIO_Pin_10, OUTPUT_HIGH);
}

void McalPort_SplashLedLow(void)
{
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, OUTPUT_LOW);
}