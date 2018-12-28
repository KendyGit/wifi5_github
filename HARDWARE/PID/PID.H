/***************************用于定义各种接口******************************/
#ifndef		__PID_H__
#define		__PID_H__

#include "sys.h"



typedef struct PID_Value
{
    long liEkVal[3];          //差值保存，给定和反馈的差值
    u8 uEkFlag[3];          //符号，1则对应的为负数，0为对应的为正数    
    u8 uKP_Coe;             //比例系数
    u8 uKI_Coe;             //积分常数
    u8 uKD_Coe;             //微分常数
    long iPriVal;             //上一时刻值
    u16 iSetVal;             //设定值
    u16 iCurVal;             //实际值
}PID_ValueStr;

extern PID_ValueStr PID;               //定义一个结构体，这个结构体用来存算法中要用到的各种数据
extern u16 iTemp;		   //马达要执行的百分百比0~100
extern u16	Pid_up;		   //pid执行的上下限
extern u16	Pid_down;		   //pid执行的上下限
extern u8 g_bPIDRunFlag;          //PID运行标志位，PID算法不是一直在运算。而是每隔一定时间，算一次。

void PID_Init(void);
void PID_Operation(u16 in_temp);

#endif
