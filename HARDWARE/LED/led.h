#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


#define LED_backlight PAout(2)	  //δʹ��
#define LED_debug_Power PBout(10)	  //δʹ��


#define LED_ON_DEBUG 0	//��	
#define LED_OFF_DEBUG 1	//��	
#define LED_ON_EN 1//��	
#define LED_OFF_EN 0	//��	

void LED_Init(void);//��ʼ��

		 				    
#endif
