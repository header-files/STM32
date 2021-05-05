#include "stm32f10x.h"
#include "oled.h"
#include "key.h"
#include "ZE08.h"
#include "sim.h"
#include <math.h>
#include <string.h>

extern double HCHO_Value;//��ǰ������ȩ��������������
extern unsigned char USART1_RX[1024];

char Order = '9';//���յ���ָ��
unsigned char key;
char   tel_num[12]="12345678910";//�ֻ���
double HCHO_YZ=0.08;//��ȩ��ֵ
char HCHO_Char[8];//��ǰ������ȩ�������ַ��ͣ�
char Win_Flag=0;//�Ƿ��յ�����ָ��

void Car(void);
void Init(void);
void Show(void);

//����ģ�顢��ȩ��⣨���������⣬ֵ�����⣩

int main(void)
{
    char *temp;
    char mes[20];
	  int time;
    Init();

    OLED_Clear();

    while(1) {

        Show();

        if(HCHO_Value>=HCHO_YZ) {//��ȩ�������ߣ�����
            TIM_SetCompare3(TIM4,130);
            Delay(100);
        }else if(Win_Flag==0){//δ���յ�����ָ��
					  TIM_SetCompare3(TIM4,30);
				}

        temp=strstr((const char *)USART1_RX, (const char *)"AT");//��ȡָ��
        if(temp!=NULL) {
            Order=*(temp+2);
            *temp='B';//��ֹ�������ݸ����´��ж�
        } else {
            Order='9';
        }

        switch(Order) {//ִ�ж���ָ��
        case '9'://δ�յ�����
            break;
        case '0':
            //����ָ��
            TIM_SetCompare3(TIM4,130);
            Delay(100);
            Order='9';
			    	Win_Flag=1;
            break;
        case'1'://�ش�ָ��
            TIM_SetCompare3(TIM4,30);
            Delay(100);
            Order='9';
			     	Win_Flag=0;
            break;
        case '2':
            //��ɨָ����ȴ򿪡�С��ǰ��ģ����ɨ10s
            time =200;
            while(time--) {
                TIM_SetCompare4(TIM4,30);
                Car();
                Show();
                Delay(10);
            }
            TIM_SetCompare1(TIM4,0);
            TIM_SetCompare2(TIM4,0);//�رյ��
            TIM_SetCompare4(TIM4,0);//�رշ���
            Order='9';
            break;
        case '3':
            //���ͼ�ȩֵ
				    sprintf(HCHO_Char,"%6.4lf",HCHO_Value);
            strcat(mes, HCHO_Char);
            strcat(mes, "mg/m3");//�ַ���ƴ��
            Text_T(tel_num,mes);
            Order='9';
            break;
        default:
            break;
        }

    }
    return 0;
}


/* ���ù��ܡ����̵�GPIO  */
void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,ENABLE);

    //���ܡ����̺��ģ���������
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //���ģ��������
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14| GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/* ����TIM4�������PWMʱ�õ���I/O  */
void TIM4__Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /* TIM4 ʹ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*����TIM4�����PWM�źŵ�ģʽ�����ڡ����ԡ�ռ�ձ� */
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

    /* TIM4ʹ�� */
    TIM_CtrlPWMOutputs(TIM4, ENABLE);
    TIM_Cmd(TIM4, ENABLE);
}

void Init(void) {
    int i = 0,j=0;

    SystemInit();

    //��ģ���ʼ��
    TIM4__Init();//�����L298N��ʼ��
    TIM_SetCompare1(TIM4,0);//����
    TIM_SetCompare2(TIM4,0);//�ҵ��
    TIM_SetCompare3(TIM4,0);//���
    TIM_SetCompare4(TIM4,0);//����

    GPIO_Config();//���̡����ܡ�ֱ�������ʼ��
    OLED_Init();//��ʼ��OLED
    OLED_Clear();
    Sim_Init();//����ģ���ʼ��
    ZE08_Config();//��ȩģ���ʼ��

    //��ǰ�ֻ��ţ�
    OLED_ShowCHinese(0,0,6);
    OLED_ShowCHinese(16,0,7);
    OLED_ShowCHinese(32,0,8);
    OLED_ShowCHinese(48,0,9);
    OLED_ShowCHinese(64,0,10);
    OLED_ShowChar(80,0,':',16);
    for( i = 0; i<11; i++) {//��ʾ�ֻ���
        OLED_ShowChar(i*8,3,tel_num[i],16);
    }
    //A:����
    OLED_ShowChar(0,5,'A',16);
    OLED_ShowChar(16,5,':',16);
    OLED_ShowCHinese(32,5,11);
    OLED_ShowCHinese(48,5,12);
    //B:����
    OLED_ShowChar(64,5,'B',16);
    OLED_ShowChar(80,5,':',16);
    OLED_ShowCHinese(96,5,13);
    OLED_ShowCHinese(112,5,14);
    while(1) {
        if((key=Key())==11) break;
        if((key=Key())==10) { //�޸��ֻ���
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
                for( j = 0; j<i; j++) {//��ʾ�ֻ���
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

    //��ǰ��ֵ��
    OLED_ShowCHinese(0,0,6);
    OLED_ShowCHinese(16,0,7);
    OLED_ShowCHinese(32,0,2);
    OLED_ShowCHinese(48,0,3);
    OLED_ShowChar(64,0,':',16);
    //A:����
    OLED_ShowChar(0,5,'A',16);
    OLED_ShowChar(16,5,':',16);
    OLED_ShowCHinese(32,5,11);
    OLED_ShowCHinese(48,5,12);
    //B:����
    OLED_ShowChar(64,5,'B',16);
    OLED_ShowChar(80,5,':',16);
    OLED_ShowCHinese(96,5,13);
    OLED_ShowCHinese(112,5,14);
    sprintf(HCHO_Char,"%6.4lf",HCHO_YZ);
    OLED_ShowString(0,3,HCHO_Char,16);
    while(1) {
        if((key=Key())==11) break;
        if((key=Key())==10) {//�޸���ֵ
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
/* С������  */
void Car(void) {
    if(PAin(0)==0&&PAin(1)==0) {
        TIM_SetCompare1(TIM4,50);//����
        TIM_SetCompare2(TIM4,0);//�ҵ��
    }
    if(PAin(0)==0) { //��ת
        TIM_SetCompare1(TIM4,50);//����
        TIM_SetCompare2(TIM4,40);//�ҵ��
    } else if(PAin(1)==0) { //��ת
        TIM_SetCompare1(TIM4,40);//����
        TIM_SetCompare2(TIM4,50);//�ҵ��
    } else {
        TIM_SetCompare1(TIM4,40);//����
        TIM_SetCompare2(TIM4,40);//�ҵ��
    }
}
/* ��ʾ��Ϣ  */
void Show(void) {
    int i =0;
    //��ʾ�ֻ���
    OLED_ShowCHinese(0,0,8);
    OLED_ShowCHinese(16,0,9);
    OLED_ShowChar(32,0,':',16);
    for( i = 0; i<11; i++) {
        OLED_ShowChar(40+i*7,0,tel_num[i],16);
    }

    //��ʾ��ֵ
    OLED_ShowCHinese(0,2,2);
    OLED_ShowCHinese(16,2,3);
    OLED_ShowChar(32,2,':',16);
    sprintf(HCHO_Char,"%6.4lf",HCHO_YZ);//��ʾ��ֵ
    OLED_ShowString(40,2,HCHO_Char,16);

    //��ʾ��ȩ������
    OLED_ShowCHinese(0,4,0);
    OLED_ShowCHinese(16,4,1);
    OLED_ShowChar(32,4,':',16);
    sprintf(HCHO_Char,"%6.4lf",HCHO_Value);//��ʾ��ȩ����
    OLED_ShowString(40,4,HCHO_Char,16);
		OLED_ShowString(88,4,"mg/m3",16);
}
