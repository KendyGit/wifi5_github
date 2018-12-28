#ifndef __key_actual_H__
#define __key_actual_H__

#include "sys.h"

void BSP_Key_Actual_Single_Continuous(void);   //放入10ms定时器中
void BSP_Key_Actual_Short_Long(void);		  //放入10ms定时器中
static void KeyScan(void);

extern unsigned char Key_Actual_SinglePressOK,
                     Key_Actual_ContinuousPressOK;
extern unsigned char Key_Actual_ShortPressOK,
                     Key_Actual_LongPressOK;
extern unsigned char Key_Actual_DoublePressOK;

#endif
