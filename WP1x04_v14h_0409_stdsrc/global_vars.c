
#include "user_protocol.h"
#include "global_vars.h"


#if 1 
  // This claim will make non-portable.
  volatile S16 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ;//_at_ Start_MuTargetAddr;
#else
  // This claim can be portable.
  S16 xdata MutuRawBuf0[MAX_MUTUAL_NODE] ;
  S16 xdata MutuRawBuf1[MAX_MUTUAL_NODE] ;
  S16 xdata MutuRawBuf2[MAX_MUTUAL_NODE] ;
  #if REDUCE_XRAM
    S16 xdata *MutuTargetAddr[Raw_buf_len]=
    {
      MutuRawBuf0,
      MutuRawBuf1,
      MutuRawBuf2 
    };
  #else
    S16 xdata MutuRawBuf3[MAX_MUTUAL_NODE] ;
    S16 xdata MutuRawBuf4[MAX_MUTUAL_NODE] ;
    S16 xdata *MutuTargetAddr[Raw_buf_len]=
    {
      MutuRawBuf0,
      MutuRawBuf1,
      MutuRawBuf2,
      MutuRawBuf3,
      MutuRawBuf4 
    };
  #endif
#endif

#define RPT_BUF_SIZE 58
U8 xdata report_Buffer[RPT_BUF_SIZE];

#define I2C_RCV_BUF_SIZE 32
U8 xdata i2c_RcvBuf[I2C_RCV_BUF_SIZE];
const U8 I2C_BufferSize = I2C_RCV_BUF_SIZE;

  
S16 LastQuaDelta[MAX_MUTUAL_NODE];
S16 LastQuaDelta_1[MAX_MUTUAL_NODE];

volatile S16 xdata NoiseTargetAddr[RX_LENGTH*2] ;//_at_ Start_MuTargetAddr;


S16 MutualBaseLine[3][MAX_MUTUAL_NODE];
U16 MBaseLine[3][MAX_MUTUAL_NODE]; // [twokey]

U8 AreaChkFlag[TX_LENGTH][MAX_RX_LENGTH]; //JS20130616a

U8 CtrlByte[4];

U8 charger_on_status = 0;//JS20130522a //JS20130619c : For FAE

//JS20130619d : Start : Reserve for FAE usage function call
bit userFunctionCallBit0 = 0; //userKeyCheck() to replace keyCheck() if set 1
bit userFunctionCallBit1 = 0; //null
bit userFunctionCallBit2 = 0; //null
bit userFunctionCallBit3 = 0; //null
bit userFunctionCallBit4 = 0; //null
bit userFunctionCallBit5 = 0; //null
bit userFunctionCallBit6 = 0; //null
bit userFunctionCallBit7 = 0; //null


#if  DETECTING_KEY_CHANGE
  U8 KeyState;  //For every 2 bits, b0=1: Down, b1=1:Up, otherwise no changing.
          //KEY0=b0~b1, KEY1=b2~b3, KEY2=b4~b5, KEY3=b6~b7.
  U8 DetectKeyChanging(U8 key_mask);  // Return 0 if key not changing, non-0 if any Key changing.
  /*
    After calling DetectKeyChanging(..), The programmer can read KeyState to get the UP/DOWN status.
  */
#endif

#if  DETECTING_POINT_CHANGE
  tPointState PointState[5]; //Ordered by point ID.
  U8 DetectPointChanging(void); // Return 0 if no any point changing, non-0 if any point changing.
  /*
    After calling DetectPointChanging(), The programmer can read PointState[] to get the UP/DOWN/MOVE/EXIST status for every ID.
  */
#endif



               

#if DETECTING_KEY_CHANGE
U8 DetectKeyChanging(U8 key_mask) // Return 0 if key not changing, non-0 if any Key changing.
{
  static U8 LastKeyMask=0;  
  U8 x;             
  U8 b_mask=0x01;
  U8 mask_key_state=0x01; // First checking key_down !.
  KeyState=0;
  for(x=0; x<KEY_NUMBER ; x++)
  {
    if(key_mask & b_mask) //The key is pressing now.
    {                     // Now the key is pressing, Check key down.
      if(!(LastKeyMask & b_mask))
      {                   //  Last keybit is empty, so report DOWN because it is the first down.
        KeyState |= mask_key_state; //The mask is key_down now.
      }
      mask_key_state<<=2; // Shifted for next key.
    }
    else
    {                     // Now the key is empty.
      mask_key_state<<=1; //Change mask to key_up.
      if(LastKeyMask & b_mask)
      {                   // Last keybit was pressing, so it's leaving, report a UP event.
        KeyState |= mask_key_state;
      }
      mask_key_state<<=1; //Change mask to key_down for next key.
    }
    b_mask <<= 1;         //Shift to detect next key status.
  }    
  LastKeyMask = key_mask; //Record current key status for next comparison.
  return(KeyState);
}
#endif

#if  DETECTING_POINT_CHANGE

U8 u8LastXL[5];
U8 u8LastYL[5];
U8 u8LastFingerStatus=0;
extern tFingerIDList FingerIDList_Output;

U8 DetectPointChanging(void)  // Return 0 if no any point changing, non-0 if any point changing.
{
  U8 x, b_mask=0x01;
  U8 xpos, ypos;  // The low 8 bits of the coordinates.
  U8 state_changing=0;

  for(x=0; x < 5; x++)    //Check every point ID to see if it was occupied.
  {
    PointState[x].Flags = 0;  // Clear the state bits.
    if(FingerIDList_Output.Touch_Info[x])
    { // Touching for this finger ID.
      PointState[x].Bit.Exist=1;
        //...Calculate XY address
      xpos=FingerIDList_Output.Pos[1][x];
      ypos=FingerIDList_Output.Pos[0][x];
        //... Check if new finger.
      if(!(u8LastFingerStatus & b_mask))  // Is this ID existed at previos frame?
      { // No, so it's new finger, report an point DOWN.
        PointState[x].Bit.Down=1;
        state_changing=1;
      }
      else
      {   // It's old finger, then check if moving.
        if((u8LastXL[x]!=xpos) || (u8LastYL[x]!=ypos))  // Moving ?
        {
          PointState[x].Bit.Move = 1;
          state_changing=1;
        }
      }
      u8LastXL[x] = xpos;
      u8LastYL[x] = ypos;
      u8LastFingerStatus |= b_mask; // Remember touching
    }
    else
    { // Not touching for this finger ID.
      if(u8LastFingerStatus & b_mask)
      { // was pressing last time, so report a UP event
        PointState[x].Bit.Up = 1;
        state_changing=1;
      }
      u8LastFingerStatus &= ~b_mask;  // Remember empty.
    }
    b_mask <<=1;
  }
  return(state_changing);
}
#endif

