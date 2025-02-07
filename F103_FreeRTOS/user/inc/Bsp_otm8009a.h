
#ifndef _BSP_OTM8009A_
#define _BSP_OTM8009A_

#include "stdint.h"

#define USE_HORIZONTAL  	       0  

#define LCD_W 480
#define LCD_H 800

//define the lcd color
#define LCD_WHITE       0xFFFF
#define LCD_BLACK      	0x0000	  
#define LCD_BLUE       	0x001F  
#define LCD_BRED        0XF81F
#define LCD_GRED 	    0XFFE0
#define LCD_GBLUE	    0X07FF
#define LCD_RED         0xF800 
#define LCD_MAGENTA     0xF81F
#define LCD_GREEN       0x07E0
#define LCD_CYAN        0x7FFF
#define LCD_YELLOW      0xFFE0
#define LCD_BROWN 		0XBC40
#define LCD_BRRED 		0XFC07
#define LCD_GRAY  		0X8430 
#define LCD_ORANGE      0xFD20
#define LCD_INDIGO      0x4810
#define LCD_PURPLE      0x8010


void Otm8009a_PinInit(void);
void Otm8009a_Reset_Clr(void);
void Otm8009a_Reset_Set(void);
void Otm8009a_Init_Command(void);
void Otm8009a_Write_Command(uint16_t value);
void Otm8009a_Set_Direction_and_Clear(uint8_t direction, uint16_t color);
void Otm8009a_Set_Led(void);
void Otm8009a_DrawPixel(uint16_t sx, uint16_t sy, uint16_t color);
void Otm8009a_RainbowTest(void);
void Otm8009a_FillBlock(uint16_t xstart, uint16_t xend, uint16_t ystart, uint16_t yend, uint16_t color);
void Otm8009a_ShowString(char* string, uint16_t x,uint16_t y,uint16_t font_color, uint16_t back_color, uint8_t size);

#endif