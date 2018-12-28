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
//////////////////////////全局变量////////////////////////////////////
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
u16 In_Temp=0;				//炉内温度
u16 In_Temp_Uncompensated; //Temp in temperature uncompensated
u16 In_Temp_Set=350;				//炉内设定温度，默认350
u16 In_Temp_Set_Max=500;				//炉内设定温度最高

u16 Probe_Temp_Set=225;				//probe，默认225
u16 Probe_Temp_Set_Max=225;				//probe，默认最大225
u16 Probe_Temp_Set_Min=50;				//probe，默认最小50

u16 Probe_Temp1=0;			//probe1温度
u16 Probe_Temp2=0;			//probe2温度
u16 Probe_TempPC=0;			//probe2温度

u8 Time_Set_Hour=0;				//小时时间设定
u8 Time_Set_Min=0;				//分钟时间设定
u8 Time_Actual_Hour=0;				//小时时间运行
u8 Time_Actual_Min=0;				//分钟时间运行
u16 Time_Actual=0;				//总体的运行分钟时间
u8 Time_End=0;				//时间到时标志

u8 Temp_Press_Step=0;//0：未按压，1：按一次闪烁，设定温度，2：设定温度
u8 Time_Press_Step=0;//0：未按压，1：按一次小时闪烁，设定小时，2：按一次分钟闪烁，设定分钟，3：设定时间

u8 Start_Hot=1;//为1则开始计时加热,GRILL一开始就加热
u8 HOT_Continue_On=0;//连续加热超过限定时间，停一下

u8  Flags_Err=0;//err
u8  Flags_Prime=0;//prime

u16 Set_Interface_Count=0;				//在set界面5S没有动作自动转到actual界面

u8 uCounter=0; //PID
u16 PID_Cycle=0;	//pid运行周期
u16 PID_Counter=0;	//pid运行周期计数

u32 System_Run_Counter=0;	//机器运行时间，开机开始，关机结束

u8 Key_Up_Down_Send_to_APP=0; //长按上下按键，松手后向app发送一次数据
u8 Wifi_Blink=0; //不同的状态闪烁间隔不同

u8 Lcd_refresh_CNT=0;
u16 count_hot_on_CNT=0,count_hot_on_CNT_buf=0;
u8 count_hot_on_CNT_Enable=0;
char main_char[6];

u8 Set_temp_change=0;
u8 count_low_temp_enable=0;
u16 count_low_temp=0;	  //低于130f计数

u8 count_hot_on_CNT_Enable_buf=0,count_low_temp_enable_buf=0;	
u8	Key_UpDown_Long_flag = 0; //only for long UP/DOWN specified function Kendy 20181217

////main_task start//////////////
 	u8 tmr2sta=1;	//软件定时器2开关状态   
 	u8 tmr3sta=0;	//软件定时器3开关状态
	u16 count_hot_on=0;	  //第一次进入点火
	u8 start_num=0,start_num_buf=0;//点火次数
	u16 start_temp=0,start_temp_buf=0;//开始点火时的温度
	u16 start_temp_rise=0;//点火结束的温升
	u8 start_mode=0,start_mode_buf=0;//点火开始

	u8 low_temp_start_mode=0;//低于130f计数，进去start

	u8 APP_Lock_Flags=0;
	u8 FAN_buf=0,HOT_buf=0,MOT_buf=0;
////main_task end//////////////

////adc_task start//////////////
	u8 fan_sensor=0,fan_sensor_plus=0;
	u8 hot_sensor=0,hot_sensor_plus=0;
	u8 mot_sensor=0,mot_sensor_plus=0;
	u8 fan_sensor_1=0,hot_sensor_1=0,mot_sensor_1=0;
////adc_task end//////////////

u16	RTD_TAB_PT1000[62]={10000,10195,10390,10584,10779,10973,11167,11360,11554,11747, //0--45		5度一个档位	
									11939,12132,12324,12516,12707,12898,13089,13280,13470,13660,//50--95
									13850,14040,14229,14418,14606,14795,14983,15170,15358,15545,//100--145
									15732,15919,16105,16291,16477,16662,16847,17032,17217,17401,//150--195
									17585,17769,17952,18135,18318,18501,18683,18865,19047,19228,//200--245
									19409,19590,19771,19951,20131,20311,20490,20669,20848,21026,//250--295
									21205,21400};//300--305		

float PT1000_CalculateTemperature(u16 fR);

/////////////////////////UCOSII任务设置///////////////////////////////////
//START 任务
//设置任务优先级
#define START_TASK_PRIO      			15 //开始任务的优先级设置为最低
//设置任务堆栈大小
#define START_STK_SIZE  				64
//任务堆栈	
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);	
 			   
//LED任务
//设置任务优先级
#define LED_TASK_PRIO       			8 
//设置任务堆栈大小
#define LED_STK_SIZE  		    		64
//任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);


//队列消息显示任务
//设置任务优先级
#define QMSGSHOW_TASK_PRIO    			6
//设置任务堆栈大小
#define QMSGSHOW_STK_SIZE  		 		64
//任务堆栈	
OS_STK QMSGSHOW_TASK_STK[QMSGSHOW_STK_SIZE];
//任务函数
void qmsgshow_task(void *pdata);


//主任务
//设置任务优先级
#define MAIN_TASK_PRIO       			5 
//设置任务堆栈大小
#define MAIN_STK_SIZE  					128
//任务堆栈	
OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];
//任务函数
void main_task(void *pdata);

/*//信号量集任务
//设置任务优先级
#define FLAGS_TASK_PRIO       			4 
//设置任务堆栈大小
#define FLAGS_STK_SIZE  		 		64
//任务堆栈	
OS_STK FLAGS_TASK_STK[FLAGS_STK_SIZE];
//任务函数
void flags_task(void *pdata); */


//按键扫描任务			 
//设置任务优先级
#define KEY_TASK_PRIO       			2 
//设置任务堆栈大小
#define KEY_STK_SIZE  					64
//任务堆栈	
OS_STK KEY_TASK_STK[KEY_STK_SIZE];
//任务函数
void key_task(void *pdata);

//按键扫描任务得到按键后处理			 
//设置任务优先级
#define KEY_Work_TASK_PRIO       			4
//设置任务堆栈大小
#define KEY_Work_STK_SIZE  					64
//任务堆栈	
OS_STK KEY_Work_TASK_STK[KEY_Work_STK_SIZE];
//任务函数
void KEY_Work_task(void *pdata);


//ADC任务			 
//设置任务优先级
#define ADC_TASK_PRIO       			3 
//设置任务堆栈大小
#define ADC_STK_SIZE  					64
//任务堆栈	
OS_STK ADC_TASK_STK[ADC_STK_SIZE];
//任务函数
void adc_task(void *pdata);


//err任务			 
//设置任务优先级
#define ERR_TASK_PRIO       			9 
//设置任务堆栈大小
#define ERR_STK_SIZE  					64
//任务堆栈	
OS_STK ERR_TASK_STK[ERR_STK_SIZE];
//任务函数
void err_task(void *pdata);

//关机任务			 
//设置任务优先级
#define OFF_TASK_PRIO       			10 
//设置任务堆栈大小
#define OFF_STK_SIZE  					64
//任务堆栈	
OS_STK OFF_TASK_STK[OFF_STK_SIZE];
//任务函数
void off_task(void *pdata);

//LCD显示任务			 
//设置任务优先级
#define LCD_TASK_PRIO       			11 
//设置任务堆栈大小
#define LCD_STK_SIZE  					64
//任务堆栈	
OS_STK LCD_TASK_STK[LCD_STK_SIZE];
//任务函数
void lcd_task(void *pdata);

//运行中其他任务显示任务			 
//设置任务优先级
#define RUNOTHER_TASK_PRIO       			12 
//设置任务堆栈大小
#define RUNOTHER_STK_SIZE  					64
//任务堆栈	
OS_STK RUNOTHER_TASK_STK[LCD_STK_SIZE];
//任务函数
void runother_task(void *pdata);

//////////////////////////////////////////////////////////////////////////////
    
OS_EVENT * msg_key;			//按键邮箱事件块	  
OS_EVENT * q_msg;			//消息队列
OS_TMR   * tmr1;			//软件定时器1
OS_TMR   * tmr2;			//软件定时器2
OS_TMR   * tmr3;			//软件定时器3
//OS_FLAG_GRP * flags_key;	//按键信号量集
OS_FLAG_GRP * flags_run;	//运行中的一些信号量集
u16 Flags_Run_Post=0;		//发送运行状态出去
/**********16位，从低到高说明，1有效***************
0：开机位
1：关机位
2：错误1，没有RTD
3：错误2，预留
4：错误3，预留
5：错误4，预留
6: FC
7：Prime
8：Set/Actuall
*/
u16  Flags_Run_On=0x0001;//开机
u16  Flags_Run_Off=0x0002;//关机
u16  Flags_Run_Err1=0x0004;//错误1 没有RTD
u16  Flags_Run_Err2=0x0008;//错误1 点火没达到设定温度，或运行中重新点火失败
u16  Flags_Run_Err3=0x0010;//错误3 重新点火失败
u16  Flags_Run_Err4=0x0020;//错误4 温度过高
u16  Flags_C_DIS=0x0040;//F
u16  Flags_Set_Actuall=0x0100;//Set/Actuall


//#define  Flags_Run_On  0x0001//开机
//#define  Flags_Run_Off  0x0002//关机
//#define  Flags_Run_Err1  0x0004//错误1


void * MsgGrp[6];			//消息队列存储地址,最大支持6个消息


//软件定时器1的回调函数	
//每100ms执行一次	   
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
/*********机器运行时间，开机开始，关机结束*************/
	if(Start_Hot) System_Run_Counter++;
	else System_Run_Counter=0;
	if(System_Run_Counter>=599940)	System_Run_Counter=0;//最大59994S，999.9MIN
/*********菜单开始运行*************/
	if(Recipe_Start==1)//菜单开始运行
	{
	 	count_recipe++;
		if(count_recipe>=600) //一分钟
		{
			Recipe_Time_Count++;
			count_recipe=0;
		}

		if(Recipe_Time_Count>=Recipe_List[5+7*(Recipe_Step-1)]*100+Recipe_List[6+7*(Recipe_Step-1)]*10+Recipe_List[7+7*(Recipe_Step-1)]) //取出相应步骤的时间
		{
			Recipe_Step++;//步骤加1，下一个步骤
			if(Recipe_Step>Recipe_List[0])
			{
				Recipe_Start=2;//菜单结束，保温
				Recipe_Step=0;
			}
			Recipe_Time_Count=0;
		}

		In_Temp_Set=Recipe_List[2+7*(Recipe_Step-1)]*100+Recipe_List[3+7*(Recipe_Step-1)]*10+Recipe_List[4+7*(Recipe_Step-1)];
	
	}
	else if(Recipe_Start==2)//菜单结束后的保温
	{
	 	In_Temp_Set=180;
		Recipe_Time_Count=0;
	
	}
	/*	static u16 count=0;	
	static u16 HOT_Continue_count=0;	  //加热棒连续工作计数，59分55秒，停5s，灯还是亮的
	static u16 HOT_Stop_count=0;	  //加热棒连续工作计数，59分55秒，停5s，灯还是亮的
	    
	if(Start_Hot) //有开始加热信号			   //grill屏蔽
	{
		if(Time_Actual) //时间不为0开始加热
		{
			count++;
			if(count>=600) //1分钟
			{
				count=0;

				Time_Actual--;
			}
			Time_Actual_Hour=Time_Actual/60;//分钟化成小时
			Time_Actual_Min=Time_Actual%60;//分钟
		}
		else
		{
			Time_Actual_Hour=0;		  //清零
			Time_Actual_Min=0;
			Time_Actual=0;
			Time_Set_Hour=0;
			Time_Set_Min=0;	
			In_Temp_Set=100;
			Start_Hot=0;

			Time_End=1;//说明时间到时
		}*/
/*********连续加热到限定时间，停止几秒钟******/
/*		if(HOT_Status==Control_ON)					  //grill屏蔽
		{
		 	HOT_Continue_count++;
			if(HOT_Continue_count>=35950)		 //运行59分55秒	  35950
			{
				HOT=Control_OFF;
//				LED_Element=LED_OFF;
				HOT_Continue_On=1;
			}
		}
		else HOT_Continue_count=0;

		if(HOT_Continue_On)		//停止计时
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
/*******feed循环进料******/	
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

//软件定时器2的回调函数				  	   
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
//软件定时器3的回调函数				  	   
void tmr3_callback(OS_TMR *ptmr,void *p_arg) 
{	
	u8* p;	 
	u8 err; 
	static u8 msg_cnt=0;	//msg编号	  
	if(p)
	{
	 	sprintf((char*)p,"ALIENTEK %03d",msg_cnt);
		msg_cnt++;
		err=OSQPost(q_msg,p);	//发送队列
		if(err!=OS_ERR_NONE) 	//发送失败
		{
			OSTmrStop(tmr3,OS_TMR_OPT_NONE,0,&err);	//关闭软件定时器3
 		}
	}
} 
	 
										

 int main(void)
 {	 
  
 	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);
	Usart_DMA_Init();
	Adc_Init();
	LED_Init();		  		//初始化与LED连接的硬件接口
 	BEEP_Init();			//蜂鸣器初始化	
//	KEY_Init();				//按键初始化 
	Control_Init();			//控制的三个变量
	Ht1621_Init();			//LCD初始化
	EXTI_Key_Config(); 
	//Wifi_Init();

	TIM_Cmd(GENERAL_TIM4, ENABLE);
	delay_ms(250);
	TIM_Cmd(GENERAL_TIM4, DISABLE);
	LED_debug_Power=LED_ON_DEBUG;	//FC backlight will be on after power on
	LED_backlight=LED_ON_EN;	//ON/OFF backlight will be on after power on
	
	BBQ_Status_Struct.onoff=2;	 //一开机应该是关机状态
	BBQ_Status_Struct.fc=0x01;//一开机显示F

	OSInit();  	 			//初始化UCOSII

 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();	    
}							    
//开始任务
void start_task(void *pdata)
{
  OS_CPU_SR cpu_sr=0;
	u8 err;	
	    	    
	pdata = pdata; 	
	msg_key=OSMboxCreate((void*)0);		//创建消息邮箱
	q_msg=OSQCreate(&MsgGrp[0],256);	//创建消息队列
// 	flags_key=OSFlagCreate(0,&err); 	//创建信号量集		  
 	flags_run=OSFlagCreate(0,&err); 	//创建信号量集		  
	  
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    
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
	
 	OSTaskSuspend(START_TASK_PRIO);	//挂起起始任务.
	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)
}
//LED任务
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
		flags=OSFlagPend(flags_run,0X013d,OS_FLAG_WAIT_SET_ANY,1,&err);//有开机信号,err信号
		t++;
		delay_ms(10);
	
			/*****ERR结束初始化*****/
			if(!(flags&0X003c))	//ERR标志位没有一个为1
			{

				if(!Flags_Run_Post)	//为0为刚开机或err
				{
					LCD_All_Off();
				}
			}
	}
}
//队列消息显示任务
void qmsgshow_task(void *pdata)
{
//	u8 *p;
//	u8 err;
	while(1)
	{
//		p=OSQPend(q_msg,0,&err);//请求消息队列
		delay_ms(500);	 
	}									 
}
//主任务
void main_task(void *pdata)
{							 
	u8 err;
	u16 flags,temp;
// 	tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100ms执行一次
//	tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200ms执行一次
//	tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100ms执行一次
//	OSTmrStart(tmr1,&err);//启动软件定时器1				 
//	OSTmrStart(tmr2,&err);//启动软件定时器2				 
 	while(1)
	{

		flags=OSFlagPend(flags_run,0X0003,OS_FLAG_WAIT_SET_ANY,0,&err);//一直等待开机关机信号
		 
		temp=In_Temp;//110;//
		 
		if(flags&Flags_Run_Off)
		{
			count_hot_on=0;		//关机后一些信号要清零
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
					start_temp_buf=start_temp;//start模式前获取当前温度
					start_num_buf=start_num;   //点火次数
					start_mode_buf=start_mode;//开始点火
					FAN_buf=FAN;	  //风扇全开
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
					start_temp=start_temp_buf;//start模式前获取当前温度
					start_num=start_num_buf;   //点火次数
					start_mode=start_mode_buf;//开始点火
					FAN=FAN_buf;	  //风扇全开
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
					start_temp=temp;//start模式前获取当前温度
					start_num++;   //点火次数
					start_mode=1;//开始点火
					FAN=Control_ON;	  //风扇全开
					HOT=Control_ON;   
					All_Action_sensor_Status_flags|=0x05;
				}
				if(count_hot_on_CNT==3599)	 
				{
					if(temp>=start_temp) start_temp_rise=temp-start_temp;//start模式结束的温升
					else start_temp_rise=0;//start模式结束的温升
				}


				if(count_hot_on_CNT<3600)	  //10 6分钟start 3600
				{
					//count_hot_on_CNT_Enable=1;

					//All_Action_sensor_Status_flags|=0x05;
					
					if(!Flags_Prime)	 //prime没按下
					{

					/*	if (count_hot_on<1800)  MOT=Control_ON;		//前3分钟进料
						else if (count_hot_on>2400&&count_hot_on<2700)  MOT=Control_ON;		//4分钟后进料30秒
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
						else if(count_hot_on_CNT<=1800) 	//4。5	   270
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
						else if(count_hot_on_CNT<=3600)   //等待1分钟后检测温度	360 change from 3600 to 3200 Kendy20180710
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
					if(start_mode)		//start结束判定	  start 中
					{
						if(temp>130||start_temp_rise>40)   //温度大于130说明点火成功
						{
							start_num=0;
							count_hot_on_CNT_Enable=2;
						}
						else 
						{
							count_hot_on=0; //重新点火
							count_hot_on_CNT_Enable=4;
							count_hot_on_CNT=0;
						}
						if(low_temp_start_mode)	  //重新点火
						{
							low_temp_start_mode=0;
							if(start_num==1) 
							{
								OSFlagPost(flags_run,Flags_Run_Err3,OS_FLAG_SET,&err);//设置对应的信号量为1 计数2次说明两次点火失败
							}
						}
						else	   //start模式
						{
							if(start_num==4) 
							{
								OSFlagPost(flags_run,Flags_Run_Err2,OS_FLAG_SET,&err);//设置对应的信号量为1 计数2次说明两次点火失败
							}
						
						}
					}
					else		 //运行中检测是否低于130
					{
//						if(temp<130) count_low_temp++;
//						else count_low_temp=0;
					  if(temp<130) count_low_temp_enable=1;
						else count_low_temp_enable=0;
						//count_hot_on_CNT_Enable=0;
						if (count_low_temp>=6000)	   //大于10分钟还是温度低
						{
							count_low_temp_enable=0;
							low_temp_start_mode=1;
							count_hot_on=0; //重新点火
							count_hot_on_CNT_Enable=4;
							count_hot_on_CNT=0;
//							HOT=Control_OFF; 
//							OSFlagPost(flags_run,Flags_Run_Err3,OS_FLAG_SET,&err);//设置对应的信号量为1 计数2次说明两次点火失败
						}
//						else if(count_low_temp>=6000)	HOT=Control_ON; 	 //10分钟  
//						else  HOT=Control_OFF; 								//平常都是开启的

					
					}

					start_mode=0;
					HOT=Control_OFF; 
					All_Action_sensor_Status_flags&=0xFB;
				
				
/*					if(temp<130)   //温度大于130说明点火成功，同时运行中如果低于130则重新点火
					{
					 	count_hot_on=0;//重新点火
					}
					else start_num=0;

					if(start_num==2) OSFlagPost(flags_run,Flags_Run_Err2,OS_FLAG_SET,&err);//设置对应的信号量为1 计数2次说明两次点火失败
					*/
					HOT=Control_OFF; 
					All_Action_sensor_Status_flags&=0xFB;

					  PID.uKP_Coe=30;             //比例系数	10
				    PID.uKI_Coe=10;             //积分常数
				    PID.uKD_Coe=5;             //微分常数（为0不动）
				    PID.iSetVal=In_Temp_Set;             //设定值温度

				/***不同温度区分上下限，温度稳定****/
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

					PID_Cycle=6;//60S，要以10


					if(!Flags_Prime)	 //prime没按下
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

				    if(g_bPIDRunFlag)   //定时中断为多少MS，则周期为多少秒
				    {
				        g_bPIDRunFlag = 0;
				        if(iTemp) iTemp--;      //只有iTemp>0，才有必要减“1”
				        uCounter++;
				        if(100 <= uCounter)
				        {
				            PID_Operation(temp);    
				
				            uCounter = 0; 
		
/*							sprintf(main_char,"%hd",iTemp);	//h表示短整型
							printf(main_char);
							printf("\r\n");
						
							sprintf(main_char,"%hd",(u16)PID.iSetVal);	//h表示短整型
							printf(main_char);
							printf("\r\n");
						
							sprintf(main_char,"%hd",(u16)temp);	//h表示短整型
							printf(main_char);
							printf("\r\n");	   */
		
						}
					}


				
				}
		  }
		}
/*		  if(Time_End)//时间到时，蜂鸣器发出哔-哔-哔三短连声，响5次
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
			OSFlagPost(flags_key,1<<(key-1),OS_FLAG_SET,&err);//设置对应的信号量为1
		} */


/*		switch(key)
		{
			case 1://控制DS1
				LED1=!LED1;

//				OSFlagPost(flags_run,Flags_Run_Post&0x0001,OS_FLAG_SET,&err);//设置对应的信号量为1
				break;
			case 2://控制软件定时器3	 
				tmr3sta=!tmr3sta;
				if(tmr3sta)OSTmrStart(tmr3,&err);  
				else OSTmrStop(tmr3,OS_TMR_OPT_NONE,0,&err);		//关闭软件定时器3
 				break;
			case 3://清除
				break;
			case 4://校准
				OSTaskSuspend(QMSGSHOW_TASK_PRIO);	 				//挂起队列信息显示任务		 
 				OSTmrStop(tmr1,OS_TMR_OPT_NONE,0,&err);				//关闭软件定时器1
				if(tmr2sta)OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//关闭软件定时器2				 
 //				TP_Adjust();	   
				OSTmrStart(tmr1,&err);				//重新开启软件定时器1
				if(tmr2sta)OSTmrStart(tmr2,&err);	//重新开启软件定时器2	 
 				OSTaskResume(QMSGSHOW_TASK_PRIO); 	//解挂
				break;
			case 5://软件定时器2 开关
				tmr2sta=!tmr2sta;
				if(tmr2sta)OSTmrStart(tmr2,&err);			  	//开启软件定时器2
				else 
				{		    		    
  					OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//关闭软件定时器2
				}
				break;				 
				
		}  */
		delay_ms(100);
	}
}		   
/*//信号量集处理任务
void flags_task(void *pdata)
{	
	u16 flags;	
	u8 err;	 
	while(1)
	{
		flags=OSFlagPend(flags_key,0X001F,OS_FLAG_WAIT_SET_ANY,0,&err);//等待信号量

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
	    OSFlagPost(flags_key,0X001F,OS_FLAG_CLR,&err);//全部信号量清零，不清除一直存在
 	}
}	  */


//检测按键后的任务处理
void KEY_Work_task(void *pdata)
{	
	u8 err;	
	u32 key=0;	
// 	u16 flags;	
	u8 fc=0;
	u16 count=0;


	while(1)
	{

	   key=(u32)OSMboxPend(msg_key,0,&err);   //一直等待直到有按键邮件，只取一次信号便不存在
	   count++;

		switch(key)
		{
			case 1:
				if(Flags_Run_Post==0||Flags_Run_Post==Flags_Run_Off)	//为0则刚开电源，为Flags_Run_Off则刚执行关机
				{
					Key_UpDown_Long_flag = 0;
					OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//全部信号量清零
					Flags_Run_Post=Flags_Run_On|Flags_Set_Actuall;   //开机,及set信号
					OSFlagPost(flags_run,Flags_Run_Post,OS_FLAG_SET,&err);//设置对应的信号量为1

					HOT=Control_OFF;	 //开机清下状态
					All_Action_sensor_Status_flags&=0xFB;
//					LED_Element=LED_OFF;
					Time_Set_Hour=0;
					Time_Set_Min=0;	
					Temp_Press_Step=0;
					Time_Press_Step=0;
					In_Temp_Set=350;
					Probe_Temp_Set=225;
					Start_Hot=1;   //grill开机运行
					//BBQ_Status_Struct.onoff=1;

//					if (SetPC_flag == 1)
//					LCD_On_Init_Dis(In_Temp_Set,Probe_Temp_Set);
//					else LCD_On_NoPC_Init_Dis(In_Temp_Set);
					Temp_Press_Step=1;//一开机就set闪烁

					//Send_On_Message();//发送开机命令
					//Send_Display_Data_Message(0x11);//开机发送一次状态命令
//					Send_Set_Temp_Message(In_Temp_Set);//一开机就进入编辑模式
				tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100ms执行一次
				tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200ms执行一次
				tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100ms执行一次
				OSTmrStart(tmr1,&err);	//START软件定时器1
				OSTmrStart(tmr2,&err);	//START软件定时器2
				}
				break;
			case 2:
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//全部信号量清零
				Flags_Run_Post=Flags_Run_Off;	 //发送关机信号,
				OSFlagPost(flags_run,Flags_Run_Post,OS_FLAG_SET,&err);//设置对应的信号量为1
				/***********一些信号量的清除*********/
				HOT=Control_OFF;
				All_Action_sensor_Status_flags&=0xFB;
//				LED_Element=LED_OFF;
				Start_Hot=0;
				HOT_Continue_On=0;

				BBQ_Status_Struct.onoff=2;
//				Send_Off_Message();//发送关机命令
//				printf("KEY off\n"); 

 				break;
			case 3://PRIME按键,按下马达一直开
					Key_UpDown_Long_flag = 0;
				OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//有开机信号

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
				fc=!fc;	   //默认是0，为F
				if(fc) OSFlagPost(flags_run,Flags_C_DIS,OS_FLAG_SET,&err);//设置对应的信号量为1	 C
				else   OSFlagPost(flags_run,Flags_C_DIS,OS_FLAG_CLR,&err);//设置对应的信号量为0  F
				if(BBQ_Status_Struct.fc==0x01) BBQ_Status_Struct.fc=0x00;
			    else BBQ_Status_Struct.fc=0x01;
				Send_Display_Data_Message(0x09);
				break;
			case 5://set
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//设置对应的信号量为1
				Key_Temp_ShortPressOK=1;//无需再按下temp按键，温度直接闪烁

					
				break;

				if(Recipe_Start==2)//在菜单结束后的保温按下set按键，进入无菜单状态
				{
				 	Recipe_Start=0;	//清除保温
					Recipe_Step=0;//无菜单
				}
			case 6://actual
					Key_UpDown_Long_flag = 0;
				OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_CLR,&err);//设置对应的信号量为0  	 
				break;
			case 9://up
					Key_UpDown_Long_flag = 0;
				if(Key_Up_App_ShortPressOK)				  //APP发送
				{
					Key_Up_App_ShortPressOK=0;
					if(Temp_Press_Step==1) In_Temp_Set+=(u16)Key_Up_Down_App_Temp;	
					else if(Temp_Press_Step==2) Probe_Temp_Set+=(u16)Key_Up_Down_App_Temp;	
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//设置对应的信号量为1
				}
				else									 //本机
				{
					if(Temp_Press_Step==1)	  //temp闪烁设定
					{
						In_Temp_Set+=5;			 //每次加5F
					}
					else if((Temp_Press_Step==2) && (SetPC_flag == 1))	  //Probe temp闪烁设定
					{
						Probe_Temp_Set+=5;			 //每次加5F
					}
				}
			
				if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //最大500F
				if((Probe_Temp_Set>Probe_Temp_Set_Max) && (SetPC_flag == 1)) Probe_Temp_Set=Probe_Temp_Set_Max;		  //最大225F

				if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);
				else if((Temp_Press_Step==2)&& (SetPC_flag == 1))   Send_Set_Temp_Message(Probe_Temp_Set);//)
				break;
//			case 10://up长按
//					if(Temp_Press_Step==1)	  //temp闪烁设定
//					{
//						In_Temp_Set+=5;			 //每次加5F
//					}
//				
//					if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //最大500F
//				break;

			case 11://down
						Key_UpDown_Long_flag = 0;
					if(Key_Down_App_ShortPressOK)				  //APP发送
					{
						Key_Down_App_ShortPressOK=0;
							
						if(Temp_Press_Step==1) 	   //in temp
						{
							if(In_Temp_Set>180) In_Temp_Set-=(u16)Key_Up_Down_App_Temp;			 
							else In_Temp_Set=180;		  //最大500F
						}
						else if(Temp_Press_Step==2) 	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=(u16)Key_Up_Down_App_Temp;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //最大500F
						}
						OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//设置对应的信号量为1
					}
					else									 //本机
					{
						if(Temp_Press_Step==1)	  //temp闪烁设定
						{
							if(In_Temp_Set!=180) In_Temp_Set-=5;			 //每减加5F
							else In_Temp_Set=180;		  //最大500F
		
						}
						else if((Temp_Press_Step==2) && (SetPC_flag == 1)) 	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=5;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //最大500F
						}	
			    	}
					if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);
					else if((Temp_Press_Step==2)&& (SetPC_flag == 1))  Send_Set_Temp_Message(Probe_Temp_Set);//) 
			
				break;
//			case 12://down长按

//				if(Temp_Press_Step==1)	  //temp闪烁设定
//				{
//					if(In_Temp_Set>180) In_Temp_Set-=5;			 //每减加5F
//					else In_Temp_Set=180;		  //最大500F
//				}
//			
//				break;
			case 13://APP发过来直接设定温度
					Key_UpDown_Long_flag = 0;
				if(Temp_Press_Step==1) 	   //in temp
				{
					if(Key_Up_Down_App_Temp>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //最大500F
					else if(Key_Up_Down_App_Temp<180) In_Temp_Set=180;	//最小180
					else  In_Temp_Set=Key_Up_Down_App_Temp;
					Send_Set_Temp_Message(In_Temp_Set);	 //返回给APP
				}
				else if((Temp_Press_Step==2) && (SetPC_flag == 1)) 	//probe temp
				{
					if(Key_Up_Down_App_Temp>Probe_Temp_Set_Max) Probe_Temp_Set=Probe_Temp_Set_Max;		  //最大225F
					else if(Key_Up_Down_App_Temp<Probe_Temp_Set_Min) Probe_Temp_Set=Probe_Temp_Set_Min;	//最小50
					else  Probe_Temp_Set=Key_Up_Down_App_Temp;
					Send_Set_Temp_Message(Probe_Temp_Set);	 //返回给APP
				}	
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_SET,&err);//设置对应的信号量为1

				break;
			case 14://发送wifi配置命令
					Key_UpDown_Long_flag = 0;
				Send_Goto_WiFi_Password_Mode();
				break;
			case 20://不起作用，只是让蜂鸣器响
					Key_UpDown_Long_flag = 0;
				break;
		}  

		BEEP=1;
		delay_ms(50);
		BEEP=0;

		Set_Interface_Count=0;				//在set界面5S没有动作自动转到actual界面
 	}
}															    
   	   		    
//按键扫描任务
 void key_task(void *pdata)
{	
	u8 key=0;
	u8 err;	
	u8 key_prime_press=0;
	u8 key_off_press=0;
	u8 key_set_press=0;

 	u16 flags;
	u16 count=0;
	u8 long_press_beep=0;//长按结束蜂鸣器不响
	u8 send_prime_cmd=1;//向APP发送PRIME命令
		    						 
	while(1)
	{

		flags=OSFlagPend(flags_run,0X013D,OS_FLAG_WAIT_SET_ANY,1,&err);//有开机信号或者是错误

	
		//if(!(flags&Flags_Run_Err1))	//ERR1标志位 RTD 不为1，才扫描按键
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
				key=1;						  //开机
				Key_Onoff_ShortPressOK=0;
			} 
		

				if(!key_off_press)				   // key_off_press为1情况下只检测什么时候释放
				{
					if(Key_Onoff_LongPressOK)
					{
						key=2;						  
						key_off_press=1;
					}

					if(Key_Onoff_App_LongPressOK)		//app发送关机命令
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
	
		if(flags&0x0001)	//开机状态下
		{
			BSP_Key_FC_Short_Long();
			BSP_Key_Set_Short_Long();// in CVW-V1 no set button, 20180618 Kendy
			BSP_Key_Actual_Short_Long();// in CVW-V1 no actual button, 20180618 Kendy


//			if(Key_Prime_LongPressOK)
//			{
//				key=3;						  //prime按键
//				Key_Prime_LongPressOK=0;
//			}  
/*************prime***************/	
			if(Start_Hot)							//在开始工作加热的情况下PRIME才起作用
			{
					if(Key_Prime_ContinuousPressOK||Key_Prime_App_ContinuousPressOK)
					{
						key=3;						  //prime按键
//						Key_Prime_ContinuousPressOK=0;
						if(send_prime_cmd) 
						{
//							Send_Feed_Start_Message();
							send_prime_cmd=0;
						}
					}
					else 
					{
						Flags_Prime=0;				 //松开清除PRIME状态 
						if(!send_prime_cmd) 
						{
//							Send_Feed_End_Message();
							send_prime_cmd=1;
						}
					}
					 


	 		 }
/**************app发送过来的设定温度************/
				/******up********/	
				if(Key_Up_App_ShortPressOK)
				{
					if(Key_Up_ShortPressOK)
					{
						key=9;						  //up按键
						Key_Up_ShortPressOK=0;
					}
				}


				/******down********/	
				if(Key_Down_App_ShortPressOK)
				{
					if(Key_Down_ShortPressOK)
					{
						key=11;						  //down按键
						Key_Down_ShortPressOK=0;
					}
				} 
  /************APP直接设置命令***************/
				if(Key_Direct_App_ShortPressOK)
				{
					key=13;						  //down按键
					Key_Direct_App_ShortPressOK=0;
				}

/*************fc***************/	
			if(Key_FC_ShortPressOK)
			{
				key=4;						  //FC按键
				Key_FC_ShortPressOK=0;
			} 
/*************set***************/	
			if(Key_Set_ShortPressOK)
			{
				key=5;						  //set按键
				Key_Set_ShortPressOK=0;
			}


			if(!key_set_press)				   // key_set_press为1情况下只检测什么时候释放
			{
				if(Key_Set_LongPressOK)		  //长按发送wifi配置命令
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
				key=6;						  //ACTUAL按键
				Key_Actual_ShortPressOK=0;
			}

			if(flags&Flags_Set_Actuall)	//SET界面下，才允许上下按键和time按键
			{
			
			
				BSP_Key_Down_Short_Long();
				BSP_Key_Up_Short_Long();
//				BSP_Key_Temp_Short_Long();
				//BSP_Key_Time_Short_Long();
			
			
				/******TIME********/				 //grill没有TIME按键  //grill屏蔽
//	/*			if(Key_Time_ShortPressOK)
//				{
//					key=20;						  //time按键
//					Key_Time_ShortPressOK=0;

//					Time_Press_Step++;
//					if(Time_Press_Step>=3) 
//					{
//						Time_Press_Step=0;
//						
//						Time_Actual_Hour=Time_Set_Hour;	   
//						Time_Actual_Min=Time_Set_Min;
//						Time_Actual=(u16)Time_Actual_Hour*60+(u16)Time_Actual_Min;

//						if(Time_Actual) //时间不为0才开始加热
//						{
//							Start_Hot=1;//开始加热
//						}
//					}
//					else if(Time_Press_Step>=1) //1和2的情况  时间变动立刻停止加热初始化
//					{
//						Start_Hot=0;//停止加热
//						Time_Actual_Hour=0;		  //清零
//						Time_Actual_Min=0;
//						Time_Actual=0;
//					}

//					Temp_Press_Step=0;//按下time，temp不再闪烁

//				}  	  */
				/******Temp********/	
				if(Key_Temp_ShortPressOK)
				{
//					key=20;						  //temp按键
					Key_Temp_ShortPressOK=0;
//					Temp_Press_Step=1;//一直闪烁
					if (SetPC_flag == 1)Temp_Press_Step++;
					else Temp_Press_Step=1;
					
					if(Temp_Press_Step>2) Temp_Press_Step=1;  //1 grill set 2 probe set

					if(Temp_Press_Step==1) Send_Set_Temp_Message(In_Temp_Set);//进入温度编辑模式
					else  if(SetPC_flag == 1)Send_Set_Temp_Message(Probe_Temp_Set);//进入probe1温度编辑模式//

/*					if(Temp_Press_Step>=2) 					 //grill屏蔽
					{
					
						Time_Actual_Hour=Time_Set_Hour;	   	  //设定完温度，计算一下时间，不为0则开始加热
						Time_Actual_Min=Time_Set_Min;
						Time_Actual=(u16)Time_Actual_Hour*60+(u16)Time_Actual_Min;
						
						if(Time_Actual) //时间不为0才开始加热			
						{
							Start_Hot=1;//开始加热
						}
						Temp_Press_Step=0;
					}
					else if(Temp_Press_Step==1) Start_Hot=0;//停止加热,正在设定温度	  */


					Time_Press_Step=0;	   //按下temp。time不再闪烁

				}

				/******up********/	
				if(Key_Up_ShortPressOK)//()Rotation_Positive
				{
					key=9;						  //up按键
					Key_Up_ShortPressOK=0;//Rotation_Positive=0;
				}


				/******down********/	
				if(Key_Down_ShortPressOK)//()Rotation_Negative
				{
					key=11;						  //down按键
					Key_Down_ShortPressOK=0;//Rotation_Negative=0;
				}


				if(Key_Up_LongPressOK)
				{
					Key_Up_Down_Send_to_APP=1;		  //发送up温度
					Set_Interface_Count=0;				//在set界面5S没有动作自动转到actual界面

					key=10;			
					Key_UpDown_Long_flag = 1;
					Key_Up_LongPressOK=0;
				//	BEEP=1;
					long_press_beep=1;
					count++;
					if(count>=2)
					{
						count=0;
						if(Temp_Press_Step==1)	  //temp闪烁设定
						{
							In_Temp_Set+=5;			 //每次加1F
						}
						if((Temp_Press_Step==2)&&	(SetPC_flag == 1))  //probe闪烁设定
						{
							Probe_Temp_Set+=5;			 //每次加1F
						}

					

						if(In_Temp_Set>In_Temp_Set_Max) In_Temp_Set=In_Temp_Set_Max;		  //最大500F
						if((Probe_Temp_Set>Probe_Temp_Set_Max)&&	(SetPC_flag == 1))  
						{
							Probe_Temp_Set=Probe_Temp_Set_Max;		  //最大225F//
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
					
					Key_Up_Down_Send_to_APP=1;	  //发送down 温度

					Set_Interface_Count=0;				//在set界面5S没有动作自动转到actual界面
					Key_UpDown_Long_flag = 1;
					key=12;						   
					Key_Down_LongPressOK=0;
	//				BEEP=1;
					long_press_beep=1;
					count++;
					if(count>=2)
					{
						count=0;
						if(Temp_Press_Step==1)	  //temp闪烁设定
						{
							if(In_Temp_Set>180) In_Temp_Set-=5;			 //每减加1F
							else In_Temp_Set=180;		  //最大400F
						}
						else if((Temp_Press_Step==2)&&	(SetPC_flag == 1))  	//probe temp
						{
							if(Probe_Temp_Set>Probe_Temp_Set_Min) Probe_Temp_Set-=5;			 
							else Probe_Temp_Set=Probe_Temp_Set_Min;		  //最大500F
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

		if(key)OSMboxPost(msg_key,(void*)key);//发送消息
		key=0;						 
 		delay_ms(10);
	}
}

//ADC任务
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
		
		
		
/*************炉内温度计算*****************/

	  if(temp_in>=2100)			//	   高温高于640F报警
		{							
			OSFlagPost(flags_run,Flags_Run_Err1,OS_FLAG_SET,&err);//设置对应的信号量为1
			//加上补偿40，实际640才会高温报警
			ProbeTempIn_ERR=1;
		}
		else if(temp_in>=2065)			//3535 2.84 315	   高温高于600F报警
		{							
			OSFlagPost(flags_run,Flags_Run_Err4,OS_FLAG_SET,&err);//设置对应的信号量为1
			//加上补偿40，实际640才会高温报警
		}
		else
		{
			ProbeTempIn_ERR=0;
			OSFlagPost(flags_run,Flags_Run_Err1,OS_FLAG_CLR,&err);//设置对应的信号量为0  	 
			OSFlagPost(flags_run,Flags_Run_Err4,OS_FLAG_CLR,&err);//设置对应的信号量为0  	
			if(temp_in<=1356) R_Value_in=10000;	  //0度 1K ACTUAL KENDY
			else if(temp_in>=2100) R_Value_in=21205;	  //300度
			else
			{
			 temp1=2000*(u32)temp_in;
			 temp2=(u32)(4096-temp_in);
			 R_Value_in = 10*(temp1/temp2);
			} 		 
	
			In_Temp=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			In_Temp=In_Temp*9/5+32;  //转成华氏度
			
			//In_Temp=345;
			/*****补偿*****/
			In_Temp_Uncompensated=In_Temp;

			
			if(In_Temp>=300&&In_Temp<=369) In_Temp=In_Temp+In_Temp%100;	//300--369每隔1F就多补偿1F
			else if (In_Temp>=370) In_Temp=In_Temp+70;					//370以上补偿70F
		}
/*************probe1温度计算*****************/
	  if(temp_probe1>=2100)			//2124(2.2K actual) 1.728V >300	   高温高于600F报警		
		{
//			printf("no probe\n"); 
			Probe_Temp1=0;  //没有插显示0
			Probe1_ERR=1;
			BBQ_Status_Struct.probe_status=0;//只有一个probe没有插,就都没插

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//只有一个probe没有插,就都没插
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			Probe1_ERR=0;
			if(temp_probe1<=1356) R_Value_in=10000;	  //0度 KENDY
			else if(temp_probe1>=2100) R_Value_in=21205;	  //300度
			else
			{
			 temp1=2000*(u32)temp_probe1;
			 temp2=(u32)(4096-temp_probe1);
			 R_Value_in =10*(temp1/temp2);
			} 		 
	
			Probe_Temp1=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_Temp1=Probe_Temp1*9/5+32;  //转成华氏度
	
		}
/*************probe2温度计算*****************/
	  if(temp_probe2>=2100)			//2124(2.2K actual) 1.728V >300	   高温高于600F报警		
		{
//			printf("no probe\n"); 
			Probe_Temp2=0;  //没有插显示0
			Probe2_ERR=1;
			BBQ_Status_Struct.probe_status=0;//只有一个probe没有插,就都没插

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//只有一个probe没有插,就都没插
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			Probe2_ERR=0;
			if(temp_probe2<=1356) R_Value_in=10000;	  //0度 KENDY
			else if(temp_probe2>=2100) R_Value_in=21205;	  //300度
			else
			{
			 temp1=2000*(u32)temp_probe2;
			 temp2=(u32)(4096-temp_probe2);
			 R_Value_in =10*(temp1/temp2);
			} 		 
	
			Probe_Temp2=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_Temp2=Probe_Temp2*9/5+32;  //转成华氏度
	
		}
/*************probePC温度计算*****************/
	  if(temp_probePC>=2100)			//2124(2.2K actual) 1.728V >300	   高温高于600F报警				
		{
//			printf("no probe\n"); 
			Probe_TempPC=0;  //没有插显示0
			ProbePC_ERR=1;
			BBQ_Status_Struct.probe_status=0;//只有一个probe没有插,就都没插

			if(probe_status_none) 
			{
//				Send_Probe_Statue_Message(0);
				probe_status_none=0;
				probe_status=1;
			}
		}
		else
		{
			BBQ_Status_Struct.probe_status=1;//只有一个probe没有插,就都没插
			if(probe_status) 
			{
//				Send_Probe_Statue_Message(1);
				probe_status=0;
				probe_status_none=1;
			}

			ProbePC_ERR=0;
			if(temp_probePC<=1356) R_Value_in=10000;	  //0度 KENDY
			else if(temp_probePC>=2100) R_Value_in=21205;	  //300度
			else
			{
			 temp1=2000*(u32)temp_probePC;
			 temp2=(u32)(4096-temp_probePC);
			 R_Value_in = 10*(temp1/temp2);
			} 		 
	
			Probe_TempPC=(u16)PT1000_CalculateTemperature((u16)(R_Value_in));
			Probe_TempPC=Probe_TempPC*9/5+32;  //转成华氏度
	
		}			
	
		delay_ms(100);
	}			
}

//ERR任务
void err_task(void *pdata)
{
	u16 flags;
	u8 err;	
	while(1)
	{
// 		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//一直等待开机信号
		
		flags=OSFlagPend(flags_run,0X003c,OS_FLAG_WAIT_SET_ANY,0,&err);//等待错误信号量
		

	
		if(flags&0X0004)	//ERR1
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
		
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			
			In_Temp=980;	//默认为ER1置为980
		}
		else if(flags&0X0008)	//ERR2	
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;

			All_Action_sensor_Status_flags&=0xDA;
			In_Temp=975;	//默认为ER2置为975
		}
		else if(flags&0X0010)	//ERR3	
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			All_Action_sensor_Status_flags|=0x80;
			In_Temp=970;	//默认为ER3置为970
		}
		else if(flags&0X0020)	//ERR4	
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
			All_Action_sensor_Status_flags&=0xDA;
			In_Temp=965;	//默认为ERH置为965
		}

		
		Flags_Run_Post=0;//清零，开机键
		OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零

		HOT=Control_OFF;
		MOT=Control_OFF;
		FAN=Control_OFF;
		All_Action_sensor_Status_flags&=0xDA;

/*		if(flags&0X003c)	//ERR标志位有一个为1则进入ERR模式
		{
//			printf("ERR Mode\n");
			HOT=Control_OFF;
		}
		else 
		{
			OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//全部信号量清零
			OSFlagPost(flags_run,Flags_Run_On,OS_FLAG_SET,&err);////没有错误发送开机

		}  */


	   		  //蜂鸣器发热哔-哔两短连声，一直持续直至断开电源
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


//off任务
void off_task(void *pdata)
{
	u8 err;	
	u16 flags;
	u16 count=0;

	while(1)
	{
		flags=OSFlagPend(flags_run,0X0003,OS_FLAG_WAIT_SET_ANY,0,&err);//一直等待开机关机信号
		if(flags&Flags_Run_On) count=0;
		else//(flags&Flags_Run_Off)
		{
				 All_Action_sensor_Status_flags=0;
				 tmr2sta=1;	//软件定时器2开关状态   
				 tmr3sta=0;	//软件定时器3开关状态
				 count_hot_on=0;	  //第一次进入点火
				 start_num=0,start_num_buf=0;//点火次数
				 start_temp=0,start_temp_buf=0;//开始点火时的温度
				 start_temp_rise=0;//点火结束的温升
				 start_mode=0,start_mode_buf=0;//点火开始
					count_hot_on_CNT_Enable=0;
					count_hot_on_CNT=0;
				  low_temp_start_mode=0;//低于130f计数，进去start
					count_low_temp_enable=0;
				  count_low_temp=0;
			
				 APP_Lock_Flags=0;
				 FAN_buf=0,HOT_buf=0,MOT_buf=0;
//				tmr1=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr1_callback,0,"tmr1",&err);		//100ms执行一次
//				tmr2=OSTmrCreate(10,20,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr2_callback,0,"tmr2",&err);		//200ms执行一次
//				tmr3=OSTmrCreate(10,10,OS_TMR_OPT_PERIODIC,(OS_TMR_CALLBACK)tmr3_callback,0,"tmr3",&err);		//100ms执行一次
				OSTmrStop(tmr1,OS_TMR_OPT_NONE,0,&err);	//关闭软件定时器3
				OSTmrStop(tmr2,OS_TMR_OPT_NONE,0,&err);	//关闭软件定时器3
					HOT=Control_OFF;
					MOT=Control_OFF;

					fan_sensor=0,fan_sensor_plus=0;
					hot_sensor=0,hot_sensor_plus=0;
					mot_sensor=0,mot_sensor_plus=0;
					fan_sensor_1=0,hot_sensor_1=0,mot_sensor_1=0;
	
				if(count<=2800)		  //时间控制
				{
					count++;
					FAN=Control_ON;
			    All_Action_sensor_Status_flags|=0x01;
					
				}
				else 
				{
					if(In_Temp<130)		  //温度控制
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

//LCD任务
void lcd_task(void *pdata)
{
	u8 err;	
	u16 flags;
	u16 in_temp_set,in_temp,temp,in_tempkendy;
	u16 probe_temp;
	u16 count=0;
	u16 temp_x;
	u16 send_to_app_count=0;//每隔多长时间向APP发送
	u8 set_to_actuall=0;
	u16 in_temp_old=0;
	u16 probe_temp_old=0;
	u16 probe_temp_set=0;	
  
	
	while(1)
	{
//		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//一直等待开机信号
		flags=OSFlagPend(flags_run,0XFFFF,OS_FLAG_WAIT_SET_ANY,1,&err);//只要有信号就接收，根据不同信号显示
		count++;
		send_to_app_count++;
		if(send_to_app_count==1)
		{
			in_temp_old=In_Temp-In_Temp%5;
			probe_temp_old=Probe_Temp1-Probe_Temp1%5;//存储温度，4S后做比较，变化了才发送0b
		}


/*			if(In_Temp>=300&&In_Temp<=339)
			{
				temp_x=In_Temp-300;	//每隔1F减1
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
		else if(flags&0X0002)	//关机
		{
			if(APP_Display_Override==2) LCD_All_Off();
			else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
		 	//LCD_All_Off();
			BBQ_Status_Struct.onoff=2;
			All_Action_sensor_Status_display = 0;
		}
		else if(flags&0X0001)	//开机
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
//							LCD_ProbePC_Temp(Probe_TempPC-Probe_TempPC%5);  //pc温度
//							LCD_Probe1_Temp(Probe_Temp1-Probe_Temp1%5);  //PROBE1温度
//							LCD_Probe2_Temp(Probe_Temp2-Probe_Temp2%5);  //PROBE2温度		
//						}

				if(APP_Control_Override)   
				{
					LCD_Dis_App();
				}
				else
				{
//				LCD_Action_sensor_Status_display(All_Action_sensor_Status_display);
	/***************F/C计算************/
//			if((flags&Flags_C_DIS)==1||BBQ_Status_Struct.fc==0x01)  //标志位为1 C
//			{
//				in_temp_set=(In_Temp_Set-32)*5/9;					  //设定温度转换为摄氏度

//				if(temp>=32) in_temp=(temp-32)*5/9;					  //设定温度转换为摄氏度
//				else in_temp=0; 
//				in_tempkendy = (temp-temp%5);
//				LCD_In_Temp((in_tempkendy),All_Action_sensor_Status_display);				  //只显示5的倍数
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
//			else				//正常模式
//			{
//	//			BBQ_Status_Struct.fc=1;
//	//			LCD_Dis_F();
//	

				in_temp_set=In_Temp_Set;					  //设定温度转换为摄氏度
				in_temp=temp;
//				LCD_In_Temp(temp-temp%5);				  //只显示5的倍数
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
									LCD_All_Off();	   //开机全关
									delay_ms(500);
									LCD_All_Use();
									delay_ms(500);
									LCD_All_Off();	   //开机全关
									delay_ms(500);	
									LCD_All_Use();
									delay_ms(500);
									LCD_All_Off();	   //开机全关
									delay_ms(500);	
								}
								else
								{
									delay_ms(100);
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //开机全关
									delay_ms(100);
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //开机全关
									delay_ms(100);	
									LCD_On_NoPC_Init_Dis(0x00);
									delay_ms(100);
									LCD_All_Off();	   //开机全关
									delay_ms(100);	
								}
							}
			else if(flags&Flags_Set_Actuall)	//Set界面
			{
				BBQ_Status_Struct.grill_set_act=1;
				set_to_actuall=1;

				Set_Interface_Count++;				//在set界面5S没有动作自动转到actual界面
				if(Set_Interface_Count>=250) 	//5S
				{
					OSFlagPost(flags_run,Flags_Set_Actuall,OS_FLAG_CLR,&err);//设置对应的信号量为0  
					Set_Interface_Count=0;	 
				}
			
				LCD_Dis_Set(); 	//显示SET
				LCD_Probe1_Temp(0x00);  //PROBE1温度
		  	LCD_Probe2_Temp(0x00);  //PROBE2温度	
				
//				LCD_Dis_Time_Dot();//显示时间点
			if(Temp_Press_Step==1)	 //闪烁temp
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
								LCD_UnDis_Set(); 	//显示SET
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
			else if((Temp_Press_Step==2) &&(SetPC_flag == 1))	 //闪烁probe temp
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
					if(APP_Display_Override==2)LCD_In_Temp(in_temp_set,All_Action_sensor_Status_display);			  //其他情况不闪烁
					else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
				}
			}
			else	  //Actuall界面
			{
			if(Lcd_refresh_CNT==5)
				{
				BBQ_Status_Struct.grill_set_act=2;

				if(set_to_actuall)
				{
					set_to_actuall=0;
					Temp_Press_Step=0;//进入actual界面，temp相当于确定了			
				}
				
//				if(Temp_Press_Step>=1)
//				{
//					Temp_Press_Step=0;//进入actual界面，temp相当于确定了
//					Send_Set_Temp_Message(6,2,in_temp-in_temp%5);//发送显示正常温度
//				}
				Set_Interface_Count=0;				//在set界面5S没有动作自动转到actual界面
				//LCD_Dis_Actual();//actual显示
				if(APP_Display_Override==2) 			LCD_In_Temp(in_temp,All_Action_sensor_Status_display); //(in_temp-in_temp%5) //炉内温度
				else LCD_Dis_APP_RTD_Override(APP_Display_RTD);
				if (SetPC_flag == 1)
				LCD_ProbePC_Temp(Probe_TempPC);  //pc温度
				LCD_Probe1_Temp(Probe_Temp1);  //PROBE1温度
		  	LCD_Probe2_Temp(Probe_Temp2);  //PROBE2温度		

/*				if(send_to_app_count>200)  //4S
				{
					send_to_app_count=0;
			//		Send_Live_Temp_Message(System_Run_Counter,in_temp-in_temp%5,Probe_Temp1-Probe_Temp1%5,Probe_Temp2-Probe_Temp2%5);		//发送实时温度
					Send_Display_Data_Message();
				} 	 */
			}

			if(send_to_app_count>200)  //4S
			{
				send_to_app_count=0;
		//		Send_Live_Temp_Message(System_Run_Counter,in_temp-in_temp%5,Probe_Temp1-Probe_Temp1%5,Probe_Temp2-Probe_Temp2%5);		//发送实时温度
//				if((in_temp_old!=(In_Temp-In_Temp%5))||(probe_temp_old!=(Probe_Temp1-Probe_Temp1%5))) //温度变化了才发送
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
//RUNOTHER任务
void runother_task(void *pdata)
{
	u16 flags;
	u8 err;	
	u16 probe_temp_set_beep=0;
	u16 wifi_blink_counter=0;
	u16 recipe_beep_counter=0;
	while(1)
	{
 		OSFlagPend(flags_run,0X0001,OS_FLAG_WAIT_SET_ANY,0,&err);//一直等待开机信号
//		LCD_Wifi_Dis();
/*		flags=OSFlagPend(flags_run,0X003c,OS_FLAG_WAIT_SET_ANY,0,&err);//等待错误信号量
		
		In_Temp=995;	//默认为ERR置为995
		Send_Display_Data_Message();

	
		if(flags&0X0004)	//ERR1
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
	
			HOT=Control_OFF;
			MOT=Control_OFF;
			FAN=Control_OFF;
		}
		else if(flags&0X0008)	//ERR2	
		{
			Flags_Run_Post=0;//清零，开机键
			OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零
	
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

		
		Flags_Run_Post=0;//清零，开机键
		OSFlagPost(flags_run,0XFFC3,OS_FLAG_CLR,&err);//除了ERR位置，其他清零

		HOT=Control_OFF;
		MOT=Control_OFF;
		FAN=Control_OFF;   */


/*		if(flags&0X003c)	//ERR标志位有一个为1则进入ERR模式
		{
//			printf("ERR Mode\n");
			HOT=Control_OFF;
		}
		else 
		{
			OSFlagPost(flags_run,0XFFFF,OS_FLAG_CLR,&err);//全部信号量清零
			OSFlagPost(flags_run,Flags_Run_On,OS_FLAG_SET,&err);////没有错误发送开机

		}  */
/********WiFi标志闪烁**************/

		 if((Wifi_Blink==0x00) || (Wifi_Blink>0x04))   //联网成功，常暗
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
/********收到菜单命令，蜂鸣器响3次**************/
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
			if(Flags_Prime)BEEP=1;	 //prime没按下
			else BEEP = 0;
	}									 
}
//========================================================================
// 函数: float PT1000_CalculateTemperature(u16 fR)    
// 描述: 查表得出PT1000的温度
// 参数: 放大10倍的电阻值
// 返回: 温度
//========================================================================
float PT1000_CalculateTemperature(u16 fR)
{
    float fTem;
    float fLowRValue;
    float fHighRValue;        
    s16   iTem;
    u8 i;

    u8 cBottom, cTop;  //数组元素最高和最低

    if (fR < RTD_TAB_PT1000[0])     // 电阻值小于表格最小值，低于量程下限。
    {
            return 0;
    }

    if (fR > RTD_TAB_PT1000[60])    // 电阻值大于表格最大值，超出量程上限。
    {
            return 300;				//最高在60元素，300度
    }

    cBottom = 0; 
    cTop    =  61;

    for (i=31; (cTop-cBottom)!=1; )        // 2分法查表。i=元素总个数/2
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
            else								//正好找到阻值在数组中
            {
                    iTem = (u16)i * 5;	 //此计算是元素标号和温度的对应，比如电阻在1元素中，对应的温度为5
                    fTem = (float)iTem;

                    return fTem;
            }
    }

    iTem = (u16)i * 5;

    fLowRValue  = RTD_TAB_PT1000[cBottom];
    fHighRValue = RTD_TAB_PT1000[cTop];

    fTem = ( ((fR - fLowRValue)*5) / (fHighRValue - fLowRValue) ) + iTem;        // 表格是以5度为一步的。
                                                                                                                                                    // 两点内插进行运算。

    return fTem;
}


void KEY1_IRQHandler(void)
{
  OSIntEnter();
	//确保是否产生了EXTI Line中断
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
			Set_temp_change=1;//改变温度屏幕不闪烁
		}
		
		EXTI_ClearITPendingBit(KEY1_INT_EXTI_LINE);     
	} 
OSIntExit();	
}
