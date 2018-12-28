#include "key_temp.h"
#include "key.h"
/*扫描时间为10ms*/
// 按下多长时间(PRESS_TIME * 扫描时间)记为连按
#define PRESS_TIME 100u		//u代表无符号

// 连按后每隔多长时间(EXECUTE_TIME * 扫描时间)执行一次操作
#define EXECUTE_TIME 50u

// 使用单击与双击与连击(1u)还是短击与双击与长击(0u)
#define USE_SINGLE_AND_CONTINUOUS 0u

// 按下多长时间(LONG_PRESS_FLAG * 扫描时间)记为长按
#define LONG_PRESS_FLAG 80u

// 双击判定间隔时间
#define SPACE_TIME 25u

// 如果为上拉输入(1u),则按下为低电平,下拉输入(0u),则按下为高电平,为便于修改将状态取反
#define GPIO_Mode_IPU 1u
#if GPIO_Mode_IPU
  #define SET RESET
#else
  #define SET SET
#endif

#if USE_SINGLE_AND_CONTINUOUS
u8 Key_Temp_SinglePressOK = 0,Key_Temp_ContinuousPressOK = 0;
#else
u8 Key_Temp_ShortPressOK = 0,Key_Temp_LongPressOK = 0;
#endif
u8 Key_Temp_DoublePressOK = 0;

static u8 key_Status;//  按键的状态
static u8 pressTimeCount = 0;//  按下时间计数
static u8 doublePressTimeCount = 0,// 两次按下间隔时间计数
          DoublePressStatus = 0,// 按键双击判定变量
          pressTimes = 0;          // 按下次数

enum _KEY_STATUS{
                waitForPress = 0,		   // 0
                keyVerifyOne,			   // 1
                keyVerifyTwo,			   // 2
                waitForRelease,			   // 3
                keyReleasing,			   // 4
               };

#if USE_SINGLE_AND_CONTINUOUS  
  static u8 continuous_PressTimeout = 0;//  是否到达连按时间标志
    // 按键核验后处理
  void BSP_Key_Temp_Single_Continuous(void)
  {
    KeyScan();
  }
#else
  void BSP_Key_Temp_Short_Long(void)
  {
    KeyScan();
  }
#endif

typedef u8 (* procedure)(void);		  //定义一个指针，返回类型是uchar，所指向函数的参数为void

/*****状态1 第一个10ms检测到按下******/
static u8 Step_WaitForPress(void)
{
  if(SET == key_Status)
  {
    return keyVerifyOne;
  }
  return waitForPress;
}
/*****状态2 第二个10ms检测到按下******/
static u8 Step_KeyVerifyOne(void)
{
  if(SET == key_Status)
  {
    return keyVerifyTwo;
  }
  return waitForPress;
}

/*****状态3 第三个10ms检测到按下及状态4 第三个10ms检测放开******/
#if USE_SINGLE_AND_CONTINUOUS
  static u8 Step_KeyVerifyTwo(void)
  {
    if(SET == key_Status)
    {
      // 单击标志置为有效
      Key_Temp_SinglePressOK = 1;
      
      // 点击次数(如果短按标志有效说明按下了一次,置位双击判定变量,并且记录按下的次数)
      pressTimes++;
      DoublePressStatus = 1;
      // 如果按下了两次
      if(pressTimes == 2)
      {
        // 且时间未超过 SPACE_TIME,则说明双击有效
        if(doublePressTimeCount < SPACE_TIME)
        {
          pressTimes = 0;
          doublePressTimeCount = 0;
          DoublePressStatus = 0;
          
          // 将双击有效状态置为有效
          Key_Temp_DoublePressOK = 1;
          return keyReleasing;
        }
      }
      
      return waitForRelease;
    }
    return waitForPress;
  }
  
  static u8 Step_WaitForRelease(void)
  {
    if(SET == key_Status)
    {
      pressTimeCount++;
      if(0 == continuous_PressTimeout)
      {
        if(PRESS_TIME == pressTimeCount)
        {
          pressTimeCount = 0;
          continuous_PressTimeout = 1;
        }
        return waitForRelease;
      }
      else
      {
        if(EXECUTE_TIME == pressTimeCount)
        {
          pressTimeCount = 0;
          // 连击标志置为有效
          Key_Temp_ContinuousPressOK = 1;
          
        }
        return waitForRelease;
      }
    }
    return keyReleasing;
  }
  
#else
  static u8 Step_KeyVerifyTwo(void)
  {
    if(SET == key_Status)
    {
      return waitForRelease;
    }
    return waitForPress;
  }
  
  static u8 Step_WaitForRelease(void)
  {
    if(SET == key_Status)
    {
      pressTimeCount++;
      
      // 如果需要长按在松开后才执行,则将下面屏蔽的代码打开,然后屏蔽掉后面带 /**/ 符号的
      
      if(pressTimeCount >= LONG_PRESS_FLAG)   /**/
      {                                       /**/
        // 长按标志置为有效                    /**/
        Key_Temp_LongPressOK = 1;                   /**/
                                              /**/
        return keyReleasing;                  /**/
      }                                       /**/
      
      return waitForRelease;
    }
    else
    {
//      if(pressTimeCount >= LONG_PRESS_FLAG)
//      {
//        // 长按标志置为有效
//        Key_Temp_LongPressOK = 1;
//        
//        return keyReleasing;
//      }
//      else
      if(pressTimeCount < LONG_PRESS_FLAG)     /**/
      {
        // 短按标志置为有效
        Key_Temp_ShortPressOK = 1;
        
        // 点击次数(如果短按标志有效说明按下了一次,置位双击判定变量,并且记录按下的次数)
        pressTimes++;
        DoublePressStatus = 1;
        // 如果按下了两次
        if(pressTimes == 2)
        {
          // 且时间未超过 SPACE_TIME,则说明双击有效
          if(doublePressTimeCount < SPACE_TIME)
          {
            pressTimes = 0;
            doublePressTimeCount = 0;
            DoublePressStatus = 0;
            
            // 将双击有效状态置为有效
            Key_Temp_DoublePressOK = 1;
            
            return keyReleasing;
          }
        }
        return keyReleasing;                 
      }
      return keyReleasing;                    /**/
    }
  }
  
#endif

/****状态5 释放*******/
static u8 Step_KeyReleasing(void)
{
  if(SET == key_Status)
  {
    pressTimeCount++;
    return waitForRelease;
  }
  else
  {
	pressTimeCount = 0;
    #if USE_SINGLE_AND_CONTINUOUS
      continuous_PressTimeout = 0;
    #endif
  }
  return waitForPress;
}
/***定义步骤****/
static procedure step[] = {Step_WaitForPress, Step_KeyVerifyOne, Step_KeyVerifyTwo, Step_WaitForRelease, Step_KeyReleasing,};
/***扫描函数***/
static void KeyScan(void)
{
  static u8 nextStatus = waitForPress;// 初始化状态为等待按键按下
  // 如果双击判定变量有效,则进入判定
  if(1 == DoublePressStatus)
  {
    // 如果双击状态无效
    if(1 != Key_Temp_DoublePressOK)
    {
      // 双击按下间隔时间计时
      doublePressTimeCount++;
      
      // 如果按下一次后,时间超时,则说明未进行双击操作,清空变量及标志位
      if(doublePressTimeCount > SPACE_TIME)
      {
        pressTimes = 0;
        doublePressTimeCount = 0;
        DoublePressStatus = 0;
      }
    }
  }
  key_Status = KEY_Temp;// 获取按键的状态
  nextStatus = step[nextStatus]();
}
