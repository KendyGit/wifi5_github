#ifndef __key_set_H__
#define __key_set_H__

#include "sys.h"

void BSP_Key_Set_Single_Continuous(void);   //放入10ms定时器中
void BSP_Key_Set_Short_Long(void);		  //放入10ms定时器中
static void KeyScan(void);

extern unsigned char Key_Set_SinglePressOK,
                     Key_Set_ContinuousPressOK;
extern unsigned char Key_Set_ShortPressOK,
                     Key_Set_LongPressOK;
extern unsigned char Key_Set_DoublePressOK;

#endif
