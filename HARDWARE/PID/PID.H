/***************************���ڶ�����ֽӿ�******************************/
#ifndef		__PID_H__
#define		__PID_H__

#include "sys.h"



typedef struct PID_Value
{
    long liEkVal[3];          //��ֵ���棬�����ͷ����Ĳ�ֵ
    u8 uEkFlag[3];          //���ţ�1���Ӧ��Ϊ������0Ϊ��Ӧ��Ϊ����    
    u8 uKP_Coe;             //����ϵ��
    u8 uKI_Coe;             //���ֳ���
    u8 uKD_Coe;             //΢�ֳ���
    long iPriVal;             //��һʱ��ֵ
    u16 iSetVal;             //�趨ֵ
    u16 iCurVal;             //ʵ��ֵ
}PID_ValueStr;

extern PID_ValueStr PID;               //����һ���ṹ�壬����ṹ���������㷨��Ҫ�õ��ĸ�������
extern u16 iTemp;		   //����Ҫִ�еİٷְٱ�0~100
extern u16	Pid_up;		   //pidִ�е�������
extern u16	Pid_down;		   //pidִ�е�������
extern u8 g_bPIDRunFlag;          //PID���б�־λ��PID�㷨����һֱ�����㡣����ÿ��һ��ʱ�䣬��һ�Ρ�

void PID_Init(void);
void PID_Operation(u16 in_temp);

#endif