#include "sim.h"

#pragma import(__use_no_semihosting)
//定义_sys_exit()以避免使用半主机模式
_sys_exit(int x)
{
    x = x;
}
//标准库需要的支持函数
struct __FILE
{
    int handle;
};
FILE __stdout;
FILE __stdin;

//重定义fputc函数
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1,(uint16_t)ch);
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
    return ch;
}
//重定义fgetc函数
int fgetc(FILE *f) {
    while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE) == RESET);
    return (int)USART_ReceiveData(USART1);
}

unsigned int USART1_Index = 0;
unsigned char USART1_RX[1024];

/* 初始化USART1 */
void USART1_Init() {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
    USART_DeInit(USART1);  //复位串口1
    //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9

    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级1
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
    NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    //USART 初始化设置

    USART_InitStructure.USART_BaudRate = 115200;//一般设置为9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
    USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口
}
/* USART1中断处理函数 */
void USART1_IRQHandler(void) {
    uint16_t res;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET) // 中断标志
    {
        res= USART_ReceiveData(USART1);  // 串口1 接收
        if(USART1_Index<1024) {
            USART1_RX[USART1_Index++]=res;
        }
    }
}

//GM510发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//1,得到期待的应答结果
char Sim_check_cmd(unsigned char  *str)
{
    char *strx = 0;
    if(USART1_Index!=0) {
        USART1_RX[USART1_Index++] = '\0'; //添加结束符
        strx = strstr((const char *)USART1_RX, (const char *)str);
    }

    if(strx==NULL) {
        return 0;
    } else {
        return 1;
    }
}

//GM510发送命令
//cmd:发送的命令字符串(不需要添加回车了),当cmd<0XFF的时候,发送数字(比如发送0X1A),大于的时候发送字符串.
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
unsigned char  SIM_send_cmd(unsigned char  *cmd, unsigned char  *ack, short waittime)
{
    unsigned char  res = 0;

    printf("%s\r\n", cmd);//发送命令

    while (--waittime)  //等待倒计时
    {
        Delay(10);
        if (Sim_check_cmd(ack))
        {
            break;//得到有效数据
        }
    }
    if (waittime == 0) res = 1;

    USART1_Index = 0;
    return res;
}

//GSM初始化及检测(SIM卡准备等)
void Sim_Init(void)
{
    USART1_Init();
    USART1_Index = 0;
    do { //测试串口通信是否正常
        OLED_ShowString(0,0,"in the start...",16);
    } while(SIM_send_cmd("AT", "OK", 200));
    OLED_Clear();
    do//查询SIM卡是否在位
    {
        OLED_ShowString(0,0,"no sim card...",16);
    } while(SIM_send_cmd("AT+CPIN?", "READY", 200));
    OLED_Clear();

    OLED_ShowString(0,0,"sim init...",16);
    while(SIM_send_cmd("AT+CMGF=1", "OK", 20));    //设置文本模式
    while(SIM_send_cmd("AT+CSCS=GSM", "OK", 20));  //设置TE字符集为GSM
    while(SIM_send_cmd("AT+CSMP=33,167,0,0", "OK", 20));//设置文本模式参数，英文短信
    while(SIM_send_cmd("AT+CNMI=1,2,0,2,0", "OK", 20)); //设置新消息直接输出
    OLED_Clear();
}


//GM510发送短信
//num:手机号.
//mes:短信内容
//返回值:0,发送失败
//       1,发送成功
unsigned char Text_T(char * num,char * mes) {
    char temp[50]="AT+CMGS=";

    strcat(temp, (char *)num);                     //字符串拼接函数
    while(SIM_send_cmd((unsigned char *)temp, ">", 20)); //发送短信命令+电话号码
    printf("%s", mes);   	//发送短信内容到GSM模块
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);//必须增加延时,否则接收方接收信息不全

    printf("%c",26);//发送结束字符
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);
    Delay(200);//必须增加延时,否则接收方接收信息不全
}
