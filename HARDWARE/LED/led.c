#include "led.h"


//��ʼ��PB10��PB11Ϊ�����.��ʹ���������ڵ�ʱ��, for two button back light 20180618 Kendy	    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PB,PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//|GPIO_Pin_11;	    		 //,�������, for button backlight Kendy20180628
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_ResetBits(GPIOB,GPIO_Pin_10);//For debug led
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//|GPIO_Pin_11;	    		 //,�������, for button backlight Kendy20180628
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOA,GPIO_Pin_0);//for BACKLIGHT LED
}
 
