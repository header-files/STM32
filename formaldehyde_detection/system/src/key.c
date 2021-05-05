#include"key.h"

/* ��ʱ���� */
void Delay(unsigned char time) {
    int i;
    while(time--)
    {
        for(i=0; i<12300; i++);
    }
}
/* ������� */
unsigned char Check_A(unsigned char num,unsigned char state) {
    if(PAin(num)==state) {
        Delay(50);
        if(PAin(num)==state) {
            return 1;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
void Reset(void) {
    PBout(12) = 0;
    PBout(13) = 0;
    PBout(14) = 0;
    PBout(15) = 0;
}
/*��ȡ����ֵ*/
unsigned char Key(void) {
    unsigned char Num=100;
    //����ȵͺ��
    Reset();
    if(Check_A(4,0)) { //��һ��
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//��һ��
        if(Check_A(4,1)) Num = 15;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//�ڶ���
        if(Check_A(4,1)) Num = 0;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//������
        if(Check_A(4,1)) Num = 14;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//������
        if(Check_A(4,1)) Num = 13;
        Reset();
    }
    if(Check_A(5,0)) { //�ڶ���
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//��һ��
        if(Check_A(5,1)) Num = 7;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//�ڶ���
        if(Check_A(5,1)) Num = 8;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//������
        if(Check_A(5,1)) Num = 9;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//������
        if(Check_A(5,1)) Num = 12;
        Reset();
    }
    if(Check_A(6,0)) { //������
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//��һ��
        if(Check_A(6,1)) Num = 4;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//�ڶ���
        if(Check_A(6,1)) Num = 5;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//������
        if(Check_A(6,1)) Num = 6;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//������
        if(Check_A(6,1)) Num = 11;
        Reset();
    }
    if(Check_A(7,0)) { //������
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//��һ��
        if(Check_A(7,1)) Num = 1;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//�ڶ���
        if(Check_A(7,1)) Num = 2;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//������
        if(Check_A(7,1)) Num = 3;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//������
        if(Check_A(7,1)) Num = 10;
        Reset();
    }
    return Num;
}
