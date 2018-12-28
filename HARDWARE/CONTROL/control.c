#include "control.h"


//控制MOT HOT FAN初始化
void Control_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO, ENABLE);	 //使能PC端口时钟

	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz


 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;//FAN PBout(7)// PB7; MOT PBout(8)// PB8; HOT PBout(9)// PB9	 20180618Kendy
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOB,GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_7); 						 //PB7,PB8,PB9 output high level 20180618Kendy

}
 
