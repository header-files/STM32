#include "sim.h"

#pragma import(__use_no_semihosting)
//����_sys_exit()�Ա���ʹ�ð�����ģʽ
_sys_exit(int x)
{
    x = x;
}
//��׼����Ҫ��֧�ֺ���
struct __FILE
{
    int handle;
};
FILE __stdout;
FILE __stdin;

//�ض���fputc����
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1,(uint16_t)ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
    return ch;
}
//�ض���fgetc����
int fgetc(FILE *f) {
    while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
    return (int)USART_ReceiveData(USART1);
}

unsigned int USART1_Index = 0;
unsigned char USART1_RX[1024];

/* ��ʼ��USART1 */
void USART1_Init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
    USART_DeInit(USART1);  //��λ����1
    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9

    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

    //Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

    //USART ��ʼ������

    USART_InitStructure.USART_BaudRate = 115200;//һ������Ϊ9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���
}
/* USART1�жϴ����� */
void USART1_IRQHandler(void) {
    uint16_t res;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) // �жϱ�־
    {
        res= USART_ReceiveData(USART1);  // ����1 ����
        if(USART1_Index<1024) {
            USART1_RX[USART1_Index++]=res;
        }
    }
}

//GM510���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//1,�õ��ڴ���Ӧ����
char Sim_check_cmd(unsigned char  *str)
{
    char *strx = 0;
    if(USART1_Index!=0) {
        USART1_RX[USART1_Index++] = '\0'; //��ӽ�����
        strx = strstr((const char *)USART1_RX, (const char *)str);
    }

    if(strx==NULL) {
        return 0;
    } else {
        return 1;
    }
}

//GM510��������
//cmd:���͵������ַ���(����Ҫ��ӻس���),��cmd<0XFF��ʱ��,��������(���緢��0X1A),���ڵ�ʱ�����ַ���.
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
unsigned char  SIM_send_cmd(unsigned char  *cmd, unsigned char  *ack, short waittime)
{
    unsigned char  res = 0;

    printf("%s\r\n", cmd);//��������

    while (--waittime)  //�ȴ�����ʱ
    {
        Delay(10);
        if (Sim_check_cmd(ack))
        {
            break;//�õ���Ч����
        }
    }
    if (waittime == 0) res = 1;

    USART1_Index = 0;
    return res;
}

//GSM��ʼ�������(SIM��׼����)
void Sim_Init(void)
{
    USART1_Init();
    USART1_Index = 0;
    do { //���Դ���ͨ���Ƿ�����
        OLED_ShowString(0,0,"in the start...",16);
    } while(SIM_send_cmd("AT", "OK", 200));
    OLED_Clear();
    do//��ѯSIM���Ƿ���λ
    {
        OLED_ShowString(0,0,"no sim card...",16);
    } while(SIM_send_cmd("AT+CPIN?", "READY", 200));
    OLED_Clear();

    OLED_ShowString(0,0,"sim init...",16);
    while(SIM_send_cmd("AT+CMGF=1", "OK", 20));    //�����ı�ģʽ
    while(SIM_send_cmd("AT+CSCS=GSM", "OK", 20));  //����TE�ַ���ΪGSM
    while(SIM_send_cmd("AT+CSMP=33,167,0,0", "OK", 20));//�����ı�ģʽ������Ӣ�Ķ���
    while(SIM_send_cmd("AT+CNMI=1,2,0,2,0", "OK", 20)); //��������Ϣֱ�����
    OLED_Clear();
}


//GM510���Ͷ���
//num:�ֻ���.
//mes:��������
//����ֵ:0,����ʧ��
//       1,���ͳɹ�
unsigned char Text_T(char * num,char * mes) {
    char temp[50]="AT+CMGS=";

    strcat(temp, (char *)num);                     //�ַ���ƴ�Ӻ���
    while(SIM_send_cmd((unsigned char *)temp, ">", 20)); //���Ͷ�������+�绰����
    printf("%s", mes);   	//���Ͷ������ݵ�GSMģ��
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);//����������ʱ,������շ�������Ϣ��ȫ

    printf("%c",26);//���ͽ����ַ�
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);//����������ʱ,������շ�������Ϣ��ȫ
}
