
#include "Bsp_htu21d.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

static void Htu21d_ReadRegisterValue(I2C_TypeDef* I2Cx, uint8_t* res_buff);
static uint8_t test_buffer[BSP_HTU21D_TEMP_BUFFER_SIZE] = {0};

void Htu21d_Init(void)
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
    I2c_InitStruct.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2c_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2c_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2c_InitStruct.I2C_OwnAddress1 = BSP_HTU21D_DEVICE_ADDRESS;

    I2C_Init(I2C1, &I2c_InitStruct);
    I2C_Cmd(I2C1, ENABLE);
}

void Htu21d_Gettemperature(I2C_TypeDef* I2Cx, uint8_t* res_buff)
{
    Htu21d_ReadRegisterValue(I2Cx, res_buff);
}

static void Htu21d_ReadRegisterValue(I2C_TypeDef* I2Cx, uint8_t* res_buff)
{
    uint8_t num_read;
    uint8_t data_temp = 0;

    //genrate the start signal.
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(I2Cx, BSP_HTU21D_DEVICE_ADDRESS, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(I2Cx, BSP_HTU21D_TRIGGER_TEMP_MEASURE);

    //check if data is transmitted
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    //start bit for register read
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address
    I2C_Send7bitAddress(I2Cx, BSP_HTU21D_DEVICE_ADDRESS, I2C_Direction_Receiver);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) == ERROR);

    for (num_read = 0 ; num_read < BSP_HTU21D_TEMP_BUFFER_SIZE; num_read++) 
    {
        //wait until the resule is received
        while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);

        //reed out data from the result buffer
        data_temp = I2C_ReceiveData(I2Cx);
        test_buffer[num_read] = data_temp;
        *(res_buff + num_read) = data_temp;
    }

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}

