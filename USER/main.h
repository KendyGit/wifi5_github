#ifndef __MAIN_H
#define __MAIN_H
#include "sys.h" 
extern u16 In_Temp;				//炉内温度
extern u16 In_Temp_Uncompensated; //Temp in temperature uncompensated
extern u16 Probe_Temp_Set;				//炉内设定温度，默认350
extern u16 Probe_TempPC;
extern u16 Probe_Temp1;			//probe1温度
extern u16 Probe_Temp2;			//probe2温度
extern u16 In_Temp_Set;				//炉内设定温度，默认350
extern u16 In_Temp_Set_Max;				//炉内设定温度最高
extern u32 System_Run_Counter;	//机器运行时间，开机开始，关机结束
extern u8 Temp_Press_Step;	
extern u8 Wifi_Blink; //不同的状态闪烁间隔不同
extern u8 All_Action_sensor_Status_flags;
extern u8 SetPC_flag;


#endif

