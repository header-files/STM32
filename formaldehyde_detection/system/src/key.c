#include"key.h"

/* 延时函数 */
void Delay(unsigned char time) {
    int i;
    while(time--)
    {
        for(i=0; i<12300; i++);
    }
}
/* 软件消抖 */
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
/*读取键盘值*/
unsigned char Key(void) {
    unsigned char Num=100;
    //输出先低后高
    Reset();
    if(Check_A(4,0)) { //第一列
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//第一行
        if(Check_A(4,1)) Num = 15;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//第二行
        if(Check_A(4,1)) Num = 0;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//第三行
        if(Check_A(4,1)) Num = 14;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//第四行
        if(Check_A(4,1)) Num = 13;
        Reset();
    }
    if(Check_A(5,0)) { //第二列
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//第一行
        if(Check_A(5,1)) Num = 7;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//第二行
        if(Check_A(5,1)) Num = 8;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//第三行
        if(Check_A(5,1)) Num = 9;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//第四行
        if(Check_A(5,1)) Num = 12;
        Reset();
    }
    if(Check_A(6,0)) { //第三列
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//第一行
        if(Check_A(6,1)) Num = 4;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//第二行
        if(Check_A(6,1)) Num = 5;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//第三行
        if(Check_A(6,1)) Num = 6;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//第四行
        if(Check_A(6,1)) Num = 11;
        Reset();
    }
    if(Check_A(7,0)) { //第四列
        PBout(12)=1;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=0;//第一行
        if(Check_A(7,1)) Num = 1;
        PBout(12)=0;
        PBout(13)=1;
        PBout(14)=0;
        PBout(15)=0;//第二行
        if(Check_A(7,1)) Num = 2;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=1;
        PBout(15)=0;//第三行
        if(Check_A(7,1)) Num = 3;
        PBout(12)=0;
        PBout(13)=0;
        PBout(14)=0;
        PBout(15)=1;//第四行
        if(Check_A(7,1)) Num = 10;
        Reset();
    }
    return Num;
}
