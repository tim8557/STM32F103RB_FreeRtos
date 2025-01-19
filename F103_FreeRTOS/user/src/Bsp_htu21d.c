
#include "Bsp_htu21d.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

static void Htu21d_ReadRegisterValue(I2C_TypeDef* I2Cx, Htu21d_Temp_Type* res, uint8_t cmd);


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

void Htu21d_Gettemperature(I2C_TypeDef* I2Cx, Htu21d_Temp_Type* res)
{
    float cal_res;
    int32_t data_shift;

    Htu21d_ReadRegisterValue(I2Cx, res, BSP_HTU21D_TRIGGER_TEMP_MEASURE);
    
    data_shift = (res->res_buff[0] << 8) | (res->res_buff[1]);
    cal_res = (data_shift * 175.72)/65536 - 46.85;

    res->temp = cal_res;
}

void Htu21d_Gethumidity(I2C_TypeDef* I2Cx, Htu21d_Humidity_Type* res)
{
    uint32_t data_shift;
    uint32_t cal_res;

    Htu21d_ReadRegisterValue(I2Cx, res, BSP_HTU21D_TRIGGER_HUMIDITY_MEASURE);
    
    data_shift = (res->res_buff[0] << 8) | (res->res_buff[1]);
    cal_res = (data_shift * 125)/65536 - 6;

    res->humidity = cal_res;
}


static void Htu21d_ReadRegisterValue(I2C_TypeDef* I2Cx, Htu21d_Temp_Type* res, uint8_t cmd)
{
    uint8_t num_read;

    //genrate the start signal.
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(I2Cx, BSP_HTU21D_DEVICE_ADDRESS, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(I2Cx, cmd);

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

    for (num_read = 0 ; num_read < BSP_HTU21D_BUFFER_SIZE; num_read++) 
    {
        //wait until the resule is received
        while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);

        //reed out data from the result buffer
        res->res_buff[num_read] = I2C_ReceiveData(I2Cx);
    }

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}

