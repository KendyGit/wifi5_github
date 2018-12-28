#ifndef __key_temp_H__
#define __key_temp_H__

#include "sys.h"

void BSP_Key_Temp_Single_Continuous(void);   //放入10ms定时器中
void BSP_Key_Temp_Short_Long(void);		  //放入10ms定时器中
static void KeyScan(void);

extern unsigned char Key_Temp_SinglePressOK,
                     Key_Temp_ContinuousPressOK;
extern unsigned char Key_Temp_ShortPressOK,
                     Key_Temp_LongPressOK;
extern unsigned char Key_Temp_DoublePressOK;

#endif
