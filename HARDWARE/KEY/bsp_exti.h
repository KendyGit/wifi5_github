#ifndef __EXTI_H
#define	__EXTI_H


#include "stm32f10x.h"


//Òý½Å¶¨Òå
#define KEY1_INT_GPIO_PORT         GPIOB
#define KEY1_INT_GPIO_CLK          (RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO)
#define KEY1_INT_GPIO_PIN          GPIO_Pin_1
#define KEY1_INT_EXTI_PORTSOURCE   GPIO_PortSourceGPIOB
#define KEY1_INT_EXTI_PINSOURCE    GPIO_PinSource1
#define KEY1_INT_EXTI_LINE         EXTI_Line1
#define KEY1_INT_EXTI_IRQ          EXTI1_IRQn

#define KEY1_IRQHandler            EXTI1_IRQHandler



void EXTI_Key_Config(void);


#endif /* __EXTI_H */
