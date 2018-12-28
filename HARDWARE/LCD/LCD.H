/***************************用于定义各种接口******************************/
#ifndef		__lcd_H__
#define		__lcd_H__

#include "sys.h"
#include "delay.h"
#include "dma.h"	

#define  LCD_BIAS     0x52             //0b1000 0101 0010  1/3duty 4com 
#define  LCD_SYSDIS   0X00             //0b1000 0000 0000  关振系统荡器和LCD偏压发生器 
#define  LCD_SYSEN    0X02             //0b1000 0000 0010 打开系统振荡器 
#define  LCD_OFF   0X04             //0b1000 0000 0100  关LCD偏压 
#define  LCD_ON    0X06             //0b1000 0000 0110  打开LCD偏压 
#define  LCD_XTAL     0x28             //0b1000 0010 1000 外部接时钟 
#define  LCD_RC256    0X30             //0b1000 0011 0000  内部时钟 
#define  LCD_WDTDIS1  0X0A            //0b1000 0000 1010  禁止看门狗

#define LCD_BK_Light PBout(4)	//背光灯	
#define LCD_DA PBout(13)		
#define LCD_WR PBout(14)		
#define LCD_RD PAout(15)		
#define LCD_CS PBout(3)	

#define LCD_BK_Light_ON 0	//背光灯开背光	
#define LCD_BK_Light_OFF 1	//背光灯关背光	


//void Ht1621_VCC_0();  //VCC_power
//void Ht1621_VCC_1();
void Ht1621_CS_0(void);// CS
void Ht1621_CS_1(void);
void Ht1621_WR_0(void); // WR
void Ht1621_WR_1(void);
void Ht1621_DO_0(void);// DAT
void Ht1621_DO_1(void);
void Ht1621Wr_Data(u8 Data,u8 cnt); 
void Ht1621WrCmd(u8 Cmd);
void Ht1621WrAllData(u8 Addr,u8 *p,u8 cnt);
void Ht1621WrOneData(u8 Addr,u8 data);
void Ht1621WrHalfData(u8 Addr,u8 data);


void Ht1621_Init(void);// 在主函数中初始化
//void LCD_In_Temp(u16 temp);//显示炉内温度
void LCD_In_Temp(u16 temp, u8 All_Action_sensor_Status_display);
void LCD_In_Temp_Undis(void);//关闭炉内温度显示 

void LCD_Probe1_Temp(u16 temp);//显示PROBE1温度 
void LCD_Probe2_Temp(u16 temp);//显示PROBE2温度  
void LCD_ProbePC_Temp(u16 temp); // Display Probe PC temperature 20180618 Kendy
void LCD_All_On(void); 
void LCD_All_Off(void); 
void LCD_Dis_Err(void);
void LCD_Dis_Er1(void); 
void LCD_Dis_Er2(void); 
void LCD_Dis_Er3(void); 
void LCD_Dis_ErH(void); 
void LCD_Dis_F(void); 
void LCD_Dis_C(void); 
void LCD_Dis_Set(void); 
void LCD_UnDis_Set(void); 
void LCD_Dis_Actual(void); 
void LCD_On_Init_Dis(u16 in_temp_set,u16 probe_temp_set); 
void LCD_On_NoPC_Init_Dis(u16 in_temp_set);
void LCD_Dis_Time_Dot(void); 
void LCD_Undis_Time_Dot(void); 
void LCD_Time_Hour(u8 hour); 
void LCD_Time_Hour_Dis(void); 
void LCD_Time_Min(u8 min);
void LCD_Time_Min_Dis(void); 
void LCD_All_Use(void);
void LCD_Probe1_Nodis(void); 
void LCD_ProbePC_Set_Temp(u16 probe_set_temp); 
void LCD_Probe_Set_Temp_Undis(void); 
void LCD_Wifi_Dis(u8 temp);
void LCD_Wifi_Undis(void); 
void LCD_BlueTooth_Dis(void); 
void LCD_BlueTooth_Undis(void); 
void LCD_Action_sensor_Status_display(u8 temp);
void LCD_Dis_APP_RTD_Override(u8 *dis); 
void LCD_Dis_App(void);

//extern u8 LCD_26_27_Data;
//extern u8 LCD_28_Data; 
extern u8 Probe1_ERR; //probe1没有接
extern u8 Probe2_ERR; //probe2没有接
extern u8 ProbePC_ERR; //probePC没有接
extern u8 ProbeTempIn_ERR;

#endif
