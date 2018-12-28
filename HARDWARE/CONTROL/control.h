#ifndef __control_H
#define __control_H	 
#include "sys.h"


#define FAN PBout(7)// PB7
#define MOT PBout(8)// PB8	
#define HOT PBout(9)// PB9
#define Control_ON 0// 开	
#define Control_OFF 1// 开

#define HOT_Status  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15) 
	


void Control_Init(void);//初始化

		 				    
#endif
