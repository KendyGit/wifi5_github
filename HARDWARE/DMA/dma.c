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
bbq_struct BBQ_Status_Struct={0,0,0,0,0,0};		  //Ŀǰ�������е�״̬����Ļ����ʾ

u8 All_Action_sensor_Status_flags_old=0;
u8 APP_Control_Override=0; //OverrideģʽappȫȨ���� 
u8 APP_Control_Override_Auger=0; //OverrideģʽappȫȨ�������״̬
u8 APP_Control_Override_Fan=0; //OverrideģʽappȫȨ���Ʒ���״̬
u16 APP_Control_Override_RTD=0; //Overrideģʽapp����ȡRTD�¶�
u8 APP_Display_Override=2; //��ʾ��OverrideģʽappȫȨ���ƣ�1Ϊ���ƣ�2Ϊ������ 
u8 APP_Display_RTD[3]={0x20,0x20,0x20};//0x20�ǿո�Ҳ������Ļ����ʾ

u8 APP_Control_Override_flag=0;

u8 U1_DMA_R_BUF[U1_DMA_R_LEN],U1_R_BUF[U1_DMA_R_LEN];
u8 U1_DMA_T_BUF[U1_DMA_T_LEN];
u8 U1_DMA_SEND_FREE_FLAG = FREE;

u8 Key_Onoff_App_LongPressOK=0;	 //app����off����
u16 Key_Up_Down_App_Temp=0;	//app���͹������°����ı���¶�
u8 Key_Up_App_ShortPressOK=0; //��������������app���Ǳ���
u8 Key_Down_App_ShortPressOK=0; //��������������app���Ǳ���
u8 Key_Direct_App_ShortPressOK=0; //��������������app���Ǳ���
u8 Key_Prime_App_ContinuousPressOK=0; //��������������app���Ǳ���


u16 DMA1_MEM_LEN;//����DMAÿ�����ݴ��͵ĳ��� 	 


u8 Recipe_List[64];	//��app���յ��˵�����
u8 Recipe_Start=0;//1Ϊ��ʼ��2Ϊ�������£�0Ϊû��
u16 Recipe_Time_Count=0;//�˵�ʱ�����
u8 Recipe_Step=0;//1~9,0��ʾû�в˵�
u8 Recipe_Beep=0;//�յ��˵������������ʼ��
   
//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ
//cndtr:���ݴ����� 
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
    DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	  	
} 
//����һ��DMA����
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
 	DMA_SetCurrDataCounter(DMA1_Channel4,DMA1_MEM_LEN);//DMAͨ����DMA����Ĵ�С
 	DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
}	  
//����1��DMA����
void Usart_DMA_Init(void)
{ 
 	NVIC_InitTypeDef NVIC_InitStructure;

 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����

//��Ӧ��DMA����,����DMA
	DMA_DeInit(DMA1_Channel5);   //��DMA��ͨ��5�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��5
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)U1_DMA_R_BUF;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_BufferSize = U1_DMA_R_LEN;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
	
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);   //ʹ�ܴ���1 DMA����
	DMA_Cmd(DMA1_Channel5, ENABLE);  //��ʽ����DMA����

	//��Ӧ��DMA����,����DMA
	DMA_DeInit(DMA1_Channel4);   //��DMA��ͨ��4�Ĵ�������Ϊȱʡֵ  ����1��Ӧ����DMAͨ��4
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;  //DMA����ADC����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)U1_DMA_T_BUF;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴������ȡ���͵��ڴ�
	DMA_InitStructure.DMA_BufferSize = U1_DMA_T_LEN;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //��������������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);	//���������ж�
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);   //ʹ�ܴ���1 DMA����

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���}	  
 
}
//DMA�����жϳ���
void DMA1_Channel4_IRQHandler(void)
{
	#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
		OSIntEnter();    
	#endif

    if (DMA_GetFlagStatus(DMA1_FLAG_TC4) != RESET)//�ȴ�ͨ��4�������
    {
            DMA_ClearFlag(DMA1_FLAG_TC4);//���ͨ��4������ɱ�־
            U1_DMA_SEND_FREE_FLAG = FREE;
    }

	#ifdef OS_TICKS_PER_SEC	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
		OSIntExit();  											 
	#endif
}
//========================================================================
// ����:  void set_buf_free(u8 *buf,u8 len)   
// ����: �������
// ����: 
// ����: ��
//========================================================================
void Set_Buf_Free(u8 *buf,u8 len) 
{
    u8 i;
	for(i=0;i<len;i++)buf[i]=0;
}

//========================================================================
// ����:  void Uart1_DMA_Send_Array(u8 *buffer, u16 len)   
// ����: ����1��������
// ����: buffer���飬len���鳤��
// ����: ��
//========================================================================
void Uart1_DMA_Send_Array(u8 *buffer, u16 len)
{
	while (U1_DMA_SEND_FREE_FLAG == USEING);//�Ƿ���Ҫ����
	memcpy(U1_DMA_T_BUF, buffer, len);
	DMA_Cmd(DMA1_Channel4, DISABLE);  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
	DMA_SetCurrDataCounter(DMA1_Channel4, len);//DMAͨ����DMA����Ĵ�С
	DMA_Cmd(DMA1_Channel4, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
	U1_DMA_SEND_FREE_FLAG = USEING;
}

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
void Wifi_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PB,PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
/* GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	    		 //LED1-->PE.14 �˿�����, �������
 GPIO_Init(GPIOC, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 GPIO_SetBits(GPIOC,GPIO_Pin_14); 	*/					 //PE.14 �����
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //,�������
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 //GPIO_SetBits(GPIOA,GPIO_Pin_5); 						 //PE.11 13����� 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	    		 //,�������
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
 //GPIO_SetBits(GPIOA,GPIO_Pin_4); 						 //PE.11 13����� 
}

/*****************************************************************************************/
/************************ ����ȫ��Ϊ��Ƭ�����͸�app��ָ����Ϣ ****************************/
/*****************************************************************************************/
void Send_FWREV_On_Message(void)
{
  u8 tab[]={0xfe,0x5F,0x01,0X00,0X01,0X0A,0X01,0X03,0X04,0xff};//����ָ��
  Uart1_DMA_Send_Array(tab,10);
}

void Send_On_Message(void)
{
  u8 tab[]={0xfe,0x01,0x01,0xff};//����ָ��
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Off_Message(void)
{
  u8 tab[]={0xfe,0x01,0x02,0xff};//�ػ�ָ��
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Bypass_Message(void)
{
  u8 tab[]={0xfe,0x0a,0x01,0xff};//��·������Ϣ
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Feed_Start_Message(void)
{
  u8 tab[]={0xfe,0x02,0x01,0xff};//���ǿ�ƽ��Ͽ�ʼָ��
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Feed_End_Message(void)
{
  u8 tab[]={0xfe,0x02,0x02,0xff};//���ǿ�ƽ��Ͻ���ָ��
  Uart1_DMA_Send_Array(tab,4);
}
void Send_Set_Temp_Message(u16 tp)
{
  u8 tab[]={0xfe,0x06,0x01,0x00,0x00,0x00,0xff};

	if(BBQ_Status_Struct.fc==0x01)	  //F
	{
		tab[3]=(tp-tp%5)%1000/100;//�¶Ȱ�λ  probe set
		tab[4]=(tp-tp%5)%100/10;//�¶�ʮλ
		tab[5]=(tp-tp%5)%10/1;//�¶ȸ�λ
	}
	else   //c
	{
		if(tp>=32) tp=(tp-32)*5/9;
		else tp=0;
	
		tab[3]=(tp-tp%5)%1000/100;//�¶Ȱ�λ  probe set
		tab[4]=(tp-tp%5)%100/10;//�¶�ʮλ
		tab[5]=(tp-tp%5)%10/1;//�¶ȸ�λ
	}

  Uart1_DMA_Send_Array(tab,7);
}

void Send_Act_Temp_Message(u16 tp)
{
  u8 tab[]={0xfe,0x06,0x02,0x00,0x00,0x00,0xff};

	if(BBQ_Status_Struct.fc==0x01)	  //F
	{
		tab[3]=(tp-tp%5)%1000/100;//�¶Ȱ�λ  probe set
		tab[4]=(tp-tp%5)%100/10;//�¶�ʮλ
		tab[5]=(tp-tp%5)%10/1;//�¶ȸ�λ
	}
	else   //c
	{
		if(tp>=32) tp=(tp-32)*5/9;
		else tp=0;
	
		tab[3]=(tp-tp%5)%1000/100;//�¶Ȱ�λ  probe act
		tab[4]=(tp-tp%5)%100/10;//�¶�ʮλ
		tab[5]=(tp-tp%5)%10/1;//�¶ȸ�λ
	}

  Uart1_DMA_Send_Array(tab,7);
}

void Send_Probe_Temp_Message(u8 probe_ch,u16 probe_x,u16 probe_y)
{
  u8 tab[]={0xfe,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff};//¯�ں�ʳ��̽����л�ָ��
  tab[2]=probe_ch;//1~2        1��PROBE X��2��PROBE Y
  tab[3]=(probe_x-probe_x%5)%1000/100;//�¶Ȱ�λ
  tab[4]=(probe_x-probe_x%5)%100/10;//�¶�ʮλ
  tab[5]=(probe_x-probe_x%5)%10/1;//�¶ȸ�λ
  tab[6]=(probe_y-probe_y%5)%1000/100;//�¶Ȱ�λ
  tab[7]=(probe_y-probe_y%5)%100/10;//�¶�ʮλ
  tab[8]=(probe_y-probe_y%5)%10/1;//�¶ȸ�λ
  Uart1_DMA_Send_Array(tab,10);
}
//void Send_Probe_Statue_Message(u8 statue)
//{
//  u8 tab[]={0xfe,0x08,0x00,0xff};//����probe̽�����״ָ̬��
//  tab[2]=statue;
//  Uart1_DMA_Send_Array(tab,4);
//}
//void Send_Temp_Type_Message(u8 type)
//{
//  u8 tab[]={0xfe,0x09,0x00,0xff};//�����¶ȵ�λ�л�ָ��
//  tab[2]=type;//1:���϶ȣ�F����2�����϶ȣ�C��
//  Uart1_DMA_Send_Array(tab,4);
//}


void Send_Temp_in_uncompensated_On_Message(void)
{
	u8 tab[6]={0xfe};
	u16 in_temp_uncompensated=0;
	tab[1]=0x4f;	  //0x09/0x0b
			
	if(BBQ_Status_Struct.fc==0x01)	  //F
	{

		tab[2]=In_Temp_Uncompensated%1000/100;//�¶Ȱ�λ  probe set
		tab[3]=In_Temp_Uncompensated%100/10;//�¶�ʮλ
		tab[4]=In_Temp_Uncompensated%10/1;//�¶ȸ�λ
	}
	else   //c
	{
		if(In_Temp_Uncompensated>=32) in_temp_uncompensated=(In_Temp_Uncompensated-32)*5/9;
		else in_temp_uncompensated=0;

		tab[2]=in_temp_uncompensated%1000/100;//�¶Ȱ�λ  probe act
		tab[3]=in_temp_uncompensated%100/10;//�¶�ʮλ
		tab[4]=in_temp_uncompensated%10/1;//�¶ȸ�λ
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

	if(BBQ_Status_Struct.probe_status)	  //Ϊ1��˵����probe������ʵ���¶�
	{
	if(BBQ_Status_Struct.fc==0x01)	  //F
	{

		tab[2]=(Probe_TempPC-Probe_TempPC%5)%1000/100;//�¶Ȱ�λ  probe (set (Probe_TempPC-Probe_TempPC%5)
		tab[3]=(Probe_TempPC-Probe_TempPC%5)%100/10;//�¶�ʮλ
		tab[4]=(Probe_TempPC-Probe_TempPC%5)%10/1;//�¶ȸ�λ
	}
	else   //c
	{
		if(Probe_TempPC>=32) probe_temppc=(Probe_TempPC-32)*5/9;
		else probe_temppc=0;

		tab[2]=(probe_temppc-probe_temppc%5)%1000/100;//�¶Ȱ�λ  probe act probe_temppc-probe_temppc%5
		tab[3]=(probe_temppc-probe_temppc%5)%100/10;//�¶�ʮλ
		tab[4]=(probe_temppc-probe_temppc%5)%10/1;//�¶ȸ�λ
	}
}
		else	 //û��probe�������¶�960
	{
		tab[2]=960%1000/100;//�¶Ȱ�λ  probe act
		tab[3]=960%100/10;//�¶�ʮλ
		tab[4]=960%10/1;//�¶ȸ�λ
	}

	if(BBQ_Status_Struct.probe_status)	  //Ϊ1��˵����probe������ʵ���¶�
	{
		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[5]=(Probe_Temp1-Probe_Temp1%5)%1000/100;//�¶Ȱ�λ  probe act Probe_Temp1-Probe_Temp1%5
			tab[6]=(Probe_Temp1-Probe_Temp1%5)%100/10;//�¶�ʮλ
			tab[7]=(Probe_Temp1-Probe_Temp1%5)%10/1;//�¶ȸ�λ
		}
		else   //c
		{
			if(Probe_Temp1>=32) probe_temp1=(Probe_Temp1-32)*5/9;
			else probe_temp1=0;

			tab[5]=(probe_temp1-probe_temp1%5)%1000/100;//�¶Ȱ�λ  probe act probe_temp1-probe_temp1%5
			tab[6]=(probe_temp1-probe_temp1%5)%100/10;//�¶�ʮλ
			tab[7]=(probe_temp1-probe_temp1%5)%10/1;//�¶ȸ�λ
		}
	}
	else	 //û��probe�������¶�960
	{
		tab[5]=960%1000/100;//�¶Ȱ�λ  probe act
		tab[6]=960%100/10;//�¶�ʮλ
		tab[7]=960%10/1;//�¶ȸ�λ
	}

	if(BBQ_Status_Struct.probe_status)	  //Ϊ1��˵����probe������ʵ���¶�
	{
		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[8]=(Probe_Temp2-Probe_Temp2%5)%1000/100;//�¶Ȱ�λ  probe act Probe_Temp2-Probe_Temp2%5
			tab[9]=(Probe_Temp2-Probe_Temp2%5)%100/10;//�¶�ʮλ
			tab[10]=(Probe_Temp2-Probe_Temp2%5)%10/1;//�¶ȸ�λ
		}
		else   //c
		{
			if(Probe_Temp2>=32) probe_temp2=(Probe_Temp2-32)*5/9;
			else probe_temp2=0;

			tab[8]=(probe_temp2-probe_temp2%5)%1000/100;//�¶Ȱ�λ  probe act probe_temp2-probe_temp2%5
			tab[9]=(probe_temp2-probe_temp2%5)%100/10;//�¶�ʮλ
			tab[10]=(probe_temp2-probe_temp2%5)%10/1;//�¶ȸ�λ
		}
	}
	else	 //û��probe�������¶�960
	{
		tab[8]=960%1000/100;//�¶Ȱ�λ  probe act
		tab[9]=960%100/10;//�¶�ʮλ
		tab[10]=960%10/1;//�¶ȸ�λ
	}
	
//	if(BBQ_Status_Struct.grill_set_act==1)//grill set
//	{

		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[11]=(In_Temp_Set-In_Temp_Set%5)%1000/100;//�¶Ȱ�λ  probe act In_Temp_Set-In_Temp_Set%5
			tab[12]=(In_Temp_Set-In_Temp_Set%5)%100/10;//�¶�ʮλ
			tab[13]=(In_Temp_Set-In_Temp_Set%5)%10/1;//�¶ȸ�λ
		}
		else   //c
		{
			if(In_Temp_Set>=32) rtd_set_temp=(In_Temp_Set-32)*5/9;
			else rtd_set_temp=0;

			tab[11]=(rtd_set_temp-rtd_set_temp%5)%1000/100;//�¶Ȱ�λ  probe act rtd_set_temp-rtd_set_temp%5
			tab[12]=(rtd_set_temp-rtd_set_temp%5)%100/10;//�¶�ʮλ
			tab[13]=(rtd_set_temp-rtd_set_temp%5)%10/1;//�¶ȸ�λ
		}

//	}
//	else if(BBQ_Status_Struct.grill_set_act==2)//grill act
//	{

		if(BBQ_Status_Struct.fc==0x01)	  //F
		{
			tab[14]=(In_Temp-In_Temp%5)%1000/100;//�¶Ȱ�λ  probe act  Probe_TempPC-Probe_TempPC%5
			tab[15]=(In_Temp-In_Temp%5)%100/10;//�¶�ʮλ
			tab[16]=(In_Temp-In_Temp%5)%10/1;//�¶ȸ�λ
		}
		else   //c
		{
			if(In_Temp>=32) rtd_temp=(In_Temp-32)*5/9;
			else rtd_temp=0;

			tab[14]=(rtd_temp-rtd_temp%5)%1000/100;//�¶Ȱ�λ  probe act rtd_temp-rtd_temp*5
			tab[15]=(rtd_temp-rtd_temp%5)%100/10;//�¶�ʮλ
			tab[16]=(rtd_temp-rtd_temp%5)%10/1;//�¶ȸ�λ
		}

//	}
//	else //grill ����ERR����
//	{

//			tab[8]=(In_Temp)%1000/100;//�¶Ȱ�λ  probe act
//			tab[9]=(In_Temp)%100/10;//�¶�ʮλ
//			tab[10]=(In_Temp)%10/1;//�¶ȸ�λ

//	}

/*if(Probe_Temp1>=32) probe_temp=(Probe_Temp1-32)*5/9;
else probe_temp=0;	*/
		
	tab[17]=BBQ_Status_Struct.fc;//�¶ȵ�λF
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
//	tab[12]=BBQ_Status_Struct.probe_status;//̽�����״̬
//	tab[13]=BBQ_Status_Struct.grill_set_act;//grill��set����act
//	
//	tab[14]=BBQ_Status_Struct.onoff;//1:ON,2:OFF
//	
//	tab[15]=2;//1:feed,2:no feed
//	
//	tab[16]=BBQ_Status_Struct.fc;//�¶ȵ�λF
	tab[22]=0xff;
	Uart1_DMA_Send_Array(tab,23);
}

/*void Send_Display_Data_Message(void)
{
	u8 tab[18]={0xfe,0x0b};
	
	tab[2]=(Probe_Temp_Set-Probe_Temp_Set%5)%1000/100;//�¶Ȱ�λ  probe set
	tab[3]=(Probe_Temp_Set-Probe_Temp_Set%5)%100/10;//�¶�ʮλ
	tab[4]=(Probe_Temp_Set-Probe_Temp_Set%5)%10/1;//�¶ȸ�λ

	if(BBQ_Status_Struct.probe_status)	  //Ϊ1��˵����probe������ʵ���¶�
	{
		tab[5]=(Probe_Temp1-Probe_Temp1%5)%1000/100;//�¶Ȱ�λ  probe act
		tab[6]=(Probe_Temp1-Probe_Temp1%5)%100/10;//�¶�ʮλ
		tab[7]=(Probe_Temp1-Probe_Temp1%5)%10/1;//�¶ȸ�λ
	}
	else	 //û��probe�������¶�960
	{
		tab[5]=960%1000/100;//�¶Ȱ�λ  probe act
		tab[6]=960%100/10;//�¶�ʮλ
		tab[7]=960%10/1;//�¶ȸ�λ
	}
	
	if(BBQ_Status_Struct.grill_set_act==1)//grill set
	{
	  tab[8]= In_Temp_Set%1000/100;//�¶Ȱ�λ
	  tab[9]= In_Temp_Set%100/10;//�¶�ʮλ
	  tab[10]=In_Temp_Set%10/1;//�¶ȸ�λ
	}
	else
	{
	  tab[8]= (In_Temp-In_Temp%5)%1000/100;//�¶Ȱ�λ  grill act
	  tab[9]= (In_Temp-In_Temp%5)%100/10;//�¶�ʮλ
	  tab[10]=(In_Temp-In_Temp%5)%10/1;//�¶ȸ�λ
	}
	
	tab[11]=1;	 //probe x
	tab[12]=BBQ_Status_Struct.probe_status;//̽�����״̬
	tab[13]=BBQ_Status_Struct.grill_set_act;//grill��set����act
	
	tab[14]=BBQ_Status_Struct.onoff;//1:ON,2:OFF
	
	tab[15]=2;//1:feed,2:no feed
	
	tab[16]=1;//�¶ȵ�λF
	tab[17]=0xff;
	Uart1_DMA_Send_Array(tab,18);
}*/
//void Send_All_Temp_Message(void)
//{
//	u8 tab[22]={0xfe,0x0c};
//	tab[2]=Probe_Temp_Set%1000/100;//�¶Ȱ�λ		  probe1 set
//	tab[3]=Probe_Temp_Set%100/10;//�¶�ʮλ
//	tab[4]=Probe_Temp_Set%10/1;//�¶ȸ�λ
//	tab[5]=Probe_Temp1%1000/100;//�¶Ȱ�λ  probe1 act
//	tab[6]=Probe_Temp1%100/10;//�¶�ʮλ
//	tab[7]=Probe_Temp1%10/1;//�¶ȸ�λ
//  
//	tab[8]=0;//�¶Ȱ�λ		probe2 set act
//	tab[9]=0;//�¶�ʮλ
//	tab[10]=0;//�¶ȸ�λ
//	tab[11]=0;//�¶Ȱ�λ	probe2 act
//	tab[12]=0;//�¶�ʮλ
//	tab[13]=0;//�¶ȸ�λ
//	
//	tab[14]=In_Temp_Set%1000/100;//�¶Ȱ�λ	   grill set
//	tab[15]=In_Temp_Set%100/10;//�¶�ʮλ
//	tab[16]=In_Temp_Set%10/1;//�¶ȸ�λ
//	tab[17]=In_Temp%1000/100;//�¶Ȱ�λ		   grill act
//	tab[18]=In_Temp%100/10;//�¶�ʮλ
//	tab[19]=In_Temp%10/1;//�¶ȸ�λ
//	
//	tab[20]=1;//�¶ȵ�λ F
//	tab[21]=0xff;	 
//	Uart1_DMA_Send_Array(tab,22);

//}
//void Send_Live_Temp_Message(u32 time,u16 in_temp,u16 probe1_temp,u16 probe2_temp)		//����ʵʱ�¶�
//{
//	u8 tab[16]={0xfe,0x0e};
//	tab[2]=time/10/60%1000/100;//��ʱ��λ
//	tab[3]=time/10/60%100/10;//��ʱʮλ
//	tab[4]=time/10/60%10/1;//��ʱ��λ
//	tab[5]=time/60%10/1;//��ʱһ��С��λ  
//	
//	tab[6]= (in_temp-in_temp%5)%1000/100;//grill�¶Ȱ�λ
//	tab[7]= (in_temp-in_temp%5)%100/10;//�¶�ʮλ
//	tab[8]=(in_temp-in_temp%5)%10/1;//�¶ȸ�λ

//	if(BBQ_Status_Struct.probe_status)	  //Ϊ1��˵����probe������ʵ���¶�
//	{
//		tab[9]=(probe1_temp-probe1_temp%5)%1000/100;//�¶Ȱ�λ  probe act
//		tab[10]=(probe1_temp-probe1_temp%5)%100/10;//�¶�ʮλ
//		tab[11]=(probe1_temp-probe1_temp%5)%10/1;//�¶ȸ�λ
//	}
//	else	 //û��probe�������¶�960
//	{
///*		tab[5]=960%1000/100;//�¶Ȱ�λ  probe act
//		tab[6]=960%100/10;//�¶�ʮλ
//		tab[7]=960%10/1;//�¶ȸ�λ*/
//		tab[9]=0;//�¶Ȱ�λ  probe act
//		tab[10]=0;//�¶�ʮλ
//		tab[11]=0;//�¶ȸ�λ
//	}

//  
//	
//	/*  tab[12]=probe2_temp%1000/100;//probe2�¶Ȱ�λ
//	tab[13]=probe2_temp%100/10;//�¶�ʮλ
//	tab[14]=probe2_temp%10/1;//�¶ȸ�λ	*/
//	tab[12]=960%1000/100;//�¶Ȱ�λ  probe act
//	tab[13]=960%100/10;//�¶�ʮλ
//	tab[14]=960%10/1;//�¶ȸ�λ

//	
//	tab[15]=0xff;	 				   
//	Uart1_DMA_Send_Array(tab,16);

//}

//void Send_ALL_Set_Temp_Message(u16 in_temp,u16 probe1_temp)		//�����趨�¶�
//{
//  u8 tab[9]={0xfe,0x0d};

//  tab[2]=in_temp%1000/100;//grill�¶Ȱ�λ
//  tab[3]=in_temp%100/10;//�¶�ʮλ
//  tab[4]=in_temp%10/1;//�¶ȸ�λ

//  tab[5]=probe1_temp%1000/100;//probe1�¶Ȱ�λ
//  tab[6]=probe1_temp%100/10;//�¶�ʮλ
//  tab[7]=probe1_temp%10/1;//�¶ȸ�λ

//  
//  tab[8]=0xff;	 				   
//  Uart1_DMA_Send_Array(tab,9);

//}
//void Send_Run_Time_Message(u32 time)		//��������ʱ��
//{
//  u8 tab[6]={0xfe,0x0f};
//  tab[2]=time/10/60%1000/100;//��ʱ��λ
//  tab[3]=time/10/60%100/10;//��ʱʮλ
//  tab[4]=time/10/60%10/1;//��ʱ��λ
////  tab[5]=time*10/60%10/1;//��ʱһ��С��λ

//  
//  tab[5]=0xff;	 				   
//  Uart1_DMA_Send_Array(tab,6);

//}

//void Send_Recipe_Message(void)
//{
//  u8 tab[]={0xfe,0x31,0x01,0xff};//���ز˵����ճɹ�����
//  Uart1_DMA_Send_Array(tab,4);
//}

//void Send_Recipe_Over_Message(void)
//{
//  u8 tab[]={0xfe,0x31,0x02,0xff};//���ز˵����ճɹ�����
//  Uart1_DMA_Send_Array(tab,4);
//}

void Send_Goto_WiFi_Password_Mode(void)
{
  u8 tab[]={0xfe,0x22,0x01,0xff};//���� ��������WIFI����ģʽ ָ��
  Uart1_DMA_Send_Array(tab,4);
}

/******** �Խ��յ������ݽ��д��� *******
���ܣ�    ������յ�������
data�� �������ݵĻ����ַ        
num���������ݵĳ���
**************************************/
void Recv_Data_Handle(u8 *data,u16 num)
{
     u8 i=0;
	 if(num<4)return;
     
     if(data[0]==0xFE)//��ͷ
     {
          switch(data[1])
          {
                  case 0x01:/*���ػ�*/
                      if(data[2]==0x01&&data[3]==0xff)//����
                      {
						 Key_Onoff_ShortPressOK=1;
						 BBQ_Status_Struct.onoff=1;
                      }
                      else if(data[2]==0x02&&data[3]==0xff)//�ػ�
                      {
						 Key_Onoff_App_LongPressOK=1;
						 BBQ_Status_Struct.onoff=2;
                      }
                      break;
                  case 0x02:/*���ǿ�ƽ���*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
						Key_Prime_App_ContinuousPressOK=1;
//                              key_value |= Feed_Long_Press;                                         
                      }
					  else if(data[2]==0x02&&data[3]==0xff)  /*ǿ�ƽ��Ͻ���*/
					  {
						Key_Prime_App_ContinuousPressOK=0;
					  
					  }
                      break;
                  case 0x03:/*�����¶�*/
                      if(data[2]==0x01&&data[4]==0xff)//����GRILL SET�¶�
                      {
							Key_Up_ShortPressOK=1;//������һ
							Key_Up_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=1;				  	

 /*                             if(grill_set_temp+data[3]<999)grill_set_temp+=data[3];
                              send_set_temp_message(PLUS_TEMP,data[2],(u16)grill_set_temp);//����GRILL_SET�����¶���Ϣ    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	*/
                      }
                      else if( (data[2]==0x02|data[2]==0x03) &&data[4]==0xff)//����PROBE_X_SET��PROBE_X_SET�¶�
                      {
        /*                      u8 ch=data[2]-2;
                              if(probe_set_temp[ch]+data[3]<999)probe_set_temp[ch]+=data[3];
                              send_set_temp_message(PLUS_TEMP,data[2],(u16)probe_set_temp[ch]);//����PROBE_X_SET��PROBE_Y_SET�����¶���Ϣ    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	*/
							Key_Up_ShortPressOK=1;//������һ
							Key_Up_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=2;				  	

                      }
                      break;
                  case 0x04:/*�����¶�*/
                      if(data[2]==0x01&&data[4]==0xff)//����GRILL SET�¶�
                      {
							Key_Down_ShortPressOK=1;//������һ
							Key_Down_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=1;				  	

              /*                if(grill_set_temp>data[3])grill_set_temp-=data[3];
                              send_set_temp_message(MINUS_TEMP,data[2],(u16)grill_set_temp);//����GRILL_SET�����¶���Ϣ    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	  */
                      }
                      else if( (data[2]==0x02||data[2]==0x03) &&data[4]==0xff)//����PROBE_X_SET��PROBE_X_SET�¶�
                      {
                   /*           u8 ch=data[2]-2;
                              if(probe_set_temp[ch]>data[3])probe_set_temp[ch]-=data[3];
                              send_set_temp_message(MINUS_TEMP,data[2],(u16)probe_set_temp[ch]);//����PROBE_X_SET��PROBE_Y_SET�����¶���Ϣ    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	   */
							Key_Down_ShortPressOK=1;//������һ
							Key_Down_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3];
					  		Temp_Press_Step=2;				  	
                      }
                    break;
                  case 0x05:/*ֱ���趨�¶�*/
                      if(data[2]==0x01&&data[6]==0xff)//ֱ���趨GRILL SET�¶�
                      {
							Key_Direct_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3]%10*100+data[4]%10*10+data[5]%10;
					  		Temp_Press_Step=1;				  	

                 /*             grill_set_temp=data[3]%10*100+data[4]%10*10+data[5]%10;
                              send_set_temp_message(DIRECT_SET_TEMP,data[2],(u16)grill_set_temp);//����GRILL_SETֱ���趨��Ϣ    
                              if(LCD_display_bag.grill_set==ON)
                              {
                                  LCD_display_bag.grill_data=(u16)grill_set_temp;
                              }
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	*/
                      }
                      else if( (data[2]==0x02||data[2]==0x03) &&data[6]==0xff)//ֱ���趨PROBE_X_SET��PROBE_X_SET�¶�
                      {
                  /*            u8 ch=data[2]-2;
                              probe_set_temp[ch]=data[3]%10*100+data[4]%10*10+data[5]%10;;
                              send_set_temp_message(DIRECT_SET_TEMP,data[2],(u16)probe_set_temp[ch]);//����PROBE_X_SET��PROBE_Y_SETֱ���趨��Ϣ    
                              LCD_display_bag.probe_set_data[ch]=(u16)probe_set_temp[ch];
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ
                              Buzzer_Ring(100);	   */
							Key_Direct_App_ShortPressOK=1; //��������������app���Ǳ���
							Key_Up_Down_App_Temp=data[3]%10*100+data[4]%10*10+data[5]%10;
					  		Temp_Press_Step=2;				  	
                      }                    
                      break;
                  case 0x06:/*GRILL_SET��GRILL_ACT�л�*/
                      if(data[2]==0x01&&data[3]==0xff)//grill��ʾ�¶��л�ΪSET, temp in setting
                      {
													Send_Set_Temp_Message(In_Temp_Set);
                      }  
                      else if(data[2]==0x02&&data[3]==0xff)//grill��ʾ�¶��л�ΪACT, temp in ACT
                      {
													Send_Act_Temp_Message(In_Temp);
                      }  
                      else if(data[2]==0x03&&data[3]==0xff)//grill��ʾ�¶��л�ΪSET, temp in setting
                      {
												Key_Set_ShortPressOK=1;
												Temp_Press_Step=0;	              
                      }   
                      else if(data[2]==0x04&&data[3]==0xff)//grill��ʾ�¶��л�ΪSET, PC setting
                      {
												Key_Set_ShortPressOK=1;
												Temp_Press_Step=1;	              
                      }   
                      break;
                  case 0x07:/*̽�루PROBE��ͨ���л�*/
                      if( (data[2]==0x01||data[2]==0x02) &&data[3]==0xff)
                      {
                     /*         u8 ch=data[2];
                              LCD_display_bag.probe_ch=ch-1;
                              LCD_data_calcu();//ת��Ϊ��ʾ���ݸ�ʽ 
                              send_probe_temp_message(ch,LCD_display_bag.probe_set_data[ch-1],LCD_display_bag.probe_act_data[ch-1]);
                              Buzzer_Ring(100);	   */
							  Send_Probe_Temp_Message(1,Probe_Temp_Set,Probe_Temp1);
                      }
                      break;
                  case 0x08:/**/
                    
                      break;
                  case 0x09:/*�¶ȵ�λ�л�*/
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
//                  case 0x0A:/*Bypass Startup ��·����������Ԥ�Ƚ׶�*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//                   //           key_value |= Bypass_Long_Press;
//                      }
//                      break;
                  case 0x11:/*��ѯ��ǰ��ʾ����*/
                      if(data[2]==0x01&&data[3]==0xff)
                      {
					  	Send_Display_Data_Message(0x11);
                      }                    
                      break;
//                  case 0x11:/*��ѯ��������*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						 Send_All_Temp_Message();
//                         /*     send_all_temp_message();
//                              Buzzer_Ring(100);	   */
//                      }                    
//                      break;
//                  case 0x0d:/*���ι��̵�2��Ŀ���¶�*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_ALL_Set_Temp_Message(In_Temp_Set,Probe_Temp_Set);		//�����趨�¶�
//                      }                    
//                      break;
//                  case 0x0e:/*���ι��̵�2��ʱʱ�¶�*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_Live_Temp_Message(System_Run_Counter,In_Temp,Probe_Temp1,Probe_Temp2);		//����ʵʱ�¶�
//                      }                    
//                      break;
//                  case 0x0f:/*���ر��ι��̿�ʼ�����ڵ�ʱ�䣨���ӣ�*/
//                      if(data[2]==0x01&&data[3]==0xff)
//                      {
//						Send_Run_Time_Message(System_Run_Counter);		//��������ʱ��
//                      }                    
//                      break;
                  case 0x12:/*������־*/
                      if((data[3]==0xff))
                      {
												Wifi_Blink=data[2];
                      }       
                      else
                      {
												Wifi_Blink=0x00;
                      }  											
                      break;
									case 0x15:/*������ʾȨ��*/
                      if(data[6]==0xff)
                      {
												APP_Display_Override=data[2]; //1Ϊ������ʾ2Ϊ������
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
                  case 0x31:/*�˵�*/
                      if(data[2]==0x02&&data[3]==0xff)
					  {
					  	Recipe_Start=2;//�˵�����������
						Recipe_Step=0;	
						Recipe_Beep=1;//�������

//					  	Send_Recipe_Over_Message();
					  }
                      else if(data[66]==0xff)
                      {
					  	for(i=0;i<64;i++)
						{
							Recipe_List[i]=data[i+2];//Ԫ��2���ܹ����裬����ÿ��7���ֽڣ�����1+�¶�3+ʱ��3
						}

						Recipe_Step=Recipe_List[1];//��һ��
						Recipe_Start=1;
						Recipe_Beep=1;//�������
//						Send_Recipe_Message();
                      }                    
                      break;


          }
     }	
     //Device_address=STMFLASH_ReadHalfWord(DEVICE_ADDRESS)+(u32)STMFLASH_ReadHalfWord(DEVICE_ADDRESS+2)*65536;//��ȡ32λ��ַ
     //recv_data_address=(((u32)data[4])+((u32)data[5]<<8)+((u32)data[6]<<16)+((u32)data[7]<<24));//��ȡ�����е��豸��ַ

   

     //recv_crc_data=data[num-4]+data[num-3]*256;//��ȡ�����е�У��ֵ
     //clcu_crc_data=crc(data+2,num-6);//���յ��������������У��

     
}































