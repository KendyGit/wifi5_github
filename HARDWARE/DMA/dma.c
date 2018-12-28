#include "sys.h"
#include "dma.h"
#include "usart.h"
#include "string.h"
#include "main.h"

#include "key_onoff.h"
#include "key_fc.h"
#include "key_prime.h"
#include "key_actual.h"
#include "key_down.h"
#include "key_set.h"
#include "key_temp.h"
#include "key_time.h"
#include "key_up.h"

DMA_InitTypeDef DMA_InitStructure;
bbq_struct BBQ_Status_Struct={0,0,0,0,0,0};		  //目前机器运行的状态，屏幕的显示

u8 All_Action_sensor_Status_flags_old=0;
u8 APP_Control_Override=0; //Override模式app全权控制 
u8 APP_Control_Override_Auger=0; //Override模式app全权控制马达状态
u8 APP_Control_Override_Fan=0; //Override模式app全权控制风扇状态
u16 APP_Control_Override_RTD=0; //Override模式app，读取RTD温度
u8 APP_Display_Override=2; //显示屏Override模式app全权控制，1为控制，2为不控制 
u8 APP_Display_RTD[3]={0x20,0x20,0x20};//0x20是空格，也就是屏幕不显示

u8 APP_Control_Override_flag=0;

u8 U1_DMA_R_BUF[U1_DMA_R_LEN],U1_R_BUF[U1_DMA_R_LEN];
u8 U1_DMA_T_BUF[U1_DMA_T_LEN];
u8 U1_DMA_SEND_FREE_FLAG = FREE;

u8 Key_Onoff_App_LongPressOK=0;	 //app发来off命令
u16 Key_Up_Down_App_Temp=0;	//app发送过来上下按键改变的温度
u8 Key_Up_App_ShortPressOK=0; //按键处理区分是app还是本地
u8 Key_Down_App_ShortPressOK=0; //按键处理区分是app还是本地
u8 Key_Direct_App_ShortPressOK=0; //按键处理区分是app还是本地
u8 Key_Prime_App_ContinuousPressOK=0; //按键处理区分是app还是本地


u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度 	 


u8 Recipe_List[64];	//从app接收到菜单保存
u8 Recipe_Start=0;//1为开始，2为结束保温，0为没有
u16 Recipe_Time_Count=0;//菜单时间计数
u8 Recipe_Step=0;//1~9,0表示没有菜单
u8 Recipe_Beep=0;//收到菜单命令，蜂鸣器开始响
   
//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量 
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
	  	
} 
//开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
 	DMA_SetCurrDataCounter(DMA1_Channel4,DMA1_MEM_LEN);//DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}	  
//串口1的DMA设置
void Usart_DMA_Init(void)
{ 
 	NVIC_InitTypeDef NVIC_InitStructure;

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输

//相应的DMA配置,接收DMA
	DMA_DeInit(DMA1_Channel5);   //将DMA的通道5寄存器重设为缺省值  串口1对应的是DMA通道5
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)U1_DMA_R_BUF;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = U1_DMA_R_LEN;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);   //使能串口1 DMA接收
	DMA_Cmd(DMA1_Channel5, ENABLE);  //正式驱动DMA传输

	//相应的DMA配置,发送DMA
	DMA_DeInit(DMA1_Channel4);   //将DMA的通道4寄存器重设为缺省值  串口1对应的是DMA通道4
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)U1_DMA_T_BUF;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从外设读取发送到内存
	DMA_InitStructure.DMA_BufferSize = U1_DMA_T_LEN;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	//开启发送中断
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);   //使能串口1 DMA发送

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器}	  
 
}
//DMA发送中断程序
void DMA1_Channel4_IRQHandler(void)
{
	#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
		OSIntEnter();    
	#endif

    if (DMA_GetFlagStatus(DMA1_FLAG_TC4) != RESET)//等待通道4传输完成
    {
            DMA_ClearFlag(DMA1_FLAG_TC4);//清除通道4传输完成标志
            U1_DMA_SEND_FREE_FLAG = FREE;
    }

	#ifdef OS_TICKS_PER_SEC	 	//如果时钟节拍数定义了,说明要使用ucosII了.
		OSIntExit();  											 
	#endif
}
//========================================================================
// 函数:  void set_buf_free(u8 *buf,u8 len)   
// 描述: 清空数组
// 参数: 
// 返回: 无
//========================================================================
void Set_Buf_Free(u8 *buf,u8 len) 
{
    u8 i;
	for(i=0;i<len;i++)buf[i]=0;
}

//========================================================================
// 函数:  void Uart1_DMA_Send_Array(u8 *buffer, u16 len)   
// 描述: 串口1发送数组
// 参数: buffer数组，len数组长度
// 返回: 无
//========================================================================
void Uart1_DMA_Send_Array(u8 *buffer, u16 len)
{
	while (U1_DMA_SEND_FREE_FLAG == USEING);//是否需要锁死
	memcpy(U1_DMA_T_BUF, buffer, len);
	DMA_Cmd(DMA1_Channel4, DISABLE);  //关闭USART1 TX DMA1 所指示的通道      
	DMA_SetCurrDataCounter(DMA1_Channel4, len);//DMA通道的DMA缓存的大小
	DMA_Cmd(DMA1_Channel4, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
	U1_DMA_SEND_FREE_FLAG = USEING;
}

//初始化PB5和PE5为输出口.并使能这两个口的时钟		    
void Wifi_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PC端口时钟
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
/* GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	    		 //LED1-->PE.14 端口配置, 推挽输出
 GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 GPIO_SetBits(GPIOC,GPIO_Pin_14); 	*/					 //PE.14 输出高
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //,推挽输出
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 //GPIO_SetBits(GPIOA,GPIO_Pin_5); 						 //PE.11 13输出高 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 //,推挽输出
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
 //GPIO_SetBits(GPIOA,GPIO_Pin_4); 						 //PE.11 13输出高 
}

/*****************************************************************************************/
/************************ 以下全部为单片机发送给app的指令消息 ****************************/
/*****************************************************************************************/
void Send_FWREV_On_Message(void)
{
  u8 tab[]={0xfe,0x5F,0x01,0X00,0X01,0X0A,0X01,0X03,0X04,0xff};//开机指令
  Uart1_DMA_Send_Array(tab,10);
}

void Send_On_Message(void)
{
  u8 tab[]={0xfe,0x01,0x01,0xff};//开机指令
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Off_Message(void)
{
  u8 tab[]={0xfe,0x01,0x02,0xff};//关机指令
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Bypass_Message(void)
{
  u8 tab[]={0xfe,0x0a,0x01,0xff};//旁路启动消息
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Feed_Start_Message(void)
{
  u8 tab[]={0xfe,0x02,0x01,0xff};//马达强制进料开始指令
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Feed_End_Message(void)
{
  u8 tab[]={0xfe,0x02,0x02,0xff};//马达强制进料结束指令
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Set_Temp_Message(u16 tp)
{
  u8 tab[]={0xfe,0x06,0x01,0x00,0x00,0x00,0xff};

	if(BBQ_Status_Struct.fc==0x01)	  //F
	{
		tab[3]=(tp-tp%5)%1000/100;//温度百位  probe set
		tab[4]=(tp-tp%5)%100/10;//温度十位
		tab[5]=(tp-tp%5)%10/1;//温度个位
	}
	else   //c
	{
		if(tp>=32) tp=(tp-32)*5/9;
		else tp=0;
	
		tab[3]=(tp-tp%5)%1000/100;//温度百位  probe set
		tab[4]=(tp-tp%5)%100/10;//温度十位
		tab[5]=(tp-tp%5)%10/1;//温度个位
	}

  Uart1_DMA_Send_Array(tab,7);
}

void Send_Act_Temp_Message(u16 tp)
{
  u8 tab[]={0xfe,0x06,0x02,0x00,0x00,0x00,0xff};

	if(BBQ_Status_Struct.fc==0x01)	  //F
	{
		tab[3]=(tp-tp%5)%1000/100;//温度百位  probe set
		tab[4]=(tp-tp%5)%100/10;//温度十位
		tab[5]=(tp-tp%5)%10/1;//温度个位
	}
	else   //c
	{
		if(tp>=32) tp=(tp-32)*5/9;
		else tp=0;
	
		tab[3]=(tp-tp%5)%1000/100;//温度百位  probe act
		tab[4]=(tp-tp%5)%100/10;//温度十位
		tab[5]=(tp-tp%5)%10/1;//温度个位
	}

  Uart1_DMA_Send_Array(tab,7);
}

void Send_Probe_Temp_Message(u8 probe_ch,u16 probe_x,u16 probe_y)
{
  u8 tab[]={0xfe,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff};//炉内和食物探针的切换指令
  tab[2]=probe_ch;//1~2        1：PROBE X，2：PROBE Y
  tab[3]=(probe_x-probe_x%5)%1000/100;//温度百位
  tab[4]=(probe_x-probe_x%5)%100/10;//温度十位
  tab[5]=(probe_x-probe_x%5)%10/1;//温度个位
  tab[6]=(probe_y-probe_y%5)%1000/100;//温度百位
  tab[7]=(probe_y-probe_y%5)%100/10;//温度十位
  tab[8]=(probe_y-probe_y%5)%10/1;//温度个位
  Uart1_DMA_Send_Array(tab,10);
}
//void Send_Probe_Statue_Message(u8 statue)
//{
//  u8 tab[]={0xfe,0x08,0x00,0xff};//发送probe探针插入状态指令
//  tab[2]=statue;
//  Uart1_DMA_Send_Array(tab,4);
//}
//void Send_Temp_Type_Message(u8 type)
//{
//  u8 tab[]={0xfe,0x09,0x00,0xff};//发送温度单位切换指令
//  tab[2]=type;//1:华氏度（F），2：摄氏度（C）
//  Uart1_DMA_Send_Array(tab,4);
//}


void Send_Temp_in_uncompensated_On_Message(void)
{
	u8 tab[6]={0xfe};
	u16 in_temp_uncompensated=0;
	tab[1]=0x4f;	  //0x09/0x0b
			
	if(BBQ_Status_Struct.fc==0x01)	  //F
	{

		tab[2]=In_Temp_Uncompensated%1000/100;//温度百位  probe set
		tab[3]=In_Temp_Uncompensated%100/10;//温度十位
		tab[4]=In_Temp_Uncompensated%10/1;//温度个位
	}
	else   //c
	{
		if(In_Temp_Uncompensated>=32) in_temp_uncompensated=(In_Temp_Uncompensated-32)*5/9;
		else in_temp_uncompensated=0;

		tab[2]=in_temp_uncompensated%1000/100;//温度百位  probe act
		tab[3]=in_temp_uncompensated%100/10;//温度十位
		tab[4]=in_temp_uncompensated%10/1;//温度个位
	}
	
	tab[5]=0xff;
	Uart1_DMA_Send_Array(tab,6);
}

void Send_Display_Data_Message(u8 command)
{
	u8 tab[23]={0xfe};
	u16 probe_temp1=0;
	u16 probe_temp2=0;
	u16 probe_temp_set=0;
	u16 probe_temppc=0;
	u16 rtd_temp=0;
	u16 rtd_set_temp=0;
	u8 errtemp1=0,errtemp2=0,errtemp3=0;
	
	tab[1]=command;	  //0x09/0x0b

	if(BBQ_Status_Struct.probe_status)	  //为1，说明有probe，发送实际温度
	{
	if(BBQ_Status_Struct.fc==0x01)	  //F
	{

		tab[2]=(Probe_TempPC-Probe_TempPC%5)%1000/100;//温度百位  probe (set (Probe_TempPC-Probe_TempPC%5)
		tab[3]=(Probe_TempPC-Probe_TempPC%5)%100/10;//温度十位
		tab[4]=(Probe_TempPC-Probe_TempPC%5)%10/1;//温度个位
	}
	else   //c
	{
		if(Probe_TempPC>=32) probe_temppc=(Probe_TempPC-32)*5/9;
		else probe_temppc=0;

		tab[2]=(probe_temppc-probe_temppc%5)%1000/100;//温度百位  probe act probe_temppc-probe_temppc%5
		tab[3]=(probe_temppc-probe_temppc%5)%100/10;//温度十位
		tab[4]=(probe_temppc-probe_temppc%5)%10/1;//温度个位
	}
}
		else	 //没有probe，发送温度960
	{
		tab[2]=960%1000/100;//温度百位  probe act
		tab[3]=960%100/10;//温度十位
		tab[4]=960%10/1;//温度个位
	}

	if(BBQ_Status_Struct.probe_status)	  //为1，说明有probe，发送实际温度
	{
		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[5]=(Probe_Temp1-Probe_Temp1%5)%1000/100;//温度百位  probe act Probe_Temp1-Probe_Temp1%5
			tab[6]=(Probe_Temp1-Probe_Temp1%5)%100/10;//温度十位
			tab[7]=(Probe_Temp1-Probe_Temp1%5)%10/1;//温度个位
		}
		else   //c
		{
			if(Probe_Temp1>=32) probe_temp1=(Probe_Temp1-32)*5/9;
			else probe_temp1=0;

			tab[5]=(probe_temp1-probe_temp1%5)%1000/100;//温度百位  probe act probe_temp1-probe_temp1%5
			tab[6]=(probe_temp1-probe_temp1%5)%100/10;//温度十位
			tab[7]=(probe_temp1-probe_temp1%5)%10/1;//温度个位
		}
	}
	else	 //没有probe，发送温度960
	{
		tab[5]=960%1000/100;//温度百位  probe act
		tab[6]=960%100/10;//温度十位
		tab[7]=960%10/1;//温度个位
	}

	if(BBQ_Status_Struct.probe_status)	  //为1，说明有probe，发送实际温度
	{
		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[8]=(Probe_Temp2-Probe_Temp2%5)%1000/100;//温度百位  probe act Probe_Temp2-Probe_Temp2%5
			tab[9]=(Probe_Temp2-Probe_Temp2%5)%100/10;//温度十位
			tab[10]=(Probe_Temp2-Probe_Temp2%5)%10/1;//温度个位
		}
		else   //c
		{
			if(Probe_Temp2>=32) probe_temp2=(Probe_Temp2-32)*5/9;
			else probe_temp2=0;

			tab[8]=(probe_temp2-probe_temp2%5)%1000/100;//温度百位  probe act probe_temp2-probe_temp2%5
			tab[9]=(probe_temp2-probe_temp2%5)%100/10;//温度十位
			tab[10]=(probe_temp2-probe_temp2%5)%10/1;//温度个位
		}
	}
	else	 //没有probe，发送温度960
	{
		tab[8]=960%1000/100;//温度百位  probe act
		tab[9]=960%100/10;//温度十位
		tab[10]=960%10/1;//温度个位
	}
	
//	if(BBQ_Status_Struct.grill_set_act==1)//grill set
//	{

		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[11]=(In_Temp_Set-In_Temp_Set%5)%1000/100;//温度百位  probe act In_Temp_Set-In_Temp_Set%5
			tab[12]=(In_Temp_Set-In_Temp_Set%5)%100/10;//温度十位
			tab[13]=(In_Temp_Set-In_Temp_Set%5)%10/1;//温度个位
		}
		else   //c
		{
			if(In_Temp_Set>=32) rtd_set_temp=(In_Temp_Set-32)*5/9;
			else rtd_set_temp=0;

			tab[11]=(rtd_set_temp-rtd_set_temp%5)%1000/100;//温度百位  probe act rtd_set_temp-rtd_set_temp%5
			tab[12]=(rtd_set_temp-rtd_set_temp%5)%100/10;//温度十位
			tab[13]=(rtd_set_temp-rtd_set_temp%5)%10/1;//温度个位
		}

//	}
//	else if(BBQ_Status_Struct.grill_set_act==2)//grill act
//	{

		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[14]=(In_Temp-In_Temp%5)%1000/100;//温度百位  probe act  Probe_TempPC-Probe_TempPC%5
			tab[15]=(In_Temp-In_Temp%5)%100/10;//温度十位
			tab[16]=(In_Temp-In_Temp%5)%10/1;//温度个位
		}
		else   //c
		{
			if(In_Temp>=32) rtd_temp=(In_Temp-32)*5/9;
			else rtd_temp=0;

			tab[14]=(rtd_temp-rtd_temp%5)%1000/100;//温度百位  probe act rtd_temp-rtd_temp*5
			tab[15]=(rtd_temp-rtd_temp%5)%100/10;//温度十位
			tab[16]=(rtd_temp-rtd_temp%5)%10/1;//温度个位
		}

//	}
//	else //grill 发送ERR代码
//	{

//			tab[8]=(In_Temp)%1000/100;//温度百位  probe act
//			tab[9]=(In_Temp)%100/10;//温度十位
//			tab[10]=(In_Temp)%10/1;//温度个位

//	}

/*if(Probe_Temp1>=32) probe_temp=(Probe_Temp1-32)*5/9;
else probe_temp=0;	*/
		
	tab[17]=BBQ_Status_Struct.fc;//温度单位F
	if(All_Action_sensor_Status_flags&0x01)//fan status
	{tab[18]=0x64;} //fan on/off, fan need be worked
	else
	{tab[18]=0x00;} 	//fan not is worked
	if((All_Action_sensor_Status_flags&0x20))//motor status
	{tab[19]=0x01;} //motor need be work
	else
	{tab[19]=0x00;} 	//motor need not be work
	tab[20]=BBQ_Status_Struct.onoff;//1:ON,2:OFF
	if((All_Action_sensor_Status_flags>>3)&0x01)errtemp1=0x08;//motor error
	if((All_Action_sensor_Status_flags>>1)&0x01)errtemp2=0x04;//hot error
	if((All_Action_sensor_Status_flags>>4)&0x01)errtemp3=0x02; //fan error

	tab[21]=errtemp1|errtemp2|errtemp3;

//	tab[11]=1;	 //probe x
//	tab[12]=BBQ_Status_Struct.probe_status;//探针插入状态
//	tab[13]=BBQ_Status_Struct.grill_set_act;//grill是set还是act
//	
//	tab[14]=BBQ_Status_Struct.onoff;//1:ON,2:OFF
//	
//	tab[15]=2;//1:feed,2:no feed
//	
//	tab[16]=BBQ_Status_Struct.fc;//温度单位F
	tab[22]=0xff;
	Uart1_DMA_Send_Array(tab,23);
}

/*void Send_Display_Data_Message(void)
{
	u8 tab[18]={0xfe,0x0b};
	
	tab[2]=(Probe_Temp_Set-Probe_Temp_Set%5)%1000/100;//温度百位  probe set
	tab[3]=(Probe_Temp_Set-Probe_Temp_Set%5)%100/10;//温度十位
	tab[4]=(Probe_Temp_Set-Probe_Temp_Set%5)%10/1;//温度个位

	if(BBQ_Status_Struct.probe_status)	  //为1，说明有probe，发送实际温度
	{
		tab[5]=(Probe_Temp1-Probe_Temp1%5)%1000/100;//温度百位  probe act
		tab[6]=(Probe_Temp1-Probe_Temp1%5)%100/10;//温度十位
		tab[7]=(Probe_Temp1-Probe_Temp1%5)%10/1;//温度个位
	}
	else	 //没有probe，发送温度960
	{
		tab[5]=960%1000/100;//温度百位  probe act
		tab[6]=960%100/10;//温度十位
		tab[7]=960%10/1;//温度个位
	}
	
	if(BBQ_Status_Struct.grill_set_act==1)//grill set
	{
	  tab[8]= In_Temp_Set%1000/100;//温度百位
	  tab[9]= In_Temp_Set%100/10;//温度十位
	  tab[10]=In_Temp_Set%10/1;//温度个位
	}
	else
	{
	  tab[8]= (In_Temp-In_Temp%5)%1000/100;//温度百位  grill act
	  tab[9]= (In_Temp-In_Temp%5)%100/10;//温度十位
	  tab[10]=(In_Temp-In_Temp%5)%10/1;//温度个位
	}
	
	tab[11]=1;	 //probe x
	tab[12]=BBQ_Status_Struct.probe_status;//探针插入状态
	tab[13]=BBQ_Status_Struct.grill_set_act;//grill是set还是act
	
	tab[14]=BBQ_Status_Struct.onoff;//1:ON,2:OFF
	
	tab[15]=2;//1:feed,2:no feed
	
	tab[16]=1;//温度单位F
	tab[17]=0xff;
	Uart1_DMA_Send_Array(tab,18);
}*/
//void Send_All_Temp_Message(void)
//{
//	u8 tab[22]={0xfe,0x0c};
//	tab[2]=Probe_Temp_Set%1000/100;//温度百位		  probe1 set
//	tab[3]=Probe_Temp_Set%100/10;//温度十位
//	tab[4]=Probe_Temp_Set%10/1;//温度个位
//	tab[5]=Probe_Temp1%1000/100;//温度百位  probe1 act
//	tab[6]=Probe_Temp1%100/10;//温度十位
//	tab[7]=Probe_Temp1%10/1;//温度个位
//  
//	tab[8]=0;//温度百位		probe2 set act
//	tab[9]=0;//温度十位
//	tab[10]=0;//温度个位
//	tab[11]=0;//温度百位	probe2 act
//	tab[12]=0;//温度十位
//	tab[13]=0;//温度个位
//	
//	tab[14]=In_Temp_Set%1000/100;//温度百位	   grill set
//	tab[15]=In_Temp_Set%100/10;//温度十位
//	tab[16]=In_Temp_Set%10/1;//温度个位
//	tab[17]=In_Temp%1000/100;//温度百位		   grill act
//	tab[18]=In_Temp%100/10;//温度十位
//	tab[19]=In_Temp%10/1;//温度个位
//	
//	tab[20]=1;//温度单位 F
//	tab[21]=0xff;	 
//	Uart1_DMA_Send_Array(tab,22);

//}
//void Send_Live_Temp_Message(u32 time,u16 in_temp,u16 probe1_temp,u16 probe2_temp)		//发送实时温度
//{
//	u8 tab[16]={0xfe,0x0e};
//	tab[2]=time/10/60%1000/100;//计时百位
//	tab[3]=time/10/60%100/10;//计时十位
//	tab[4]=time/10/60%10/1;//计时个位
//	tab[5]=time/60%10/1;//计时一个小数位  
//	
//	tab[6]= (in_temp-in_temp%5)%1000/100;//grill温度百位
//	tab[7]= (in_temp-in_temp%5)%100/10;//温度十位
//	tab[8]=(in_temp-in_temp%5)%10/1;//温度个位

//	if(BBQ_Status_Struct.probe_status)	  //为1，说明有probe，发送实际温度
//	{
//		tab[9]=(probe1_temp-probe1_temp%5)%1000/100;//温度百位  probe act
//		tab[10]=(probe1_temp-probe1_temp%5)%100/10;//温度十位
//		tab[11]=(probe1_temp-probe1_temp%5)%10/1;//温度个位
//	}
//	else	 //没有probe，发送温度960
//	{
///*		tab[5]=960%1000/100;//温度百位  probe act
//		tab[6]=960%100/10;//温度十位
//		tab[7]=960%10/1;//温度个位*/
//		tab[9]=0;//温度百位  probe act
//		tab[10]=0;//温度十位
//		tab[11]=0;//温度个位
//	}

//  
//	
//	/*  tab[12]=probe2_temp%1000/100;//probe2温度百位
//	tab[13]=probe2_temp%100/10;//温度十位
//	tab[14]=probe2_temp%10/1;//温度个位	*/
//	tab[12]=960%1000/100;//温度百位  probe act
//	tab[13]=960%100/10;//温度十位
//	tab[14]=960%10/1;//温度个位

//	
//	tab[15]=0xff;	 				   
//	Uart1_DMA_Send_Array(tab,16);

//}

//void Send_ALL_Set_Temp_Message(u16 in_temp,u16 probe1_temp)		//发送设定温度
//{
//  u8 tab[9]={0xfe,0x0d};

//  tab[2]=in_temp%1000/100;//grill温度百位
//  tab[3]=in_temp%100/10;//温度十位
//  tab[4]=in_temp%10/1;//温度个位

//  tab[5]=probe1_temp%1000/100;//probe1温度百位
//  tab[6]=probe1_temp%100/10;//温度十位
//  tab[7]=probe1_temp%10/1;//温度个位

//  
//  tab[8]=0xff;	 				   
//  Uart1_DMA_Send_Array(tab,9);

//}
//void Send_Run_Time_Message(u32 time)		//发送运行时间
//{
//  u8 tab[6]={0xfe,0x0f};
//  tab[2]=time/10/60%1000/100;//计时百位
//  tab[3]=time/10/60%100/10;//计时十位
//  tab[4]=time/10/60%10/1;//计时个位
////  tab[5]=time*10/60%10/1;//计时一个小数位

//  
//  tab[5]=0xff;	 				   
//  Uart1_DMA_Send_Array(tab,6);

//}

//void Send_Recipe_Message(void)
//{
//  u8 tab[]={0xfe,0x31,0x01,0xff};//返回菜单接收成功命令
//  Uart1_DMA_Send_Array(tab,4);
//}

//void Send_Recipe_Over_Message(void)
//{
//  u8 tab[]={0xfe,0x31,0x02,0xff};//返回菜单接收成功命令
//  Uart1_DMA_Send_Array(tab,4);
//}

void Send_Goto_WiFi_Password_Mode(void)
{
  u8 tab[]={0xfe,0x22,0x01,0xff};//发送 进入配置WIFI密码模式 指令
  Uart1_DMA_Send_Array(tab,4);
}

/******** 对接收到的数据进行处理 *******
功能：    处理接收到的数据
data： 接收数据的缓存地址        
num：接收数据的长度
**************************************/
void Recv_Data_Handle(u8 *data,u16 num)
{
     u8 i=0;
	 if(num<4)return;
     
     if(data[0]==0xFE)//包头
     {
          switch(data[1])
          {
                  case 0x01:/*开关机*/
                      if(data[2]==0x01&&data[3]==0xff)//开机
                      {
						 Key_Onoff_ShortPressOK=1;
						 BBQ_Status_Struct.onoff=1;
                      }
                      else if(data[2]==0x02&&data[3]==0xff)//关机
                      {
						 Key_Onoff_App_LongPressOK=1;
						 BBQ_Status_Struct.onoff=2;
                      }
                      break;
                  case 0x02:/*马达强制进料*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
						Key_Prime_App_ContinuousPressOK=1;
//                              key_value |= Feed_Long_Press;                                         
                      }
					  else if(data[2]==0x02&&data[3]==0xff)  /*强制进料结束*/
					  {
						Key_Prime_App_ContinuousPressOK=0;
					  
					  }
                      break;
                  case 0x03:/*升高温度*/
                      if(data[2]==0x01&&data[4]==0xff)//升高GRILL SET温度
                      {
							Key_Up_ShortPressOK=1;//按键置一
							Key_Up_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=1;				  	

 /*                             if(grill_set_temp+data[3]<999)grill_set_temp+=data[3];
                              send_set_temp_message(PLUS_TEMP,data[2],(u16)grill_set_temp);//发送GRILL_SET升高温度消息    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	*/
                      }
                      else if( (data[2]==0x02|data[2]==0x03) &&data[4]==0xff)//升高PROBE_X_SET和PROBE_X_SET温度
                      {
        /*                      u8 ch=data[2]-2;
                              if(probe_set_temp[ch]+data[3]<999)probe_set_temp[ch]+=data[3];
                              send_set_temp_message(PLUS_TEMP,data[2],(u16)probe_set_temp[ch]);//发送PROBE_X_SET或PROBE_Y_SET升高温度消息    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	*/
							Key_Up_ShortPressOK=1;//按键置一
							Key_Up_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=2;				  	

                      }
                      break;
                  case 0x04:/*降低温度*/
                      if(data[2]==0x01&&data[4]==0xff)//降低GRILL SET温度
                      {
							Key_Down_ShortPressOK=1;//按键置一
							Key_Down_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=1;				  	

              /*                if(grill_set_temp>data[3])grill_set_temp-=data[3];
                              send_set_temp_message(MINUS_TEMP,data[2],(u16)grill_set_temp);//发送GRILL_SET降低温度消息    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	  */
                      }
                      else if( (data[2]==0x02||data[2]==0x03) &&data[4]==0xff)//降低PROBE_X_SET和PROBE_X_SET温度
                      {
                   /*           u8 ch=data[2]-2;
                              if(probe_set_temp[ch]>data[3])probe_set_temp[ch]-=data[3];
                              send_set_temp_message(MINUS_TEMP,data[2],(u16)probe_set_temp[ch]);//发送PROBE_X_SET或PROBE_Y_SET降低温度消息    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	   */
							Key_Down_ShortPressOK=1;//按键置一
							Key_Down_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=2;				  	
                      }
                    break;
                  case 0x05:/*直接设定温度*/
                      if(data[2]==0x01&&data[6]==0xff)//直接设定GRILL SET温度
                      {
							Key_Direct_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3]%10*100+data[4]%10*10+data[5]%10;
					  		Temp_Press_Step=1;				  	

                 /*             grill_set_temp=data[3]%10*100+data[4]%10*10+data[5]%10;
                              send_set_temp_message(DIRECT_SET_TEMP,data[2],(u16)grill_set_temp);//发送GRILL_SET直接设定消息    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	*/
                      }
                      else if( (data[2]==0x02||data[2]==0x03) &&data[6]==0xff)//直接设定PROBE_X_SET和PROBE_X_SET温度
                      {
                  /*            u8 ch=data[2]-2;
                              probe_set_temp[ch]=data[3]%10*100+data[4]%10*10+data[5]%10;;
                              send_set_temp_message(DIRECT_SET_TEMP,data[2],(u16)probe_set_temp[ch]);//发送PROBE_X_SET或PROBE_Y_SET直接设定消息    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//转化为显示数据格式
                              Buzzer_Ring(100);	   */
							Key_Direct_App_ShortPressOK=1; //按键处理区分是app还是本地
							Key_Up_Down_App_Temp=data[3]%10*100+data[4]%10*10+data[5]%10;
					  		Temp_Press_Step=2;				  	
                      }                    
                      break;
                  case 0x06:/*GRILL_SET和GRILL_ACT切换*/
                      if(data[2]==0x01&&data[3]==0xff)//grill显示温度切换为SET, temp in setting
                      {
													Send_Set_Temp_Message(In_Temp_Set);
                      }  
                      else if(data[2]==0x02&&data[3]==0xff)//grill显示温度切换为ACT, temp in ACT
                      {
													Send_Act_Temp_Message(In_Temp);
                      }  
                      else if(data[2]==0x03&&data[3]==0xff)//grill显示温度切换为SET, temp in setting
                      {
												Key_Set_ShortPressOK=1;
												Temp_Press_Step=0;	              
                      }   
                      else if(data[2]==0x04&&data[3]==0xff)//grill显示温度切换为SET, PC setting
                      {
												Key_Set_ShortPressOK=1;
												Temp_Press_Step=1;	              
                      }   
                      break;
                  case 0x07:/*探针（PROBE）通道切换*/
                      if( (data[2]==0x01||data[2]==0x02) &&data[3]==0xff)
                      {
                     /*         u8 ch=data[2];
                              LCD_display_bag.probe_ch=ch-1;
                              LCD_data_calcu();//转化为显示数据格式 
                              send_probe_temp_message(ch,LCD_display_bag.probe_set_data[ch-1],LCD_display_bag.probe_act_data[ch-1]);
                              Buzzer_Ring(100);	   */
							  Send_Probe_Temp_Message(1,Probe_Temp_Set,Probe_Temp1);
                      }
                      break;
                  case 0x08:/**/
                    
                      break;
                  case 0x09:/*温度单位切换*/
                      if( (data[2]==0x01) &&data[3]==0xff)
                      {
                     /*         RegData[Temp_Type_Address]=data[2];//1:F,2:C
                              if(data[2]==1)//C->F
                              {
                                  C_to_F();
                              }
                              else if(data[2]==2)//F->C
                              {
                                  F_to_C();
                              }	 */
												BBQ_Status_Struct.fc=0x01;
                      }
                      else if( (data[2]==0x02)&&data[3]==0xff)
                      {
												BBQ_Status_Struct.fc=0x00;
											}
											Send_Display_Data_Message(0x09);
                      break;
//                  case 0x0A:/*Bypass Startup 旁路启动，跳过预热阶段*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//                   //           key_value |= Bypass_Long_Press;
//                      }
//                      break;
                  case 0x11:/*查询当前显示数据*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
					  	Send_Display_Data_Message(0x11);
                      }                    
                      break;
//                  case 0x11:/*查询所有数据*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						 Send_All_Temp_Message();
//                         /*     send_all_temp_message();
//                              Buzzer_Ring(100);	   */
//                      }                    
//                      break;
//                  case 0x0d:/*本次过程的2个目标温度*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_ALL_Set_Temp_Message(In_Temp_Set,Probe_Temp_Set);		//发送设定温度
//                      }                    
//                      break;
//                  case 0x0e:/*本次过程的2个时时温度*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_Live_Temp_Message(System_Run_Counter,In_Temp,Probe_Temp1,Probe_Temp2);		//发送实时温度
//                      }                    
//                      break;
//                  case 0x0f:/*返回本次过程开始到现在的时间（分钟）*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_Run_Time_Message(System_Run_Counter);		//发送运行时间
//                      }                    
//                      break;
                  case 0x12:/*联网标志*/
                      if((data[3]==0xff))
                      {
												Wifi_Blink=data[2];
                      }       
                      else
                      {
												Wifi_Blink=0x00;
                      }  											
                      break;
									case 0x15:/*控制显示权限*/
                      if(data[6]==0xff)
                      {
												APP_Display_Override=data[2]; //1为控制显示2为不控制
												APP_Display_RTD[0]=data[3];
												APP_Display_RTD[1]=data[4];
												APP_Display_RTD[2]=data[5];
                      }                    
                      break;
					  
                  case 0x1f:/*Control Override*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
												All_Action_sensor_Status_flags_old=All_Action_sensor_Status_flags;
												All_Action_sensor_Status_flags=0x00;
												APP_Control_Override=1;
												APP_Control_Override_flag=1;
                      }   
                      else if(data[2]==0x02&&data[3]==0xff)
                      {
												APP_Control_Override=0;
												All_Action_sensor_Status_flags=All_Action_sensor_Status_flags_old;
												APP_Control_Override_flag=1;
                      }                    
                      break;
                  case 0x2f:/*Control Override Auger*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
												APP_Control_Override_Auger=1;
                      }   
                      else if(data[2]==0x02&&data[3]==0xff)
                      {
												APP_Control_Override_Auger=0;
                      }                    
                      break;
                  case 0x3f:/*Control Override Fan*/
                      if(data[2]>=0x01&&data[3]==0xff)
                      {
												APP_Control_Override_Fan=1;
                      }  
                      else if(data[2]==0x00&&data[3]==0xff)
                      {
												APP_Control_Override_Fan=0;
                      }   
												
                      break;
                  case 0x4f:/*Temp in temperature uncompensated read back*/
                      if((data[2]==0x01)&&(data[3]==0xff))
                      {
												Send_Temp_in_uncompensated_On_Message();
                      }       						
                      break;
                  case 0x5f:/*FW read back*/
                      if((data[2]==0x01)&&(data[3]==0xff))
                      {
												Send_FWREV_On_Message();
                      }       						
                      break;
                  case 0x31:/*菜单*/
                      if(data[2]==0x02&&data[3]==0xff)
					  {
					  	Recipe_Start=2;//菜单结束，保温
						Recipe_Step=0;	
						Recipe_Beep=1;//响蜂鸣器

//					  	Send_Recipe_Over_Message();
					  }
                      else if(data[66]==0xff)
                      {
					  	for(i=0;i<64;i++)
						{
							Recipe_List[i]=data[i+2];//元素2是总共步骤，后面每步7个字节：步骤1+温度3+时间3
						}

						Recipe_Step=Recipe_List[1];//第一步
						Recipe_Start=1;
						Recipe_Beep=1;//响蜂鸣器
//						Send_Recipe_Message();
                      }                    
                      break;


          }
     }	
     //Device_address=STMFLASH_ReadHalfWord(DEVICE_ADDRESS)+(u32)STMFLASH_ReadHalfWord(DEVICE_ADDRESS+2)*65536;//读取32位地址
     //recv_data_address=(((u32)data[4])+((u32)data[5]<<8)+((u32)data[6]<<16)+((u32)data[7]<<24));//获取命令中的设备地址

   

     //recv_crc_data=data[num-4]+data[num-3]*256;//获取命令中的校验值
     //clcu_crc_data=crc(data+2,num-6);//将收到的命令进行重新校验

     
}































