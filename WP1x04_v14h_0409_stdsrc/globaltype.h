#ifndef _GLOBALTYPE_H_
#define _GLOBALTYPE_H_

#include "it51.h" //JS20130619b

#define _8051_    // Enable this only for 8051.

#if defined(_8051_)
  #define xdata xdata
  typedef unsigned char U8;
  typedef unsigned int U16;
  typedef unsigned long U32;
  typedef char S8;
  typedef int S16;
  typedef bit FLAG;
  #define lobyte(AA)  ((unsigned char) (AA))
  #define hibyte(AA)  ((unsigned char) (((unsigned int) (AA)) >> 8))
#elif defined(_X86_)
  typedef unsigned char U8;
  typedef unsigned short U16;
  typedef unsigned int U32;
  typedef char S8;
  typedef short S16;
  typedef unsigned char FLAG;
  #define xdata
#endif

//###################################
#define TP_CUSTOMER_PROTOCOL 1
//#define flashpara
//###################################


sbit INTO = P0^0; //JS20130619b : For library-ize
#define FW_SUPPORT_MAX_FINGERS 5       //JS20130616a : for library-ize //JS20130619a

#define SELF_KEY    0x81                                         //JS20130616a : Don't change the define
#define MUTU_DOWNKEY  0x82 //Mutu_key is at TP "Bottom" side.  //JS20130616a : Don't change the define
#define MUTU_RIGHTKEY   0x83 //Mutu_key is at TP "Right" side.  //JS20130616a : Don't change the define
#define SELF_SHARE_KEY  0x84 //Mutu_key is at TP "Right" side.  //JS20130616a : Don't change the define
//---- new add ----
typedef volatile struct       // 0x7C00
{
  U8 FW_Parameter_Status;   //0xAA = Enable FW Parameter function, otherwise = Disable
  U8 Customer_ID;     //0
  U8 Project_ID;      //0
  U8 Version_Main;    //0
  U8 Version_Sub;     //0
  U16 Parameter_Checksum;
  U16 FW_Checksum;
  U16 Co_FWAP_Ptl;      //0
  U16 FW_ReleaseVER;      //0
  U16 u16Rev1;
  U16 u16Rev2;
  U8 u8Rev1;        //0
  U8 u8Rev2;      //0
}tFlash;  
typedef volatile struct
{ 
  U16 SelfPulseCount;
  U16 SelfPulseLen;
  U16 MutualPulseCount; 
  U16 MutualPulseLen; 
  U16 Self_first_TH; 
  U8 Self_fb;
  U8 Mutu_fb;
  U16 Self_TH;
  U16 Mutu_TH;    
  U16 Mutu_first_TH ;
  U16 MutuCC_MinRaw ;
  U8 Mutu_offsetCC ;
  U8 CC_byRx[32] ;  
}tRawcountPara;

typedef volatile struct{
  U8 type; // 0x81:self , 0x82 : MUTU_DOWNKEY ,0x83: MUTU_RIGHTKEY ,  0 : none
  U8 num; 
  U16 CHS[8]; // Tx
  S16 KeyThreshold;
  U8 keydebounce;
  U8 KeyGain[8];   //JS20130619e : Add for key handler
  U8 KeyIIRRate;   //JS20130619e : Add for key handler
  S16 KeyCeiling;  //JS20130619e : Add for key handler
  S16 KeyBaseTH1;  //JS20130619e : Add for key handler
  S16 KeyBaseTH2;  //JS20130619e : Add for key handler
}tKeyPara;

typedef volatile struct
{ 
  U8 SUPPORT_FINGERS;
  U8 Y_TX;
  U8 X_RX;
  U8 scale; //64 .48  
  U8 HW_VER;
  U8 FW_VER;
  U32 SERIAL_NO;
  U8 u8Rev1;
  U8 u8Rev2;
  U16 V_ID;
  U16 P_ID; 
}tPannel;

typedef volatile struct
{
  U8 Clock_rate;  
  U8 I2C_DevAddr;
  U16 Timer_period_Normal;
  U16 Timer_period_Idle;
  U8 ReportPoint_para;  // bit0 : Y inverse enable , bit1 : X inverse enable , bit2 : X Y change
  U16 Tapping_distance; 
}tSystemPara; 

typedef volatile struct
{
  U8 Customer;
  U8 Project_ID;
  U8 CustomerDefault;
}tProtocol; 

//typedef volatile struct // at 0xXXXX
//{
//  U16 POWERON_TIMEOut;  
//  S16 PWOnfingerMin_TH;  
//  S16 POF_Neibor_TH; 
//  U16 POF_FrameCount_TH; 
//} tPofPara; // power on finger parameter

//20130531JACKSON
typedef volatile struct   
{
  U8 EnableFlag;
  S16 PowerOnUpdateCN1;
  S16 PowerOnUpdateCN2;
  S16 self_minRaw;
}tMutuPofPara;

typedef volatile struct    
{
  U16 LargeArea_thd;
  U8 LargeArea_thd_ratio;
  U8 LargeArea_fig;
  U8 LargeArea_rej;
  U8 LargeArea_weight_thd;
}tLargeAreaPara;

//typedef volatile struct
//{
//  U8 EnableFlag;
//  U16 Lock_TH_Normal;
//  U16 Unlock_TH_Normal;
//}tJitterIIRPara;

//JS20130710a modify by Poly
typedef volatile struct
{
  U8 u8Rev1;
  U8 CHR_TH_Scale_Ratio;
  U16 Lock_TH_Normal;
  U16 Unlock_TH_Normal;
  U8 IIR_Point;
  U16 Unlock_Screen;
  U16 StaticCounterTimes;
}tJitterIIRPara;

typedef volatile struct
{
  S16 Self_CalTHD1;
  S16 Self_CalTHD2;
  S16 Self_CalSum;
  S16 Mu_CalTHD1;
  S16 Mu_CalTHD2;
  S16 Mu_CalTHD3;
  S16 Mu_CalTHD4; 
}tEnvironmentTHDPara;

//JS20130710a : add new strcu from Poly new struct
typedef volatile struct
{
  U8 u8Rev3;
  U8 u8Rev4;
  U8 paraSELFTrigMode;
  U8 u8Rev2;
  U8 TestMode_SelfCC;
  U8 TestMode_SelfFb;
  U16 TestMode_SelfRawCount;
  U16 TestMode_SelfRawPulse;
}tDEBUGSettingPara;

typedef volatile struct
{
  U8 WideScanEnable;    // 0 /1
  U8 WideFreqStart;   // ex: 12
  U16 U16Reserved[3];   // ex: 13, 17, 15
  U8 WideFreqRange;   // 9
  U8 DoTimes;     // 4
}tFREQAnalysis;

typedef volatile struct
{

  U8 Touch_Info[FW_SUPPORT_MAX_FINGERS]; // fingerDown = 1, fingerUp = 0 //JS20130616a : For library-ize
  S16 Pos[2][FW_SUPPORT_MAX_FINGERS]; //[0]=Y, [1]=X  //JS20130616a : For library-ize
  U8 Z_Area[FW_SUPPORT_MAX_FINGERS];  // 5bits          //JS20130616a : For library-ize
  U8 Z_Force[FW_SUPPORT_MAX_FINGERS]; // 3bits         //JS20130616a : For library-ize
  U8 fingers_number;
} tFingerIDList;

typedef volatile struct
{
  U8 LCMEnable;
  U8 IIREnable;
  U8 LFChno;
  U8 RTChno;
}tLCMAnalysis;

#endif //_GLOBALTYPE_H_