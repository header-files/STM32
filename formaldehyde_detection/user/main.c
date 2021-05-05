#include "stm32f10x.h"
#include "oled.h"
#include "key.h"
#include "ZE08.h"
#include "sim.h"
#include <math.h>
#include <string.h>

extern double HCHO_Value;//当前环境甲醛含量（浮点数）
extern unsigned char USART1_RX[1024];

char Order = '9';//接收到的指令
unsigned char key;
char   tel_num[12]="12345678910";//手机号
double HCHO_YZ=0.08;//甲醛阈值
char HCHO_Char[8];//当前环境甲醛含量（字符型）
char Win_Flag=0;//是否收到开窗指令

void Car(void);
void Init(void);
void Show(void);

//短信模块、甲醛检测（接线无问题，值有问题）

int main(void)
{
    char *temp;
    char mes[20];
	  int time;
    Init();

    OLED_Clear();

    while(1) {

        Show();

        if(HCHO_Value>=HCHO_YZ) {//甲醛含量过高，开窗
            TIM_SetCompare3(TIM4,130);
            Delay(100);
        }else if(Win_Flag==0){//未接收到开窗指令
					  TIM_SetCompare3(TIM4,30);
				}

        temp=strstr((const char *)USART1_RX, (const char *)"AT");//提取指令
        if(temp!=NULL) {
            Order=*(temp+2);
            *temp='B';//防止残留数据干扰下次判断
        } else {
            Order='9';
        }

        switch(Order) {//执行短信指令
        case '9'://未收到短信
            break;
        case '0':
            //开窗指令
            TIM_SetCompare3(TIM4,130);
            Delay(100);
            Order='9';
			    	Win_Flag=1;
            break;
        case'1'://关窗指令
            TIM_SetCompare3(TIM4,30);
            Delay(100);
            Order='9';
			     	Win_Flag=0;
            break;
        case '2':
            //清扫指令，风扇打开、小车前进模拟清扫10s
            time =200;
            while(time--) {
                TIM_SetCompare4(TIM4,30);
                Car();
                Show();
                Delay(10);
            }
            TIM_SetCompare1(TIM4,0);
            TIM_SetCompare2(TIM4,0);//关闭电机
            TIM_SetCompare4(TIM4,0);//关闭风扇
            Order='9';
            break;
        case '3':
            //发送甲醛值
				    sprintf(HCHO_Char,"%6.4lf",HCHO_Value);
            strcat(mes, HCHO_Char);
            strcat(mes, "mg/m3");//字符串拼接
            Text_T(tel_num,mes);
            Order='9';
            break;
        default:
            break;
        }

    }
    return 0;
}


/* 配置光电管、键盘的GPIO  */
void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);

    //光电管、键盘横四，浮空输入
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //纵四，推挽输出
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/* 配置TIM4复用输出PWM时用到的I/O  */
void TIM4__Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* TIM4 使能 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*配置TIM4输出的PWM信号的模式，周期、极性、占空比 */
    TIM_TimeBaseStructure.TIM_Period = 143;//500Hz
    TIM_TimeBaseStructure.TIM_Prescaler = 999;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 50;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM4, &TIM_OCInitStructure);
    TIM_OC2Init(TIM4, &TIM_OCInitStructure);
    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);

    /* TIM4使能 */
    TIM_CtrlPWMOutputs(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void Init(void) {
    int i = 0,j=0;

    SystemInit();

    //各模块初始化
    TIM4__Init();//舵机、L298N初始化
    TIM_SetCompare1(TIM4,0);//左电机
    TIM_SetCompare2(TIM4,0);//右电机
    TIM_SetCompare3(TIM4,0);//舵机
    TIM_SetCompare4(TIM4,0);//风扇

    GPIO_Config();//键盘、光电管、直流电机初始化
    OLED_Init();//初始化OLED
    OLED_Clear();
    Sim_Init();//短信模块初始化
    ZE08_Config();//甲醛模块初始化

    //当前手机号：
    OLED_ShowCHinese(0,0,6);
    OLED_ShowCHinese(16,0,7);
    OLED_ShowCHinese(32,0,8);
    OLED_ShowCHinese(48,0,9);
    OLED_ShowCHinese(64,0,10);
    OLED_ShowChar(80,0,':',16);
    for( i = 0; i<11; i++) {//显示手机号
        OLED_ShowChar(i*8,3,tel_num[i],16);
    }
    //A:输入
    OLED_ShowChar(0,5,'A',16);
    OLED_ShowChar(16,5,':',16);
    OLED_ShowCHinese(32,5,11);
    OLED_ShowCHinese(48,5,12);
    //B:跳过
    OLED_ShowChar(64,5,'B',16);
    OLED_ShowChar(80,5,':',16);
    OLED_ShowCHinese(96,5,13);
    OLED_ShowCHinese(112,5,14);
    while(1) {
        if((key=Key())==11) break;
        if((key=Key())==10) { //修改手机号
            OLED_Clear();
            OLED_ShowCHinese(0,0,8);
            OLED_ShowCHinese(16,0,9);
            OLED_ShowCHinese(32,0,10);
            OLED_ShowChar(48,0,':',16);
            i=0;
            while(1) {
                key=Key();
                if(key>=0&&key<=9) {
                    tel_num[i]='0'+key;
                    i++;
                }
                for( j = 0; j<i; j++) {//显示手机号
                    OLED_ShowChar(j*8,3,tel_num[j],16);
                }
                if(i==11) {
                    i=0;
                    tel_num[11]='\0';
                    break;
                }
            }
            break;
        }
    }
    OLED_Clear();

    //当前阈值：
    OLED_ShowCHinese(0,0,6);
    OLED_ShowCHinese(16,0,7);
    OLED_ShowCHinese(32,0,2);
    OLED_ShowCHinese(48,0,3);
    OLED_ShowChar(64,0,':',16);
    //A:输入
    OLED_ShowChar(0,5,'A',16);
    OLED_ShowChar(16,5,':',16);
    OLED_ShowCHinese(32,5,11);
    OLED_ShowCHinese(48,5,12);
    //B:跳过
    OLED_ShowChar(64,5,'B',16);
    OLED_ShowChar(80,5,':',16);
    OLED_ShowCHinese(96,5,13);
    OLED_ShowCHinese(112,5,14);
    sprintf(HCHO_Char,"%6.4lf",HCHO_YZ);
    OLED_ShowString(0,3,HCHO_Char,16);
    while(1) {
        if((key=Key())==11) break;
        if((key=Key())==10) {//修改阈值
            OLED_Clear();
            OLED_ShowCHinese(0,0,2);
            OLED_ShowCHinese(16,0,3);
            OLED_ShowChar(32,0,':',16);
            i=0;
            HCHO_YZ=0;
            while(1) {
                key=Key();
                if(key>=0&&key<=9) {
                    i++;
                    HCHO_YZ+=pow(10,1.0-i)*key;
                }
                sprintf(HCHO_Char,"%5.4lf",HCHO_YZ);
                OLED_ShowString(0,3,HCHO_Char,16);
                if(i==6) {
                    break;
                }
            }
            break;
        }
    }
}
/* 小车运行  */
void Car(void) {
    if(PAin(0)==0&&PAin(1)==0) {
        TIM_SetCompare1(TIM4,50);//左电机
        TIM_SetCompare2(TIM4,0);//右电机
    }
    if(PAin(0)==0) { //右转
        TIM_SetCompare1(TIM4,50);//左电机
        TIM_SetCompare2(TIM4,40);//右电机
    } else if(PAin(1)==0) { //左转
        TIM_SetCompare1(TIM4,40);//左电机
        TIM_SetCompare2(TIM4,50);//右电机
    } else {
        TIM_SetCompare1(TIM4,40);//左电机
        TIM_SetCompare2(TIM4,40);//右电机
    }
}
/* 显示信息  */
void Show(void) {
    int i =0;
    //显示手机号
    OLED_ShowCHinese(0,0,8);
    OLED_ShowCHinese(16,0,9);
    OLED_ShowChar(32,0,':',16);
    for( i = 0; i<11; i++) {
        OLED_ShowChar(40+i*7,0,tel_num[i],16);
    }

    //显示阈值
    OLED_ShowCHinese(0,2,2);
    OLED_ShowCHinese(16,2,3);
    OLED_ShowChar(32,2,':',16);
    sprintf(HCHO_Char,"%6.4lf",HCHO_YZ);//显示阈值
    OLED_ShowString(40,2,HCHO_Char,16);

    //显示甲醛含量：
    OLED_ShowCHinese(0,4,0);
    OLED_ShowCHinese(16,4,1);
    OLED_ShowChar(32,4,':',16);
    sprintf(HCHO_Char,"%6.4lf",HCHO_Value);//显示甲醛含量
    OLED_ShowString(40,4,HCHO_Char,16);
		OLED_ShowString(88,4,"mg/m3",16);
}
