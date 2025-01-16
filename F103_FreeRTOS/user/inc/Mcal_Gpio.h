/* file name Mcal_Gpio.h */

void McalPort_GpioConfig(void);
void McalPort_SplashLedHigh(GPIO_TypeDef* port, uint16_t pin);
void McalPort_SplashLedLow(GPIO_TypeDef* port, uint16_t pin);