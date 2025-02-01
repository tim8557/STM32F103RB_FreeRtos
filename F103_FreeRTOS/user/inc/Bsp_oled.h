
/*file name Bsp_oled.h*/

#ifndef _BSP_SSD1315__
#define _BSP_SSD1315__

#include "stdint.h"

void Ssd1315_Init(void);
void Ssd1315_InitCmd(void);
void Ssd1315_WriteCmd(uint8_t cmd);
void Ssd1315_WriteData(uint8_t data);
void Ssd1315_OledSetPosition(uint8_t page, uint8_t col);

#endif