#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "misc.h"

static void Rs232_NvicInit(void);
//void USART2_IRQHandler(void);
uint16_t Rs232_ResultBuf = 0;

void Rs232_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    GPIO_InitTypeDef Gpio_InitStruct;
    USART_InitTypeDef Usart_InitStruct;

    Gpio_InitStruct.GPIO_Pin = GPIO_Pin_2;
    Gpio_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    Gpio_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &Gpio_InitStruct);

    Gpio_InitStruct.GPIO_Pin = GPIO_Pin_3;
    Gpio_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &Gpio_InitStruct);

    Usart_InitStruct.USART_BaudRate = 9600;
    Usart_InitStruct.USART_WordLength = USART_WordLength_8b;
    Usart_InitStruct.USART_StopBits = USART_StopBits_1;
    Usart_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    Usart_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    Usart_InitStruct.USART_Parity = USART_Parity_No;
    USART_Init(USART2, &Usart_InitStruct);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE); 

    Rs232_NvicInit();

}

static void Rs232_NvicInit(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  

    NVIC_InitStructure  .NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1 ;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0 ;      
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;       
    NVIC_Init(& NVIC_InitStructure); 

}

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		Rs232_ResultBuf = USART_ReceiveData(USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		
		while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
	}
}
