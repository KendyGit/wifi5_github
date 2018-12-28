#include "led.h"


//初始化PB10和PB11为输出口.并使能这两个口的时钟, for two button back light 20180618 Kendy	    
//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB,PC端口时钟
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//|GPIO_Pin_11;	    		 //,推挽输出, for button backlight Kendy20180628
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_ResetBits(GPIOB,GPIO_Pin_10);//For debug led
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//|GPIO_Pin_11;	    		 //,推挽输出, for button backlight Kendy20180628
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOA,GPIO_Pin_0);//for BACKLIGHT LED
}
 
