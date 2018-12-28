#ifndef __key_up_H__
#define __key_up_H__

#include "sys.h"

void BSP_Key_Up_Single_Continuous(void);   //����10ms��ʱ����
void BSP_Key_Up_Short_Long(void);		  //����10ms��ʱ����
static void KeyScan(void);

extern unsigned char Key_Up_SinglePressOK,
                     Key_Up_ContinuousPressOK;
extern unsigned char Key_Up_ShortPressOK,
                     Key_Up_LongPressOK;
extern unsigned char Key_Up_DoublePressOK;

#endif
