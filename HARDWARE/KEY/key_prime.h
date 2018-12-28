#ifndef __key_prime_H__
#define __key_prime_H__

#include "sys.h"

void BSP_Key_Prime_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Prime_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Prime_SinglePressOK,
                     Key_Prime_ContinuousPressOK;
extern unsigned char Key_Prime_ShortPressOK,
                     Key_Prime_LongPressOK;
extern unsigned char Key_Prime_DoublePressOK;

#endif
