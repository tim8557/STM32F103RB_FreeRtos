#ifndef _BSP_XPT2046__
#define _BSP_XPT2046__

#include "stdint.h"

#define XPT2046_RES_BUFFER_SIZE   2

void Xpt2046_Runnable_1ms(void);
void Xpt2046_PinInit(void);
void Xpt2046_SpiRead_Xposition(uint8_t* res, uint8_t len);
void Xpt2046_SpiRead_Yposition(uint8_t* res, uint8_t len);
void Xpt2046_SpiRead_Xposition_Yposition(uint16_t* Xres, uint16_t* Yres);

#endif