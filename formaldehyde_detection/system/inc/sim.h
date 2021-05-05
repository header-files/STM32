#ifndef __SIM_H
#define __SIM_H

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "oled.h"
#include "key.h"
#include "stm32f10x.h"

void USART1_Init();
char Sim_check_cmd(unsigned char  *str);
unsigned char  SIM_send_cmd(unsigned char  *cmd, unsigned char  *ack, short waittime);
void Sim_Init(void);
unsigned char Text_T(char * num,char *mes);
#endif