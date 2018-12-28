
#include "PID.h"

PID_ValueStr PID;               //����һ���ṹ�壬����ṹ���������㷨��Ҫ�õ��ĸ�������
u8 g_bPIDRunFlag = 0;          //PID���б�־λ��PID�㷨����һֱ�����㡣����ÿ��һ��ʱ�䣬��һ�Ρ�
u16 iTemp=0;		   //���Ҫִ�еİٷְٱ�0~100
u16	Pid_up=0;		   //pidִ�е�������
u16	Pid_down=0;		   //pidִ�е�������


/* ********************************************************
 �������ƣ�void PID_Init(void)                                  
 �������ܣ�PID��ʼ������                    
 ��ڲ�����                      
 ���ڲ�������
 ����˵����                                      
******************************************************** */
void PID_Init(void)
{
    PID.uKP_Coe=0;             //����ϵ��
    PID.uKI_Coe=0;             //���ֳ���
    PID.uKD_Coe=0;             //΢�ֳ�����Ϊ0������
    PID.iPriVal=0;             //��һʱ��ֵ��Ϊ0������
    PID.iSetVal=0;             //�趨ֵ�¶�
    PID.iCurVal=0;             //ʵ��ֵ��Ϊ0������

    PID.liEkVal[2] = 0;
    PID.liEkVal[1] = 0;
    PID.liEkVal[0] = 0;

}
/* ********************************************************
 �������ƣ�PID_Operation()                                  
 �������ܣ�PID����                    
��ڲ�������ǰ�¶�                      
 ���ڲ������ޣ����������U(k)��
 ����˵����U(k)+KP*[E(k)-E(k-1)]+KI*E(k)+KD*[E(k)-2E(k-1)+E(k-2)]                                      
******************************************************** */
void PID_Operation(u16 in_temp)
{
    long Temp[3] = {0};   //�м���ʱ����
    u32 PostSum = 0;     //������
    u32 NegSum = 0;      //������
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
			
			Temp[0] = (signed long)PID.iSetVal - (signed long)PID.iCurVal;    //ƫ��<=10,����E(k)
            /* ��ֵ������λ��ע��˳�򣬷���Ḳ�ǵ�ǰ�����ֵ */
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

			PID.iPriVal=iTemp*50;//�����´�PID���㣬iTempΪʵ��ִ������PID.iPriValΪ�������
			}
			else if (PID.iSetVal>=PID.iCurVal)	
			{
				iTemp=Pid_up;
				PID.iPriVal=0;
	            PID.liEkVal[2] = 0;
	            PID.liEkVal[1] = 0;
	            PID.liEkVal[0] = 0;
				PID.iPriVal=iTemp*50;//�����´�PID���㣬iTempΪʵ��ִ������PID.iPriValΪ�������
			}
			else 
			{
				iTemp=Pid_down;
				PID.iPriVal=0;
	            PID.liEkVal[2] = 0;
	            PID.liEkVal[1] = 0;
	            PID.liEkVal[0] = 0;
				PID.iPriVal=iTemp*50;//�����´�PID���㣬iTempΪʵ��ִ������PID.iPriValΪ�������
			}
}
