#include "main.h"
#include "led.h"
#include "beep.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "includes.h"
#include "key_onoff.h"
#include "adc.h"
#include "control.h"
#include "key_fc.h"
#include "key_prime.h"
#include "lcd.h"
#include "key_actual.h"
#include "key_down.h"
#include "key_set.h"
#include "key_temp.h"
#include "key_time.h"
#include "key_up.h"
#include "PID.h"
#include "dma.h"	 
#include "bsp_exti.h" 
//////////////////////////ȫ�ֱ���////////////////////////////////////
u8  SetPC_flag = 0; 	//0: disable PC set function; 1: enable PC set function
u8  LCD_temp_grid_L=0;
u8  LCD_temp_grid_H=0;
u8  LCD_S14_S22_A30A31 = 0;
//S17  S16   S15   S14    S18   S19   S22   S21
//FAN HOTERR HOT MOTERR FANERR PELLET BAR2 BAR1
u8  LCD_S31_S13_A0A1 = 0;
//S1      S13 S34 	S33 	S32 	S31
//BT      MOT WIFI4 WIFI3 WIFI2 WIFI1
u8  All_Action_sensor_Status_display=0;
u8  All_Action_sensor_Status_flags=0;
//  NOpellet  NA  MOT FANERR MOTERR HOT  HOTERR FAN
//  7   			6    5    4     3      2    1     0
u16 temp_test=100;
u16 In_Temp=0;				//¯���¶�
u16 In_Temp_Uncompensated; //Temp in temperature uncompensated
u16 In_Temp_Set=350;				//¯���趨�¶ȣ�Ĭ��350
u16 In_Temp_Set_Max=500;				//¯���趨�¶����

u16 Probe_Temp_Set=225;				//probe��Ĭ��225
u16 Probe_Temp_Set_Max=225;				//probe��Ĭ�����225
u16 Probe_Temp_Set_Min=50;				//probe��Ĭ����С50

u16 Probe_Temp1=0;			//probe1�¶�
u16 Probe_Temp2=0;			//probe2�¶�
u16 Probe_TempPC=0;			//probe2�¶�

u8 Time_Set_Hour=0;				//Сʱʱ���趨
u8 Time_Set_Min=0;				//����ʱ���趨
u8 Time_Actual_Hour=0;				//Сʱʱ������
u8 Time_Actual_Min=0;				//����ʱ������
u16 Time_Actual=0;				//��������з���ʱ��
u8 Time_End=0;				//ʱ�䵽ʱ��־

u8 Temp_Press_Step=0;//0��δ��ѹ��1����һ����˸���趨�¶ȣ�2���趨�¶�
u8 Time_Press_Step=0;//0��δ��ѹ��1����һ��Сʱ��˸���趨Сʱ��2����һ�η�����˸���趨���ӣ�3���趨ʱ��

u8 Start_Hot=1;//Ϊ1��ʼ��ʱ����,GRILLһ��ʼ�ͼ���
u8 HOT_Continue_On=0;//�������ȳ����޶�ʱ�䣬ͣһ��

u8  Flags_Err=0;//err
u8  Flags_Prime=0;//prime

u16 Set_Interface_Count=0;				//��set����5Sû�ж����Զ�ת��actual����

u8 uCounter=0; //PID
u16 PID_Cycle=0;	//pid��������
u16 PID_Counter=0;	//pid�������ڼ���

u32 System_Run_Counter=0;	//��������ʱ�䣬������ʼ���ػ�����

u8 Key_Up_Down_Send_to_APP=0; //�������°��������ֺ���app����һ������
u8 Wifi_Blink=0; //��ͬ��״̬��˸�����ͬ

u8 Lcd_refresh_CNT=0;
u16 count_hot_on_CNT=0,count_hot_on_CNT_buf=0;
u8 count_hot_on_CNT_Enable=0;
char main_char[6];

u8 Set_temp_change=0;
u8 count_low_temp_enable=0;
u16 count_low_temp=0;	  //����130f����

u8 count_hot_on_CNT_Enable_buf=0,count_low_temp_enable_buf=0;	
u8	Key_UpDown_Long_flag = 0; //only for long UP/DOWN specified function Kendy 20181217

////main_task start//////////////
 	u8 tmr2sta=1;	//�����ʱ��2����״̬   
 	u8 tmr3sta=0;	//�����ʱ��3����״̬
	u16 count_hot_on=0;	  //��һ�ν�����
	u8 start_num=0,start_num_buf=0;//������
	u16 start_temp=0,start_temp_buf=0;//��ʼ���ʱ���¶�
	u16 start_temp_rise=0;//������������
	u8 start_mode=0,start_mode_buf=0;//���ʼ

	u8 low_temp_start_mode=0;//����130f��������ȥstart

	u8 APP_Lock_Flags=0;
	u8 FAN_buf=0,HOT_buf=0,MOT_buf=0;
////main_task end//////////////

////adc_task start//////////////
	u8 fan_sensor=0,fan_sensor_plus=0;
	u8 hot_sensor=0,hot_sensor_plus=0;
	u8 mot_sensor=0,mot_sensor_plus=0;
	u8 fan_sensor_1=0,hot_sensor_1=0,mot_sensor_1=0;
////adc_task end//////////////

u16	RTD_TAB_PT1000[62]={10000,10195,10390,10584,10779,10973,11167,11360,11554,11747, //0--45		5��һ����λ	
									11939,12132,12324,12516,12707,12898,13089,13280,13470,13660,//50--95
									13850,14040,14229,14418,14606,14795,14983,15170,15358,15545,//100--145
									15732,15919,16105,16291,16477,16662,16847,17032,17217,17401,//150--195
									17585,17769,17952,18135,18318,18501,18683,18865,19047,19228,//200--245
									19409,19590,19771,19951,20131,20311,20490,20669,20848,21026,//250--295
									21205,21400};//300--305		

float PT1000_CalculateTemperature(u16 fR);

/////////////////////////UCOSII��������///////////////////////////////////
//START ����
//�����������ȼ�
#define START_TASK_PRIO      			15 //��ʼ��������ȼ�����Ϊ���
//���������ջ��С
#define START_STK_SIZE  				64
//�����ջ	
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);	
 			   
//LED����
//�����������ȼ�
#define LED_TASK_PRIO       			8 
//���������ջ��С
#define LED_STK_SIZE  		    		64
//�����ջ
OS_STK LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);


//������Ϣ��ʾ����
//�����������ȼ�
#define QMSGSHOW_TASK_PRIO    			6
//���������ջ��С
#define QMSGSHOW_STK_SIZE  		 		64
//�����ջ	
OS_STK QMSGSHOW_TASK_STK[QMSGSHOW_STK_SIZE];
//������
void qmsgshow_task(void *pdata);


//������
//�����������ȼ�
#define MAIN_TASK_PRIO       			5 
//���������ջ��С
#define MAIN_STK_SIZE  					128
//�����ջ	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//������
void main_task(void *pdata);

/*//�ź���������
//�����������ȼ�
#define FLAGS_TASK_PRIO       			4 
//���������ջ��С
#define FLAGS_STK_SIZE  		 		64
//�����ջ	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//������
void flags_task(void *pdata); */


//����ɨ������			 
//�����������ȼ�
#define KEY_TASK_PRIO       			2 
//���������ջ��С
#define KEY_STK_SIZE  					64
//�����ջ	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//������
void key_task(void *pdata);

//����ɨ������õ���������			 
//�����������ȼ�
#define KEY_Work_TASK_PRIO       			4
//���������ջ��С
#define KEY_Work_STK_SIZE  					64
//�����ջ	
OS_STK KEY_Work_TASK_STK[KEY_Work_STK_SIZE];
//������
void KEY_Work_task(void *pdata);


//ADC����			 
//�����������ȼ�
#define ADC_TASK_PRIO       			3 
//���������ջ��С
#define ADC_STK_SIZE  					64
//�����ջ	
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//������
void adc_task(void *pdata);


//err����			 
//�����������ȼ�
#define ERR_TASK_PRIO       			9 
//���������ջ��С
#define ERR_STK_SIZE  					64
//�����ջ	
OS_STK ERR_TASK_STK[ERR_STK_SIZE];
//������
void err_task(void *pdata);

//�ػ�����			 
//�����������ȼ�
#define OFF_TASK_PRIO       			10 
//���������ջ��С
#define OFF_STK_SIZE  					64
//�����ջ	
OS_STK OFF_TASK_STK[OFF_STK_SIZE];
//������
void off_task(void *pdata);

//LCD��ʾ����			 
//�����������ȼ�
#define LCD_TASK_PRIO       			11 
//���������ջ��С
#define LCD_STK_SIZE  					64
//�����ջ	
OS_STK LCD_TASK_STK[LCD_STK_SIZE];
//������
void lcd_task(void *pdata);

//����������������ʾ����			 
//�����������ȼ�
#define RUNOTHER_TASK_PRIO       			12 
//���������ջ��С
#define RUNOTHER_STK_SIZE  					64
//�����ջ	
OS_STK RUNOTHER_TASK_STK[LCD_STK_SIZE];
//������
void runother_task(void *pdata);

//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//���������¼���	  
OS_EVENT * q_msg;			//��Ϣ����
OS_TMR   * tmr1;			//�����ʱ��1
OS_TMR   * tmr2;			//�����ʱ��2
OS_TMR   * tmr3;			//�����ʱ��3
//OS_FLAG_GRP * flags_key;	//�����ź�����
OS_FLAG_GRP * flags_run;	//�����е�һЩ�ź�����
u16 Flags_Run_Post=0;		//��������״̬��ȥ
/**********16λ���ӵ͵���˵����1��Ч***************
0������λ
1���ػ�λ
2������1��û��RTD
3������2��Ԥ��
4������3��Ԥ��
5������4��Ԥ��
6: FC
7��Prime
8��Set/Actuall
*/
u16  Flags_Run_On=0x0001;//����
u16  Flags_Run_Off=0x0002;//�ػ�
u16  Flags_Run_Err1=0x0004;//����1 û��RTD
u16  Flags_Run_Err2=0x0008;//����1 ���û�ﵽ�趨�¶ȣ������������µ��ʧ��
u16  Flags_Run_Err3=0x0010;//����3 ���µ��ʧ��
u16  Flags_Run_Err4=0x0020;//����4 �¶ȹ���
u16  Flags_C_DIS=0x0040;//F
u16  Flags_Set_Actuall=0x0100;//Set/Actuall


//#define  Flags_Run_On  0x0001//����
//#define  Flags_Run_Off  0x0002//�ػ�
//#define  Flags_Run_Err1  0x0004//����1


void * MsgGrp[6];			//��Ϣ���д洢��ַ,���֧��6����Ϣ


//�����ʱ��1�Ļص�����	
//ÿ100msִ��һ��	   
void tmr1_callback(OS_TMR *ptmr,void *p_arg) 
{
	static u16 count_recipe=0;
	////////scount_hot_on_CNT///////
	if((count_hot_on_CNT_Enable==1)||(count_hot_on_CNT_Enable==4)){count_hot_on_CNT++;}
	else if(count_hot_on_CNT_Enable==2){count_hot_on_CNT=count_hot_on_CNT;}
	else count_hot_on_CNT=0;	
	
	if (count_low_temp_enable==1){count_low_temp++;}
	else if (count_low_temp_enable==2){count_low_temp=count_low_temp;}
  else count_low_temp=0;
	
	
	/////LCD refresh interval//////
	if(Lcd_refresh_CNT++>=5)Lcd_refresh_CNT=0;
	
/***********PID***********/
	PID_Counter++;
	if(PID_Counter>=PID_Cycle)			  
	{
	    g_bPIDRunFlag = 1;
		PID_Counter=0;
	}  
/*********��������ʱ�䣬������ʼ���ػ�����*************/
	if(Start_Hot) System_Run_Counter++;
	else System_Run_Counter=0;
	if(System_Run_Counter>=599940)	System_Run_Counter=0;//���59994S��999.9MIN
/*********�˵���ʼ����*************/
	if(Recipe_Start==1)//�˵���ʼ����
	{
	 	count_recipe++;
		if(count_recipe>=600) //һ����
		{
			Recipe_Time_Count++;
			count_recipe=0;
		}

		if(Recipe_Time_Count>=Recipe_List[5+7*(Recipe_Step-1)]*100+Recipe_List[6+7*(Recipe_Step-1)]*10+Recipe_List[7+7*(Recipe_Step-1)]) //ȡ����Ӧ�����ʱ��
		{
			Recipe_Step++;//�����1����һ������
			if(Recipe_Step>Recipe_List[0])
			{
				Recipe_Start=2;//�˵�����������
				Recipe_Step=0;
			}
			Recipe_Time_Count=0;
		}

		In_Temp_Set=Recipe_List[2+7*(Recipe_Step-1)]*100+Recipe_List[3+7*(Recipe_Step-1)]*10+Recipe_List[4+7*(Recipe_Step-1)];
	
	}
	else if(Recipe_Start==2)//�˵�������ı���
	{
	 	In_Temp_Set=180;
		Recipe_Time_Count=0;
	
	}
	/*	static u16 count=0;	
	static u16 HOT_Continue_count=0;	  //���Ȱ���������������59��55�룬ͣ5s���ƻ�������
	static u16 HOT_Stop_count=0;	  //���Ȱ���������������59��55�룬ͣ5s���ƻ�������
	    
	if(Start_Hot) //�п�ʼ�����ź�			   //grill����
	{
		if(Time_Actual) //ʱ�䲻Ϊ0��ʼ����
		{
			count++;
			if(count>=600) //1����
			{
				count=0;

				Time_Actual--;
			}
			Time_Actual_Hour=Time_Actual/60;//���ӻ���Сʱ
			Time_Actual_Min=Time_Actual%60;//����
		}
		else
		{
			Time_Actual_Hour=0;		  //����
			Time_Actual_Min=0;
			Time_Actual=0;
			Time_Set_Hour=0;
			Time_Set_Min=0;	
			In_Temp_Set=100;
			Start_Hot=0;

			Time_End=1;//˵��ʱ�䵽ʱ
		}*/
/*********�������ȵ��޶�ʱ�䣬ֹͣ������******/
/*		if(HOT_Status==Control_ON)					  //grill����
		{
		 	HOT_Continue_count++;
			if(HOT_Continue_count>=35950)		 //����59��55��	  35950
			{
				HOT=Control_OFF;
//				LED_Element=LED_OFF;
				HOT_Continue_On=1;
			}
		}
		else HOT_Continue_count=0;

		if(HOT_Continue_On)		//ֹͣ��ʱ
		{
			HOT_Stop_count++;
			if(HOT_Stop_count>=50)		   //5s
			{
			 	HOT_Continue_On=0;
				HOT_Stop_count=0;
				HOT_Continue_count=0;
			}
		}
		else HOT_Stop_count=0;



	}
	else count=0;	   */
/*******feedѭ������******/	
//	Feed_X_Y_Count++;
/* 	static u16 cpuusage=0;
	static u8 tcnt=0;	    
	if(tcnt==5)
	{
		cpuusage=0;
		tcnt=0; 
	}
	cpuusage+=OSCPUUsage;
	tcnt++;	*/			    
 }

//�����ʱ��2�Ļص�����				  	   
void tmr2_callback(OS_TMR *ptmr,void *p_arg) 
{	
	static u8 sta=0;
	switch(sta)
	{
		case 0:
			break;
		case 1:
			break;
		case 2:		    
			break;
		case 3:
			break;
 		case 4:
			break;
		case 5:
			break;
		case 6:
			break;	 
	}
	sta++;
	if(sta>6)sta=0;	 											   
}
//�����ʱ��3�Ļص�����				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	u8* p;	 
	u8 err; 
	static u8 msg_cnt=0;	//msg���	  
	if(p)
	{
	 	sprintf((char*)p,"ALIENTEK %03d",msg_cnt);
		msg_cnt++;
		err=OSQPost(q_msg,p);	//���Ͷ���
		if(err!=OS_ERR_NONE) 	//����ʧ��
		{
			OSTmrStop(tmr3,OS_TMR_OPT_NONE,0,&err);	//�ر������ʱ��3
 		}
	}
} 
	 
										

 int main(void)
 {	 
  
 	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);
	Usart_DMA_Init();
	Adc_Init();
	LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
 	BEEP_Init();			//��������ʼ��	
//	KEY_Init();				//������ʼ�� 
	Control_Init();			//���Ƶ���������
	Ht1621_Init();			//LCD��ʼ��
	EXTI_Key_Config(); 
	//Wifi_Init();

	TIM_Cmd(GENERAL_TIM4, ENABLE);
	delay_ms(250);
	TIM_Cmd(GENERAL_TIM4, DISABLE);
	LED_debug_Power=LED_ON_DEBUG;	//FC backlight will be on after power on
	LED_backlight=LED_ON_EN;	//ON/OFF backlight will be on after power on
	
	BBQ_Status_Struct.onoff=2;	 //һ����Ӧ���ǹػ�״̬
	BBQ_Status_Struct.fc=0x01;//һ������ʾF

	OSInit();  	 			//��ʼ��UCOSII

 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();	    
}							    
//��ʼ����
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	u8 err;	
	    	    
	pdata = pdata; 	
	msg_key=OSMboxCreate((void*)0);		//������Ϣ����
	q_msg=OSQCreate(&MsgGrp[0],256);	//������Ϣ����
// 	flags_key=OSFlagCreate(0,&err); 	//�����ź�����		  
 	flags_run=OSFlagCreate(0,&err); 	//�����ź�����		  
	  
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    
 	OSTaskCreate(led_task,(void *)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO);						   
 	OSTaskCreate(qmsgshow_task,(void *)0,(OS_STK*)&QMSGSHOW_TASK_STK[QMSGSHOW_STK_SIZE-1],QMSGSHOW_TASK_PRIO);	 				   
 	OSTaskCreate(main_task,(void *)0,(OS_STK*)&MAIN_TASK_STK[MAIN_STK_SIZE-1],MAIN_TASK_PRIO);	 				   
// 	OSTaskCreate(flags_task,(void *)0,(OS_STK*)&FLAGS_TASK_STK[FLAGS_STK_SIZE-1],FLAGS_TASK_PRIO);	 				   
 	OSTaskCreate(key_task,(void *)0,(OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],KEY_TASK_PRIO);	
 	OSTaskCreate(KEY_Work_task,(void *)0,(OS_STK*)&KEY_Work_TASK_STK[KEY_Work_STK_SIZE-1],KEY_Work_TASK_PRIO);	
 	OSTaskCreate(adc_task,(void *)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO);	
 	OSTaskCreate(err_task,(void *)0,(OS_STK*)&ERR_TASK_STK[ERR_STK_SIZE-1],ERR_TASK_PRIO);
 	OSTaskCreate(off_task,(void *)0,(OS_STK*)&OFF_TASK_STK[OFF_STK_SIZE-1],OFF_TASK_PRIO);	
 	OSTaskCreate(lcd_task,(void *)0,(OS_STK*)&LCD_TASK_STK[LCD_STK_SIZE-1],LCD_TASK_PRIO);	
 	OSTaskCreate(runother_task,(void *)0,(OS_STK*)&RUNOTHER_TASK_STK[RUNOTHER_STK_SIZE-1],RUNOTHER_TASK_PRIO);	
	
 	OSTaskSuspend(START_TASK_PRIO);	//������ʼ����.
	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)
}
//LED����
void led_task(void *pdata)
{
	u8 t;
	u8 err;	
 	u16 flags;	
	u8 on=0;
	u8 button=0;
	u16 k;

		while(1)
	{
		flags=OSFlagPend(flags_run,0X013d,OS_FLAG_WAIT_SET_ANY,1,&err);//�п����ź�,err�ź�
		t++;
		delay_ms(10);
	
			/*****ERR������ʼ��*****/
			if(!(flags&0X003c))	//ERR��־λû��һ��Ϊ1
			{

				if(!Flags_Run_Post)	//Ϊ0Ϊ�տ�����err
				{
					LCD_All_Off();
				}
			}
	}
}
//������Ϣ��ʾ����
void qmsgshow_task(void *pdata)
{
//	u8 *p;
//	u8 err;
	while(1)
	{
//		p=OSQPend(q_msg,0,&err);//������Ϣ����
		delay_ms(500);	 
	}									 
}
//������
void main_task(void *pdata)
{							 
	u8 err;
	u16 flags,temp;
// 	tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100msִ��һ��
//	tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200msִ��һ��
//	tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100msִ��һ��
//	OSTmrStart(tmr1,&err);//���������ʱ��1				 
//	OSTmrStart(tmr2,&err);//���������ʱ��2				 
 	while(1)
	{

		flags=OSFlagPend(flags_run,0X0003,OS_FLAG_WAIT_SET_ANY,0,&err);//һֱ�ȴ������ػ��ź�
		 
		temp=In_Temp;//110;//
		 
		if(flags&Flags_Run_Off)
		{
			count_hot_on=0;		//�ػ���һЩ�ź�Ҫ����
			start_num=0;
			start_mode=0;
			low_temp_start_mode=0;
		}
		else if(Start_Hot)
		{
			//ONLY FOR DEBUG THE UARST START KENDY
			//Send_Set_Temp_Message(100);
			//	delay_ms(250);
			//ONLY FOR DEBUG THE UARST END KENDY
				if((APP_Control_Override)&& (APP_Lock_Flags==0))
				{
					count_hot_on_CNT_Enable_buf=count_hot_on_CNT_Enable;
					count_hot_on_CNT_buf=count_hot_on_CNT;
					start_temp_buf=start_temp;//startģʽǰ��ȡ��ǰ�¶�
					start_num_buf=start_num;   //������
					start_mode_buf=start_mode;//��ʼ���
					FAN_buf=FAN;	  //����ȫ��
					HOT_buf=HOT; 
					MOT_buf=MOT;		
					APP_Lock_Flags=1;
					
					FAN=Control_OFF;	  
					HOT=Control_OFF;
					MOT=Control_OFF;
					
					All_Action_sensor_Status_flags&=0xFB;
				 	if(APP_Control_Override_Auger) 	
					{
						MOT=Control_ON;
						All_Action_sensor_Status_flags|=0x20;
					}
					else 
					{
						MOT=Control_OFF; 
						All_Action_sensor_Status_flags&=0xDF;
				 	}
					if(APP_Control_Override_Fan)
					{
						FAN=Control_ON;
						All_Action_sensor_Status_flags|=0x01;
					}
					else 
					{
						FAN=Control_OFF;
						All_Action_sensor_Status_flags&=0xFE;
					}
				}
				else if((APP_Control_Override))
				{

					APP_Lock_Flags=1;
					
					All_Action_sensor_Status_flags&=0xFB;
				 	if(APP_Control_Override_Auger) 	
					{
						MOT=Control_ON;
						All_Action_sensor_Status_flags|=0x20;
					}
					else 
					{
						MOT=Control_OFF; 
						All_Action_sensor_Status_flags&=0xDF;
				 	}
					if(APP_Control_Override_Fan)
					{
						FAN=Control_ON;
						All_Action_sensor_Status_flags|=0x01;
					}
					else 
					{
						FAN=Control_OFF;
						All_Action_sensor_Status_flags&=0xFE;
					}
				}
				else if(APP_Lock_Flags==1)
				{
					count_hot_on_CNT_Enable=count_hot_on_CNT_Enable_buf;
					count_hot_on_CNT=count_hot_on_CNT_buf;
					start_temp=start_temp_buf;//startģʽǰ��ȡ��ǰ�¶�
					start_num=start_num_buf;   //������
					start_mode=start_mode_buf;//��ʼ���
					FAN=FAN_buf;	  //����ȫ��
					HOT=HOT_buf;  
					MOT=MOT_buf;
					APP_Lock_Flags=0;
				}
				else
				{
				/********************START*******************/
				if((count_hot_on_CNT==0)|(count_hot_on_CNT_Enable==4))	 
				{
					count_hot_on_CNT_Enable=1;
					count_hot_on_CNT=0;
					start_temp=temp;//startģʽǰ��ȡ��ǰ�¶�
					start_num++;   //������
					start_mode=1;//��ʼ���
					FAN=Control_ON;	  //����ȫ��
					HOT=Control_ON;   
					All_Action_sensor_Status_flags|=0x05;
				}
				if(count_hot_on_CNT==3599)	 
				{
					if(temp>=start_temp) start_temp_rise=temp-start_temp;//startģʽ����������
					else start_temp_rise=0;//startģʽ����������
				}


				if(count_hot_on_CNT<3600)	  //10 6����start 3600
				{
					//count_hot_on_CNT_Enable=1;

					//All_Action_sensor_Status_flags|=0x05;
					
					if(!Flags_Prime)	 //primeû����
					{

					/*	if (count_hot_on<1800)  MOT=Control_ON;		//ǰ3���ӽ���
						else if (count_hot_on>2400&&count_hot_on<2700)  MOT=Control_ON;		//4���Ӻ����30��
						else MOT=Control_OFF;  */
						if(count_hot_on_CNT<=50)	 //1	  60
						{
							MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;

						}
						else if(count_hot_on_CNT<=300)	 //1	  60
						{
							MOT=Control_OFF; 
							All_Action_sensor_Status_flags&=0xDF;

						}
						else if(count_hot_on_CNT<=900) 	 //3.5
						{
							MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;
							
						}
						else if(count_hot_on_CNT<=1500) 	//4
						{
							MOT=Control_OFF; 
							All_Action_sensor_Status_flags&=0xDF;
						}
						else if(count_hot_on_CNT<=1800) 	//4��5	   270
						{
							MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;
						}
						else if(count_hot_on_CNT<=2100)   //5
						{
							MOT=Control_OFF; 
							All_Action_sensor_Status_flags&=0xDF;
						}
						else if(count_hot_on_CNT<=2400)   //5
						{
							MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;
						}
						else if(count_hot_on_CNT<=2700)   //5
						{
							MOT=Control_OFF; 
							All_Action_sensor_Status_flags&=0xDF;
						}
						else if(count_hot_on_CNT<=3000)   //5
						{
							MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;
						}
//						else if(count_hot_on_CNT<=3300)   //5
//						{
//							MOT=Control_OFF; 
//							All_Action_sensor_Status_flags&=0xDF;
//						}
						else if(count_hot_on_CNT<=3300)   //5
						{
							MOT=Control_OFF; 
							HOT=Control_OFF;
							All_Action_sensor_Status_flags&=0xDB;
						}
						else if(count_hot_on_CNT<=3600)   //�ȴ�1���Ӻ����¶�	360 change from 3600 to 3200 Kendy20180710
						{
							MOT=Control_ON;
							HOT=Control_OFF; 
							All_Action_sensor_Status_flags|=0x20;
//							All_Action_sensor_Status_flags&=0xDB;
						}
					}  	
	
				}
				else
				{
					if(start_mode)		//start�����ж�	  start ��
					{
						if(temp>130||start_temp_rise>40)   //�¶ȴ���130˵�����ɹ�
						{
							start_num=0;
							count_hot_on_CNT_Enable=2;
						}
						else 
						{
							count_hot_on=0; //���µ��
							count_hot_on_CNT_Enable=4;
							count_hot_on_CNT=0;
						}
						if(low_temp_start_mode)	  //���µ��
						{
							low_temp_start_mode=0;
							if(start_num==1) 
							{
								OSFlagPost(flags_run,Flags_Run_Err3,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1 ����2��˵�����ε��ʧ��
							}
						}
						else	   //startģʽ
						{
							if(start_num==4) 
							{
								OSFlagPost(flags_run,Flags_Run_Err2,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1 ����2��˵�����ε��ʧ��
							}
						
						}
					}
					else		 //�����м���Ƿ����130
					{
//						if(temp<130) count_low_temp++;
//						else count_low_temp=0;
					  if(temp<130) count_low_temp_enable=1;
						else count_low_temp_enable=0;
						//count_hot_on_CNT_Enable=0;
						if (count_low_temp>=6000)	   //����10���ӻ����¶ȵ�
						{
							count_low_temp_enable=0;
							low_temp_start_mode=1;
							count_hot_on=0; //���µ��
							count_hot_on_CNT_Enable=4;
							count_hot_on_CNT=0;
//							HOT=Control_OFF; 
//							OSFlagPost(flags_run,Flags_Run_Err3,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1 ����2��˵�����ε��ʧ��
						}
//						else if(count_low_temp>=6000)	HOT=Control_ON; 	 //10����  
//						else  HOT=Control_OFF; 								//ƽ�����ǿ�����

					
					}

					start_mode=0;
					HOT=Control_OFF; 
					All_Action_sensor_Status_flags&=0xFB;
				
				
/*					if(temp<130)   //�¶ȴ���130˵�����ɹ���ͬʱ�������������130�����µ��
					{
					 	count_hot_on=0;//���µ��
					}
					else start_num=0;

					if(start_num==2) OSFlagPost(flags_run,Flags_Run_Err2,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1 ����2��˵�����ε��ʧ��
					*/
					HOT=Control_OFF; 
					All_Action_sensor_Status_flags&=0xFB;

					  PID.uKP_Coe=30;             //����ϵ��	10
				    PID.uKI_Coe=10;             //���ֳ���
				    PID.uKD_Coe=5;             //΢�ֳ�����Ϊ0������
				    PID.iSetVal=In_Temp_Set;             //�趨ֵ�¶�

				/***��ͬ�¶����������ޣ��¶��ȶ�****/
					if(In_Temp_Set>=180&&In_Temp_Set<=225)
					{
						Pid_down=15;
						Pid_up=40;
					}
					else if(In_Temp_Set>225&&In_Temp_Set<=250)
					{
						Pid_down=15;
						Pid_up=60;
					}
					else if(In_Temp_Set>250&&In_Temp_Set<=350)
					{
						Pid_down=15;
						Pid_up=80;
					}
					else if(In_Temp_Set>350&&In_Temp_Set<=400)
					{
						Pid_down=30;
						Pid_up=95;
					}
					else 
					{
						Pid_down=50;
						Pid_up=100;
					}

					PID_Cycle=6;//60S��Ҫ��10


					if(!Flags_Prime)	 //primeû����
					{
					    if(iTemp == 0) 
							{MOT=Control_OFF;
							 All_Action_sensor_Status_flags&=0xDF;

					    }
							else 
							{MOT=Control_ON;
							All_Action_sensor_Status_flags|=0x20;
							}
					}

				    if(g_bPIDRunFlag)   //��ʱ�ж�Ϊ����MS��������Ϊ������
				    {
				        g_bPIDRunFlag = 0;
				        if(iTemp) iTemp--;      //ֻ��iTemp>0�����б�Ҫ����1��
				        uCounter++;
				        if(100 <= uCounter)
				        {
				            PID_Operation(temp);    
				
				            uCounter = 0; 
		
/*							sprintf(main_char,"%hd",iTemp);	//h��ʾ������
							printf(main_char);
							printf("\r\n");
						
							sprintf(main_char,"%hd",(u16)PID.iSetVal);	//h��ʾ������
							printf(main_char);
							printf("\r\n");
						
							sprintf(main_char,"%hd",(u16)temp);	//h��ʾ������
							printf(main_char);
							printf("\r\n");	   */
		
						}
					}


				
				}
		  }
		}
/*		  if(Time_End)//ʱ�䵽ʱ��������������-��-��������������5��
		  {
		   	for(i=0;i<5;i++)
			{
				BEEP=1;
				delay_ms(250);
				BEEP=0;
				delay_ms(250);
				BEEP=1;
				delay_ms(250);
				BEEP=0;
				delay_ms(250);
				BEEP=1;
				delay_ms(250);
				BEEP=0;
				delay_ms(800);

			}
		  	Time_End=0;
		  }	   */

								




//		key=(u32)OSMboxPend(msg_key,10,&err); 

/*		if(key)
		{
			OSFlagPost(flags_key,1<<(key-1),OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
		} */


/*		switch(key)
		{
			case 1://����DS1
				LED1=!LED1;

//				OSFlagPost(flags_run,Flags_Run_Post&0x0001,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
				break;
			case 2://���������ʱ��3	 
				tmr3sta=!tmr3sta;
				if(tmr3sta)OSTmrStart(tmr3,&err);  
				else OSTmrStop(tmr3,OS_TMR_OPT_NONE,0,&err);		//�ر������ʱ��3
 				break;
			case 3://���
				break;
			case 4://У׼
				OSTaskSuspend(QMSGSHOW_TASK_PRIO);	 				//���������Ϣ��ʾ����		 
 				OSTmrStop(tmr1,OS_TMR_OPT_NONE,0,&err);				//�ر������ʱ��1
				if(tmr2sta)OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//�ر������ʱ��2				 
 //				TP_Adjust();	   
				OSTmrStart(tmr1,&err);				//���¿��������ʱ��1
				if(tmr2sta)OSTmrStart(tmr2,&err);	//���¿��������ʱ��2	 
 				OSTaskResume(QMSGSHOW_TASK_PRIO); 	//���
				break;
			case 5://�����ʱ��2 ����
				tmr2sta=!tmr2sta;
				if(tmr2sta)OSTmrStart(tmr2,&err);			  	//���������ʱ��2
				else 
				{		    		    
  					OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//�ر������ʱ��2
				}
				break;				 
				
		}  */
		delay_ms(100);
	}
}		   
/*//�ź�������������
void flags_task(void *pdata)
{	
	u16 flags;	
	u8 err;	 
	while(1)
	{
		flags=OSFlagPend(flags_key,0X001F,OS_FLAG_WAIT_SET_ANY,0,&err);//�ȴ��ź���

 		if(flags&0X0001)
		{
			printf("KEY0 DOWN\n"); 
//			printf("%d\n",567); 
		}
		if(flags&0X0002)
		{
			printf("KEY1 DOWN\n");  
		} 
		if(flags&0X0004)
		{
			printf("KEY2 DOWN\n");  
		} 
		if(flags&0X0008)
		{
			printf("KEY_UP\n");  
		} 
		if(flags&0X0010)
		{
			printf("TPAD DOWN\n");  
		} 


//		BEEP=1;
		delay_ms(50);
//		BEEP=0;
	    OSFlagPost(flags_key,0X001F,OS_FLAG_CLR,&err);//ȫ���ź������㣬�����һֱ����
 	}
}	  */


//��ⰴ�����������
void KEY_Work_task(void *pdata)
{	
	u8 err;	
	u32 key=0;	
// 	u16 flags;	
	u8 fc=0;
	u16 count=0;


	while(1)
	{

	   key=(u32)OSMboxPend(msg_key,0,&err);   //һֱ�ȴ�ֱ���а����ʼ���ֻȡһ���źű㲻����
	   count++;

		switch(key)
		{
			case 1:
				if(Flags_Run_Post==0||Flags_Run_Post==Flags_Run_Off)	//Ϊ0��տ���Դ��ΪFlags_Run_Off���ִ�йػ�
				{
					Key_UpDown_Long_flag = 0;
					OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//ȫ���ź�������
					Flags_Run_Post=Flags_Run_On|Flags_Set_Actuall;   //����,��set�ź�
					OSFlagPost(flags_run,Flags_Run_Post,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1

					HOT=Control_OFF;	 //��������״̬
					All_Action_sensor_Status_flags&=0xFB;
//					LED_Element=LED_OFF;
					Time_Set_Hour=0;
					Time_Set_Min=0;	
					Temp_Press_Step=0;
					Time_Press_Step=0;
					In_Temp_Set=350;
					Probe_Temp_Set=225;
					Start_Hot=1;   //grill��������
					//BBQ_Status_Struct.onoff=1;

//					if (SetPC_flag == 1)
//					LCD_On_Init_Dis(In_Temp_Set,Probe_Temp_Set);
//					else LCD_On_NoPC_Init_Dis(In_Temp_Set);
					Temp_Press_Step=1;//һ������set��˸

					//Send_On_Message();//���Ϳ�������
					//Send_Display_Data_Message(0x11);//��������һ��״̬����
//					Send_Set_Temp_Message(In_Temp_Set);//һ�����ͽ���༭ģʽ
				tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100msִ��һ��
				tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200msִ��һ��
				tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100msִ��һ��
				OSTmrStart(tmr1,&err);	//START�����ʱ��1
				OSTmrStart(tmr2,&err);	//START�����ʱ��2
				}
				break;
			case 2:
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//ȫ���ź�������
				Flags_Run_Post=Flags_Run_Off;	 //���͹ػ��ź�,
				OSFlagPost(flags_run,Flags_Run_Post,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
				/***********һЩ�ź��������*********/
				HOT=Control_OFF;
				All_Action_sensor_Status_flags&=0xFB;
//				LED_Element=LED_OFF;
				Start_Hot=0;
				HOT_Continue_On=0;

				BBQ_Status_Struct.onoff=2;
//				Send_Off_Message();//���͹ػ�����
//				printf("KEY off\n"); 

 				break;
			case 3://PRIME����,�������һֱ��
					Key_UpDown_Long_flag = 0;
				OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//�п����ź�

				if(Start_Hot)
				{
					Flags_Prime=1;//prime
	
					MOT=Control_ON;
					All_Action_sensor_Status_flags|=0x20;
//					LED_Element=LED_ON;
				}

				break;
			case 4://FC
					Key_UpDown_Long_flag = 0;
				fc=!fc;	   //Ĭ����0��ΪF
				if(fc) OSFlagPost(flags_run,Flags_C_DIS,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1	 C
				else   OSFlagPost(flags_run,Flags_C_DIS,OS_FLAG_CLR,&err);//���ö�Ӧ���ź���Ϊ0  F
				if(BBQ_Status_Struct.fc==0x01) BBQ_Status_Struct.fc=0x00;
			    else BBQ_Status_Struct.fc=0x01;
				Send_Display_Data_Message(0x09);
				break;
			case 5://set
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
				Key_Temp_ShortPressOK=1;//�����ٰ���temp�������¶�ֱ����˸

					
				break;

				if(Recipe_Start==2)//�ڲ˵�������ı��°���set�����������޲˵�״̬
				{
				 	Recipe_Start=0;	//�������
					Recipe_Step=0;//�޲˵�
				}
			case 6://actual
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_CLR,&err);//���ö�Ӧ���ź���Ϊ0  	 
				break;
			case 9://up
					Key_UpDown_Long_flag = 0;
				if(Key_Up_App_ShortPressOK)				  //APP����
				{
					Key_Up_App_ShortPressOK=0;
					if(Temp_Press_Step==1) In_Temp_Set+=(u16)Key_Up_Down_App_Temp;	
					else if(Temp_Press_Step==2) Probe_Temp_Set+=(u16)Key_Up_Down_App_Temp;	
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
				}
				else									 //����
				{
					if(Temp_Press_Step==1)	  //temp��˸�趨
					{
						In_Temp_Set+=5;			 //ÿ�μ�5F
					}
					else if((Temp_Press_Step==2) && (SetPC_flag == 1))	  //Probe temp��˸�趨
					{
						Probe_Temp_Set+=5;			 //ÿ�μ�5F
					}
				}
			
				if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //���500F
				if((Probe_Temp_Set>Probe_Temp_Set_Max) && (SetPC_flag == 1)) Probe_Temp_Set=Probe_Temp_Set_Max;		  //���225F

				if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);
				else if((Temp_Press_Step==2)&& (SetPC_flag == 1))   Send_Set_Temp_Message(Probe_Temp_Set);//)
				break;
//			case 10://up����
//					if(Temp_Press_Step==1)	  //temp��˸�趨
//					{
//						In_Temp_Set+=5;			 //ÿ�μ�5F
//					}
//				
//					if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //���500F
//				break;

			case 11://down
						Key_UpDown_Long_flag = 0;
					if(Key_Down_App_ShortPressOK)				  //APP����
					{
						Key_Down_App_ShortPressOK=0;
							
						if(Temp_Press_Step==1) 	   //in temp
						{
							if(In_Temp_Set>180) In_Temp_Set-=(u16)Key_Up_Down_App_Temp;			 
							else In_Temp_Set=180;		  //���500F
						}
						else if(Temp_Press_Step==2) 	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=(u16)Key_Up_Down_App_Temp;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //���500F
						}
						OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
					}
					else									 //����
					{
						if(Temp_Press_Step==1)	  //temp��˸�趨
						{
							if(In_Temp_Set!=180) In_Temp_Set-=5;			 //ÿ����5F
							else In_Temp_Set=180;		  //���500F
		
						}
						else if((Temp_Press_Step==2) && (SetPC_flag == 1)) 	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=5;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //���500F
						}	
			    	}
					if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);
					else if((Temp_Press_Step==2)&& (SetPC_flag == 1))  Send_Set_Temp_Message(Probe_Temp_Set);//) 
			
				break;
//			case 12://down����

//				if(Temp_Press_Step==1)	  //temp��˸�趨
//				{
//					if(In_Temp_Set>180) In_Temp_Set-=5;			 //ÿ����5F
//					else In_Temp_Set=180;		  //���500F
//				}
//			
//				break;
			case 13://APP������ֱ���趨�¶�
					Key_UpDown_Long_flag = 0;
				if(Temp_Press_Step==1) 	   //in temp
				{
					if(Key_Up_Down_App_Temp>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //���500F
					else if(Key_Up_Down_App_Temp<180) In_Temp_Set=180;	//��С180
					else  In_Temp_Set=Key_Up_Down_App_Temp;
					Send_Set_Temp_Message(In_Temp_Set);	 //���ظ�APP
				}
				else if((Temp_Press_Step==2) && (SetPC_flag == 1)) 	//probe temp
				{
					if(Key_Up_Down_App_Temp>Probe_Temp_Set_Max) Probe_Temp_Set=Probe_Temp_Set_Max;		  //���225F
					else if(Key_Up_Down_App_Temp<Probe_Temp_Set_Min) Probe_Temp_Set=Probe_Temp_Set_Min;	//��С50
					else  Probe_Temp_Set=Key_Up_Down_App_Temp;
					Send_Set_Temp_Message(Probe_Temp_Set);	 //���ظ�APP
				}	
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1

				break;
			case 14://����wifi��������
					Key_UpDown_Long_flag = 0;
				Send_Goto_WiFi_Password_Mode();
				break;
			case 20://�������ã�ֻ���÷�������
					Key_UpDown_Long_flag = 0;
				break;
		}  

		BEEP=1;
		delay_ms(50);
		BEEP=0;

		Set_Interface_Count=0;				//��set����5Sû�ж����Զ�ת��actual����
 	}
}															    
   	   		    
//����ɨ������
 void key_task(void *pdata)
{	
	u8 key=0;
	u8 err;	
	u8 key_prime_press=0;
	u8 key_off_press=0;
	u8 key_set_press=0;

 	u16 flags;
	u16 count=0;
	u8 long_press_beep=0;//������������������
	u8 send_prime_cmd=1;//��APP����PRIME����
		    						 
	while(1)
	{

		flags=OSFlagPend(flags_run,0X013D,OS_FLAG_WAIT_SET_ANY,1,&err);//�п����źŻ����Ǵ���

	
		//if(!(flags&Flags_Run_Err1))	//ERR1��־λ RTD ��Ϊ1����ɨ�谴��
		{
	//		key=KEY_Scan(0);   
//			BSP_Key_Prime_Short_Long();
			BSP_Key_Onoff_Short_Long();
			BSP_Key_Prime_Single_Continuous();// in CVW-V1 no prime button, 20180618 Kendy
																				// In WF5, the prime function is enable. 20181210a Kendy
	//		BSP_Key_Onoff_Single_Continuous();
	/*		if(Key_Onoff_SinglePressOK)
			{
				key=1;
				Key_Onoff_SinglePressOK=0;
			} */	
	
	/*		if(key==0)
			{
				if(TPAD_Scan(0))key=5;
			}  */
	
		}

			if(Key_Onoff_ShortPressOK)
			{
				key=1;						  //����
				Key_Onoff_ShortPressOK=0;
			} 
		

				if(!key_off_press)				   // key_off_pressΪ1�����ֻ���ʲôʱ���ͷ�
				{
					if(Key_Onoff_LongPressOK)
					{
						key=2;						  
						key_off_press=1;
					}

					if(Key_Onoff_App_LongPressOK)		//app���͹ػ�����
					{
						key=2;						  
						Key_Onoff_App_LongPressOK=0;
					}
				 }
				 else
				 {
					if(!Key_Onoff_LongPressOK)
					{
						key_off_press=0;
					}
				 
				 }
	
		if(flags&0x0001)	//����״̬��
		{
			BSP_Key_FC_Short_Long();
			BSP_Key_Set_Short_Long();// in CVW-V1 no set button, 20180618 Kendy
			BSP_Key_Actual_Short_Long();// in CVW-V1 no actual button, 20180618 Kendy


//			if(Key_Prime_LongPressOK)
//			{
//				key=3;						  //prime����
//				Key_Prime_LongPressOK=0;
//			}  
/*************prime***************/	
			if(Start_Hot)							//�ڿ�ʼ�������ȵ������PRIME��������
			{
					if(Key_Prime_ContinuousPressOK||Key_Prime_App_ContinuousPressOK)
					{
						key=3;						  //prime����
//						Key_Prime_ContinuousPressOK=0;
						if(send_prime_cmd) 
						{
//							Send_Feed_Start_Message();
							send_prime_cmd=0;
						}
					}
					else 
					{
						Flags_Prime=0;				 //�ɿ����PRIME״̬ 
						if(!send_prime_cmd) 
						{
//							Send_Feed_End_Message();
							send_prime_cmd=1;
						}
					}
					 


	 		 }
/**************app���͹������趨�¶�************/
				/******up********/	
				if(Key_Up_App_ShortPressOK)
				{
					if(Key_Up_ShortPressOK)
					{
						key=9;						  //up����
						Key_Up_ShortPressOK=0;
					}
				}


				/******down********/	
				if(Key_Down_App_ShortPressOK)
				{
					if(Key_Down_ShortPressOK)
					{
						key=11;						  //down����
						Key_Down_ShortPressOK=0;
					}
				} 
  /************APPֱ����������***************/
				if(Key_Direct_App_ShortPressOK)
				{
					key=13;						  //down����
					Key_Direct_App_ShortPressOK=0;
				}

/*************fc***************/	
			if(Key_FC_ShortPressOK)
			{
				key=4;						  //FC����
				Key_FC_ShortPressOK=0;
			} 
/*************set***************/	
			if(Key_Set_ShortPressOK)
			{
				key=5;						  //set����
				Key_Set_ShortPressOK=0;
			}


			if(!key_set_press)				   // key_set_pressΪ1�����ֻ���ʲôʱ���ͷ�
			{
				if(Key_Set_LongPressOK)		  //��������wifi��������
				{
					key=14;						  
					key_set_press=1;
				}
			 }
			 else
			 {
				if(!Key_Set_LongPressOK)
				{
					key_set_press=0;
				}
			 }

/*************ACTUAL***************/	
			if(Key_Actual_ShortPressOK)
			{
				key=6;						  //ACTUAL����
				Key_Actual_ShortPressOK=0;
			}

			if(flags&Flags_Set_Actuall)	//SET�����£����������°�����time����
			{
			
			
				BSP_Key_Down_Short_Long();
				BSP_Key_Up_Short_Long();
//				BSP_Key_Temp_Short_Long();
				//BSP_Key_Time_Short_Long();
			
			
				/******TIME********/				 //grillû��TIME����  //grill����
//	/*			if(Key_Time_ShortPressOK)
//				{
//					key=20;						  //time����
//					Key_Time_ShortPressOK=0;

//					Time_Press_Step++;
//					if(Time_Press_Step>=3) 
//					{
//						Time_Press_Step=0;
//						
//						Time_Actual_Hour=Time_Set_Hour;	   
//						Time_Actual_Min=Time_Set_Min;
//						Time_Actual=(u16)Time_Actual_Hour*60+(u16)Time_Actual_Min;

//						if(Time_Actual) //ʱ�䲻Ϊ0�ſ�ʼ����
//						{
//							Start_Hot=1;//��ʼ����
//						}
//					}
//					else if(Time_Press_Step>=1) //1��2�����  ʱ��䶯����ֹͣ���ȳ�ʼ��
//					{
//						Start_Hot=0;//ֹͣ����
//						Time_Actual_Hour=0;		  //����
//						Time_Actual_Min=0;
//						Time_Actual=0;
//					}

//					Temp_Press_Step=0;//����time��temp������˸

//				}  	  */
				/******Temp********/	
				if(Key_Temp_ShortPressOK)
				{
//					key=20;						  //temp����
					Key_Temp_ShortPressOK=0;
//					Temp_Press_Step=1;//һֱ��˸
					if (SetPC_flag == 1)Temp_Press_Step++;
					else Temp_Press_Step=1;
					
					if(Temp_Press_Step>2) Temp_Press_Step=1;  //1 grill set 2 probe set

					if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);//�����¶ȱ༭ģʽ
					else  if(SetPC_flag == 1)Send_Set_Temp_Message(Probe_Temp_Set);//����probe1�¶ȱ༭ģʽ//

/*					if(Temp_Press_Step>=2) 					 //grill����
					{
					
						Time_Actual_Hour=Time_Set_Hour;	   	  //�趨���¶ȣ�����һ��ʱ�䣬��Ϊ0��ʼ����
						Time_Actual_Min=Time_Set_Min;
						Time_Actual=(u16)Time_Actual_Hour*60+(u16)Time_Actual_Min;
						
						if(Time_Actual) //ʱ�䲻Ϊ0�ſ�ʼ����			
						{
							Start_Hot=1;//��ʼ����
						}
						Temp_Press_Step=0;
					}
					else if(Temp_Press_Step==1) Start_Hot=0;//ֹͣ����,�����趨�¶�	  */


					Time_Press_Step=0;	   //����temp��time������˸

				}

				/******up********/	
				if(Key_Up_ShortPressOK)//()Rotation_Positive
				{
					key=9;						  //up����
					Key_Up_ShortPressOK=0;//Rotation_Positive=0;
				}


				/******down********/	
				if(Key_Down_ShortPressOK)//()Rotation_Negative
				{
					key=11;						  //down����
					Key_Down_ShortPressOK=0;//Rotation_Negative=0;
				}


				if(Key_Up_LongPressOK)
				{
					Key_Up_Down_Send_to_APP=1;		  //����up�¶�
					Set_Interface_Count=0;				//��set����5Sû�ж����Զ�ת��actual����

					key=10;			
					Key_UpDown_Long_flag = 1;
					Key_Up_LongPressOK=0;
				//	BEEP=1;
					long_press_beep=1;
					count++;
					if(count>=2)
					{
						count=0;
						if(Temp_Press_Step==1)	  //temp��˸�趨
						{
							In_Temp_Set+=5;			 //ÿ�μ�1F
						}
						if((Temp_Press_Step==2)&&	(SetPC_flag == 1))  //probe��˸�趨
						{
							Probe_Temp_Set+=5;			 //ÿ�μ�1F
						}

					

						if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //���500F
						if((Probe_Temp_Set>Probe_Temp_Set_Max)&&	(SetPC_flag == 1))  
						{
							Probe_Temp_Set=Probe_Temp_Set_Max;		  //���225F//
						}
					 }
				} 
//				else if(long_press_beep)
//				{
//					//BEEP=0;
//					long_press_beep=0;
//					count=0;
//				}  
				else if(Key_Down_LongPressOK)
				{
					
					Key_Up_Down_Send_to_APP=1;	  //����down �¶�

					Set_Interface_Count=0;				//��set����5Sû�ж����Զ�ת��actual����
					Key_UpDown_Long_flag = 1;
					key=12;						   
					Key_Down_LongPressOK=0;
	//				BEEP=1;
					long_press_beep=1;
					count++;
					if(count>=2)
					{
						count=0;
						if(Temp_Press_Step==1)	  //temp��˸�趨
						{
							if(In_Temp_Set>180) In_Temp_Set-=5;			 //ÿ����1F
							else In_Temp_Set=180;		  //���400F
						}
						else if((Temp_Press_Step==2)&&	(SetPC_flag == 1))  	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=5;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //���500F
						}	

					 }	

				} 
//				else if(long_press_beep)
//				{
//					//BEEP=0;
//					long_press_beep=0;
//					count=0;

//					if(Key_Up_Down_Send_to_APP==1)
//					{
//						Key_Up_Down_Send_to_APP=0;
//						if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);
//						else if(Temp_Press_Step==2) Send_Set_Temp_Message(Probe_Temp_Set);
//					}
//				}


			
			}


		}

		if(key)OSMboxPost(msg_key,(void*)key);//������Ϣ
		key=0;						 
 		delay_ms(10);
	}
}

//ADC����
void adc_task(void *pdata)
{
	u16 temp_in=0;
	u16 In_Temp_temp=0;
	u32 R_Value_in=0;
	u16 temp_probe1=0;
	u16 temp_probe2=0;
	u16 temp_probePC=0;
	u32 R_Value_probe1=0;
	u32 R_Value_probe2=0;
	u32 R_Value_probePC=0;

	u8 err;
	u8 probe_status_none=1;
	u8 probe_status=1;
	u32 temp1,temp2;

	while(1)
	{
		temp_in    = Get_Adc_Average(ADC_Channel_0,10); //1800; //only for WF5 test//
		temp_probe1=Get_Adc_Average(ADC_Channel_1,10);		 
		temp_probe2=Get_Adc_Average(ADC_Channel_6,10);
		temp_probePC=Get_Adc_Average(ADC_Channel_7,10);//2150; //

//		if(!((All_Action_sensor_Status_flags&0x02)|(All_Action_sensor_Status_flags&0x10)|(All_Action_sensor_Status_flags&0x08)))
//		{		}
			fan_sensor=Get_Mot_hot_fan_Average(ADC_Channel_4,20);
			hot_sensor=Get_Mot_hot_fan_Average(ADC_Channel_5,20);		 
			mot_sensor=Get_Mot_hot_fan_Average(ADC_Channel_8,20);

			
		
		if((APP_Control_Override_flag==1))
		{	
			hot_sensor_1=0;
			hot_sensor_plus=0;
			fan_sensor_1=0;
			fan_sensor_plus=0;
			mot_sensor_1=0;
			mot_sensor_plus=0;
			APP_Control_Override_flag=0;
			fan_sensor = 0;
			hot_sensor = 0;
			mot_sensor = 0;
		}
		
		
		if(All_Action_sensor_Status_flags&0x04)
		{
			if(hot_sensor_1++<=5) hot_sensor_plus+=hot_sensor;
			else 
			{
				if (hot_sensor_plus<=10){All_Action_sensor_Status_flags|=0x02;}
				else {All_Action_sensor_Status_flags&=0xFD;}
				hot_sensor_1=0;
				hot_sensor_plus=0;
			}
		}
		else
		{
			  All_Action_sensor_Status_flags&=0xFD;
				hot_sensor_1=0;
				hot_sensor_plus=0;
		}

		
		if(All_Action_sensor_Status_flags&0x01)
		{
			if(fan_sensor_1++<=5) fan_sensor_plus+=fan_sensor;
			else 
			{
				if (fan_sensor_plus<=10){All_Action_sensor_Status_flags|=0x10;}
				else {All_Action_sensor_Status_flags&=0xEF;}
				fan_sensor_1=0;
				fan_sensor_plus=0;
			}
		}
		else
		{
			  All_Action_sensor_Status_flags&=0xEF;
				fan_sensor_1=0;
				fan_sensor_plus=0;
		}
			
		
		if(All_Action_sensor_Status_flags&0x20)
		{
			if(mot_sensor_1++<=5) {mot_sensor_plus+=mot_sensor;}
			else 
			{
				if (mot_sensor_plus<=10){All_Action_sensor_Status_flags|=0x08;}
				else {All_Action_sensor_Status_flags&=0xF7;}
				mot_sensor_1=0;
				mot_sensor_plus=0;
			}	
		}
		else
		{
			  All_Action_sensor_Status_flags&=0xF7;
				mot_sensor_1=0;
				mot_sensor_plus=0;
		}
		
		
		
/*************¯���¶ȼ���*****************/

	  if(temp_in>=2100)			//	   ���¸���640F����
		{							
			OSFlagPost(flags_run,Flags_Run_Err1,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
			//���ϲ���40��ʵ��640�Ż���±���
			ProbeTempIn_ERR=1;
		}
		else if(temp_in>=2065)			//3535 2.84 315	   ���¸���600F����
		{							
			OSFlagPost(flags_run,Flags_Run_Err4,OS_FLAG_SET,&err);//���ö�Ӧ���ź���Ϊ1
			//���ϲ���40��ʵ��640�Ż���±���
		}
		else
		{
			ProbeTempIn_ERR=0;
			OSFlagPost(flags_run,Flags_Run_Err1,OS_FLAG_CLR,&err);//���ö�Ӧ���ź���Ϊ0  	 
			OSFlagPost(flags_run,Flags_Run_Err4,OS_FLAG_CLR,&err);//���ö�Ӧ���ź���Ϊ0  	
			if(temp_in<=1356) R_Value_in=10000;	  //0�� 1K ACTUAL KENDY
			else if(temp_in>=2100) R_Value_in=21205;	  //300��
			else
			{
			 temp1=2000*(u32)temp_in;
			 temp2=(u32)(4096-temp_in);
			 R_Value_in = 10*(temp1/temp2);
			} 		 
	
			In_Temp=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			In_Temp=In_Temp*9/5+32;  //ת�ɻ��϶�
			
			//In_Temp=345;
			/*****����*****/
			In_Temp_Uncompensated=In_Temp;

			
			if(In_Temp>=300&&In_Temp<=369) In_Temp=In_Temp+In_Temp%100;	//300--369ÿ��1F�Ͷಹ��1F
			else if (In_Temp>=370) In_Temp=In_Temp+70;					//370���ϲ���70F
		}
/*************probe1�¶ȼ���*****************/
	  if(temp_probe1>=2100)			//2124(2.2K actual) 1.728V >300	   ���¸���600F����		
		{
//			printf("no probe\n"); 
			Probe_Temp1=0;  //û�в���ʾ0
			Probe1_ERR=1;
			BBQ_Status_Struct.probe_status=0;//ֻ��һ��probeû�в�,�Ͷ�û��

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//ֻ��һ��probeû�в�,�Ͷ�û��
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			Probe1_ERR=0;
			if(temp_probe1<=1356) R_Value_in=10000;	  //0�� KENDY
			else if(temp_probe1>=2100) R_Value_in=21205;	  //300��
			else
			{
			 temp1=2000*(u32)temp_probe1;
			 temp2=(u32)(4096-temp_probe1);
			 R_Value_in =10*(temp1/temp2);
			} 		 
	
			Probe_Temp1=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_Temp1=Probe_Temp1*9/5+32;  //ת�ɻ��϶�
	
		}
/*************probe2�¶ȼ���*****************/
	  if(temp_probe2>=2100)			//2124(2.2K actual) 1.728V >300	   ���¸���600F����		
		{
//			printf("no probe\n"); 
			Probe_Temp2=0;  //û�в���ʾ0
			Probe2_ERR=1;
			BBQ_Status_Struct.probe_status=0;//ֻ��һ��probeû�в�,�Ͷ�û��

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//ֻ��һ��probeû�в�,�Ͷ�û��
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			Probe2_ERR=0;
			if(temp_probe2<=1356) R_Value_in=10000;	  //0�� KENDY
			else if(temp_probe2>=2100) R_Value_in=21205;	  //300��
			else
			{
			 temp1=2000*(u32)temp_probe2;
			 temp2=(u32)(4096-temp_probe2);
			 R_Value_in =10*(temp1/temp2);
			} 		 
	
			Probe_Temp2=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_Temp2=Probe_Temp2*9/5+32;  //ת�ɻ��϶�
	
		}
/*************probePC�¶ȼ���*****************/
	  if(temp_probePC>=2100)			//2124(2.2K actual) 1.728V >300	   ���¸���600F����				
		{
//			printf("no probe\n"); 
			Probe_TempPC=0;  //û�в���ʾ0
			ProbePC_ERR=1;
			BBQ_Status_Struct.probe_status=0;//ֻ��һ��probeû�в�,�Ͷ�û��

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//ֻ��һ��probeû�в�,�Ͷ�û��
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			ProbePC_ERR=0;
			if(temp_probePC<=1356) R_Value_in=10000;	  //0�� KENDY
			else if(temp_probePC>=2100) R_Value_in=21205;	  //300��
			else
			{
			 temp1=2000*(u32)temp_probePC;
			 temp2=(u32)(4096-temp_probePC);
			 R_Value_in = 10*(temp1/temp2);
			} 		 
	
			Probe_TempPC=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_TempPC=Probe_TempPC*9/5+32;  //ת�ɻ��϶�
	
		}			
	
		delay_ms(100);
	}			
}

//ERR����
void err_task(void *pdata)
{
	u16 flags;
	u8 err;	
	while(1)
	{
// 		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//һֱ�ȴ������ź�
		
		flags=OSFlagPend(flags_run,0X003c,OS_FLAG_WAIT_SET_ANY,0,&err);//�ȴ������ź���
		

	
		if(flags&0X0004)	//ERR1
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
		
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			
			In_Temp=980;	//Ĭ��ΪER1��Ϊ980
		}
		else if(flags&0X0008)	//ERR2	
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;

			All_Action_sensor_Status_flags&=0xDA;
			In_Temp=975;	//Ĭ��ΪER2��Ϊ975
		}
		else if(flags&0X0010)	//ERR3	
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			All_Action_sensor_Status_flags|=0x80;
			In_Temp=970;	//Ĭ��ΪER3��Ϊ970
		}
		else if(flags&0X0020)	//ERR4	
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			In_Temp=965;	//Ĭ��ΪERH��Ϊ965
		}

		
		Flags_Run_Post=0;//���㣬������
		OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������

		HOT=Control_OFF;
		MOT=Control_OFF;
		FAN=Control_OFF;
		All_Action_sensor_Status_flags&=0xDA;

/*		if(flags&0X003c)	//ERR��־λ��һ��Ϊ1�����ERRģʽ
		{
//			printf("ERR Mode\n");
			HOT=Control_OFF;
		}
		else 
		{
			OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//ȫ���ź�������
			OSFlagPost(flags_run,Flags_Run_On,OS_FLAG_SET,&err);////û�д����Ϳ���

		}  */


	   		  //������������-������������һֱ����ֱ���Ͽ���Դ
		BBQ_Status_Struct.grill_set_act=2;//act
//		Send_Display_Data_Message(0x11);
	TIM_Cmd(GENERAL_TIM4, ENABLE);
	delay_ms(250);
	TIM_Cmd(GENERAL_TIM4, DISABLE);
	delay_ms(250);
	TIM_Cmd(GENERAL_TIM4, ENABLE);
	delay_ms(250);
	TIM_Cmd(GENERAL_TIM4, DISABLE);
		delay_ms(800);

	}									 
}


//off����
void off_task(void *pdata)
{
	u8 err;	
	u16 flags;
	u16 count=0;

	while(1)
	{
		flags=OSFlagPend(flags_run,0X0003,OS_FLAG_WAIT_SET_ANY,0,&err);//һֱ�ȴ������ػ��ź�
		if(flags&Flags_Run_On) count=0;
		else//(flags&Flags_Run_Off)
		{
				 All_Action_sensor_Status_flags=0;
				 tmr2sta=1;	//�����ʱ��2����״̬   
				 tmr3sta=0;	//�����ʱ��3����״̬
				 count_hot_on=0;	  //��һ�ν�����
				 start_num=0,start_num_buf=0;//������
				 start_temp=0,start_temp_buf=0;//��ʼ���ʱ���¶�
				 start_temp_rise=0;//������������
				 start_mode=0,start_mode_buf=0;//���ʼ
					count_hot_on_CNT_Enable=0;
					count_hot_on_CNT=0;
				  low_temp_start_mode=0;//����130f��������ȥstart
					count_low_temp_enable=0;
				  count_low_temp=0;
			
				 APP_Lock_Flags=0;
				 FAN_buf=0,HOT_buf=0,MOT_buf=0;
//				tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100msִ��һ��
//				tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200msִ��һ��
//				tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100msִ��һ��
				OSTmrStop(tmr1,OS_TMR_OPT_NONE,0,&err);	//�ر������ʱ��3
				OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//�ر������ʱ��3
					HOT=Control_OFF;
					MOT=Control_OFF;

					fan_sensor=0,fan_sensor_plus=0;
					hot_sensor=0,hot_sensor_plus=0;
					mot_sensor=0,mot_sensor_plus=0;
					fan_sensor_1=0,hot_sensor_1=0,mot_sensor_1=0;
	
				if(count<=2800)		  //ʱ�����
				{
					count++;
					FAN=Control_ON;
			    All_Action_sensor_Status_flags|=0x01;
					
				}
				else 
				{
					if(In_Temp<130)		  //�¶ȿ���
					{
						FAN=Control_OFF;
						All_Action_sensor_Status_flags&=0xFE;
						
						//NVIC_SystemReset();
					}
					else 
					{
						FAN=Control_ON;	
						All_Action_sensor_Status_flags|=0x01;
					}
				}
		}


		delay_ms(100);
	}									 
}

//LCD����
void lcd_task(void *pdata)
{
	u8 err;	
	u16 flags;
	u16 in_temp_set,in_temp,temp,in_tempkendy;
	u16 probe_temp;
	u16 count=0;
	u16 temp_x;
	u16 send_to_app_count=0;//ÿ���೤ʱ����APP����
	u8 set_to_actuall=0;
	u16 in_temp_old=0;
	u16 probe_temp_old=0;
	u16 probe_temp_set=0;	
  
	
	while(1)
	{
//		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//һֱ�ȴ������ź�
		flags=OSFlagPend(flags_run,0XFFFF,OS_FLAG_WAIT_SET_ANY,1,&err);//ֻҪ���źžͽ��գ����ݲ�ͬ�ź���ʾ
		count++;
		send_to_app_count++;
		if(send_to_app_count==1)
		{
			in_temp_old=In_Temp-In_Temp%5;
			probe_temp_old=Probe_Temp1-Probe_Temp1%5;//�洢�¶ȣ�4S�����Ƚϣ��仯�˲ŷ���0b
		}


/*			if(In_Temp>=300&&In_Temp<=339)
			{
				temp_x=In_Temp-300;	//ÿ��1F��1
				temp=In_Temp-10-temp_x;
			}
			else if(In_Temp>=340) temp=In_Temp-40;
			else temp=In_Temp;*/	 

			temp=In_Temp;



		if(flags&0X0004)	//ERR1
		{
		  	LCD_Dis_Er1();
		}
		else if(flags&0X0008)	//ERR2
		{
		  	LCD_Dis_Er2();
		}
		else if(flags&0X0010)	//ERR3
		{
		  	LCD_Dis_Er3();
		}
		else if(flags&0X0020)	//ERh
		{
		  	LCD_Dis_ErH();
		}
		else if(flags&0X0002)	//�ػ�
		{
			if(APP_Display_Override==2) LCD_All_Off();
			else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
		 	//LCD_All_Off();
			BBQ_Status_Struct.onoff=2;
			All_Action_sensor_Status_display = 0;
		}
		else if(flags&0X0001)	//����
		{
				//BBQ_Status_Struct.onoff=1;
				All_Action_sensor_Status_display=(All_Action_sensor_Status_flags);
////				if(((All_Action_sensor_Status_flags&0x02)|(All_Action_sensor_Status_flags&0x10)|(All_Action_sensor_Status_flags&0x08)))
////				{				}
//					count++;
//					  if(count==25) 
//						{
//							LCD_In_Temp_Undis();
//						}
//						else if(count>=50)
//						{
//						 	count=0;   
//							LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);
//							LCD_ProbePC_Temp(Probe_TempPC-Probe_TempPC%5);  //pc�¶�
//							LCD_Probe1_Temp(Probe_Temp1-Probe_Temp1%5);  //PROBE1�¶�
//							LCD_Probe2_Temp(Probe_Temp2-Probe_Temp2%5);  //PROBE2�¶�		
//						}

				if(APP_Control_Override)   
				{
					LCD_Dis_App();
				}
				else
				{
//				LCD_Action_sensor_Status_display(All_Action_sensor_Status_display);
	/***************F/C����************/
//			if((flags&Flags_C_DIS)==1||BBQ_Status_Struct.fc==0x01)  //��־λΪ1 C
//			{
//				in_temp_set=(In_Temp_Set-32)*5/9;					  //�趨�¶�ת��Ϊ���϶�

//				if(temp>=32) in_temp=(temp-32)*5/9;					  //�趨�¶�ת��Ϊ���϶�
//				else in_temp=0; 
//				in_tempkendy = (temp-temp%5);
//				LCD_In_Temp((in_tempkendy),All_Action_sensor_Status_display);				  //ֻ��ʾ5�ı���
//				if(Probe_Temp1>=32) probe_temp=(Probe_Temp1-32)*5/9;
//				else probe_temp=0;
//				LCD_Probe1_Temp(probe_temp);
//				
//				if(Probe_Temp2>=32) probe_temp=(Probe_Temp2-32)*5/9;
//				else probe_temp=0;
//				LCD_Probe2_Temp(probe_temp);
//				
//				if(Probe_TempPC>=32) probe_temp=(Probe_TempPC-32)*5/9;
//				else probe_temp=0;
//				LCD_ProbePC_Temp(probe_temp);
//				
//				if(Probe_Temp_Set>=32) probe_temp_set=(Probe_Temp_Set-32)*5/9;
//				else probe_temp_set=0;

///*				if(Probe_Temp2>=32) probe_temp=(Probe_Temp2-32)*5/9;
//				else probe_temp=0;
//				LCD_Probe2_Temp(probe_temp-probe_temp%5);  */
//				
//			}
//			else				//����ģʽ
//			{
//	//			BBQ_Status_Struct.fc=1;
//	//			LCD_Dis_F();
//	

				in_temp_set=In_Temp_Set;					  //�趨�¶�ת��Ϊ���϶�
				in_temp=temp;
//				LCD_In_Temp(temp-temp%5);				  //ֻ��ʾ5�ı���
//				LCD_Probe1_Temp(Probe_Temp1);
//				LCD_Probe2_Temp(Probe_Temp2);
//				LCD_ProbePC_Temp(Probe_TempPC);
//				
////				LCD_Probe2_Temp(Probe_Temp2-Probe_Temp2%5);
				probe_temp_set=Probe_Temp_Set;
//			} 	

					if(BBQ_Status_Struct.onoff==2)
					{
						BBQ_Status_Struct.onoff=1;	
						if (SetPC_flag == 1)
								{
									delay_ms(10);
									LCD_All_Use();
									delay_ms(500);
									LCD_All_Off();	   //����ȫ��
									delay_ms(500);
									LCD_All_Use();
									delay_ms(500);
									LCD_All_Off();	   //����ȫ��
									delay_ms(500);	
									LCD_All_Use();
									delay_ms(500);
									LCD_All_Off();	   //����ȫ��
									delay_ms(500);	
								}
								else
								{
									delay_ms(100);
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //����ȫ��
									delay_ms(100);
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //����ȫ��
									delay_ms(100);	
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //����ȫ��
									delay_ms(100);	
								}
							}
			else if(flags&Flags_Set_Actuall)	//Set����
			{
				BBQ_Status_Struct.grill_set_act=1;
				set_to_actuall=1;

				Set_Interface_Count++;				//��set����5Sû�ж����Զ�ת��actual����
				if(Set_Interface_Count>=250) 	//5S
				{
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_CLR,&err);//���ö�Ӧ���ź���Ϊ0  
					Set_Interface_Count=0;	 
				}
			
				LCD_Dis_Set(); 	//��ʾSET
				LCD_Probe1_Temp(0x00);  //PROBE1�¶�
		  	LCD_Probe2_Temp(0x00);  //PROBE2�¶�	
				
//				LCD_Dis_Time_Dot();//��ʾʱ���
			if(Temp_Press_Step==1)	 //��˸temp
				{
						if ((Set_temp_change==1) || (Key_UpDown_Long_flag == 1))
						{
								if(APP_Display_Override==2) LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);
								else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
							Set_temp_change=0;
							count=0; 
						}
						else //(Set_temp_change==0)
						{

							if(count==25) 
							{
								LCD_In_Temp_Undis();
								LCD_UnDis_Set(); 	//��ʾSET
							}
							else if(count>=50)
							{
								count=0;   
								if(APP_Display_Override==2) LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);
								else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
								
							}
						}
					if (SetPC_flag == 1)
					LCD_ProbePC_Set_Temp(probe_temp_set);

				}
			else if((Temp_Press_Step==2) &&(SetPC_flag == 1))	 //��˸probe temp
				{
						if ((Set_temp_change==1) || (Key_UpDown_Long_flag == 1))
						{
							LCD_ProbePC_Set_Temp(probe_temp_set);
							Set_temp_change=0;
							count=0; 
						}
						else//(Set_temp_change==0)
						{
							if(count==25) 
							{
								
								LCD_Probe_Set_Temp_Undis();

							}
							else if(count>=50)
							{
								count=0;   
								LCD_ProbePC_Set_Temp(probe_temp_set);
							}
						}				
					LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);

				}

				else 
				{
					if(APP_Display_Override==2)LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);			  //�����������˸
					else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
				}
			}
			else	  //Actuall����
			{
			if(Lcd_refresh_CNT==5)
				{
				BBQ_Status_Struct.grill_set_act=2;

				if(set_to_actuall)
				{
					set_to_actuall=0;
					Temp_Press_Step=0;//����actual���棬temp�൱��ȷ����			
				}
				
//				if(Temp_Press_Step>=1)
//				{
//					Temp_Press_Step=0;//����actual���棬temp�൱��ȷ����
//					Send_Set_Temp_Message(6,2,in_temp-in_temp%5);//������ʾ�����¶�
//				}
				Set_Interface_Count=0;				//��set����5Sû�ж����Զ�ת��actual����
				//LCD_Dis_Actual();//actual��ʾ
				if(APP_Display_Override==2) 			LCD_In_Temp(in_temp,All_Action_sensor_Status_display); //(in_temp-in_temp%5) //¯���¶�
				else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
				if (SetPC_flag == 1)
				LCD_ProbePC_Temp(Probe_TempPC);  //pc�¶�
				LCD_Probe1_Temp(Probe_Temp1);  //PROBE1�¶�
		  	LCD_Probe2_Temp(Probe_Temp2);  //PROBE2�¶�		

/*				if(send_to_app_count>200)  //4S
				{
					send_to_app_count=0;
			//		Send_Live_Temp_Message(System_Run_Counter,in_temp-in_temp%5,Probe_Temp1-Probe_Temp1%5,Probe_Temp2-Probe_Temp2%5);		//����ʵʱ�¶�
					Send_Display_Data_Message();
				} 	 */
			}

			if(send_to_app_count>200)  //4S
			{
				send_to_app_count=0;
		//		Send_Live_Temp_Message(System_Run_Counter,in_temp-in_temp%5,Probe_Temp1-Probe_Temp1%5,Probe_Temp2-Probe_Temp2%5);		//����ʵʱ�¶�
//				if((in_temp_old!=(In_Temp-In_Temp%5))||(probe_temp_old!=(Probe_Temp1-Probe_Temp1%5))) //�¶ȱ仯�˲ŷ���
//				{
//					Send_Display_Data_Message(0x11);   //xuyao
//				}
			} 

		}
			}
		}

		delay_ms(20);
	}			
}		
//RUNOTHER����
void runother_task(void *pdata)
{
	u16 flags;
	u8 err;	
	u16 probe_temp_set_beep=0;
	u16 wifi_blink_counter=0;
	u16 recipe_beep_counter=0;
	while(1)
	{
 		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//һֱ�ȴ������ź�
//		LCD_Wifi_Dis();
/*		flags=OSFlagPend(flags_run,0X003c,OS_FLAG_WAIT_SET_ANY,0,&err);//�ȴ������ź���
		
		In_Temp=995;	//Ĭ��ΪERR��Ϊ995
		Send_Display_Data_Message();

	
		if(flags&0X0004)	//ERR1
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
		}
		else if(flags&0X0008)	//ERR2	
		{
			Flags_Run_Post=0;//���㣬������
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
		}
		else if(flags&0X0010)	//ERR3	
		{
			printf("ERR3\n"); 
		}
		else if(flags&0X0020)	//ERR4	
		{
			printf("ERR4\n"); 
		}

		
		Flags_Run_Post=0;//���㣬������
		OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//����ERRλ�ã���������

		HOT=Control_OFF;
		MOT=Control_OFF;
		FAN=Control_OFF;   */


/*		if(flags&0X003c)	//ERR��־λ��һ��Ϊ1�����ERRģʽ
		{
//			printf("ERR Mode\n");
			HOT=Control_OFF;
		}
		else 
		{
			OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//ȫ���ź�������
			OSFlagPost(flags_run,Flags_Run_On,OS_FLAG_SET,&err);////û�д����Ϳ���

		}  */
/********WiFi��־��˸**************/

		 if((Wifi_Blink==0x00) || (Wifi_Blink>0x04))   //�����ɹ�������
		 {
		  	LCD_Wifi_Undis();
		 }
		 else if((Wifi_Blink==0x01))
		 {
		  	LCD_Wifi_Dis(0x80);
		 }
		 else if((Wifi_Blink==0x02))
		 {
		  	LCD_Wifi_Dis(0xc0);
		 }
		 else if((Wifi_Blink==0x03))
		 {
		  	LCD_Wifi_Dis(0xe0);
		 }
		 else if((Wifi_Blink==0x04))
		 {
		  	LCD_Wifi_Dis(0xf0);
		 }
			
/********probe SET**************/
		 //for beep in CVW, it will use the PWM circuit, Kendy 20180701
		if((Probe_TempPC>=Probe_Temp_Set) && (SetPC_flag == 1))
		{
			probe_temp_set_beep++;
			Probe_Temp_Set=180;
			if(probe_temp_set_beep==1) BEEP=1;
			else if	(probe_temp_set_beep==6) BEEP=0;	  //(6-1)*50=250ms
			else if	(probe_temp_set_beep==11) BEEP=1;	  //(6-1)*50=250ms
			else if	(probe_temp_set_beep==16) BEEP=0;	  //(6-1)*50=250ms
			else if (probe_temp_set_beep>=32) probe_temp_set_beep=0;	  //(32-16)*50=800ms

		}
/********�յ��˵������������3��**************/
				 //for beep in CVW, it will use the PWM circuit, Kendy 20180701
		 if(Recipe_Beep==1)
		 {
		  	recipe_beep_counter++;
			if(recipe_beep_counter<=4) BEEP=1;
			else if(recipe_beep_counter<=8) BEEP=0;
			else if(recipe_beep_counter<=12) BEEP=1;
			else if(recipe_beep_counter<=16) BEEP=0;
			else if(recipe_beep_counter<=20) BEEP=1;
			else if(recipe_beep_counter<=24) BEEP=0;
			else 
			{
			 	Recipe_Beep=0;
				recipe_beep_counter=0;
			}
		 }
			delay_ms(50);
			if(Flags_Prime)BEEP=1;	 //primeû����
			else BEEP = 0;
	}									 
}
//========================================================================
// ����: float PT1000_CalculateTemperature(u16 fR)    
// ����: ���ó�PT1000���¶�
// ����: �Ŵ�10���ĵ���ֵ
// ����: �¶�
//========================================================================
float PT1000_CalculateTemperature(u16 fR)
{
    float fTem;
    float fLowRValue;
    float fHighRValue;        
    s16   iTem;
    u8 i;

    u8 cBottom, cTop;  //����Ԫ����ߺ����

    if (fR < RTD_TAB_PT1000[0])     // ����ֵС�ڱ����Сֵ�������������ޡ�
    {
            return 0;
    }

    if (fR > RTD_TAB_PT1000[60])    // ����ֵ���ڱ�����ֵ�������������ޡ�
    {
            return 300;				//�����60Ԫ�أ�300��
    }

    cBottom = 0; 
    cTop    =  61;

    for (i=31; (cTop-cBottom)!=1; )        // 2�ַ����i=Ԫ���ܸ���/2
    {
            if (fR < RTD_TAB_PT1000[i])
            {
                    cTop = i;
                    i = (cTop + cBottom) / 2;
            }
            else if (fR > RTD_TAB_PT1000[i])
            {
                    cBottom = i;
                    i = (cTop + cBottom) / 2;
            }
            else								//�����ҵ���ֵ��������
            {
                    iTem = (u16)i * 5;	 //�˼�����Ԫ�ر�ź��¶ȵĶ�Ӧ�����������1Ԫ���У���Ӧ���¶�Ϊ5
                    fTem = (float)iTem;

                    return fTem;
            }
    }

    iTem = (u16)i * 5;

    fLowRValue  = RTD_TAB_PT1000[cBottom];
    fHighRValue = RTD_TAB_PT1000[cTop];

    fTem = ( ((fR - fLowRValue)*5) / (fHighRValue - fLowRValue) ) + iTem;        // �������5��Ϊһ���ġ�
                                                                                                                                                    // �����ڲ�������㡣

    return fTem;
}


void KEY1_IRQHandler(void)
{
  OSIntEnter();
	//ȷ���Ƿ������EXTI Line�ж�
	if(EXTI_GetITStatus(KEY1_INT_EXTI_LINE) != RESET) 
	{
		delay_ms(20);
		if((KEY_Encoder_b==0))//(KEY_Encoder_a==1)&&
		{
			Rotation_Positive = 0;
			Rotation_Negative = 1;
			Set_temp_change=1;
		}
		else //if((KEY_Encoder_b==1))//(KEY_Encoder_a==0)&&
		{
			Rotation_Positive = 1;
			Rotation_Negative = 0;
			Set_temp_change=1;//�ı��¶���Ļ����˸
		}
		
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     
	} 
OSIntExit();	
}
