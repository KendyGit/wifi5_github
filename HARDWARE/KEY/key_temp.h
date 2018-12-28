#ifndef __key_temp_H__
#define __key_temp_H__

#include "sys.h"

void BSP_Key_Temp_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Temp_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Temp_SinglePressOK,
                     Key_Temp_ContinuousPressOK;
extern unsigned char Key_Temp_ShortPressOK,
                     Key_Temp_LongPressOK;
extern unsigned char Key_Temp_DoublePressOK;

#endif
