#ifndef __key_down_H__
#define __key_down_H__

#include "sys.h"

void BSP_Key_Down_Single_Continuous(void);   //放入10ms定时器中
void BSP_Key_Down_Short_Long(void);		  //放入10ms定时器中
static void KeyScan(void);

extern unsigned char Key_Down_SinglePressOK,
                     Key_Down_ContinuousPressOK;
extern unsigned char Key_Down_ShortPressOK,
                     Key_Down_LongPressOK;
extern unsigned char Key_Down_DoublePressOK;

#endif
