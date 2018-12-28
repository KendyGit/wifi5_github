#include "key_temp.h"
#include "key.h"
/*ɨ��ʱ��Ϊ10ms*/
// ���¶೤ʱ��(PRESS_TIME * ɨ��ʱ��)��Ϊ����
#define PRESS_TIME 100u		//u�����޷���

// ������ÿ���೤ʱ��(EXECUTE_TIME * ɨ��ʱ��)ִ��һ�β���
#define EXECUTE_TIME 50u

// ʹ�õ�����˫��������(1u)���Ƕ̻���˫���볤��(0u)
#define USE_SINGLE_AND_CONTINUOUS 0u

// ���¶೤ʱ��(LONG_PRESS_FLAG * ɨ��ʱ��)��Ϊ����
#define LONG_PRESS_FLAG 80u

// ˫���ж����ʱ��
#define SPACE_TIME 25u

// ���Ϊ��������(1u),����Ϊ�͵�ƽ,��������(0u),����Ϊ�ߵ�ƽ,Ϊ�����޸Ľ�״̬ȡ��
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

static u8 key_Status;//  ������״̬
static u8 pressTimeCount = 0;//  ����ʱ�����
static u8 doublePressTimeCount = 0,// ���ΰ��¼��ʱ�����
          DoublePressStatus = 0,// ����˫���ж�����
          pressTimes = 0;          // ���´���

enum _KEY_STATUS{
                waitForPress = 0,		   // 0
                keyVerifyOne,			   // 1
                keyVerifyTwo,			   // 2
                waitForRelease,			   // 3
                keyReleasing,			   // 4
               };

#if USE_SINGLE_AND_CONTINUOUS  
  static u8 continuous_PressTimeout = 0;//  �Ƿ񵽴�����ʱ���־
    // �����������
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

typedef u8 (* procedure)(void);		  //����һ��ָ�룬����������uchar����ָ�����Ĳ���Ϊvoid

/*****״̬1 ��һ��10ms��⵽����******/
static u8 Step_WaitForPress(void)
{
  if(SET == key_Status)
  {
    return keyVerifyOne;
  }
  return waitForPress;
}
/*****״̬2 �ڶ���10ms��⵽����******/
static u8 Step_KeyVerifyOne(void)
{
  if(SET == key_Status)
  {
    return keyVerifyTwo;
  }
  return waitForPress;
}

/*****״̬3 ������10ms��⵽���¼�״̬4 ������10ms���ſ�******/
#if USE_SINGLE_AND_CONTINUOUS
  static u8 Step_KeyVerifyTwo(void)
  {
    if(SET == key_Status)
    {
      // ������־��Ϊ��Ч
      Key_Temp_SinglePressOK = 1;
      
      // �������(����̰���־��Ч˵��������һ��,��λ˫���ж�����,���Ҽ�¼���µĴ���)
      pressTimes++;
      DoublePressStatus = 1;
      // �������������
      if(pressTimes == 2)
      {
        // ��ʱ��δ���� SPACE_TIME,��˵��˫����Ч
        if(doublePressTimeCount < SPACE_TIME)
        {
          pressTimes = 0;
          doublePressTimeCount = 0;
          DoublePressStatus = 0;
          
          // ��˫����Ч״̬��Ϊ��Ч
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
          // ������־��Ϊ��Ч
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
      
      // �����Ҫ�������ɿ����ִ��,���������εĴ����,Ȼ�����ε������ /**/ ���ŵ�
      
      if(pressTimeCount >= LONG_PRESS_FLAG)   /**/
      {                                       /**/
        // ������־��Ϊ��Ч                    /**/
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
//        // ������־��Ϊ��Ч
//        Key_Temp_LongPressOK = 1;
//        
//        return keyReleasing;
//      }
//      else
      if(pressTimeCount < LONG_PRESS_FLAG)     /**/
      {
        // �̰���־��Ϊ��Ч
        Key_Temp_ShortPressOK = 1;
        
        // �������(����̰���־��Ч˵��������һ��,��λ˫���ж�����,���Ҽ�¼���µĴ���)
        pressTimes++;
        DoublePressStatus = 1;
        // �������������
        if(pressTimes == 2)
        {
          // ��ʱ��δ���� SPACE_TIME,��˵��˫����Ч
          if(doublePressTimeCount < SPACE_TIME)
          {
            pressTimes = 0;
            doublePressTimeCount = 0;
            DoublePressStatus = 0;
            
            // ��˫����Ч״̬��Ϊ��Ч
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

/****״̬5 �ͷ�*******/
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
/***���岽��****/
static procedure step[] = {Step_WaitForPress, Step_KeyVerifyOne, Step_KeyVerifyTwo, Step_WaitForRelease, Step_KeyReleasing,};
/***ɨ�躯��***/
static void KeyScan(void)
{
  static u8 nextStatus = waitForPress;// ��ʼ��״̬Ϊ�ȴ���������
  // ���˫���ж�������Ч,������ж�
  if(1 == DoublePressStatus)
  {
    // ���˫��״̬��Ч
    if(1 != Key_Temp_DoublePressOK)
    {
      // ˫�����¼��ʱ���ʱ
      doublePressTimeCount++;
      
      // �������һ�κ�,ʱ�䳬ʱ,��˵��δ����˫������,��ձ�������־λ
      if(doublePressTimeCount > SPACE_TIME)
      {
        pressTimes = 0;
        doublePressTimeCount = 0;
        DoublePressStatus = 0;
      }
    }
  }
  key_Status = KEY_Temp;// ��ȡ������״̬
  nextStatus = step[nextStatus]();
}
