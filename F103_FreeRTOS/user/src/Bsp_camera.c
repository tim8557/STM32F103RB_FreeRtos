
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"

#define OV7670_DEVICE_ADDRESS   0x42

#define OV7670_DEVICE_ADDRESS

static void NVIC_Configuration(void);

void Camera_Init(void)
{
    RCC_APB1PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );

    //GPIO init
    GPIO_InitTypeDef  GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
    
    //EXIT init
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;

    /* EXTI为中断模式 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 上升沿中断 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    /* 使能中断 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_Configuration();
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 配置NVIC为优先级组1 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

    /* 配置中断源：按键1 */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    /* 配置抢占优先级：1 */
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    /* 配置子优先级：1 */
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    /* 使能中断通道 */
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Camera_Send_Command(I2C_TypeDef* I2Cx, uint8_t register_addr, uint8_t cmd)
{

    uint8_t num_read;

    //genrate the start signal.
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(I2Cx, OV7670_DEVICE_ADDRESS, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(I2Cx, cmd);

    //check if data is transmitted
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}