#ifndef __BEEP_H
#define __BEEP_H	 
#include "sys.h"
//�������˿ڶ���
#define BEEP PBout(6)	// BEEP,�������ӿ�	
#define BEEP_ON 1		   
#define BEEP_OFF 0			   

#define            GENERAL_TIM4_Period            200//9
#define            GENERAL_TIM4_Prescaler         71
#define            GENERAL_TIM4                   TIM4
#define						 CCR1_Val											  100	   

void BEEP_Init(void);	//��ʼ��
		 				    
#endif

