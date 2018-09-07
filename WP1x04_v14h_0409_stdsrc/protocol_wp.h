#ifndef _PROTOVOL_H_
#define _PROTOVOL_H_


//#include "bios.h"
#include "tProtocolDefine.h"
#include "tp_lib.h"
#include "user_protocol.h"
//#define JUST_for_MobilePhone_DEBUG
//Mobile for Wp1004 Protocol  

//#define TP_CUSTOMER_PROTOCOL //JS20130616a XX : For library-ize :: move to flash_para.c
#define SZ_Floating_DEBUGProtocol   0



/*
#define SUO
#ifndef SUO
    #ifndef WP1004
      #define WP1004
    #endif
#endif
*/
enum
{
  // ---Customer
  Winpower = 0,
  Wintek  = 1,    
  SUO = 2,  
  // ---Project_ID
  Inch_4_7 = 1,
};

extern U8 APActiveCC;
extern U8 GETCCmode;
//#define Customer_default Winpower
//JS20130616a move to flash_para.c
//#ifdef TP_CUSTOMER_PROTOCOL
//  #define Customer_default SUO
//  #define Customer_byte SUO
//#else
//  #define Customer_default Winpower
//  #define Customer_byte   Winpower
//#endif
//#define Project_ID_default Inch_4_7

typedef volatile struct
{
  U8 key4f_fnum4;
  //tFingerReportExt finger[MAX_FINGERS];
  tFingerReportExt finger[FW_SUPPORT_MAX_FINGERS]; //JS20130616a : For library-ize
} tTpReportsExt;


extern tProtocolHeader xdata tp_header;
extern U8 xdata report_Buffer[];
extern U8 report_BufPtr;
#define MODE_ENABLE_MUTUAL_RAW 0x01
extern S16 xdata report_DebugInfo_1[];    // add 20130527 *p
extern S16 xdata report_DebugInfo_2[];    // add 20130527 *p

extern U16 Timer_Counter;

#ifdef JUST_for_MobilePhone_DEBUG
extern S16 Debug_MutuDeltaMax;
extern S16 Debug_SelfDeltaMax;
#endif
void ProtocolInit(void);
void report_SendDebug(U8 , U8 , U16);
void report_SendPoint();//tFingerIDList
#if DebugForAPKTest
  void report_DebugInfo_UserDefine1(U8 , S16);    // add 20130527 for user define function call *p
  void report_DebugInfo_UserDefine2(U8 , S16);    // add 20130527 for user define function call *p
  void report_DebugInfo_FillAPKMessage(void);
  void report_DebugInfo_UserDefine_Clear(void);         // clear user debug buffer
#endif
void report_TxOut(void);
void report_SendKey(U8 );
void report_Reset(void);

void SUO_IdlePacket(void);
void report_SendPoint_SUO();
void report_SendKey_SUO(U8 key_mask);
void report_SendPoint_Sitronix();
void report_SendKey_Sitronix(U8 key_mask);


#endif
extern U8 I2C_Send_Enable;
#ifdef SUO_LIB  
typedef volatile struct
{
  U8 finger_number; 
  U8 touch_key;
  U8 touch_id[MAX_FINGERS];
  S16 Pos[2][MAX_FINGERS];
  
}tSUO_Report_Send;
#endif