#ifndef __key_onoff_H__
#define __key_onoff_H__

#include "sys.h"

void BSP_Key_Onoff_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Onoff_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Onoff_SinglePressOK,
                     Key_Onoff_ContinuousPressOK;
extern unsigned char Key_Onoff_ShortPressOK,
                     Key_Onoff_LongPressOK;
extern unsigned char Key_Onoff_DoublePressOK;

#endif
