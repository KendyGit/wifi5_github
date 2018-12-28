#ifndef __key_fc_H__
#define __key_fc_H__

#include "sys.h"

void BSP_Key_FC_Single_Continuous(void);   //放入10ms定时器中
void BSP_Key_FC_Short_Long(void);		  //放入10ms定时器中
static void KeyScan(void);

extern unsigned char Key_FC_SinglePressOK,
                     Key_FC_ContinuousPressOK;
extern unsigned char Key_FC_ShortPressOK,
                     Key_FC_LongPressOK;
extern unsigned char Key_FC_DoublePressOK;

#endif
