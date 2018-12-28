#ifndef __key_time_H__
#define __key_time_H__

#include "sys.h"

void BSP_Key_Time_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Time_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Time_SinglePressOK,
                     Key_Time_ContinuousPressOK;
extern unsigned char Key_Time_ShortPressOK,
                     Key_Time_LongPressOK;
extern unsigned char Key_Time_DoublePressOK;

#endif
