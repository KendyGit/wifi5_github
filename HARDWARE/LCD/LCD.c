
#include "lcd.h"
#include "delay.h"
#include "main.h"

//u8 All_Status_flags;
static u8 LCD_0_1_Data=0; //wifi, bluetooth,motor status display
static u8 LCD_20_21_Data=0;//�����ַ20��21��8���ֽ�Ҫ��ʾ������	P1 P2 F C F C SET ACTUAL
//static u8 LCD_22_Data=0;//�����ַ22��4���ֽ�Ҫ��ʾ������(ֻ�ø���λ)	wifi F C ��
u8 Probe1_ERR=0; //probe1û�н�
u8 Probe2_ERR=0; //probe2û�н�
u8 ProbePC_ERR=0; //probe2û�н�
u8 ProbeTempIn_ERR=0;

u8 disbuff_TempIn[4]={0,0,0,0};
u8 disbuff_TempP1[3]={0,0,0};
u8 disbuff_TempP2[3]={0,0,0};
u8 disbuff_TempPC[3]={0,0,0};
u8 Lcd_A2829A3031[2]={0,0};

u8 DispAZTab[]={0x77,0xE6,0xF0,0xC7,0xF2,0x72,0xF4,0x67,0x40,0xC5,0xE3,0xE0,0x56,0x46,0xC6,0x73,0x37,0x72,0x36,0xE2,0xE5,0xE7,0xD4,0xA3,0xA7,0x92};
//				  A	  B	   C	D	  E	   F	G	 H	  I	   J	K	 L	  M	   N   O	P	 Q	   R	S	T	  U	   V	W	X	 Y
u8 DispAppTab[]={0x77,0x73,0x73,0x00};//ȫ����ʾapp
//							a	p	   p
	
	/***˵��***
 1621������CS��д�����100����ʼ��LCD����д�����ݣ�101��
 ���ݸ�ʽ 101(����)+A5 A4 A3 A2 A1 A0����ַ��+D0 D1 D2 D3(����)
 ����дֻ��Ҫд��һ������͵�ַ������д�����ݣ�д����������CS
**********/
u8 LcdOffTab[]={0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00,
								0x00,0x00,0x00,0x00};  //����ʹ��ʽLCD��ʾ�ر�
u8 LcdOnTab[]={0xF8,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF};  //����ʹ��ʽLCD��ʾȫ��
u8 LcdUseOnNoPCTab[]={0xF8,0x00,0x00,0x00,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF};  //only display the used segments
u8 LcdUseOnTab[]={0xF8,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF};  //only display the used segments
/***********��ʾ************/
u8 DispErrTab[]={0xF2,0x42,0x42,0x00};//ȫ����ʾErr
//								                     E	r	   r
u8 DispErfTab[]={0xF2,0x42,0x72,0x00};//ȫ����ʾErf no used 20180618Kendy
//								                     E	r	   f
//u8 DispEr1Tab[]={0xF2,0x42,0x05,0x00};//ȫ����ʾEr1 no used 20180618Kendy
//								                      E	r	   1
u8 DispEr1Tab[]={0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,
							 0x00,0x00,0xF2,0x42,0x05,0x00,
							 0x00,0x00};
//u8 DispEr2Tab[]={0xF2,0x42,0xd3,0x00};//ȫ����ʾEr2 no used 20180618Kendy
//								                      E	r	   2
u8 DispEr2Tab[]={0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,
							 0x00,0x00,0xF2,0x42,0xd3,0x00,
							 0x00,0x00};
//u8 DispEr3Tab[]={0xF2,0x42,0x9F,0x00};//ȫ����ʾEr3 no used 20180618Kendy
//								                      E	r	   3
u8 DispEr3Tab[]={0x00,0x00,0x00,0x00,
							 0x00,0x00,0x00,0x00,
							 0x00,0x00,0xF2,0x42,0x97,0x00,
							 0x00,0x00};
u8 DispErHTab[]={0xF2,0x42,0x67,0x00};//ȫ����ʾErH no used 20180618Kendy
//								                      E	r	   H
u8 DispNoprobeOFFTab[]={0x46,0xC6,0x73};//noP
//
u8 DispBypTab[]={0xe6,0xa7,0x73};//byp no used 20180618Kendy
//
u8 DispStaTab[]={0x36,0xe2,0x77};//sta no used 20180618Kendy
//	
u8 DispSmoTab[]={0x36,0x56,0xc6};//smo no used 20180618Kendy
//	
u8 DispOffTab[]={0xc6,0x72,0x72};//off
//
u8 DispPriTab[]={0x73,0x42,0x40};//pri
//	
u8 DispFedTab[]={0x72,0xF2,0xcf};//fed
//								                    	   

u8 DispNumTab[]={0xFD,0x05,0xd3,0x9F,0x27,0xB6,0xF6,0x15,0xFF,0xB7};
//				         0	  1	   2	  3	  4	   5	   6	 7	  8	   9
u8 DispOnInitTab[]={0xF8,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF};  //only display the used segments																

//void Ht1621_VCC_0() { LCD_BK_Light = 0;}  //VCC_power
//void Ht1621_VCC_1() { LCD_BK_Light = 1;}

void Ht1621_CS_0(void) { LCD_CS = 0;}  // CS
void Ht1621_CS_1(void) { LCD_CS = 1;}

void Ht1621_WR_0(void) { LCD_WR = 0;}  // WR
void Ht1621_WR_1(void) { LCD_WR = 1;}

void Ht1621_DO_0(void) { LCD_DA = 0;}  // DAT
void Ht1621_DO_1(void) { LCD_DA = 1;}

//========================================================================
// ����:  void Ht1621Wr_Data(u8 Data,u8 cnt)   
// ����: д������
// ����: DataҪд������ݣ�cntҪд�������ж���λ
// ����: ��
//========================================================================
void Ht1621Wr_Data(u8 Data,u8 cnt) 
{ 
  u8 i; 
  for (i=0;i<cnt;i++) 
   {   
     Ht1621_WR_0();
	 delay_us(2);
 
     if((Data & 0x80)==0x80) {Ht1621_DO_1();}  //�ж����λ��1����0
     else {Ht1621_DO_0();}
     Ht1621_WR_1(); 
	 delay_us(2);
     Data<<=1; 
   } 
} 
//========================================================================
// ����:  void Ht1621WrCmd(u8 Cmd)   
// ����: ����д������
// ����: CmdҪд�������
// ����: ��
//========================================================================
void Ht1621WrCmd(u8 Cmd) 
{ 
   Ht1621_CS_0(); 
   Ht1621Wr_Data(0x80,4);          //д�������־100 
   Ht1621Wr_Data(Cmd,8);           //д���������� 
   Ht1621_CS_1(); 
} 
//========================================================================
// ����:  void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt)   
// ����: ����д��һ������
// ����: Addr��ַ0~31��pҪд������飬cnt����Ԫ�ض��ٸ�
// ����: ��
//========================================================================
void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt)
{
  u8 i;
  Ht1621_CS_0();
  Ht1621Wr_Data(0xa0,3); //д�����ݱ�־101
  Ht1621Wr_Data(Addr<<2,6); //д���ַ���ݣ��ܹ��е�ַ011111��2^5=32
  for (i=0;i<cnt;i++)
   {
    Ht1621Wr_Data(*p,8); //д������,32*4λ�ļĴ�����������������д��ÿ��д8��
    p++;
   }
  Ht1621_CS_1();
}
//========================================================================
// ����:  void Ht1621WrOneData(u8 Addr,u8 data)   
// ����: ��һ����ַдһ������
// ����: Addr��ַ0~31��data����
// ����: ��
//========================================================================
void Ht1621WrOneData(u8 Addr,u8 data)
{
  Ht1621_CS_0();
  Ht1621Wr_Data(0xa0,3); //д�����ݱ�־101
  Ht1621Wr_Data(Addr<<2,6); //д���ַ���ݣ��ܹ��е�ַ011111��2^5=32
  Ht1621Wr_Data(data,8); //д������,32*4λ�ļĴ�����������������д��ÿ��д8��
  Ht1621_CS_1();
}
//========================================================================
// ����:  void Ht1621WrHalfData(u8 Addr,u8 data)   
// ����: ��һ����ַд��4λ
// ����: Addr��ַ0~31��data����
// ����: ��
//========================================================================
void Ht1621WrHalfData(u8 Addr,u8 data)
{
  Ht1621_CS_0();
  Ht1621Wr_Data(0xa0,3); //д�����ݱ�־101
  Ht1621Wr_Data(Addr<<2,6); //д���ַ���ݣ��ܹ��е�ַ011111��2^5=32
  Ht1621Wr_Data(data,4); //д������,32*4λ�ļĴ�����ÿ����ַ���ĸ���
  Ht1621_CS_1();
}

//========================================================================
// ����:  void Ht1621_Init(void)   
// ����: LCD��ʼ��
// ����: ��
// ����: ��
//========================================================================
void Ht1621_Init(void) 
{ 
	/**********��ʼ��IO*********/
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);	 //ʹ��PB,PA�˿�ʱ��
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);  //ʹ�ܽ�ֹJTAG
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_13|GPIO_Pin_14;	    		 //�������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_13|GPIO_Pin_14); 						 //PB4,12,13,14,15����� 


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	    		 //�������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	GPIO_SetBits(GPIOA,GPIO_Pin_15); 

	delay_ms(10);
   	/**********��ʼ��LCD**********/
	Ht1621WrCmd(LCD_BIAS); 
	Ht1621WrCmd(LCD_RC256);             //ʹ���ڲ����� 
	//Ht1621WrCmd(LCD_XTAL);             //ʹ���ⲿ���� 
	Ht1621WrCmd(LCD_SYSDIS); 
	Ht1621WrCmd(LCD_WDTDIS1); 
	Ht1621WrCmd(LCD_SYSEN); 
	Ht1621WrCmd(LCD_ON); 
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
		delay_ms(10);
		LCD_On_NoPC_Init_Dis(0x00);
		delay_ms(500);
		LCD_All_Off();	   //����ȫ��
		delay_ms(500);
		LCD_On_NoPC_Init_Dis(0x00);
		delay_ms(500);
		LCD_All_Off();	   //����ȫ��
		delay_ms(500);	
		LCD_On_NoPC_Init_Dis(0x00);
		delay_ms(500);
		LCD_All_Off();	   //����ȫ��
		delay_ms(500);	
	}
} 

//========================================================================
// ����:  void LCD_In_Temp(void)   
// ����: ��ʾ¯���¶�
// ����: temp¯���¶ȣ����999
// ����: ��
//========================================================================
void LCD_In_Temp(u16 temp, u8 All_Action_sensor_Status_display) 
{ 
	u8  MOT_status_temp=0;
	u8  All_Action_sensor_Status_flags=0;
//  NA  NA  MOT FANERR MOTERR HOT  HOTERR FAN
//  7   6    5    4     3      2    1     0
	MOT_status_temp = All_Action_sensor_Status_display & 0x20;
	if (MOT_status_temp==0x20)LCD_0_1_Data|=0x08;
	else LCD_0_1_Data&=0xf7;
	
	
		if(ProbeTempIn_ERR)	//û�壬��ʾNOP
	{
		disbuff_TempIn[0]=DispNoprobeOFFTab[0]|0x08;
		disbuff_TempIn[1]=DispNoprobeOFFTab[1]&0xF7;
		disbuff_TempIn[2]=DispNoprobeOFFTab[2]&0xF7;
		disbuff_TempIn[3]=0x00;
		Lcd_A2829A3031[0]=0;
		Lcd_A2829A3031[1]=0;
	} 
	else
	{
		if (temp<=180)
		{
		Lcd_A2829A3031[0]=0;
		Lcd_A2829A3031[1]=0x80;
		}
		else if((temp<=200)&& (temp>180))
		{
		Lcd_A2829A3031[0]=0x00;
		Lcd_A2829A3031[1]=0xc0;
		}
		else if((temp<=225)&& (temp>200))
		{
		Lcd_A2829A3031[0]=0x01;
		Lcd_A2829A3031[1]=0xc0;
		}
		else if((temp<=250)&& (temp>225))
		{
		Lcd_A2829A3031[0]=0x03;
		Lcd_A2829A3031[1]=0xc0;
		}
		else if((temp<=300)&& (temp>250))
		{
		Lcd_A2829A3031[0]=0x07;
		Lcd_A2829A3031[1]=0xc0;
		}		
		else if((temp<=350)&& (temp>300))
		{
		Lcd_A2829A3031[0]=0x0f;
		Lcd_A2829A3031[1]=0xc0;
		}	
		else if((temp<=400)&& (temp>350))
		{
		Lcd_A2829A3031[0]=0x8f;
		Lcd_A2829A3031[1]=0xc0;
		}	
		else if((temp<=450)&& (temp>400))
		{
		Lcd_A2829A3031[0]=0xcf;
		Lcd_A2829A3031[1]=0xc0;
		}	
		else if((temp<=475)&& (temp>450))
		{
		Lcd_A2829A3031[0]=0xef;
		Lcd_A2829A3031[1]=0xc0;
		}	
		else if((temp>475)&& (temp<600))
		{
		Lcd_A2829A3031[0]=0xff;
		Lcd_A2829A3031[1]=0xc0;
		}
		else 
		{
		Lcd_A2829A3031[0]=0;
		Lcd_A2829A3031[1]=0;
		};
		
		Lcd_A2829A3031[1]+=All_Action_sensor_Status_display;
		Lcd_A2829A3031[1]&=0xDF;
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
		{
			if(temp<=32)temp=0;
			else temp = (temp-32)*5/9;
		};
	disbuff_TempIn[0]=(temp)%1000/100;//-temp%5
	disbuff_TempIn[1]=(temp)%100/10;//-temp%5
	disbuff_TempIn[2]=(temp)%10;//-temp%5

	disbuff_TempIn[0]=DispNumTab[disbuff_TempIn[0]]|0x08;	 //query segment table, the lowest significant bit will be the S20.
	if(BBQ_Status_Struct.grill_set_act==1)		//set����
	{
		disbuff_TempIn[1]=DispNumTab[disbuff_TempIn[1]]|0x08;
		disbuff_TempIn[2]=DispNumTab[disbuff_TempIn[2]]&0xf7; 
	}
	else
	{
		disbuff_TempIn[1]=DispNumTab[disbuff_TempIn[1]]&0xf7;
		disbuff_TempIn[2]=DispNumTab[disbuff_TempIn[2]]|0x08; 
	}
	if(BBQ_Status_Struct.fc==0x01)//һ������ʾF					  //0ΪF��1Ϊc
		{disbuff_TempIn[3]=0x72;}
	else
		{disbuff_TempIn[3]=0xF0;};
	}
	
	Ht1621WrOneData(0,LCD_0_1_Data);
	Ht1621WrAllData(20,disbuff_TempIn,4); //�ӵ�ַ14��ʼд��
	Ht1621WrAllData(28,Lcd_A2829A3031,2); //�ӵ�ַ14��ʼд��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
	


}  

//========================================================================
// ����:  void LCD_In_Temp_Undis(void)   
// ����: �ر���ʾ¯���¶�
// ����: 
// ����: ��
//========================================================================
void LCD_In_Temp_Undis(void) 
{ 
	disbuff_TempIn[0]=0x08;
	disbuff_TempIn[1]=0x00;
	disbuff_TempIn[2]=0x00;
	disbuff_TempIn[3]=0x00;
	Ht1621WrAllData(20,disbuff_TempIn,4); //�ӵ�ַ14��ʼд��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
}

//========================================================================
// ����:  void LCD_Probe1_Temp(void)   
// ����: ��ʾprobe1�¶�
// ����: temp probe1�¶ȣ����999 
// ����: ��
//========================================================================
void LCD_Probe1_Temp(u16 temp) 
{ 
	if((temp==0x00)&&(BBQ_Status_Struct.grill_set_act==1))
	{
	disbuff_TempP1[0]=0;
	disbuff_TempP1[1]=0;
	disbuff_TempP1[2]=0;
	}
	else if(Probe1_ERR)	//û�壬��ʾNOP
	{
		disbuff_TempP1[0]=DispNoprobeOFFTab[0]|0x08;
		disbuff_TempP1[1]=DispNoprobeOFFTab[1]&0xF7;
		disbuff_TempP1[2]=DispNoprobeOFFTab[2]&0xF7;
	} 
	else
	{	
			if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
		{
			if(temp<=32)temp=0;
			else temp = (temp-32)*5/9;
		};
	disbuff_TempP1[0]=(temp)%1000/100;//-temp%5
	disbuff_TempP1[1]=(temp)%100/10;//-temp%5
	disbuff_TempP1[2]=(temp)%10;//-temp%5
	if (temp!=0)
	{
		disbuff_TempP1[0]=DispNumTab[disbuff_TempP1[0]]|0x08;	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
		if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
				{
					disbuff_TempP1[1]=DispNumTab[disbuff_TempP1[1]]|0x08;
					disbuff_TempP1[2]=DispNumTab[disbuff_TempP1[2]]&0xf7;
				}
		else
				{
					disbuff_TempP1[1]=DispNumTab[disbuff_TempP1[1]]&0xf7;
					disbuff_TempP1[2]=DispNumTab[disbuff_TempP1[2]]|0x08;
				}
				
		}
	}
	Ht1621WrAllData(8,disbuff_TempP1,3);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  void LCD_Probe2_Temp(void)   
// ����: ��ʾprobe2�¶�
// ����: temp probe2�¶ȣ����999 
// ����: ��
//========================================================================
void LCD_Probe2_Temp(u16 temp) 
{ 
	if((temp==0x00)&&(BBQ_Status_Struct.grill_set_act==1))
	{
	disbuff_TempP2[0]=0;
	disbuff_TempP2[1]=0;
	disbuff_TempP2[2]=0;
	}
	else if(Probe2_ERR)	//û�壬��ʾNOP
	{
		disbuff_TempP2[0]=DispNoprobeOFFTab[0]|0x08;
		disbuff_TempP2[1]=DispNoprobeOFFTab[1]&0xF7;
		disbuff_TempP2[2]=DispNoprobeOFFTab[2]&0xF7;
	} 

	else
	{
		
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
		{
			if(temp<=32)temp=0;
			else temp = (temp-32)*5/9;
		};
	disbuff_TempP2[0]=(temp)%1000/100;//-temp%5
	disbuff_TempP2[1]=(temp)%100/10;//-temp%5
	disbuff_TempP2[2]=(temp)%10;//-temp%5
	if (temp!=0)
	{
		disbuff_TempP2[0]=DispNumTab[disbuff_TempP2[0]]|0x08;	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
			
		if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
				{
					disbuff_TempP2[1]=DispNumTab[disbuff_TempP2[1]]|0x08;
					disbuff_TempP2[2]=DispNumTab[disbuff_TempP2[2]]&0xf7;
				}
		else
				{
					disbuff_TempP2[1]=DispNumTab[disbuff_TempP2[1]]&0xf7;
					disbuff_TempP2[2]=DispNumTab[disbuff_TempP2[2]]|0x08;
				}
				
		}
	}
	Ht1621WrAllData(14,disbuff_TempP2,3);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  void LCD_ProbePC_Temp(void)   
// ����: ��ʾprobe2�¶�
// ����: temp probe2�¶ȣ����999 
// ����: ��
//========================================================================
void LCD_ProbePC_Temp(u16 temp) 
{ 
	if(ProbePC_ERR)	//û�壬��ʾNOP
	{
		disbuff_TempPC[0]=DispNoprobeOFFTab[0]|0x08;
		disbuff_TempPC[1]=DispNoprobeOFFTab[1]&0xF7;
		disbuff_TempPC[2]=DispNoprobeOFFTab[2]&0xF7;
	} 
	else
	{
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
		{
			if(temp<=32)temp=0;
			else temp = (temp-32)*5/9;
		};
	disbuff_TempPC[0]=(temp)%1000/100;//-temp%5
	disbuff_TempPC[1]=(temp)%100/10;//-temp%5
	disbuff_TempPC[2]=(temp)%10;//-temp%5

	disbuff_TempPC[0]=DispNumTab[disbuff_TempPC[0]]|0x08;	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
			{
				disbuff_TempPC[1]=DispNumTab[disbuff_TempPC[1]]|0x08;
				disbuff_TempPC[2]=DispNumTab[disbuff_TempPC[2]]&0xf7;
			}
	else
			{
				disbuff_TempPC[1]=DispNumTab[disbuff_TempPC[1]]&0xf7;
				disbuff_TempPC[2]=DispNumTab[disbuff_TempPC[2]]|0x08;
			}
			
	}
	Ht1621WrAllData(2,disbuff_TempPC,3);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  void LCD_All_On(void)    
// ����: ��������ʾ
// ����: ��
// ����: ��
//========================================================================
void LCD_All_On(void) 
{ 
	Ht1621WrAllData(0,LcdOnTab,16);	 //����ʾ
	LCD_BK_Light=LCD_BK_Light_ON;				 //������,�ȿ���ʾ�ٿ�����

} 
//========================================================================
// ����:  void LCD_All_Off(void)       
// ����: ��������ʾ
// ����: ��
// ����: ��
//========================================================================
void LCD_All_Off(void) 
{ 
	LCD_BK_Light=LCD_BK_Light_OFF;				 //������
	Ht1621WrAllData(0,LcdOffTab,16);	 //����ʾ
} 
//========================================================================
// ����:  void LCD_Dis_Er1(void)     
// ����: ȫ����ʾEr1
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Er1(void) 
{ 
	Ht1621WrAllData(0,DispEr1Tab,16);	 //����ʾ

	LCD_BK_Light=LCD_BK_Light_ON;				 //������
}
//========================================================================
// ����:  void LCD_Dis_Er2(void)     
// ����: ȫ����ʾErr
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Er2(void) 
{ 
	Ht1621WrAllData(0,DispEr2Tab,16);	 //����ʾ
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_Er3(void)     
// ����: ȫ����ʾErr
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Er3(void) 
{ 
	Ht1621WrAllData(0,DispEr3Tab,16);	 //����ʾ
	LCD_Action_sensor_Status_display(All_Action_sensor_Status_flags);
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_ErH(void)     
// ����: ȫ����ʾErr
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_ErH(void) 
{ 
	Ht1621WrAllData(20,DispErHTab,4);	 //����ʾ
		Lcd_A2829A3031[0]=0;
		Lcd_A2829A3031[1]=0;
	Ht1621WrAllData(28,Lcd_A2829A3031,2); //�ӵ�ַ14��ʼд��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_Err(void)     
// ����: ȫ����ʾErr
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Err(void) 
{ 
	Ht1621WrAllData(20,DispErrTab,4);	 //����ʾ
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_F(void)     
// ����: ��ʾF, in CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_F(void) 
{ 

	LCD_20_21_Data&=0xF7;//���probe C
	LCD_20_21_Data|=0x10;//��ʾprobe F


	Ht1621WrOneData(20,LCD_20_21_Data);
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_C(void)     
// ����: ��ʾc in CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_C(void) 
{ 
	LCD_20_21_Data&=0xEF;//���probeF
	LCD_20_21_Data|=0x08;//��ʾprobeC

	Ht1621WrOneData(20,LCD_20_21_Data);
	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 


//========================================================================
// ����:  void LCD_Dis_Set(void)     
// ����: ��ʾSET
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Set(void) 
{ 
				disbuff_TempIn[0]=disbuff_TempIn[0]|0x08;
				disbuff_TempIn[1]=disbuff_TempIn[1]|0x08;
				disbuff_TempIn[2]=disbuff_TempIn[2]&0xF7; 

	Ht1621WrAllData(20,disbuff_TempIn,3);	   //write the address from 20
	//Ht1621WrAllData(20,disbuff_TempIn,1);
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  void LCD_Dis_Set(void)     
// ����: ��ʾSET
// ����: ��
// ����: ��
//========================================================================
void LCD_UnDis_Set(void) 
{ 
				disbuff_TempIn[0]=disbuff_TempIn[0]|0x08;
				disbuff_TempIn[1]=disbuff_TempIn[1]&0xF7;
				disbuff_TempIn[2]=disbuff_TempIn[2]&0xF7; 

	Ht1621WrAllData(20,disbuff_TempIn,3);	   //write the address from 20
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Dis_Actual(void)     
// ����: ��ʾActual
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Actual(void) 
{ 
				disbuff_TempIn[0]=disbuff_TempIn[0]|0x08;
				disbuff_TempIn[1]=disbuff_TempIn[1]&0xF7;
				disbuff_TempIn[2]=disbuff_TempIn[2]|0x08; 

	Ht1621WrAllData(20,disbuff_TempIn,3);	   //write the address from 20
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_On_Init_Dis(void)     
// ����: ���¿���������ʼ��
// ����: ��
// ����: ��
//========================================================================
void LCD_On_Init_Dis(u16 in_temp_set,u16 probe_temp_set) 
{ 
	Ht1621WrAllData(0,LcdUseOnTab,16);	   //write the address from 20
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  void LCD_On_Init_Dis(void)     
// ����: ���¿���������ʼ��
// ����: ��
// ����: ��
//========================================================================
void LCD_On_NoPC_Init_Dis(u16 in_temp_set) 
{ 
	Ht1621WrAllData(0,LcdUseOnNoPCTab,16);	   //write the address from 20
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 

//========================================================================
// ����:  LCD_Dis_Time_Dot(void)     
// ����: ��ʾʱ�����������in CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_Time_Dot(void) 
{ 
	Ht1621WrOneData(29,0x80);
} 
//========================================================================
// ����:  LCD_Undis_Time_Dot(void)     
// ����: ����ʾʱ�����������in CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Undis_Time_Dot(void) 
{ 
	Ht1621WrOneData(29,0x00);
} 
//========================================================================
// ����:  LCD_Time_Hour(void)     
// ����: ʱ��Сʱin CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Time_Hour(u8 hour) 
{ 
	u8 disbuff[2];

	disbuff[0]=hour%100/10;
	disbuff[1]=hour%10;

	disbuff[0]=DispNumTab[disbuff[0]];	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
	disbuff[1]=DispNumTab[disbuff[1]];

	Ht1621WrAllData(12,disbuff,2);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  LCD_Time_Hour_Dis(void)     
// ����: �ر�ʱ��Сʱ��ʾin CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Time_Hour_Dis(void) 
{ 
	u8 disbuff[]={0x00,0x00};

	Ht1621WrAllData(12,disbuff,2);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  LCD_Time_Min(void)     
// ����: ʱ�����in CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Time_Min(u8 min) 
{ 
	u8 disbuff[2];

	disbuff[0]=min%100/10;
	disbuff[1]=min%10;

	disbuff[0]=DispNumTab[disbuff[0]];	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
	disbuff[1]=DispNumTab[disbuff[1]];

	Ht1621WrAllData(16,disbuff,2);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  LCD_Time_Min_Dis(void)     
// ����: �ر�ʱ��Сʱ��ʾin CVW no use Kendy 20180701
// ����: ��
// ����: ��
//========================================================================
void LCD_Time_Min_Dis(void) 
{ 
	u8 disbuff[]={0x00,0x00};

	Ht1621WrAllData(16,disbuff,2);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_All_Use(void)   
// ����: ����ʹ�õ���ȫ����ʾ
// ����: 
// ����: ��
//========================================================================
void LCD_All_Use(void) 
{ 

	Ht1621WrAllData(0,LcdUseOnTab,16);	   //write the address from 20
	LCD_BK_Light=LCD_BK_Light_ON;				 //������

}
//========================================================================
// ����:  void LCD_Probe1_Nodis(void)   
// ����: probe1�¶�û�У���ʾ---
// ����: 
// ����: ��
//========================================================================
void LCD_Probe1_Nodis(void) 
{ 
	Ht1621WrAllData(8,DispNoprobeOFFTab,3);	   //�ӵ�ַ2д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Probe1_Set_Temp(void)   
// ����: probe1�¶�û�У���ʾ--- in CVW no use Kendy 20180701
// ����: 
// ����: ��
//========================================================================
void LCD_ProbePC_Set_Temp(u16 temp) 
{ 
//	if(ProbePC_ERR)	//û�壬��ʾNOP
//	{
//		disbuff_TempPC[0]=DispNoprobeOFFTab[0]|0x08;
//		disbuff_TempPC[1]=DispNoprobeOFFTab[1]&0xF7;
//		disbuff_TempPC[2]=DispNoprobeOFFTab[2]&0xF7;
//	} 
//	else
	{
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
		{
			if(temp<=32)temp=0;
			else temp = (temp-32)*5/9;
		};
	disbuff_TempPC[0]=temp%1000/100;
	disbuff_TempPC[1]=temp%100/10;
	disbuff_TempPC[2]=temp%10;

	disbuff_TempPC[0]=DispNumTab[disbuff_TempPC[0]]|0x08;	 //���õ����ֶ�Ӧ��ʾ�Ĵ���
	if(BBQ_Status_Struct.fc==0x00)//һ������ʾF					  //0ΪF��1Ϊc
			{
				disbuff_TempPC[1]=DispNumTab[disbuff_TempPC[1]]|0x08;
				disbuff_TempPC[2]=DispNumTab[disbuff_TempPC[2]]&0xf7;
			}
	else
			{
				disbuff_TempPC[1]=DispNumTab[disbuff_TempPC[1]]&0xf7;
				disbuff_TempPC[2]=DispNumTab[disbuff_TempPC[2]]|0x08;
			}
			
	}
	Ht1621WrAllData(2,disbuff_TempPC,3);	   //�ӵ�ַ0д��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
//========================================================================
// ����:  void LCD_Probe_Set_Temp_Undis(void)   
// ����: �ر���ʾprone set�¶�in CVW no use Kendy 20180701
// ����: 
// ����: ��
//========================================================================
void LCD_Probe_Set_Temp_Undis(void) 
{ 
	u8 disbuff[]={0x00,0x00,0x00};

	Ht1621WrAllData(2,disbuff,3); //�ӵ�ַ14��ʼд��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 
//========================================================================
// ����:  void LCD_Wifi_dis(void)   
// ����: ��ʾwifi��־
// ����: 
// ����: ��
//========================================================================
void LCD_Wifi_Dis(u8 temp) 
{ 

		LCD_0_1_Data&=0x0f;
		LCD_0_1_Data+=temp;
		Ht1621WrOneData(0,LCD_0_1_Data);

	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 
//========================================================================
// ����:  void LCD_Wifi_Undis(void)   
// ����: ����ʾwifi��־
// ����: 
// ����: ��
//========================================================================
void LCD_Wifi_Undis(void) 
{ 
	LCD_0_1_Data&=0x0f;
	
	Ht1621WrOneData(0,LCD_0_1_Data);

	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 

//========================================================================
// ����:  void LCD_Wifi_dis(void)   
// ����: ��ʾwifi��־
// ����: 
// ����: ��
//========================================================================
void LCD_BlueTooth_Dis(void) 
{ 
	LCD_0_1_Data|=0x01;
	
	Ht1621WrOneData(0,LCD_0_1_Data);

	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 
//========================================================================
// ����:  void LCD_Wifi_Undis(void)   
// ����: ����ʾwifi��־
// ����: 
// ����: ��
//========================================================================
void LCD_BlueTooth_UnDis(void) 
{ 
	LCD_0_1_Data&=0xfe;
	
	Ht1621WrOneData(0,LCD_0_1_Data);

	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 

void LCD_Action_sensor_Status_display(u8 temp)
{
	u8 temp1=0;
	u8 temp2=0;
	temp1 = temp & 0x20;
	if (temp1==0x20)LCD_0_1_Data|=0x08;
	else LCD_0_1_Data&=0xf7;
	temp2 = temp & 0x80;
	if (temp2==0x80)
			{	
				temp|=0x20;
				temp&=0x7f;
			}
	else temp&=0xDF;//0xFB; 
	Ht1621WrOneData(0,LCD_0_1_Data);
	Ht1621WrOneData(30,temp);

	LCD_BK_Light=LCD_BK_Light_ON;				 //������
}

//========================================================================
// ����:  void LCD_Dis_APP_RTD_Override(u8 *dis)   
// ����: ��ʾ¯���¶�
// ����: temp¯���¶ȣ����999 ,dot 0xff����ʾ��0xf7�Ͳ���ʾ
// ����: ��
//========================================================================
void LCD_Dis_APP_RTD_Override(u8 *dis) 
{ 
	u8 disbuff[4];
	
  disbuff[3]=0;
	if(dis[0]<=9) disbuff[0]=DispNumTab[dis[0]]; //��ʾ��������
	else if(dis[0]>=0x41&&dis[0]<=0x5a) 
	{
		disbuff[0]=DispAZTab[dis[0]-65]; //��ʾ������ĸ
	}
	else disbuff[0]=0;//��������Ҳ������ĸ��ʲô������ʾ

	if(dis[1]<=9) disbuff[1]=DispNumTab[dis[1]]; //��ʾ��������
	else if(dis[1]>=0x41&&dis[1]<=0x5a) 
	{
		disbuff[1]=DispAZTab[dis[1]-65]; //��ʾ������ĸ
	}
	else disbuff[1]=0;//��������Ҳ������ĸ��ʲô������ʾ

	if(dis[2]<=9) disbuff[2]=DispNumTab[dis[2]]; //��ʾ��������
	else if(dis[2]>=0x41&&dis[2]<=0x5a) 
	{
		disbuff[2]=DispAZTab[dis[2]-65]; //��ʾ������ĸ
	}
	else disbuff[2]=0;//��������Ҳ������ĸ��ʲô������ʾ

	Ht1621WrAllData(20,disbuff,4); //�ӵ�ַ20��ʼд��
	LCD_BK_Light=LCD_BK_Light_ON;				 //������

} 

//========================================================================
// ����:  void LCD_Dis_App(void)     
// ����: ȫ����ʾErr
// ����: ��
// ����: ��
//========================================================================
void LCD_Dis_App(void) 
{ 
	Ht1621WrAllData(20,DispAppTab,4);	 //����ʾ
	LCD_Action_sensor_Status_display(All_Action_sensor_Status_flags);
	LCD_BK_Light=LCD_BK_Light_ON;				 //������
} 
