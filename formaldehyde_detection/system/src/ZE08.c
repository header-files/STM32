#include "ZE08.h"

uint16_t HCHO[9];
uint16_t Index=0;
double HCHO_Value=0;
unsigned char Flag=0;

/* ��ʼ��USART2 */
void ZE08_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStrue;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO,ENABLE); //ʹ�ܸ��ù���ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    /* ���� USART2 Rx ��Ϊ�������� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* ���� USART2 Tx ��Ϊ����������� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* ���� USART2 */
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;


    //��ʼ�� �ж����ȼ�
    NVIC_InitStrue.NVIC_IRQChannel=USART2_IRQn;
    NVIC_InitStrue.NVIC_IRQChannelCmd=ENABLE;
    NVIC_InitStrue.NVIC_IRQChannelPreemptionPriority=1;
    NVIC_InitStrue.NVIC_IRQChannelSubPriority=2;
    NVIC_Init(&NVIC_InitStrue);


    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//���������ж�
}
void USART2_IRQHandler(void) // ����2�жϷ�����
{
    uint16_t res;
    if(USART_GetITStatus(USART2,USART_IT_RXNE)) // �жϱ�־
    {
        res= USART_ReceiveData(USART2);  // ����2 ����
        if(res==0xFF) {
            Index=0;
            Flag=1;
        }
        if(Flag==1) {
            HCHO[Index] = res;
            Index++;
        }
				if(Index==9) {//�����ȩ���������㹫ʽ��������Ũ�ȸ�λ*256+����Ũ�ȵ�λ��*0.001����λ��mg/m3
            Flag=0;
            Index=0;
            HCHO_Value=HCHO[4]*256+HCHO[5];
            HCHO_Value=HCHO_Value*0.001;
        }
    }
}
