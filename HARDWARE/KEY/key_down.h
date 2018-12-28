#ifndef __key_down_H__
#define __key_down_H__

#include "sys.h"

void BSP_Key_Down_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Down_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Down_SinglePressOK,
                     Key_Down_ContinuousPressOK;
extern unsigned char Key_Down_ShortPressOK,
                     Key_Down_LongPressOK;
extern unsigned char Key_Down_DoublePressOK;

#endif
