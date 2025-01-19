
#include "Bsp_oled.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#define SSD1315_CMD   0    //write command
#define SSD1315_DATA  1    //write data

#define SSD1315_WRITE   0    //I2C write mode
#define SSD1315_READ    1    //I2C read mode

#define SSD1315_DEVICE_ADDRESS     0x1E
#define SSD1315_DEVICE_CMD         (uint8_t)(SSD1315_DEVICE_ADDRESS << 2) | (SSD1315_CMD << 1)
#define SSD1315_DEVICE_DATA        (uint8_t)(SSD1315_DEVICE_ADDRESS << 2) | (SSD1315_DATA << 1)



#define SSD1315_I2C_PORT           I2C1 //ssd1315 use i2c2

static void Ssd1315_PinInit(void);
static uint16_t Ssd1315_StartCnt = 0;


void Ssd1315_Init(void)
{
    Ssd1315_PinInit();

    Ssd1315_WriteCmd(0xAE);
    Ssd1315_WriteCmd(0xD5);
    Ssd1315_WriteCmd(0x80);
    Ssd1315_WriteCmd(0xA8);
    Ssd1315_WriteCmd(0x3F);
    Ssd1315_WriteCmd(0xD3);
    Ssd1315_WriteCmd(0x00);
    Ssd1315_WriteCmd(0x40);
    Ssd1315_WriteCmd(0x8D);
    Ssd1315_WriteCmd(0x14);
    Ssd1315_WriteCmd(0x20);
    Ssd1315_WriteCmd(0x00);
    Ssd1315_WriteCmd(0xA1);
    Ssd1315_WriteCmd(0xC8);
    Ssd1315_WriteCmd(0xDA);
    Ssd1315_WriteCmd(0x12);
    Ssd1315_WriteCmd(0x81);
    Ssd1315_WriteCmd(0xCF);
    Ssd1315_WriteCmd(0xA4);
    Ssd1315_WriteCmd(0xA6);
    Ssd1315_WriteCmd(0xD5);
    Ssd1315_WriteCmd(0x80);
    Ssd1315_WriteCmd(0xAF);
}

static void Ssd1315_PinInit(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    GPIO_InitTypeDef Gpio_InitStruct;
    I2C_InitTypeDef I2c_InitStruct;

    Gpio_InitStruct.GPIO_Pin = GPIO_Pin_6;
    Gpio_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    Gpio_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &Gpio_InitStruct);

    Gpio_InitStruct.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &Gpio_InitStruct);

    I2c_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2c_InitStruct.I2C_ClockSpeed = 10000;
    I2c_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2c_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2c_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2c_InitStruct.I2C_OwnAddress1 = 0xF0;

    I2C_Init(SSD1315_I2C_PORT, &I2c_InitStruct);
    I2C_Cmd(SSD1315_I2C_PORT, ENABLE);

}

void Ssd1315_WriteCmd(uint8_t cmd)
{
    //genrate the start signal.
    I2C_GenerateSTART(SSD1315_I2C_PORT, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(SSD1315_I2C_PORT, SSD1315_DEVICE_CMD, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(SSD1315_I2C_PORT, cmd);

    //check if data is transmitted
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    I2C_AcknowledgeConfig(SSD1315_I2C_PORT, DISABLE);

    I2C_GenerateSTOP(SSD1315_I2C_PORT, ENABLE);

    I2C_AcknowledgeConfig(SSD1315_I2C_PORT, ENABLE);
}

void Ssd1315_WriteData(uint8_t data)
{
    //genrate the start signal.
    I2C_GenerateSTART(SSD1315_I2C_PORT, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(SSD1315_I2C_PORT, SSD1315_DEVICE_DATA, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(SSD1315_I2C_PORT, data);

    //check if data is transmitted
    while (I2C_CheckEvent(SSD1315_I2C_PORT, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    I2C_AcknowledgeConfig(SSD1315_I2C_PORT, DISABLE);

    I2C_GenerateSTOP(SSD1315_I2C_PORT, ENABLE);

    I2C_AcknowledgeConfig(SSD1315_I2C_PORT, ENABLE);
}

void Ssd1315_OledSetPosition(uint8_t page, uint8_t col)
{
    Ssd1315_WriteCmd(0xB0 + page);
    Ssd1315_WriteCmd(col & 0xF);
    Ssd1315_WriteCmd(0x10 + (col >> 4));
}


