
#include "PID.h"

PID_ValueStr PID;               //定义一个结构体，这个结构体用来存算法中要用到的各种数据
u8 g_bPIDRunFlag = 0;          //PID运行标志位，PID算法不是一直在运算。而是每隔一定时间，算一次。
u16 iTemp=0;		   //马达要执行的百分百比0~100
u16	Pid_up=0;		   //pid执行的上下限
u16	Pid_down=0;		   //pid执行的上下限


/* ********************************************************
 函数名称：void PID_Init(void)                                  
 函数功能：PID初始化参数                    
 入口参数：                      
 出口参数：无
 函数说明：                                      
******************************************************** */
void PID_Init(void)
{
    PID.uKP_Coe=0;             //比例系数
    PID.uKI_Coe=0;             //积分常数
    PID.uKD_Coe=0;             //微分常数（为0不动）
    PID.iPriVal=0;             //上一时刻值（为0不动）
    PID.iSetVal=0;             //设定值温度
    PID.iCurVal=0;             //实际值（为0不动）

    PID.liEkVal[2] = 0;
    PID.liEkVal[1] = 0;
    PID.liEkVal[0] = 0;

}
/* ********************************************************
 函数名称：PID_Operation()                                  
 函数功能：PID运算                    
入口参数：当前温度                      
 出口参数：无（隐形输出，U(k)）
 函数说明：U(k)+KP*[E(k)-E(k-1)]+KI*E(k)+KD*[E(k)-2E(k-1)+E(k-2)]                                      
******************************************************** */
void PID_Operation(u16 in_temp)
{
    long Temp[3] = {0};   //中间临时变量
    u32 PostSum = 0;     //正数和
    u32 NegSum = 0;      //负数和
	u16 i =0;
    static u16 j=0;
    static u16 k=0;

	PID.iCurVal=in_temp;
//	PID.iCurVal=210;

	if(PID.iSetVal>=PID.iCurVal) 	
	{
		j=PID.iSetVal - PID.iCurVal;
		k=100;
//	    Ctr_Hot = 1;

	}
	else 
	{
//		Ctr_Hot = 1;
		k=PID.iCurVal - PID.iSetVal;
//		j=100;
	}


//		    i=PID.iSetVal - PID.iCurVal;
//			SendData(i/100+'0');
//			Time_Sum=30;			//wait 3ms
//			Wait_F1=1;
//			while(Wait_F1);
//			SendData(i%100/10+'0');
//			Time_Sum=30;			//wait 3ms
//			Wait_F1=1;
//			while(Wait_F1);
//			SendData(i%10+'0');
//			Time_Sum=30;			//wait 3ms
//			Wait_F1=1;
//			while(Wait_F1);
		   if(j<50||k<50)
		   {
			
			Temp[0] = (signed long)PID.iSetVal - (signed long)PID.iCurVal;    //偏差<=10,计算E(k)
            /* 数值进行移位，注意顺序，否则会覆盖掉前面的数值 */
            PID.liEkVal[2] = PID.liEkVal[1];
            PID.liEkVal[1] = PID.liEkVal[0];
            PID.liEkVal[0] = Temp[0];
            /* =================================================================== */
            Temp[0] = PID.liEkVal[0] - PID.liEkVal[1];  //E(k)-E(k-1)
			Temp[2] = PID.liEkVal[0] - PID.liEkVal[1]*2 +  PID.liEkVal[2]; //E(k)-2E(k-1)+E(k-2)

            Temp[0] = (signed long)PID.uKP_Coe * Temp[0];        //KP*[E(k)-E(k-1)]
            Temp[1] = (signed long)PID.uKI_Coe * PID.liEkVal[0]; //KI*E(k)
            Temp[2] = (signed long)PID.uKD_Coe * Temp[2]; //KD*[E(k)-2E(k-1)+E(k-2)


			Temp[0]=Temp[0]+Temp[1]+Temp[2];
			PID.iPriVal=Temp[0]+PID.iPriVal;
			
			if(PID.iPriVal>0)
			{			 
	           	iTemp=PID.iPriVal/50;
	
				if(iTemp<=Pid_down) iTemp = Pid_down;
			    if(iTemp>=Pid_up) iTemp = Pid_up;
			}
			else  iTemp=Pid_down;

			PID.iPriVal=iTemp*50;//用于下次PID计算，iTemp为实际执行量，PID.iPriVal为计算的量
			}
			else if (PID.iSetVal>=PID.iCurVal)	
			{
				iTemp=Pid_up;
				PID.iPriVal=0;
	            PID.liEkVal[2] = 0;
	            PID.liEkVal[1] = 0;
	            PID.liEkVal[0] = 0;
				PID.iPriVal=iTemp*50;//用于下次PID计算，iTemp为实际执行量，PID.iPriVal为计算的量
			}
			else 
			{
				iTemp=Pid_down;
				PID.iPriVal=0;
	            PID.liEkVal[2] = 0;
	            PID.liEkVal[1] = 0;
	            PID.liEkVal[0] = 0;
				PID.iPriVal=iTemp*50;//用于下次PID计算，iTemp为实际执行量，PID.iPriVal为计算的量
			}
}
