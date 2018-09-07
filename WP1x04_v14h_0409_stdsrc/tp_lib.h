#ifndef _TP_LIB_
#define _TP_LIB_

#include "globaltype.h"
#include "bios.h"
#include "global_vars.h"

//JS20130616a XX : Start : Move to flash_para.c
//#define HW_Version 0xB
//#define FW_Version 0x19
//#define YearMonthDay 0x20130513
//JS20130616a XX : End : Move to flash_para.c

#define AMPLIFY_SELF_RX

#define Co_FWAP_Protocol  0x00D3
#define FW_ReleaseVersion   0x0000

//#define SELF_KEY 0x81
//#define MUTU_DOWNKEY 0x82 //Mutu_key is at TP "Bottom" side.
//#define MUTU_RIGHTKEY 0x83 //Mutu_key is at TP "Right" side.
//------modify by different pannel ----------//
//          
//      
//      
//      
//#define I2C_DEVADDR 0xBA
//#define RxLength 22
//#define TxLength 11
//#define key 0
//#define KeyType 0 // 0x81 = self , 0x82 = MUTU_DOWNKEY , 0x83 = MUTU_RIGHTKEY , 0 = none
//#define Customer_Ratio 40
//      
//      
//      
//      
//------modify by different pannel end----------//
//@@@ Shrink code size & Xdata @@@
//#define UseTwoFrameSum      0 // 1: shrink Xdata from 4 frames -> 2 frames; 0: original 4 frames
#define PointsQueueCaseSelection    1 // 1: decrease PQ case; 0: original 3 buffers queue
//@@@ Shrink code size & Xdata @@@
#define MAX_FINGERS     5 
#define max_ID      5 

//#define Mutu_keyTpye 1 // 1 = "MUTU_DOWNKEY" ; 0 = "MUTU_RIGHTKEY" .    //JS20130722 XX remove #define Mutu_keyTpye but judge with flKeyPara.type
//#define MAX_MUTU_KEY_NUM 4
//#define MAX_FINGERS 5
//#define FW_SUPPORT_MAX_FINGERS 5       //JS20130616a : for library-ize //JS20130619b : move to globaltype
//#define max_ID MAX_FINGERS
#define MAX_ID FW_SUPPORT_MAX_FINGERS  //JS20130616a : for library-ize
#define MAX_SELF_CHANNELS 38 // 48
#define MAX_H_CH_ToByte 4
#define RES_SCALE 64

//#define key_threshold 60 //120 //JS20130616a XX : Move to flash_para.c
#define LIM_VAL 15
#define PQ_NEARPOINTTHD   (RES_SCALE/2)
#define MaxProcessPeak 20
//JS20130618a XX : Start :: Move to flash_para.c
//#define selfENVIROMENT_CALTHD1  50
//#define selfENVIROMENT_CALTHD2  50
#define selfENVIROMENT_CALTHD3  -10 
#define selfENVIROMENT_CALTHD4 -15

//#define selfENVIROMENT_Sum  -3000
//#define muENVIROMENT_CALTHD1  60  // [Fox]
//#define muENVIROMENT_CALTHD2  80 // [Fox]
//#define muENVIROMENT_CALTHD3  -60  // -20
//#define muENVIROMENT_CALTHD4  -80 // -35 
//JS20130618a XX : End :: Move to flash_para.c
#define PEAK_DIFF_THD     30

//=== OpMode[1] Bits Define : Begin
#define NO_DATA     0x0
#define BIT_MUTUAL_MODE 0x02
#define RUN_FULL_AUTO   0x00
#define RUN_FULL_SELF   0x04
#define RUN_FULL_MUTUAL 0x08
#define MUTUAL_ROWDATA  0x10
#define MUTUAL_BASELINE 0x20
#define MUTUAL_DELTA  0x30
#define SELF_ROWDATA  0x40
#define SELF_BASELINE   0x80 //0x1
#define SELF_DELTA    0xC0 //0x2
//=== OpMode[1] Bits Define : End
#define SELF_MODE     0
#define MUTUAL_MODE   1
//#define partial_Mutual 2
//#define Self_MuSwitch 0x2
#define PointQueBufferLength 8
#define MuBaselineOnSlefCounter 16

//#define HoppingFreq0      52 // 13
//#define HoppingFreq1      68 // 17
//#define HoppingFreq2      60 // 15

//fixed
#define HAUTO_FREQ_RANGE  9   // fixed
#define HDO_TIMES   4   // fixed

#if REDUCE_XRAM
  #if RAW_FILTER
  #define Raw_buf_len 4 // need  +2
  #else
  #define Raw_buf_len 3 // need  +1
  #endif
#else
  #define Raw_buf_len 5 // need  +1
#endif

#define StandardORLibrary     1 //0: standard, 1:Library


#define SwitchLowFreqMode     0 //   1:Enable auto idel mode
#define DebugForAPKTest       0

#define Qua_delta_th  0
#define KEEP_CORNER     ( RES_SCALE * 21 / 12 )   //109 // 64 , 109 = 64*1.7
//#define tapping_distance_cellX2 9 //5//9 //JS20130616a XX : Move to flash_para.c
//#define tapping_distance (RES_SCALE/8 * tapping_distance_cellX2) * (RES_SCALE/8 * tapping_distance_cellX2) //JS20130616a XX : Move to flash_para.c


#define LCM_ANALYSIS      0 // 1      // default:0, need to arrange ADC table, max 2 seq only.
#if LCM_ANALYSIS
#define IIREnable       0     //LCM analysis relative-1
#define LFTotalChno         11//10      //LCM analysis relative-2
#define RTTotalChno         11//11      //LCM analysis relative-3
#endif

#define LCMthrLow   -10
#define LCMthrHigh    10
#define LCMDeltaHiThr 60
#define LCMDeltaLoThr -25
#define NormalRatio1  4
#define NormalRatio2  0
#define ChargerRatio1 1
#define ChargerRatio2 3

#define Add 0
#define Sub 1
#define Mul 2
#define Div 3

#define KeyDebounceTime 3

extern U8 AA_RxLen;
extern U8 AA_TxLen;

//-----------------------------------------
extern S16 peakmax, peakmin;
extern U8 nFloatDynamicTH;
//extern S16 DEBUG_mutuRAWOneCell,peak_1st,peak_2nd,peak_3rd,peak_4th,PeakChecksum,PREpeak_1st;   // for floating debug
//-----------------------------------------
extern U8 SwitchScanMode_trig;
//extern U8 ReportInfo_pre;
//extern U8 ReportInfo;
//extern U8 Runtime_scamMode_pre;
//extern U8 Runtime_scamMode;
extern U8 sim_floating;
//extern U8 UpdatePulsePara_pre; 
//extern U8 AutoPulseLen_en;
//extern U8 AutoPulseLen_trig;
extern U8 FrameCount;

extern U8 xdata preOpMode[2];
extern U8 xdata OpMode[3]; // OpMode[2] => Last_error_flags 
extern S16 *ptrReportSelfData;
extern S16 *ptrReportMutualData ,*ptrReportMutualData_1;
//extern U8 peak[2][max_ID];
extern U8 peak[2][MAX_ID]; //JS20130616a
extern U8 charger_on_status;//20130522JSa

//jackson 20121106
extern U8 KeyPress;
extern U8 fgGotoSleep;


//extern U16 xdata SelfBaselineAddr[SelfTotalNodes];
//extern U16 xdata SelfTargetAddr[SelfTotalNodes] ;

//extern U8 xdata MubaselineAddr[MuTotalNodes] ;          // byte type
//extern U8 xdata MutuTargetAddr[MuTotalNodes] ;    // nodes

typedef volatile struct
{
  //S16 Pos[2][MAX_FINGERS];  //[0]=Y, [1]=X
  //U8 ID[MAX_FINGERS];
  S16 Pos[2][FW_SUPPORT_MAX_FINGERS]; //[0]=Y, [1]=X  //JS20130616a : For library-ize
  U8 ID[FW_SUPPORT_MAX_FINGERS];                      //JS20130616a : For library-ize
  U8 fingers_number;

  U8 Z_Area[FW_SUPPORT_MAX_FINGERS];   //JS20130616a : For library-ize
  U8 Z_Force[FW_SUPPORT_MAX_FINGERS];  //JS20130616a : For library-ize
} tFingerList;



typedef volatile struct
{
  U8 Touch_Info[MAX_FINGERS]; // fingerDown = 1, fingerUp = 0
  S16 Pos[2][MAX_FINGERS];  //[0]=Y, [1]=X
} tFingerPos;

#if 0 // !TP_CUSTOMER_PROTOCOL
typedef volatile struct
{
  S16 Pos[2][PointQueBufferLength]; // set 3 point buffer
}tPoints_buffer;

typedef volatile struct
{
  tPoints_buffer Point_buf[FW_SUPPORT_MAX_FINGERS];  //JS20130616a : For library-ize
  U8 Status[FW_SUPPORT_MAX_FINGERS];                 //JS20130616a : For library-ize
  U8 Index[FW_SUPPORT_MAX_FINGERS];                  //JS20130616a : For library-ize
}tPoints_que;
#endif

typedef volatile struct
{
  //U8 Raw_buf[Raw_buf_len][MAX_MUTUAL_NODE];
  U8 index;
  U8 status;  
}tMutu_que;
//---------------------
//typedef volatile struct       // 0x7C00
//{
//  U8 FW_Parameter_Status;   //0xAA = Enable FW Parameter function, otherwise = Disable
//  U8 Customer_ID;     //0
//  U8 Project_ID;      //0
//  U8 Version_Main;    //0
//  U8 Version_Sub;     //0
//  U16 Parameter_Checksum;
//  U16 FW_Checksum;
//  U16 Co_FWAP_Ptl;      //0
//  U16 FW_ReleaseVER;      //0
//  U16 u16Rev1;
//  U16 u16Rev2;
//  U8 u8Rev1;        //0
//  U8 u8Rev2;      //0
//}tFlash;  
typedef volatile struct
{
  U8 v_rows;
  U8 h_cols;
  S8 frame_delta;
  U16 self_finger_lvl;
  U16 mutual_finger_lvl;
} tAlgorithmParametersSet;

typedef volatile struct
{
  U8 Self_enable; 
  S16 Self_TH1;
  S16 Self_TH2; 
  U16 SelfPluseCount;
  U8 Mutual_enable; 
  S16 Mutual_TH1;
  S16 Mutual_TH2;   
  U16 MutualPluseCount; 
}tFloatingPara;

typedef volatile struct
{
  S16 PalmThreshold;
  U8 bigFingerNodeTH;
  U8 PalmAreaCount;
  U8 PressureOffset;
  U8 PressureDivisor;
  U8 PalmReport_point; 
}tPalmRejection;

//JS20130616a XX : Start :: Move to globaltype.h and set globaltype.h as read only file
//typedef volatile struct
//{ 
//  U16 SelfPulseCount;
//  U16 SelfPulseLen;
//  U16 MutualPulseCount; 
//  U16 MutualPulseLen; 
//  U16 Self_first_TH; 
//  U8 Self_fb;
//  U8 Mutu_fb;
//  U16 Self_TH;
//  U16 Mutu_TH;    
//  U16 Mutu_first_TH ;
//  U16 MutuCC_MinRaw ;
//  U8 Mutu_offsetCC ;
//  U8 CC_byRx[32] ;  
//}tRawcountPara;

//typedef volatile struct{
//  U8 type; // 0x81:self , 0x82 : MUTU_DOWNKEY ,0x83: MUTU_RIGHTKEY ,  0 : none
//  U8 num; 
//  U16 CHS[8]; // Tx
//  S16 KeyThreshold;
//}tKeyPara;
//typedef volatile struct
//{ 
//  U8 SUPPORT_FINGERS;
//  U8 Y_TX;
//  U8 X_RX;
//  U8 scale; //64 .48  
//  U8 HW_VER;
//  U8 FW_VER;
//  U32 SERIAL_NO;
//  U8 u8Rev1;
//  U8 u8Rev2;
//  U16 V_ID;
//  U16 P_ID; 
//}tPannel;
//typedef volatile struct
//{
//  U8 Clock_rate;  
//  U8 I2C_DevAddr;
//  U16 Timer_period_Normal;
//  U16 Timer_period_Idle;
//  U8 ReportPoint_para;  // bit0 : Y inverse enable , bit1 : X inverse enable , bit2 : X Y change
//  U16 Tapping_distance; 
//}tSystemPara; 
//----------------------------
void TpInit(void);
S8 TpMainLoop(void);
void TpTestModeLoop(void);
void OnTimerInterrupt(void);
extern tRawcountPara RawcountPara;
extern tFingerIDList FingerIDList_Output;
#endif
