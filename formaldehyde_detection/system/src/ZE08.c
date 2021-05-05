#include "ZE08.h"

uint16_t HCHO[9];
uint16_t Index=0;
double HCHO_Value=0;
unsigned char Flag=0;

/* 初始化USART2 */
void ZE08_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStrue;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO,ENABLE); //使能复用功能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    /* 配置 USART2 Rx 作为浮空输入 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 配置 USART2 Tx 作为复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* 配置 USART2 */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;


    //初始化 中断优先级
    NVIC_InitStrue.NVIC_IRQChannel=USART2_IRQn;
    NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStrue.NVIC_IRQChannelSubPriority=2;
    NVIC_Init(&NVIC_InitStrue);


    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//开启接收中断
}
void USART2_IRQHandler(void) // 串口2中断服务函数
{
    uint16_t res;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)) // 中断标志
    {
        res= USART_ReceiveData(USART2);  // 串口2 接收
        if(res==0xFF) {
            Index=0;
            Flag=1;
        }
        if(Flag==1) {
            HCHO[Index] = res;
            Index++;
        }
				if(Index==9) {//计算甲醛含量，计算公式：（气体浓度高位*256+气体浓度低位）*0.001，单位：mg/m3
            Flag=0;
            Index=0;
            HCHO_Value=HCHO[4]*256+HCHO[5];
            HCHO_Value=HCHO_Value*0.001;
        }
    }
}
