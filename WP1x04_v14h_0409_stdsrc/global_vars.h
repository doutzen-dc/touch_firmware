#ifndef GLOBAL_VARSH
#define GLOBAL_VARSH
#include "user_protocol.h"

#define MAX_H_CH_ToByte 4
#define MAX_RX_LENGTH 24    //130722 Andy add. Cannot changed in Library mode.
#define REDUCE_XRAM     1 //0: 4x MRAW. 1:2x MRAW. //130722 Andy add. Cannot changed in Library mode.
#define RAW_FILTER        0 // 1    // default:0

#define MAX_MUTUAL_NODE  336 // (TX_LENGTH*RX_LENGTH)//336

#if  DETECTING_KEY_CHANGE
  extern U8 KeyState; //For every 2 bits, b0=1: Down, b1=1:Up, otherwise no changing.
                      //KEY0=b0~b1, KEY1=b2~b3, KEY2=b4~b5, KEY3=b6~b7.
  U8 DetectKeyChanging(U8 key_mask);  // Return 0 if key not changing, non-0 if any Key changing.
  /*
    After calling DetectKeyChanging(..), The programmer can read KeyState to get the UP/DOWN status.
  */
#endif

#if  DETECTING_POINT_CHANGE
  typedef union
  {
    struct
    {
      U8 Exist : 1 ;  // Become 1 during the touching.
      U8 Down  : 1 ;  // Become 1 only at the moment of first down. Clear to 0 at the other time.        
      U8 Up    : 1 ;  // Become 1 only at the moment of lift up, Clear to 0 at the other time.
      U8 Move  : 1 ;  // Become 1 while the x/y position changing. otherwise 0.
      U8 reserved:4 ;
    } Bit; 
    U8 Flags;
  } tPointState;  
  extern tPointState PointState[]; //Ordered by point ID.
  U8 DetectPointChanging(void); // Return 0 if no any point changing, non-0 if any point changing.
  /*
    After calling DetectPointChanging(), The programmer can read PointState[] to get the UP/DOWN/MOVE/EXIST status for every ID.
  */
#endif


#if REDUCE_XRAM
  #if RAW_FILTER
  #define Raw_buf_len 4 // need  +2
  #else
  #define Raw_buf_len 3 // need  +1
  #endif
#else
  #define Raw_buf_len 5 // need  +1   //20140104
#endif

extern U8 I2C_Send_Enable;
extern U8 xdata report_Buffer[];
extern tFingerIDList FingerIDList_Output;
extern bit notLoad_rcv_ptr;
extern U8 CtrlByte[];


#define SystemTimer1Start() {TR1=1;}    //For user use only.
#define SystemTimer1Stop()  {TR1=0;}      //For user use only.
extern void SystemTimer1Reload(void);
extern void SystemTimer1Init(U16 period);   // period: 1 unit = 1us.


#define SetI2CReadTo(xptr) { DMA_ADR_LO =  lobyte(xptr); DMA_ADR_HI =  hibyte(xptr); }
#define RequestINTOLow()  { I2C_Send_Enable = 1; }
#define SetINTOHigh()  { INTO=1; }

extern void Sleep(void);
extern void WakeUp(void);

#endif
