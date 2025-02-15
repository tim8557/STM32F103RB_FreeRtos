
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include "stm32f10x_tim.h"
#include "stm32f1xx_it.h"
#include "Bsp_camera.h"
#include "Bsp_otm8009a.h"

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
#define CAMERA_REG_HSTART       0x17
#define CAMERA_REG_VSTART       0x19

//OV7670 command value
#define CAMERA_COM7_VALUE       0x5  /* RGB selection */
#define CAMERA_COM15_VALUE      0xD0 /* RGB565 */
#define CAMERA_CLKRC_VALUE      0x80
#define CAMERA_DBLV_VALUE       0xA
#define CAMERA_EXHCH_VALUE      0x00
#define CAMERA_EXHCL_VALUE      0x00
#define CAMERA_DM_LNL_VALUE     0x66
#define CAMERA_DM_LNH_VALUE     0x00
#define CAMERA_COM11_VALUE      0xA
#define CAMERA_HSTART_VALUE     0x39
#define CAMERA_VSTART_VALUE     0x5D

//Pin name define PA2 PA3
#define CAMERA_RESTE_PIN        GPIO_Pin_2
#define CAMERA_PWDN_PIN         GPIO_Pin_3

//FIFO used pin
#define CAMERA_RCLK_PIN         GPIO_Pin_4
#define CAMERA_FIFO_WR_RST      GPIO_Pin_5
#define CAMERA_FIFO_WR_EN       GPIO_Pin_10
#define CAMERA_FIFO_RE_RST      GPIO_Pin_9
#define CAMERA_FIFO_OE          GPIO_Pin_8

//FIFO PIN high low
#define CAMERA_RCLK_H()           GPIOA->BSRR=CAMERA_RCLK_PIN
#define CAMERA_RCLK_L()           GPIOA->BRR =CAMERA_RCLK_PIN

#define CAMERA_FIFO_WR_RST_H()    GPIOA->BSRR=CAMERA_FIFO_WR_RST
#define CAMERA_FIFO_WR_RST_L()    GPIOA->BRR =CAMERA_FIFO_WR_RST

#define CAMERA_FIFO_WR_EN_H()     GPIOA->BSRR=CAMERA_FIFO_WR_EN
#define CAMERA_FIFO_WR_EN_L()     GPIOA->BRR =CAMERA_FIFO_WR_EN

#define CAMERA_FIFO_RE_RST_H()    GPIOA->BSRR=CAMERA_FIFO_RE_RST
#define CAMERA_FIFO_RE_RST_L()    GPIOA->BRR =CAMERA_FIFO_RE_RST

#define CAMERA_FIFO_OE_H()        GPIOA->BSRR=CAMERA_FIFO_OE
#define CAMERA_FIFO_OE_L()        GPIOA->BRR =CAMERA_FIFO_OE

#define CAMERA_FIFO_RD_PREPARE  do{\
                                CAMERA_FIFO_RE_RST_L();\
                                CAMERA_RCLK_L();\
                                CAMERA_RCLK_H();\
                                CAMERA_FIFO_RE_RST_H();\
                                CAMERA_RCLK_L();\
                                CAMERA_RCLK_H();\
                                }while(0)

#define CAMERA_RGB565_READ(val)     do{\
                                    val=0;\
                                    CAMERA_RCLK_L();\
                                    val = (GPIOC->IDR) & 0xff;\
                                    CAMERA_RCLK_H();\
                                    CAMERA_RCLK_L();\
                                    val = (val << 8) | ((GPIOC->IDR) & 0xff);\
                                    CAMERA_RCLK_H();\
                                    }while(0)


static void Camera_PinInit(void);
static void Camera_ExternelInt_Init(void);
static void NVIC_Configuration(void);
static void Camera_Send_Command(I2C_TypeDef* I2Cx, uint8_t register_addr, uint8_t cmd);
static void Camera_Sccb_Init(void);
static uint8_t Camera_Read_Byte(I2C_TypeDef* I2Cx, uint8_t register_addr);
static void Camera_InitCommand(void);
static void Camera_Image_Display(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend);

static uint8_t Device_id = 0;
static Camera_Status_Type Camera_Status = 0;

static const uint8_t Camera_Register_and_Value_List[][2] =
{
    {CAMERA_REG_HSTART, CAMERA_HSTART_VALUE},
    {CAMERA_REG_VSTART, CAMERA_VSTART_VALUE},
    {CAMERA_REG_COM15, CAMERA_COM15_VALUE},
    {CAMERA_REG_CLKRC, CAMERA_CLKRC_VALUE},
    {CAMERA_REG_DBLV, CAMERA_DBLV_VALUE},
    {CAMERA_REG_EXHCH, CAMERA_EXHCH_VALUE},
    {CAMERA_REG_EXHCL, CAMERA_EXHCL_VALUE},
    {CAMERA_REG_DM_LNL, CAMERA_DM_LNL_VALUE},
    {CAMERA_REG_DM_LNH, CAMERA_DM_LNH_VALUE},
    {CAMERA_REG_COM11, CAMERA_COM11_VALUE}
};

void Camera_Runnable_20ms(void)
{
    Camera_Status_Type status = 0;

    if (status == camera_fifo_ready)
    {
        CAMERA_FIFO_RD_PREPARE;
        Camera_Image_Display(0, 0, 240, 320);
    }
}

void Camera_Init(void)
{
    //camera used gpio init
    Camera_PinInit();

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

    //GPIO init
    GPIO_InitTypeDef  GPIO_InitStructure;
    //External interrupt
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

    //port C RGB565 input
    GPIO_InitStructure.GPIO_Pin = CAMERA_PORTC_PIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //SCCB communication
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //RESTE pin output
    //FIFO pin output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|CAMERA_RCLK_PIN|CAMERA_FIFO_WR_RST|CAMERA_FIFO_WR_EN|CAMERA_FIFO_RE_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    CAMERA_FIFO_OE_L();
    CAMERA_FIFO_WR_EN_H();
}

static void Camera_ExternelInt_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    //PC13 the VSYNC interrupt pin
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

static void Camera_InitCommand(void)
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

void EXTI15_10_IRQHandler(void)
{
    Camera_Status_Type status = 0;
    status = Camera_GetStatus();

    if (status == camera_fifo_not_ready)
    {
        CAMERA_FIFO_WR_RST_L();     //拉低使FIFO写(数据from摄像头)指针复位
        CAMERA_FIFO_WR_EN_H();      //拉高使FIFO写允许
        Camera_SetStatus(camera_fifo_prepare);
        CAMERA_FIFO_WR_EN_H();      //使FIFO写允许
        CAMERA_FIFO_WR_RST_H();     //允许使FIFO写(数据from摄像头)指针运动
        
    }
    else if (status == camera_fifo_prepare)
    {
        CAMERA_FIFO_WR_EN_L();      //拉低使FIFO写暂停
        Camera_SetStatus(camera_fifo_ready);
    }

    EXTI_ClearITPendingBit(EXTI_Line13);
}

Camera_Status_Type Camera_GetStatus(void)
{
    return Camera_Status;
}

void Camera_SetStatus(Camera_Status_Type status)
{
    Camera_Status = status;
}

static void Camera_Image_Display(uint16_t xstart, uint16_t ystart, uint16_t xend, uint16_t yend)
{
    uint16_t rgb565_val = 0;
    uint16_t xlength = xend - xstart;
	uint16_t ylength = yend - ystart;
	
	uint16_t xpos = 0;
	uint16_t ypos = 0;
	
	Otm8009a_SetWindows(xstart, ystart, xend, yend);

	for (xpos = 0; xpos < xlength; xpos++)
	{
		for (ypos = 0; ypos < ylength; ypos++)
		{
            CAMERA_RGB565_READ(rgb565_val);
			Otm8009a_Write_Data(rgb565_val);
		}
	}
}