#ifndef __key_actual_H__
#define __key_actual_H__

#include "sys.h"

void BSP_Key_Actual_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Actual_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Actual_SinglePressOK,
                     Key_Actual_ContinuousPressOK;
extern unsigned char Key_Actual_ShortPressOK,
                     Key_Actual_LongPressOK;
extern unsigned char Key_Actual_DoublePressOK;

#endif
