
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_tim.h"

#define OV7670_DEVICE_ADDRESS_WRITE   0x42
#define OV7670_DEVICE_ADDRESS_READ    0x43
#define CAMERA_PORTC_PIN        GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7

//OV7670 camera register
#define CAMERA_REG_COM7         0x12
#define CAMERA_REG_COM15        0x40
#define CAMERA_REG_CLKRC        0x11
#define CAMERA_REG_DBLV         0x6B
#define CAMERA_REG_EXHCH        0x2A
#define CAMERA_REG_EXHCL        0x2B
#define CAMERA_REG_DM_LNL       0x92
#define CAMERA_REG_DM_LNH       0x93
#define CAMERA_REG_COM11        0x3B

//OV7670 command value
#define CAMERA_COM7_VALUE       0x5  /* RGB selection */
#define CAMERA_COM15_VALUE      0xD0 /* RGB565 */
#define CAMERA_CLKRC_VALUE      0x00
#define CAMERA_DBLV_VALUE       0xA
#define CAMERA_EXHCH_VALUE      0x00
#define CAMERA_EXHCL_VALUE      0x00
#define CAMERA_DM_LNL_VALUE     0x00
#define CAMERA_DM_LNH_VALUE     0x00
#define CAMERA_COM11_VALUE      0xA

//Pin name define PA1 PA2 PA3
#define CAMERA_RESTE_PIN        GPIO_Pin_2
#define CAMERA_PWDN_PIN         GPIO_Pin_3
#define CAMERA_PCLK_PIN         GPIO_Pin_4
#define CAMERA_HREF_PIN         GPIO_Pin_4

static void Camera_PinInit(void);
static void Camera_ExternelInt_Init(void);
static void NVIC_Configuration(void);
static void Camera_Send_Command(I2C_TypeDef* I2Cx, uint8_t register_addr, uint8_t cmd);
static void Camera_Pwm_SetDuty(uint16_t duty);
static void Camera_Pwm_Configuration(uint16_t Prescaler, uint16_t Period);
static void Camera_Sccb_Init(void);
static uint8_t Camera_Read_Byte(I2C_TypeDef* I2Cx, uint8_t register_addr);

static uint8_t Device_id = 0;

static const uint8_t Camera_Register_and_Value_List[][2] =
{
    {CAMERA_REG_COM15, CAMERA_COM15_VALUE},
    {CAMERA_REG_CLKRC, CAMERA_CLKRC_VALUE},
    {CAMERA_REG_DBLV, CAMERA_DBLV_VALUE},
    {CAMERA_REG_EXHCH, CAMERA_EXHCH_VALUE},
    {CAMERA_REG_EXHCL, CAMERA_EXHCL_VALUE},
    {CAMERA_REG_DM_LNL, CAMERA_DM_LNL_VALUE},
    {CAMERA_REG_DM_LNH, CAMERA_DM_LNH_VALUE},
    {CAMERA_REG_COM11, CAMERA_COM11_VALUE}
};

void Camera_Init(void)
{
    //camera used gpio init
    Camera_PinInit();

    //set 24mhz output clock
    Camera_Pwm_Configuration(0, 2);

    //set duty cycle of 24mhz output clock
    Camera_Pwm_SetDuty(1);

    //SCCB init
    Camera_Sccb_Init();

    //Enable Camera
    GPIO_SetBits(GPIOA, CAMERA_RESTE_PIN);
    GPIO_ResetBits(GPIOA, CAMERA_PWDN_PIN);
    
    //setting register value
    Camera_InitCommand();

    //external interrupt init
    Camera_ExternelInt_Init();
}


static void Camera_PinInit(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    //GPIO init
    GPIO_InitTypeDef  GPIO_InitStructure;
    //External interrupt
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

    //port C VGA input
    GPIO_InitStructure.GPIO_Pin = CAMERA_PORTC_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //24mhz clock output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    //SCCB communication
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //RESTE pin output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //HREF and PCLK input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void Camera_ExternelInt_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
    
    //EXIT init
    EXTI_InitStructure.EXTI_Line = EXTI_Line13;

    /* EXTI为中断模式 */
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    /* 上升沿中断 */
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    /* 使能中断 */
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_Configuration();
}

void Camera_Read_Device_ID(void)
{
    Device_id = Camera_Read_Byte(I2C1, 0xb);
}

void Camera_InitCommand(void)
{ 
    uint8_t pos = 0;

    for (pos = 0; pos < 9; pos++)
    {
        Camera_Send_Command(I2C1, Camera_Register_and_Value_List[pos][0], Camera_Register_and_Value_List[pos][1]);
    }
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

static void Camera_Send_Command(I2C_TypeDef* I2Cx, uint8_t register_addr, uint8_t cmd)
{
    //genrate the start signal.
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(I2Cx, OV7670_DEVICE_ADDRESS_WRITE, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);

    //register address
    I2C_SendData(I2Cx, register_addr);

    //check if data is transmitted
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    //register value
    I2C_SendData(I2Cx, cmd);

    //check if data is transmitted
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}

static void Camera_Pwm_Configuration(uint16_t Prescaler, uint16_t Period)
{
    //TIM Init
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = Period;
    TIM_TimeBaseStructure.TIM_Prescaler = Prescaler;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    //TIM OC configuration
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    //TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    //TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    //TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    //TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
    TIM_Cmd(TIM2, ENABLE);
}

static void Camera_Pwm_SetDuty(uint16_t duty)
{
    TIM_SetCompare2(TIM2, duty);
}

static void Camera_Sccb_Init(void)
{
    I2C_InitTypeDef I2c_InitStruct;

    I2c_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2c_InitStruct.I2C_ClockSpeed = 10000;
    I2c_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2c_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2c_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2c_InitStruct.I2C_OwnAddress1 = OV7670_DEVICE_ADDRESS_WRITE;

    I2C_Init(I2C1, &I2c_InitStruct);
    I2C_Cmd(I2C1, ENABLE);
}


static uint8_t Camera_Read_Byte(I2C_TypeDef* I2Cx, uint8_t register_addr)
{
    uint8_t result;

    //genrate the start signal.
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address and check if the slave received the signal. 
    I2C_Send7bitAddress(I2Cx, OV7670_DEVICE_ADDRESS_READ, I2C_Direction_Transmitter);

    //check if devic address is recived
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == ERROR);
    
    //temperature read commend
    I2C_SendData(I2Cx, register_addr);

    //check if data is transmitted
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING) == ERROR);

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    //start bit for register read
    I2C_GenerateSTART(I2Cx, ENABLE);

    //check start signal is ready.
    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT) == ERROR);

    //send the device address
    I2C_Send7bitAddress(I2Cx, OV7670_DEVICE_ADDRESS_READ, I2C_Direction_Receiver);

    while (I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED) == ERROR);

    //reed out data from the result buffer
    result = I2C_ReceiveData(I2Cx);

    I2C_AcknowledgeConfig(I2Cx, DISABLE);

    I2C_GenerateSTOP(I2Cx, ENABLE);

    I2C_AcknowledgeConfig(I2Cx, ENABLE);

    return result;
}