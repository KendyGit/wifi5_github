#ifndef __MAIN_H
#define __MAIN_H
#include "sys.h" 
extern u16 In_Temp;				//¯���¶�
extern u16 In_Temp_Uncompensated; //Temp in temperature uncompensated
extern u16 Probe_Temp_Set;				//¯���趨�¶ȣ�Ĭ��350
extern u16 Probe_TempPC;
extern u16 Probe_Temp1;			//probe1�¶�
extern u16 Probe_Temp2;			//probe2�¶�
extern u16 In_Temp_Set;				//¯���趨�¶ȣ�Ĭ��350
extern u16 In_Temp_Set_Max;				//¯���趨�¶����
extern u32 System_Run_Counter;	//��������ʱ�䣬������ʼ���ػ�����
extern u8 Temp_Press_Step;	
extern u8 Wifi_Blink; //��ͬ��״̬��˸�����ͬ
extern u8 All_Action_sensor_Status_flags;
extern u8 SetPC_flag;


#endif

