#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

// 
//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define KEY3 PAin(0)	//PA0  WK_UP
#define KEY_OnOff  		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)
#define KEY_Time  		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)	  //未用
#define KEY_FC     		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12) 	  
#define KEY_Actual      GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) 	 //未用
#define KEY_Down     	GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3) //未用
#define KEY_Set     	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) //
#define KEY_Temp    	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) 	  //未用
#define KEY_Prime     	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) //未用	  
#define KEY_Up     		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) //未用
#define KEY_Encoder_a     		GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) //
#define KEY_Encoder_b     		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8) //

//sbit KEY_OnOff = P2^1;

//#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 


/*#define KEY_UP 		4
#define KEY_LEFT	3
#define KEY_DOWN	2
#define KEY_RIGHT	1*/

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数					    
#endif
