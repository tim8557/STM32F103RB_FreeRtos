#include "stm32f10x_gpio.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_rcc.h"
#include "Bsp_xpt2046.h"

#define XPT2046_S             (1 << 7u)
#define XPT2046_MODE_12_BIT   (0 << 3u)
#define XPT2046_MODE_8_BIT    (1 << 3u)
#define XPT2046_A2A1A0_YPOS   (1 << 4u)
#define XPT2046_A2A1A0_XPOS   (5 << 4u)
#define XPT2046_DFR           (0 << 2u)
#define XPT2046_SER           (1 << 2u)
#define XPT2046_POWER_LOW     0u
#define XPT2046_POWER_NORMAL  3u


#define XPT2046_DUMMY_CMD     0x0
#define XPT2046_GET_XPOS_CMD  (XPT2046_S | XPT2046_A2A1A0_XPOS | XPT2046_MODE_8_BIT | XPT2046_DFR | XPT2046_POWER_NORMAL)
#define XPT2046_GET_YPOS_CMD  (XPT2046_S | XPT2046_A2A1A0_YPOS | XPT2046_MODE_8_BIT | XPT2046_DFR | XPT2046_POWER_NORMAL)

#define XPT2046_SPI_GPIO_TYPE  GPIOA
#define XPT2046_SPI_CS         GPIO_Pin_4
#define XPT2046_SPI            SPI1
#define XPT2046_PEN_INPUT      GPIO_Pin_1

//the debounce time is 20 ms
#define XPT2046_PEN_DEBOUNCE_TIME   20u

//function define
static void Xpt2046_CS_Low(void);
static void Xpt2046_CS_High(void);
static uint8_t Xpt2046_Spi_Send_Byte(uint8_t cmd);
static void  Xpt2046_Detecte_Panel_Input(void);

//variable define
static uint16_t Panelinput_cnt = 0;
static uint16_t XPositionBuffer = 0;
static uint16_t YPositionBuffer = 0;


void Xpt2046_Runnable_1ms(void)
{

    Xpt2046_Detecte_Panel_Input();

    if (Panelinput_cnt == XPT2046_PEN_DEBOUNCE_TIME)
    {   
        Xpt2046_SpiRead_Xposition_Yposition(&XPositionBuffer, &YPositionBuffer);
    }
}

void Xpt2046_PinInit(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    //GPIO init
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   Spi_InitStructure;
    
    //SCK, MISO, MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; //PA5, PA6, PA7
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
    //CS
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    //Panel input detection
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;  
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    Spi_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    Spi_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    Spi_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    Spi_InitStructure.SPI_CRCPolynomial = 7;
    Spi_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    Spi_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    Spi_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    Spi_InitStructure.SPI_Mode = SPI_Mode_Master;
    Spi_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init(XPT2046_SPI, &Spi_InitStructure);
    SPI_Cmd(XPT2046_SPI, ENABLE);
}


static void  Xpt2046_Detecte_Panel_Input(void)
{
    if (GPIO_ReadInputDataBit(XPT2046_SPI_GPIO_TYPE, XPT2046_PEN_INPUT) == RESET)
    {
        if (Panelinput_cnt == XPT2046_PEN_DEBOUNCE_TIME)
        {
            Panelinput_cnt = XPT2046_PEN_DEBOUNCE_TIME;
        }
        else
        {
            Panelinput_cnt++;
        }
    }
    else
    {
        Panelinput_cnt = 0;
    }

}

void Xpt2046_SpiRead_Xposition(uint8_t* res, uint8_t len)
{
    uint8_t num = 0;

    Xpt2046_CS_Low();

    (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_XPOS_CMD);

    for (num = 0; num < len; num++)
    {
        *(res + num) = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);
    }

    Xpt2046_CS_High();
}

void Xpt2046_SpiRead_Yposition(uint8_t* res, uint8_t len)
{
    uint8_t num = 0;

    Xpt2046_CS_Low();

    (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_YPOS_CMD);

    for (num = 0; num < len; num++)
    {
        *(res + num) = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);
    }

    Xpt2046_CS_High();
}

static void Xpt2046_CS_Low(void)
{
    GPIO_ResetBits(XPT2046_SPI_GPIO_TYPE, XPT2046_SPI_CS);
}

static void Xpt2046_CS_High(void)
{
    GPIO_SetBits(XPT2046_SPI_GPIO_TYPE, XPT2046_SPI_CS);
}

static uint8_t Xpt2046_Spi_Send_Byte(uint8_t cmd)
{
    while(SPI_I2S_GetFlagStatus(XPT2046_SPI, SPI_I2S_FLAG_TXE) == RESET);

    SPI_I2S_SendData(XPT2046_SPI, cmd);

    while(SPI_I2S_GetFlagStatus(XPT2046_SPI, SPI_I2S_FLAG_RXNE) == RESET);

    return SPI_I2S_ReceiveData(XPT2046_SPI);
}

void Xpt2046_SpiRead_Xposition_Yposition(uint16_t* Xres, uint16_t* Yres)
{
    uint16_t data_tmp = 0;

    Xpt2046_CS_Low();

    if (XPT2046_RES_BUFFER_SIZE == 1)
    {
        (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_XPOS_CMD);

        *Xres = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);

        (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_YPOS_CMD);

        *Yres = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);
    }
    else
    {
        //read x position
        (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_XPOS_CMD);

        data_tmp = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);

        data_tmp = data_tmp << 8;

        *Xres = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD) | data_tmp;

        //read y position
        (void)Xpt2046_Spi_Send_Byte(XPT2046_GET_YPOS_CMD);

        data_tmp = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD);

        data_tmp = data_tmp << 8;

        *Yres = Xpt2046_Spi_Send_Byte(XPT2046_DUMMY_CMD) | data_tmp;
    }

    Xpt2046_CS_High();
}


