#ifndef __LED_H
#define __LED_H	 
#include "sys.h"


#define LED_backlight PAout(2)	  //未使用
#define LED_debug_Power PBout(10)	  //未使用


#define LED_ON_DEBUG 0	//开	
#define LED_OFF_DEBUG 1	//关	
#define LED_ON_EN 1//开	
#define LED_OFF_EN 0	//关	

void LED_Init(void);//初始化

		 				    
#endif
