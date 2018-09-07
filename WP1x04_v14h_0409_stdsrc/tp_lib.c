//-----------------------------------------------------------------------------
// TP Routines
// Create: Tim Chen
// Date: 2012-06-05
// Version: 0.1a
//-----------------------------------------------------------------------------
#include <stdio.h>              // prinf()
#include <math.h>               // abs()
//#include <math.h>
#include <intrins.h>
#include "bios.h"
#include "tp_lib.h"
#include "protocol_wp.h"
#include "user_protocol.h"
//#ifdef TP_CUSTOMER_PROTOCOL //JS20130616a XX: It is for Library-ize
#include "global_vars.h"
//#endif //#ifdef TP_CUSTOMER_PROTOCOL
#define TAPPING
//------------------------
#define ReportInfo_default  (NO_DATA | NO_DATA)  // NO_DATA,MUTUAL_ROWDATA , MUTUAL_BASELINE , MUTUAL_DELTA | SELF_ROWDATA , SELF_BASELINE , SELF_DELTA
#define Runtime_scamMode_default   RUN_FULL_MUTUAL  //  RUN_FULL_SELF , RUN_FULL_MUTUAL  , RUN_FULL_AUTO
#define PointGitterReject                       0
//#define _Key_Alive_ 1   //check if scan key?  // move it to Struct_Init function
#define FLOATMODE                                       0
#define PQMODIFY_TWOKEY                 1
//
#define NineGrid_SearchPeak             1 // set 0 to improve 2 finger pitch
#define palm_solution                           0  //
#define ENABLE_DUAL_FINGER_ID_LIST  0 //JS20130205A
// tim 20130321
#define debug_Self_Rawdata 0
#define Self_Process 1                   //Fixed to 1 !
#define WinpowerCalReportRate 0  // 1: Intr always enable , 0: Intr is enable when finger and key >0 .
// poly 20120928
#define OpenFWNoiseCheck                0          //2549
#define OpenFWNoiseCheck_2              0
#define CoaxialLevelCheck               0
#define PowerOnFinger_Tim              1
#define SZ_Floating_Poly2                       0               // parameter setting
#define SZ_Floating_Poly3                       0 // 1
#define SZ_Floating_DEBUG                       0               // debug block (protocol.h & tplib.h)
#define JITTER_for_INNOS_Poly         1
#define PEAK_IMPROVE                            1       // [Crovax] Improve SearchRowPeak
#define PEAK_EXTEND                                     0       // [Crovax] Search new delta peak around extend to find new peak.
#define MUTUAL_MAX_TWO                          1       // [Crovax] Calculate mutual with max-two side average.
#define ISOTRI_PEAK_ENSURE                      1
#define SEARCH_NEW_DELTA_PEAK           1       // Search mutual delta peak with last income array.
#define EDGE_CROSS                                      0       // [Crovax] Modify linearity in edge when using regression method.
#define LAZY_EDGE                                       1       // [Crovax] A lazy method for edge point.
#define POINT_PREDICT_ON                        0       // [Crovax] Predict expected coordinate to improve linearity.
#define NEWSysTimerReload_poly          1
#define SysTimerDebug_poly                      0
//#define LARGE_AREA                              0       //Fixed to 0 !// [Crovax] Large area process, Twokey first version, Crovax rewrite.
#define SELF_WEIGHTING                          0       // [Crovax] Switch self non-border area to calculate with weighting-sum.
//#define ISOTRI_CANCEL                           0       //Fixed to 0 !  // [Crovax] Cancel IsolateTriangleRA.
#define OriginalSelfModeJudge           0
#define DebugForAPKTest                         0
#define UseTrickToDEBLOCKING            0 // 1 //JS20130710a merge Poly's Jitter algorithm
#define JITTER_6                                        0       // [Towkey] For wintek 6mm stick linearity test.
#define SLASH_6MM                                       0
#define BIGFingerProcess                                0 // 1
#define ONE_KEY_TOUCH_ONLY              1 //JS20130805
#define ANDY_OPTIMIZE         1
#define CHANGE_SF_MU_SEARCH_MODE  1   // always "1"
#define SelfTrick_Poly        1
#define DetectMutualDownKeySetting  0

#if DetectMutualDownKeySetting
//Calculate Tx or Rx Key position
U8 Key0Tx,Key1Tx,Key2Tx,Key0Rx,Key1Rx,Key2Rx;
U8 CoaxisTxCHno,CoaxisRxCHno;
U8 CoaxisKey=0; //0x11: Tx, 0x22:Rx
#endif

#if StandardORLibrary
//library
#define LARGE_AREA                              0       //Fixed to 0 !// [Crovax] Large area process, Twokey first version, Crovax rewrite.
#define ISOTRI_CANCEL                           0       //Fixed to 0 !  // [Crovax] Cancel IsolateTriangleRA.
#else
//standard
#define LARGE_AREA                              1       //Fixed to 0 !// [Crovax] Large area process, Twokey first version, Crovax rewrite.
#define ISOTRI_CANCEL                           1       //Fixed to 0 !  // [Crovax] Cancel IsolateTriangleRA.
#endif

U8 EnForceSelf  = 0;
//U8 prepeak_index = 0;
//S16 MutuKeyBufferDelta[4] = {0,0,0,0};
U8 paraSELF_SING_ALL = 1;
#define RO_DelayLatency 2
U8 RO_Queue[MAX_FINGERS] = 0;

U8 MutuFingerNo,SelfFingerNo;

#if CHANGE_SF_MU_SEARCH_MODE
U8 SELF_MODE_PEAKS = 0;
U8 MUTU_MODE_PEAKS = 0;
U8 SELF_MODE_PEAKS_SUM = 0;
#endif

#ifdef JUST_for_MobilePhone_DEBUG
S16 Debug_MutuDeltaMax = 0;
S16 Debug_SelfDeltaMax=0;
#endif

#if SelfTrick_Poly
U8 Self_0x12_flag = 0;
U8 Self_0x21_flag = 0;
U8 Self_0x13_flag = 0;
U8 Self_0x31_flag = 0;
//U8 Self_0x14_flag = 0;
//U8 Self_0x41_flag = 0;
U16 Coaxis_2Finger = 0;
U16 Coaxis_3Finger = 0;
//U16 Coaxis_4Finger = 0;
#define EnterTrickCoaxisProcess 10    //100 * 10ms = 1 sec
#endif

#if BIGFingerProcess
bit fgBIGFinger=0;
#define BIGFingerLOCK           150
#define BIGFingerUnLOCK         100
#define BIGFingerTolerance      4
#define BigFingerAreaNumber     9
#endif
//S16 MutukeyDeltaArray[MAX_MUTU_KEY_NUM]=0;              //add @20130827 by poly
U8 AA_RxLen;
U8 AA_TxLen;
//S16 Delta_key_x[4];   //JS20130619a : Modify for FAE real mutual key case by Gavin
//S16 Delta_key_LPF[4]; //JS20130619a : Modify for FAE real mutual key case by Gavin
bit SelfADCEnALLBit=1;
#if UseTrickToDEBLOCKING //JS20130710a merge Poly's Jitter algorithm
bit PowerOnFlagForJitter=0;
#endif
#if PowerOnFinger_Tim
U8 mutuBaLine_NoUpdateCn=0;
#endif
U8 selfBaLine_updateCn=0;

#if Self_Process //tim
#define SearchMuPeakWidth 3
//U8 selfpeak[2][MAX_FINGERS]; // [0][] y , [1][] x
U8 selfpeak[2][FW_SUPPORT_MAX_FINGERS]; // [0][] y , [1][] x  //JS20130616a : For Library-ize
U8 *ptrSelfPeak; // [0][] y , [1][] x
U8 SELF_peak_index=0;
bit mutuPoint=1;
bit bSelfProcessEn=1;
#endif
#if SZ_Floating_Poly2
U8 nFloatDynamicTH=1;           // 1 = high TH
U8 nFloatDynamicTHCounter_LToH=0;
U8 nFloatDynamicTHCounter_HToL=0;
#define DeltaScaling    1 // 3  //  enter TH scaler
#define DynamicTH       (flRawPara.Mutu_TH * DeltaScaling)
S16 DynamLEVEL;
#endif
#if SZ_Floating_Poly3
U8 fgDyDetectLARGEAREA;
#endif
S16 DynamLEVEL_TH;
#if SZ_Floating_DEBUG
S16 DEBUG_mutuRAWOneCell,peak_1st,peak_2nd,peak_3rd,peak_4th,PREpeak_1st;
U8 peakCounter=0;
S16 PeakChecksum=0;
#endif
#if PQMODIFY_TWOKEY
#define PQBUFFER_SMOOTHTHD1             (RES_SCALE/4)
#define PQBUFFER_SMOOTHTHD2             (RES_SCALE*4)
#endif
#define POINTQUEUE_8
#ifndef POINTQUEUE_8
#define MAX_PQBUFFER    3
#else
#define MAX_PQBUFFER    8
#define DIR_PQBUFTOL_SP 1
#define DIR_PQBUFTOL_SN  -1
#define DIR_PQBUFTOL_P  10
#define DIR_PQBUFTOL_N  -10
#define DIR_PQBUFTOL_FP 40    //RES_SCALE
#define DIR_PQBUFTOL_FN  -40   //-1*RES_SCALE
#endif
#if POINT_PREDICT_ON
tFingerIDList PredictFingerIDList;
#endif
#if LARGE_AREA
//JS20130618a XX : Start :: Move to flash_para.c
//#define LARGEAREA_THD     45 //35
//#define LARGEAREA_THD_RATIO   2 // Chose area threshold base on ratio of max delta. 2013.05.07
//#define LARGEAREA_FIG     1 //6 // Big finger, report // 8
//#define LARGEAREA_REJ     25//16  // Palm, reject     //16
//#define LARGEAREA_WEIGHT_THD  10//10  // Counted weighting-Sum threshold for calculate coordinate.
//JS20130618a XX : End :: Move to flash_para.c
void AreaProcess(void);
U8 AreaCheck(U8, U8, U8 *);
S16 WeightingSumAreaMain(U8 *, U8);
U8 area_ydirection(U8, U8);
//U8 AreaChkFlag[TxLength][RxLength];
//U8 AreaChkFlag[TXRX_LENGTH_MAX][TXRX_LENGTH_MAX]; //JS20130616a
//extern U8 AreaChkFlag[TX_LENGTH][RX_LENGTH]; //AD20130722
extern U8 AreaChkFlag[][MAX_RX_LENGTH]; //AD20130722
U8 leftbound, rightbound;
U8 PeakArea[MaxProcessPeak];    // PeakArea denote how many area the peak covered.
U8 BoundSet[MaxProcessPeak][4]; // 0:left, 1:right, 2:up, 3:down ( 0,1 x-axis; 2,3 y-axis )
#endif
#if LAZY_EDGE
S8 EdgeCoeffWeightSelf[4]={ 8, 8, 8, 8};         // Right-Left-Upper-Lower ( X0, Xn, Y0, Ym )
S8 EdgeCoeffWeightMutual[4]={ 0, 0, 0, 0};   // Right-Left-Upper-Lower ( X0, Xn, Y0, Ym )
S16 EdgeWeightingExtend(S16 *, U8, U8, U8);
#endif
U8 AP_ActiveFlag=0;
#if SwitchLowFreqMode
U8 fgLowfFreq=0;
//U8 AP_ActiveFlag=0;
U16 MCUCounter=0;
//#define EnterIdleTimer  3000  // 3000 * 10ms ~= 30 sec //JS20130708a XX use the parameter in the flash_para.c
U8 CheckWkUp=0;
#endif
#if SELF_WEIGHTING
S16 WeightingSumSelfMain( S16*, U8, U8);
#endif
#if JITTER_6
//#define DEBUG_JITTER_6MM
/*
   S16 code PreditPosition_Low[9][2] ={{ 25-11,  24-7}, { 23-11, 769-6}, { 21-11,1508-6},
                                                                        {445-11,  25-7}, {   999,  1999}, {443-11,1506-6},
                                                                        {864-11,  26-7}, {861-11, 769-6}, {861-11,1509-6}};
   S16 code PreditPosition_High[9][2]={{ 25+11,  24+7}, { 23+11, 769+6}, { 21+11,1508+6},
                                                                        {445+11,  25+7}, {   999,  1999}, {443+11,1506+6},
                                                                        {864+11,  26+7}, {861+11, 769+6}, {861+11,1509+6}};
   S16 code PreditPosition_Comp[9][2]={{    17,    15}, {    19,     0}, {    21,    -14},
                                                                        {     0,    15}, {     0,     0}, {     0,    -9 },
                                                                        {   -11,    13}, {    -8,     0}, {    -8,    -15}};
 */
S16 code TargetPosition[9][2]={{    42,    41}, {    42,   767}, {    42,  1494},
    {   447,    41}, {   999,  1999}, {   447,  1494},
    {   853,    41}, {   853,   767}, {   853,  1494}
};
S16 code PreditPosition_Low[9][2]={{ 26-11,  31-7}, { 30-11, 772-6}, { 34-11,1510-6},
    {443-11,  29-7}, {   999,  1999}, {452-11,1507-6},
    {862-11,  25-7}, {862-11, 765-6}, {869-11,1506-6}
};
S16 code PreditPosition_High[9][2]={{ 26+11,  31+7}, { 30+11, 772+6}, { 34+11,1510+6},
    {443+11,  29+7}, {   999,  1999}, {452+11,1507+6},
    {862+11,  25+7}, {863+11, 765+6}, {869+11,1506+6}
};
S16 code PreditPosition_Comp[9][2]={{    16,    10}, {    12,     0}, {     8,    -16},
    {     0,    12}, {     0,     0}, {     0,    -13},
    {    -9,    16}, {   -10,     0}, {   -13,    -12}
};
#endif
//#endif
#if SLASH_6MM
//#define DEBUG_SLASH_6MM
#define TX_CHANNELNO            14
#define RX_CHANNELNO            24
#define SLASH_BOUNDARY          (RES_SCALE * RX_CHANNELNO / TX_CHANNELNO)
#define LASTCHANNELPOS          RX_CHANNELNO*RES_SCALE-1
//#define LAST2CHANNELPOS   LASTCHANNELPOS - RES_SCALE
#define LAST2CHANNELPOS         LASTCHANNELPOS - SLASH_BOUNDARY
//S16 code SlashDetRange_X[2][2]={{ 6, LAST2CHANNELPOS-6}, {LASTCHANNELPOS-6, 64+6}};
#define FourthTOUCH_X                   30 // 10
#define FourthTOUCH_Y                   30 //   6
S16 code SlashCorner[4][2]={{FourthTOUCH_Y, FourthTOUCH_X}, {RES_SCALE*TX_CHANNELNO-1-FourthTOUCH_Y,0+FourthTOUCH_X}, {FourthTOUCH_Y,RES_SCALE*RX_CHANNELNO-1-FourthTOUCH_X}, {RES_SCALE*TX_CHANNELNO-1-FourthTOUCH_Y,RES_SCALE*RX_CHANNELNO-1-FourthTOUCH_X}};
S16 SlashOutBuf_X[4], SlashOutBuf_Y[4];
U8 Slash_Flag;
#endif
//U8 data MAX_FINGERS = 5; //JS20130616a : For Library-ize
//U8 data max_ID = 5;      //JS20130616a : For Library-ize
//-----------------------------------
// Debug Control
//-----------------------------------
#define debug_log                                       0 // 1 //JS20130330a define as 0
#define INTO_debug                                      0
#define DBG_RPT                                         0
#define Crovax_debug_MutualMainLoop     0
#define Crovax_debug_TAPPING            0
#define Crovax_debug_SearchRowPeak      0
#define Twokey_debug_PointQueue         0
#define Twokey_debug_MatchFingers       0
#define Twokey_debug_SearchRowPeak      0
#define debug_Charger 0
// Debug Control End
//#endif
#define SignDelta 1
// --- extern flash Init parameter ---//
//extern code U8 Flash_data_en ;
extern code tFlash  fFlash ;
extern code tProtocol flProtocol ;
extern code tSystemPara flSystemPara ;
extern code tPannel flPannel ;
extern code tRawcountPara flRawPara ;
extern code tKeyPara flKeyPara;
extern code tMutuPofPara flMutuPofPara;
extern code tLargeAreaPara flLargeAreaPara;
extern code tJitterIIRPara flJitterIIRPara;
extern code tEnvironmentTHDPara flEnvironmentTHDPara;
extern code tDEBUGSettingPara fDEBUGSettingPara; //JS20130710a
extern code tFREQAnalysis fFREQAnalysis;
//
extern code U8 ChMapping[];
extern U8 SelfComOffset[];
extern volatile U8 MutualComOffset[];
#if !TP_CUSTOMER_PROTOCOL
extern volatile U8 TestModeSelfCC[];
#endif
//JS20130619d : Start : Reserve for FAE usage function call
extern bit userFunctionCallBit0;
extern bit userFunctionCallBit1;
extern bit userFunctionCallBit2;
extern bit userFunctionCallBit3;
extern bit userFunctionCallBit4;
extern bit userFunctionCallBit5;
extern bit userFunctionCallBit6;
extern bit userFunctionCallBit7;
//JS20130619d : End : Reserve for FAE usage function call
//extern void userKeyCheck(void); //JS20130620a
//##########################
// Poly add data for WP1004 -- 20120814
//##########################
//Self Baseline & Target setting
//#define SelfTotalNodes (TxLength+RxLength+key)    // Y+X+key
#define MAX_SELF_CHANNELS_bytes (MAX_SELF_CHANNELS+MAX_SELF_CHANNELS)
#define Start_SelfBaseAddr 0x0100
#define Start_SelfTargetAddr (Start_SelfBaseAddr + MAX_SELF_CHANNELS_bytes  )
//#define Start_MubaseAddr (Start_SelfTargetAddr+MAX_SELF_CHANNELS_bytes+MAX_SELF_CHANNELS_bytes+MAX_SELF_CHANNELS_bytes)
//#define Start_MuTargetAddr (Start_MubaseAddr + MAX_MUTUAL_NODE)
//volatile S16 xdata SelfBaselineAddr[MAX_SELF_CHANNELS] _at_ Start_SelfBaseAddr;
//volatile S16 xdata SelfTargetAddr[MAX_SELF_CHANNELS]  _at_ Start_SelfTargetAddr;
volatile S16 xdata SelfTargetAddr[3][MAX_SELF_CHANNELS]  _at_ Start_SelfTargetAddr;
S16 * pSelfTargetAddr;
//AD20130723 begin
extern S16 LastQuaDelta[];
extern S16 LastQuaDelta_1[];
extern S16 MutualBaseLine[3][MAX_MUTUAL_NODE];
extern U16 MBaseLine[3][MAX_MUTUAL_NODE]; // [twokey]
//AD20130723 end
//Mutual Baseline & Target setting
#if WP1004_B

/*for hopping:Yuho*/
extern volatile S16 xdata NoiseTargetAddr[] ;//_at_ Start_MuTargetAddr;

//volatile S16 xdata MubaselineAddr[MAX_MUTUAL_NODE] _at_ Start_MubaseAddr;
//volatile S16 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ;//AD20130723 Moved to global_vars.c
//extern volatile S16 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ;//_at_ Start_MuTargetAddr;
//extern volatile S16 xdata *MutuTargetAddr[Raw_buf_len];//_at_ Start_MuTargetAddr;
//extern S16 xdata *MutuTargetAddr[];     //AD20130723: Can be portable.
//volatile S16 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ;//_at_ Start_MuTargetAddr;
extern volatile S16 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ;//_at_ Start_MuTargetAddr;

S16 * pMutuTargetAddr;
//S16 * pMuRaw_buf_Tn1;
#if !REDUCE_XRAM
S16 * pMuRaw_buf_T0;
S16 * pMuRaw_buf_T1;
#endif
S16 * pMuRaw_buf_T2;
S16 * pMuRaw_buf_T3;
S16 * pMuRaw_buf_PRE;
#else
volatile U8 xdata MubaselineAddr[MAX_MUTUAL_NODE] _at_ Start_MubaseAddr;
volatile U8 xdata MutuTargetAddr[Raw_buf_len][MAX_MUTUAL_NODE] ; //_at_ Start_MuTargetAddr;

U8 * pMutuTargetAddr;
//U8 * pMuRaw_buf_Tn1;
U8 * pMuRaw_buf_T0;
U8 * pMuRaw_buf_T1;
U8 * pMuRaw_buf_T2;
U8 * pMuRaw_buf_T3;
#endif
//U8  MuRaw_T2[MAX_MUTUAL_NODE];  // poly add 1008
//U8  MuRaw_T3[MAX_MUTUAL_NODE];
#define PalmReportPoint 1
#define point_filter_vector 1
//---for edge solution----
#define POINT_NEWTHD            // Point Stabliztion Fucntion Definition
//#define GAMMA_TABLE
// ----- parameter ----- //
bit WDTflag=0;
//S16 OneFrDeltaSum,OneFrDeltaAvg;
//U8 fgBootError;
//U8 KeyPressCounter[MAX_MUTU_KEY_NUM]={0,0,0,0};
// proximity
#ifdef POINTQUEUE_8
S16 DataAvg(S16*, U8*, U8, U8, U8*);            // [twokey]
//bit ContinueFlag;
//U8  ContinueFlag;
#else
S16 DataAvg(S16*, U8*, U8); // [twokey]
#endif
S8 prePeaks;
S8 prePeak[2][MaxProcessPeak];
U8 Total_TX;
U8 Total_RX;
#if SEARCH_NEW_DELTA_PEAK
//S16 Last_2_Delta[MAX_MUTUAL_NODE];
S16 *ptrLastQuaDelta;
S16 *ptrLastQuaDelta_pre;
//S16 LastQuaDelta[MAX_MUTUAL_NODE];  //AD20130723 Moved to global_vars.c
//S16 LastQuaDelta_1[MAX_MUTUAL_NODE];  //AD20130723 Moved to global_vars.c
#endif
U8 fnIsOnEdge(U16, U16, U8);
//key
//U8 KeyPress;
//U16 KeyThreshold;
//void TpFillKeyPacket(U8);
void Diff_Max_In_3x3Grid(void);
//void Zero_In_3x3_Grid(void);
//Hopping   // [poly]
// struct define
tAlgorithmParametersSet AlgorithmParam ;
#if FLOATMODE
tFloatingPara FloatingPara ;
#endif
//tRawcountPara RawcountPara;
#if palm_solution
tPalmRejection PalmReject; //[ jackson ]
#endif
S16 *ptrReportSelfData;
//S16 SelfRaw0[MAX_SELF_CHANNELS];
//S16 SelfRaw1[MAX_SELF_CHANNELS];
S16 SelfBaseLine[3][MAX_SELF_CHANNELS];

U16 SBaseLine[3][MAX_SELF_CHANNELS]; // [twokey]



S16 SelfDelta[MAX_SELF_CHANNELS];
S16 SelfDelta_1[MAX_SELF_CHANNELS];
S16 *pSelfdelta;
S16 *wbuf;
S16 * wbuf_T0;
S16 *ptrReportMutualData,*ptrReportMutualData_1;
//S16 MutualRaw0[MAX_MUTUAL_NODE];
//S16 MutualRaw1[MAX_MUTUAL_NODE];

/*for hopping:Yuho*/
#define SortNum 2

#if 0
#define NonSortFreqNum 9
#define ChargerUsedFreq 15
U8 NonSortfreq[NonSortFreqNum]={ChargerUsedFreq,14,13,17,18,19,20,21,22}; //15 is used for charger
#else
#define NonSortFreqNum 4
#define ChargerUsedFreq 15
U8 NonSortfreq[NonSortFreqNum]={ChargerUsedFreq,17,18,22}; //15 is used for charger
//U8 NonSortfreq[NonSortFreqNum]={ChargerUsedFreq,15,15,15}; //15 is used for charger
#endif

U8 UsedCTFreq[3]={13,19,ChargerUsedFreq};
//U8 UsedCTFreq[3]={17,17,17};
U16 Noiselevel[9]={0,0,0,0,0,0,0,0,0};
U16 Noiselevelfirst[13]={0,0,0,0,0,0,
                                   0,0,0,0,0,0,0};


//U8 firstfreqs[sortnum];
//U16 firstlevs[sortnum];





U8 freqIndex = 0;
#define chargerfreq 0
U8 scanfreqIndex = 0;
U8 NLIndex = 0; 
U8 SelfBaseRy[3];
U8 MutuBaseRy[3];



U16 sumnoiselev1,sumnoiselev2;
#define FrameHiThr 64
#define NoiseHiThr 50
#define NSL_On_ChargerFlagTH  (NoiseHiThr-10)
#define NSL_Off_ChargerFlagTH (NoiseHiThr-20)
U8 FingerOnLockFreq = 0;


U8 freqbufnow;
U8 freqhist[4];
U8 freqcounter=0;
S16 *pbT3,*pbT2,*pbT1,*pbT0;
bit checkcorrectfig = 0;
















extern U8 SelfPulseTime; 
extern U8 MutuPulseTime;
extern U8 NLPulseTime;
U16 sum3reqnoise(U8);

void measureNLlevel(void);
void chooisefreq1(void);
void chooisefreq2(void);


//S16 MutualBaseLine[MAX_MUTUAL_NODE];  //AD20130723 Moved to global_vars.c
//S16 MutualBaseLine_unit[MAX_MUTUAL_NODE];
//U16 MBaseLine[MAX_MUTUAL_NODE]; // [twokey]   //AD20130723 Moved to global_vars.c
//S16 MutualDelta[MAX_MUTUAL_NODE];
//S16 MutualDelta1[MAX_MUTUAL_NODE];
S16 *pMutualdelta;
S16 *mubuf;
//S16 *mubuf_last;
//S16 mutukeyBaseline[3][MAX_MUTU_KEY_NUM]; // tim add mutual key buf
//S16 mutuKey_Raw[MAX_MUTU_KEY_NUM];
//U8 mukeyBaseline_en;
U8 ScanMode;
U8 NextScanMode;
U8 NextScanMode2;
U8 bPowerOnSelf[3];
U8 bPowerOnMutual[3];
#ifdef POLY_1128LIB
U8 bPowerOnMutual_Key[3];
#endif
//U8 bFZ;
int Self_nodes;
U16 Mu_nodes;
//U8 TxNumber;
//U8 I2C_buf[4];
//U8 ReportInfo_pre;
//U8 ReportInfo;
//U8 Runtime_scamMode_pre;
//U8 Runtime_scamMode;
U8 FrameCount;
//U8 TouchCount;
//S16 Tri_InputBand[TxLength+RxLength];  // Tx Rx select the max
U8 SwitchScanMode_trig_pre;
U8 SwitchScanMode_trig;
U16 gbCNter ;
tFingerList *TouchFingersList;
tFingerList *lastTouchFingersList_1;
tFingerList *lastTouchFingersList_2;
tFingerList TouchFingersList_0;
tFingerList TouchFingersList_1;
tFingerList TouchFingersList_2;
//tFingerList *TFingersList_report;
//tFingerList *TFingersList_last;
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
tFingerIDList* p0FingerIDList;
#endif
tFingerIDList* p1FingerIDList;
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
tFingerIDList FingerIDList;
#endif
tFingerIDList FingerIDList_1;
//U8 touchCount[MAX_FINGERS];      // Twokey 0918
//U8 touchCount[FW_SUPPORT_MAX_FINGERS];      // Twokey 0918  //JS20130616a : For Library-ize
tFingerIDList FingerIDList_Output;
#if 0//!TP_CUSTOMER_PROTOCOL
tPoints_que PointsQueReg;
#endif
//tKeyPara KeyPara ;
volatile tMutu_que Mutu_que;
U8 LastAvailableFingerNo; //=0
//U8 FirstTouch = 0;
//U8 lastTouch=0;
//U8 Edge_Area_En = 0;
//U8 Edge_Area_Trig = 0;
//U8 Pre_Edge_Area_En = 0;
bit isMuBaselineRdy;
U8 NoReport ;
U8 selfPowerOnBaseLine[3] = {1,1,1};// [twokey]
U8 muPowerOnBaseLine[3] = {1,1,1};// [twokey]
bit fgFrameSync;
U8 muEnviroment_cal_flag[3]=0;
U8 selfEnviroment_cal_flag[3]=0;
//U8 updateMuBaselineCN;
//U8 updateMuBaselineCN_pre;
//U8 SelfToMutuCNter;
U8 MutuToselfCNter;
bit Intr_trig;
bit bridge_i2c_done;
bit bridge_i2c_done_pre;
bit nbridge_i2c_EN;
bit APK_ReadBackOK_1=0;
bit APK_ReadBackOK_2=0;
bit APK_Enter_DEBUG_Mode=0;     // 1: enter debug mode, 0:exit
U8 KeyPress;
S16 peakmax, peakmin;
#if !TP_CUSTOMER_PROTOCOL
//U8 PQ_contFlag[MAX_FINGERS];
U8 PQ_contFlag[FW_SUPPORT_MAX_FINGERS];  //JS20130616a : For Library-ize
#endif
//U8 charger_on_status = 0;//JS20130522a //JS20130619c XX : For FAE usage
extern U8 charger_on_status; //JS20130619c : For FAE usage
extern U8 data BIT8_MASK[8] ;
extern U8 data BIT8_MASK_0[8];
//JS20130805 : Start
//#if ONE_KEY_TOUCH_ONLY
//extern bit enableOneButtonTouchOnly;
//S16 maxDelta;
//U8 maxDeltaIndex;
//#endif //ONE_KEY_TOUCH_ONLY
//JS20130805 : End
#define setP1_1(x){P1_1=x; }
/*
   OpMode flags2: (Default is 0x00).
   b7~b6:
                00= Do not read back self mode data.
                01= Self read-back is self mode ADC raw data.
                10= Self read-back is self mode baseline.
                11= Self read-back is self mode delta.
   b5~b4:
                00= Do not read back mutual mode data.
                01= Mutual read-back is mutual mode ADC raw data.
                10= Mutual read-back is mutual mode baseline.
                11= Mutual read-back is mutual mode delta.
   b3~b2: 00= Run Auto switch mode
                01= Run Full Self mode
                10= Run Full Mutual mode
   b1:    Read only ( 0: self data , 1 : mutual data )
 */
U8 xdata preOpMode[2];
U8 xdata OpMode[3]; // OpMode[2] => pLast_error_flags
U8 xdata *pLast_error_flags;
#if palm_solution
// --- palm reject ---// [ jackson ]
U8 palm_id_count=0;
S16 PalmX=0,PalmY=0;
//U8 AreaCount = 0;
U16 PalmID[MAX_MUTUAL_NODE];
//U8 id_rec[MAX_FINGERS];
U8 PID_unm[MAX_FINGERS] ;
U8 PID_Idx[MAX_FINGERS] ;
U8 PID_report_en[MAX_FINGERS] ;
U8 PID_IdxCn=0;
U8 PID_bigFingerTH;
U8 PID_flag=0;
#endif
tProtocol Protocol;
tProtocol preProtocol;
U8 last_finger=0;
U8 last_key=0;
bit TestModeFlag=0;
void ReportSelectProtocol();
//jackson20121106
extern SUO_Main();
#define WatchDog_Clear() {      TA=0xAA; TA=0x55;    RWT=1; Timer_Counter=0;}
bit selfprocess=0;
void CalSelfBaseline(void);
extern bit tp_customer_protocol; //JS20130616a
//
// ------------------- //
//#define Self_COMPUTATION 2
//#define Mutual_COMPUTATION 3
//#define IDLE 9
//U8  example[3] _at_ 0x00F0  ;
U16 code F_POS_MASK[16]=
{
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000
};
U16 code F_FULL_MASK[16]=
{
    0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF
};
//extern void Delay(uint32_t delayCnt);
#if 0
long EdgecoeffSelf[96]=
{
    /*Y01D01*/ 25,  24, -27, -21052,
    /*Y01D02*/ 22,  23, -39, -18206,
    /*Y01D03*/ 18,  22, -38, -14487,
    /*Y01D04*/ 25,  23, -24, -22193,
    /*Y01X01*/ 10,  15,  -2,  -5207,
    /*Y01D17*/ 11,  14, -14,  -4715,
    /*Y10D01*/ 42, -24, -19,  30353,
    /*Y10D02*/ 57, -23, -17,  30063,
    /*Y10D03*/ 22, -20, -14,  29228,
    /*Y10D04*/ 62, -23, -18,  30089,
    /*Y10X01*/ 5, -15, -10,  22090,
    /*Y10X17*/ -108, -13,  -8,  20384,
    /*X01D01*/ 22,  24, -58, -19103,
    /*X01D02*/ 18,  24, -42, -18744,
    /*X01D03*/ 19,  22, -40, -18551,
    /*X01D04*/ 22,  23, -53, -19476,
    /*X01Y01*/ 16,  14, -26,  -9167,
    /*X01Y10*/ 15,  11,   3,  -9140,
    /*X17D01*/ 46, -24, -20,  32251,
    /*X17D02*/ 7, -22, -19,  32211,
    /*X17D03*/ 9, -21, -18,  33120,
    /*X17D04*/ 51, -22, -22,  34066,
    /*X17Y01*/ -35, -12, -15,  24019,
    /*X17Y10*/ 30, -15, -13,  20237
};
#endif
#if !LAZY_EDGE //JStest
long EdgecoeffMutual[96]=       // 20120719 crovax
{
    26, 81, -104, -5391,    //Y1D1
    27, 59,  -88, -6683,    //Y1D2
    21, 53, -100, -4734,    //Y1D3
    24, 55, -117, -5480,    //Y1D4
    //32, 45,  230, -5571,  //Y1X18
    //32, 47,  245, -5397,  //Y1X18
    //32, 47,  100, -5394,  //Y1X18   //3th Ch Modified
    31, 43, 87, -5149,
    //25, 50,  -82, -3464,  //Y1X27
    //14, 59, 100, -5602,   //Y1X27
    15, 58, 100, -5882,
    62, -86, -43, 26796,    //Y17D1
    -6, -60, -30, 24686,    //Y17D2
    -77, -52, -13, 20951,   //Y17D3
    29, -58, -29, 24191,    //Y17D4
    //-233, -43, -17, 21244,  //Y17X18
    //100, -47, -20, 18564,
    100, -47, -20, 18278,
    //157, -48, -22, 19278, //Y17X27
    //-270, -43, -11, 20261,//Y17X27
    //-100, -43, -11, 17697,  //Y17X27  //3th Ch Modified
    22, -48, -22, 19532,
    48,  77,  88, -11375,   //X18D1
    45,  52,  63, -12344,   //X18D2
    35,  49,  40,  -9560,   //X18D3
    52,  55,   7, -13759,   //X18D4
    //20, 44, 405,  -4376,  //X18Y1
    //29, 45, 239, -5156,  //X18Y1
    //29,  45, 100,  -4645, //X18Y1   //3th Ch Modified
    28, 45, 100, -4159,
    //4,  43, 390,  -1741,  //X18Y17
    //10,  52, -27,  -307,  //X18Y17
    9, 50, 100, -1967,
    16, -87, -35,  25911,   //X27D1
    -30, -58, -21,  22954,  //X27D2
    -46, -54, -12,  20540,  //X27D3
    -5, -61, -27,  24712,   //X27D4
    //231, -48, -20,  17344,  //X27Y1
    //-100, -29, -20, 17523,  //X27Y1
    -100, -30, -21, 17545,
    //-131, -47, -21, 21587 //X27Y17
    //-164, -50, -19, 21949 //X27Y17
    //-100, -50, -19, 21143 //X27Y17  //3th Ch Modified
    -63, -52, -26, 22790
};
#endif //#if !LAZY_EDGE //JStest1
/*
        long EdgecoeffMutual[96] = {   // 20120719 crovax
                                        //61, 39, -100,-28364,  // Y1D1
                                        //43, 33, 46, -22095,   // Y1D2
                                        //34, 31, -40, -16546,  // Y1D3
                                        //42, 33, -96, -18621,  // Y1D4
                                        26, 81, -104, -5391,
                                        27, 59,  -88, -6683,
                                        21, 53, -100, -4734,
                                        24, 55, -117, -5480,
                                        32, 45,  230, -5571,    // Y1X1
                                        25, 50,  -82, -3464,    // Y1X10
                                        //-177, -35, -34, 38179,  //Y17D1
                                        //-223, -36, -40, 42964,        //Y17D2
                                        //-329, -34, -29, 39761,        //Y17D3
                                        //-250, -37, -38, 42173,        //Y17D4
                                          62, -86, -43, 26796,
                                          -6, -60, -30, 24686,
                                         -77, -52, -13, 20951,
                                          29, -58, -29, 24191,
                                         157, -48, -22, 19278,  //Y17X1
                                        -233, -43, -17, 21244,  //Y17X10
                                        //46,  41,   64, -19038,        //X18D1
                                        //39,  36,  114, -18838,        //X18D2
                                        //28,  21,  533, -31218,  //X18D3
                                        //33,  32,  301, -22491,        //X18D4
                                        48, 77,  88, -11375,
                                        45, 52,  63, -12344,
                                        35, 49,  40,  -9560,
                                        52, 55,   7, -13759,
                                        20, 44, 405,  -4376,  //X1Y1
                                         4, 43, 390,  -1741,    //X1Y17
                                         // 63, -50, -54, 38722,        //X27D1
                                         //-25, -46, -48, 41052,        //X27D2
                                         //-25, -30, -46, 40963,  //X27D3
                                         // 15, -46, -47, 39631,        //X27D4
                                         16, -87, -35,  25911,
                                        -30, -58, -21,  22954,
                                        -46, -54, -12,  20540,
                                         -5, -61, -27,  24712,
                                         231, -48, -20, 17344,  //X10Y1
                                        -131, -47, -21, 21587 //X10Y17
   };
 */
S16 edgepos_calculation(S16*, U8, long*);
//#define Twokey_LARGEFINGER
#ifdef     Twokey_LARGEFINGER
void AreaCheck(U8, U8);
U8 area_ydirection(U8, U8);
//U8 AreaChkFlag[TxLength][RxLength];
U8 AreaChkFlag[TxLength-1][RxLength];
//#define LARGEAREA_THD   50
U8 leftbound, rightbound;
//(mutual_finger_th*6/4)
#endif
#ifdef GAMMA_TABLE
const S16 VGammaTable[64]=
{
    0,   0,   0,  -1,  -1,  -2,  -2,  -2,
    -3,  -3,  -3,  -4,  -4,  -5,  -5,  -5,
    -5,  -5,  -5,  -5,  -5,  -4,  -4,  -4,
    -3,  -3,  -3,  -2,  -2,  -1,  -1,   0,
    0,   0,   0,   0,   0,   0,   0,   1,
    1,   2,   2,   2,   2,   2,   3,   3,
    3,   3,   3,   3,   3,   2,   2,   2,
    2,   2,   2,   1,   1,   1,   0,   0
};
const S16 HGammaTable[64]=
{
    0,   0,   0,  -1,  -1,  -1,  -2,  -2,
    -2,  -2,  -3,  -3,  -4,  -4,  -4,  -4,
    -4,  -4,  -4,  -4,  -3,  -3,  -3,  -3,
    -3,  -2,  -2,  -2,  -1,  -1,  -1,   0,
    0,   0,   0,   0,   0,   0,   1,   1,
    1,   1,   2,   2,   2,   2,   2,   2,
    2,   2,   2,   2,   2,   1,   1,   1,
    1,   1,   1,   1,   1,   0,   0,   0
};
#endif
void SwitchMemory(void);
void SwitchMutuMemory(void);
#ifdef TAPPING
U8 GetNewID(void);
#endif
//U8 data NFrames=0;
void Timer0_ISR (void) interrupt 1
{
//  SystemTimerSetEvent();
    TF0=0;
    if (Tcounter==0)
    {
        OnTimerInterrupt();
        if (STflag == 1)
            Tcounter=10;
        else
            Tcounter=1;
#if NEWSysTimerReload_poly
        SystemTimerStop();
#endif
    }
    else
    {
#if NEWSysTimerReload_poly
        SystemTimerReload();
#endif
    }
#ifdef SUO
    if (!WDTflag)
    {
        WatchDog_Initialize(1);
        WDTflag=1;
    }
#endif
#if NEWSysTimerReload_poly
#else
    SystemTimerReload();
    SystemTimerStart();
#endif
}

void OnTimerInterrupt(void)
{
    fgFrameSync=1;
    Timer_Counter++;
}

// -------------------------
//  Finger Matching Routines
// -------------------------
volatile S16 *T0FingersPosX;
volatile S16 *T1FingersPosX;
volatile S16 *T0FingersPosY;
volatile S16 *T1FingersPosY;
//U8 xdata *T0FingersIDList;
//volatile U8 T0FingersIDList[MAX_FINGERS];
volatile U8 T0FingersIDList[FW_SUPPORT_MAX_FINGERS]; //JS20130616a : For Library-ize
volatile U8 *T1FingersIDList;   // ID=0 ~ (MAX_FINGERS-1), 0x80=no touch, 0x81=leave, 0x82=new
//U16 T1DistanceToT0n[MAX_FINGERS][MAX_FINGERS]; // Row=T1[m] vs T0[0..n]
#define MAX_VALUE 0xffff
U16 IdFlags;
U16 preIdFlags;
// -------------------------
// Local variables for DynamicsTracking
// -------------------------
//S16 PredictPos[2][MAX_FINGERS];
//S16 VectorsByID[2][MAX_FINGERS];  // [0]=dy, [1]=dx
void MatchFingers(U8 t0_fingers, U8 t1_fingers)
// --Entering:
//      Last detected as well as ID assigned fingers are put in T0FingersPosX/Y and T1FingersIDList.
//      New detected fingers XY are put from T1FingersPosX/Y. T1FingersIDList are blank.
// --Return:
//      T1FingersIDList will be mapped to T0 closest fingers ID. New fingers will not be assigned and be 0xff. T0 old ID will be reset to 0xFF unless the finger was not mapped.
//  T0|1_mask will be changed.
// --Algorithm:
//  1. Calculate the P*Q distance first.
//  2. Do first pass to find minimum distance and assign order_p to q.
//  3. Check all un-assigned T1 fingers one by one. If current one is not duplicated, assigned T0_id[p] to T1_id and fixed. assign T0_dxy=0xff;
//    If duplicated and current one bigger, don't assign , but force it to be next pass.
//    If duplicated and current one small or equal, force the compared one to be next pass. current one continue to compare next one.
//    After current one compare to others was finished, if current one still alive, then assign id and fixed it.
//  4.  repeat current from 0 to T1 fingers.
//  5. If all T1 fingers are assigned or
{       // Before entering this routines, assume FingersDistance[][] was filled.
    // q/q1 usually point to T1 information, p usually point to T0 information.
    U16 data dx, dy;
    U8 data tn1_fingers; //JStest
    //U16 data min_distance[MAX_FINGERS];
    U16 data min_distance[FW_SUPPORT_MAX_FINGERS]; //JS20130616a : For Library-ize
    U16 data T0_mask, T1_mask;
    U8 data p, q, pass;
    //U8 data min_distance_p[MAX_FINGERS];
    U8 data min_distance_p[FW_SUPPORT_MAX_FINGERS]; //JS20130616a : For Library-ize
    U8 data u8Tmp;
    //U16 T1DistanceToT0n[MAX_FINGERS][MAX_FINGERS]; // Row=T1[m] vs T0[0..n]
    U16 T1DistanceToT0n[FW_SUPPORT_MAX_FINGERS][FW_SUPPORT_MAX_FINGERS]; // Row=T1[m] vs T0[0..n] //JS20130616a : For Library-ize
//  U16 addridx[2];
//  S16 c_delta[2];
//  U8 m;
#if Crovax_debug_TAPPING
    U8 Ci1, Ci2;
#endif
    //-- Calculate P*Q distance matrix.
    //-- And do 1st pass search the closest p from T0.
    T1_mask=(t1_fingers>0) ? F_FULL_MASK[t1_fingers-1] : 0x0;
    T0_mask=(t0_fingers>0) ? F_FULL_MASK[t0_fingers-1] : 0x0;
    for (q=0; q<t1_fingers; q++)
    {
        T1FingersIDList[q]=0xff; // Reset T1 ID list.
        for (p=0; p<t0_fingers; p++)
        { //Calculate T1 finger-q distance to T0 fingers 0..p. In the same time, do first pass ID matching
            dx=abs(T1FingersPosX[q]-T0FingersPosX[p])>>2 ;
            if (dx<255)
                dx=dx*dx;
            else
                dx=65534;
            dy=abs(T1FingersPosY[q]-T0FingersPosY[p])>>2 ;
            if (dy<255)
                dy=dy*dy;
            else
                dy=65534;
            //if ((dx/2 + dy/2) >= 32767)    // approching 8 sensor cells
            if (( (dx>>1) + (dy>>1) ) >= 32767) // approching 8 sensor cells
            {
                T1DistanceToT0n[q][p]=65534;
            }
            else
            {
                T1DistanceToT0n[q][p]=dx + dy;
            }
        }
    }
    pass=1;
    do // Multi-pass
    {
        for (q=0; q<t1_fingers; q++)
        {
            if (T1_mask & F_POS_MASK[q]) // if 1 means not solved, need to search min. value.
            {
                min_distance[q]=MAX_VALUE;   // Init to farest.
                //min_distance_p[q] = max_ID;
                min_distance_p[q]=0xff;
                for (p=0; p<t0_fingers; p++)
                { //Calculate T1 finger-q distance to T0 fingers 0..p. In the same time, do first pass ID matching
                    if (T0_mask  & F_POS_MASK[p]) // This old finger is available for compare.
                    {
                        if (T1DistanceToT0n[q][p] < min_distance[q])
                        {
                            min_distance[q]=T1DistanceToT0n[q][p];
                            min_distance_p[q]=p;
                        }
                    }
                }
            }
        }
        // Check if p repeated in min_distance_p[q].
        for (q=0 ; q<t1_fingers; q++)
        {
            //if((T1_mask & F_POS_MASK[q]) && (min_distance_p[q]<(U8)max_ID)) // if 1 means not solved, need to search min. value.
            if ((T1_mask & F_POS_MASK[q]) && (min_distance_p[q]<(U8)0xff)) // if 1 means not solved, need to search min. value.
            {
                for (p=q+1; p<t1_fingers; p++) // Search behind IDs to check if id repeated.
                {
                    if (T1_mask & F_POS_MASK[p]) // The q1 fingers id was fixed ? if yes, don't need compare q to q1.
                    { // The q1 fingers was not fixed, so compare both.
                        if (min_distance_p[q]==min_distance_p[p]) // p repeated !
                        { // When id repeated, compare and find the bigger, kill the bigger.
                            if (min_distance[q] > min_distance[p]) // Kill q.
                            {
                                min_distance_p[q]=0xff  ; // q de-assigned, next pass check again.
                                break; // Because q wae de-assigned, so skip this q loop.
                            }
                            else                                            // Kill q1
                            {
                                min_distance_p[p]=0xff ; // q1 de-assigned, next pass check again.
                            }
                        }
                    }
                }
                u8Tmp=min_distance_p[q];
                if (u8Tmp < max_ID) // No p repeated or it is the closest.
                { // then assign this id to fixed.
                    T1_mask&=~F_POS_MASK[q];   // Clear mask bit.
                    T0_mask&=~F_POS_MASK[u8Tmp];
                    T1FingersIDList[q]=T0FingersIDList[u8Tmp]; // From p get old ID.
                    T0FingersIDList[u8Tmp]=0xff;   // Clear old finger's ID.
                    if (T1FingersIDList[q]< max_ID)
                    {
                        IdFlags|=F_POS_MASK[T1FingersIDList[q]];
                    }
                    //CalculateVector(T1FingersIDList[q], q);
                }
            }
        }
        pass++;
    }
    while (T1_mask && T0_mask);
#if 1
    // process 2 3 2 issue
    //   Twokey added 0912 -- start
    T0_mask=0;
    T1_mask=0;
    tn1_fingers=lastTouchFingersList_2->fingers_number;
#if Twokey_debug_MatchFingers
    printf("Figs: %2bu, %2bu, %2bu\n\r", tn1_fingers, t0_fingers, t1_fingers);
#endif
    if ((tn1_fingers==t1_fingers) &&(t0_fingers==t1_fingers+1))
    {
        for (q=0; q<t1_fingers; q++)
        {
            for (p=0; p<tn1_fingers; p++)
            {
                if (TouchFingersList->ID[q] == lastTouchFingersList_2->ID[p])
                {
                    T0_mask|=(0x01<<p);
                    T1_mask|=(0x01<<q);
                    break;
                }
            }
        }
        //printf("Fid: %4d, %4d\n\r", T0_mask, T1_mask );
        // check IDno in t1 the same as t-1
        if (T1_mask <  F_FULL_MASK[t1_fingers-1] )
        {
            p=0;
            while ((T0_mask&0x01) && (p<t0_fingers))
            {
                T0_mask=T0_mask>>1;
                p++;
            }
            q=0;
            while ((T1_mask&0x01) && (q<t1_fingers))
            {
                T1_mask=T1_mask>>1;
                q++;
            }
#if Twokey_debug_MatchFingers
            printf("Fige: (%2bu->%2bu)%2bu, %2bu, %2bu\n\r",TouchFingersList->ID[q],lastTouchFingersList_2->ID[p],tn1_fingers,t0_fingers,t1_fingers);
#endif
            TouchFingersList->ID[q]=lastTouchFingersList_2->ID[p];
            u8Tmp=TouchFingersList->ID[q];
            p=0;
            while ((lastTouchFingersList_1->ID[p]!=u8Tmp)&&(p<t0_fingers))
            {
                p++;
            }
            /*
               for (p=0; p<t0_fingers; p++)
               {
                if(lastTouchFingersList_1->ID[p]==u8Tmp)
                {
                        break;
                }
               }
             */
            min_distance[q]=T1DistanceToT0n[q][p];
            min_distance_p[q]=p;
#if Twokey_debug_MatchFingers
            printf("q=%bu, p=%bu, MD:%4d\n\r", q, p, min_distance[q]);
#endif
        }
    }
    //  Twokey added 0912 --- end
#endif
#ifdef TAPPING
    // for tapping solution  , --- Addpoint
#if 1
    p=t1_fingers;
    for (q=0; q<p; q++)
    {
        if ((min_distance[q] > flSystemPara.Tapping_distance ) && (min_distance_p[q] < max_ID) )
        {
#if Crovax_debug_TAPPING
            printf( "** ID %bu Tapping! ***************************************************\n\r", min_distance_p[q] );
            printf("T0:\t");
            for ( Ci1=0; Ci1 < t0_fingers; Ci1++ )
                printf("(%4d, %4d)\t", T0FingersPosX[Ci1], T0FingersPosY[Ci1]);
            printf("\n\rT1:\t");
            for ( Ci2=0; Ci2 < t1_fingers; Ci2++ )
                printf("(%4d, %4d)\t", T1FingersPosX[Ci2], T1FingersPosY[Ci2]);
            printf("\n\rT1DistanceToT0n Array is:\n\r");
            for ( Ci2=0; Ci2 < t1_fingers; Ci2++ )
            {
                for ( Ci1=0; Ci1 < t0_fingers; Ci1++ )
                    printf( "%d\t", T1DistanceToT0n[Ci2][Ci1]);
                printf( "\n\r");
            }
            printf("\n\n");
#endif
            pass=GetNewID();
            if (pass<MAX_FINGERS)
            {
                T1FingersIDList[q]=pass;
            }
#if debug_Charger
            printf("new %03bu,(%04d,%04d),d %04u\r\n",T1FingersIDList[q],T1FingersPosY[q], T1FingersPosX[q],min_distance[q]);
#endif
        }
    }
#endif
#endif
    //} while(pass<4);
}

U8 GetNewID(void)       // Search the ID tables and return the first inactive ID 0~MAX_FINGERS-1. return 0xff if not found.
{
    //U8  data new_id,getMax_id; //JS20130205C XX
    U8 data new_id; //JS20130205C
    U16 tmpflag;
    //getMax_id = max_ID; //JS20130205C XX
    tmpflag=IdFlags |preIdFlags;
    //for(new_id=0; new_id < getMax_id; new_id++) //MAX_FINGERS //JS20130205C XX
    for (new_id=0; new_id < max_ID; new_id++) //MAX_FINGERS //JS20130205C
    {
        if ( (tmpflag & F_POS_MASK[new_id])==0 )
        {
            IdFlags|=F_POS_MASK[new_id];
            break; // Search the first zero flag
        }
    }
    //if(new_id < getMax_id) return (new_id); //MAX_FINGERS //JS20130205C XX
    if (new_id < max_ID) return (new_id); //MAX_FINGERS //JS20130205C
    else return 0xff;
}

/*
   void DynamicsTracking_Init(void)
   {
         //U8  x;
         //IdFlags=0;
        //for(x=0; x<MAX_FINGERS; x++)
        //{
        //  VectorsByID[0][x]= 0;
        //  VectorsByID[1][x]= 0;
        //}
   }
 */
void DynamicsTracking(tFingerList *t0_finger_list, tFingerList *t1_finger_list)
//-- Enter:
// t0_finger_list point to prior fingers information. The ID field was all valid
// t1_finger_list point to new detected fingers information. The ID field was don't care.
//-- Return:
// The t1_finger_list ID field will be filled was proper ID.
// The t0_finger_list ID will be 0xFF if the ID is assigned in T1 IDs. otherwise the ID means a touch-leaving fingers.
//    By this, caller can find out what key is un-touched.
{
    U8 data q; //, p;
    U8 t0_fingers, t1_fingers, new_id;
//  U16 tn1_mask, t1_mask;
    IdFlags=0;
    // Copy struct data to local pointer for speed up computing.
    t0_fingers=t0_finger_list->fingers_number;
    t1_fingers=t1_finger_list->fingers_number;
    //tn1_fingers= lastTouchFingersList_2->fingers_number;
    if (t1_fingers ==0) //no finger ,don't need tracking
        return;
    T0FingersPosY=t0_finger_list->Pos[0];
    T0FingersPosX=t0_finger_list->Pos[1];
    //T0FingersIDList=t0_finger_list->ID;
    for (q=0; q<MAX_FINGERS; q++)
    {
        T0FingersIDList[q]=t0_finger_list->ID[q];
    }
    // Calculate predict position.
    T1FingersPosY=t1_finger_list->Pos[0];
    T1FingersPosX=t1_finger_list->Pos[1];
    T1FingersIDList=t1_finger_list->ID;
    /*
        //-- Calculate predict position.
        for(p=0; p<t0_fingers; p++) // Simulate T0 data
        {
                PredictPos[0][p]=T0FingersPosY[p]+VectorsByID[0][T0FingersIDList[p]]; // Y axis
                PredictPos[1][p]=T0FingersPosX[p]+VectorsByID[1][T0FingersIDList[p]]; // X axis
        }
                // Before enter MatchFingers, T0|1FingerPosY|X & T1FingersIDList must be filled.
        T0FingersPosY = PredictPos[0];  // Used PredictPos as previous position to do match.
        T0FingersPosX = PredictPos[1];
     */
    MatchFingers(t0_fingers, t1_fingers);   // T0_fingers, T1 fingers
    //-- Check T1 new ID. If it is old fingers, calculate vector.
    //--   If it is new finger, look for a new id and reset the vector.
    for (q=0; q< TouchFingersList->fingers_number; q++)
    {
        if (T1FingersIDList[q]<max_ID) // This XY was assigned ID, so it is old finger.
        { // Calculate the vector with old finger pos.
            //VectorsByID[0][T1FingersIDList[q]]= T1FingersPosY[q]-T0FingersPosY[q];
            //VectorsByID[1][T1FingersIDList[q]]= T1FingersPosX[q]-T0FingersPosX[q];
        }
        else
        { // Assign new ID and reset the vector.
            new_id=GetNewID();
            if (new_id!=0xff) {} //0xff=Error, IDs are all used, no available.
            { // Got new ID !
                T1FingersIDList[q]=new_id;
                //VectorsByID[0][q]= 0;
                //VectorsByID[1][q]= 0;
            }
        }
    }
}

/*
   //---- point filter  (use IIR)---
   S16 PointFilter[2][MAX_FINGERS];
   U8 lastIDSel = 0;
   U8 nowIDSel = 0;
   void point_filter(void)
   {
        U8 r =0;
        U8 tmpID;
        nowIDSel = 0;
        if(TFingersList_last->fingers_number==0)
        {
                if( TFingersList_report->fingers_number ==0)
                {
                        //printf("[tim] 0 \r\n");
                        for(r =0;r<MAX_FINGERS;r++)
                        {
                                PointFilter[0][r] =0;
                                PointFilter[1][r] =0;
                        }
                        lastIDSel  =0;
                }
                else
                {
                        for(r =0;r<TFingersList_report->fingers_number;r++) //
                        {
                                tmpID = TFingersList_report->ID[r];
                                PointFilter[0][tmpID] =TFingersList_report->Pos[0][r];
                                PointFilter[1][tmpID] =TFingersList_report->Pos[1][r];
                                lastIDSel  |= F_POS_MASK[tmpID];
                        }
                }
        }
        else  //lastTouchFingersList->fingers_number > 0
        {
                for(r =0;r<MAX_FINGERS;r++)
                {
                        tmpID = TFingersList_report->ID[r];
                        if(tmpID!=0xff)
                        {
                                if( (F_POS_MASK[tmpID] & lastIDSel)!=0)
                                {
                                        PointFilter[0][tmpID] =PointFilter[0][tmpID] + ((TFingersList_report->Pos[0][r] -PointFilter[0][tmpID]  )>>point_filter_vector);
                                        PointFilter[1][tmpID] =PointFilter[1][tmpID] + ((TFingersList_report->Pos[1][r] -PointFilter[1][tmpID] )>>point_filter_vector);
                                        nowIDSel |= F_POS_MASK[tmpID];
                                }
                                else
                                {
                                        PointFilter[0][tmpID] =TFingersList_report->Pos[0][r];
                                        PointFilter[1][tmpID] =TFingersList_report->Pos[1][r];
                                        nowIDSel |= F_POS_MASK[tmpID];
                                }
                        }
                }
                for(r =0;r<MAX_FINGERS;r++)
                {
                        if((F_POS_MASK[r] & lastIDSel ==1) &&(F_POS_MASK[r] & nowIDSel ==0 ))
                        {
                                        PointFilter[0][r] =0;
                                        PointFilter[1][r] =0;
                        }
                }
                lastIDSel = nowIDSel;
        }
   }
 */
//#################################################
#if JITTER_for_INNOS_Poly
//JS20130616a : Start :: Modify from MAX_FINGERS to FW_SUPPORT_MAX_FINGERS
S16 JT_1stXpoint[FW_SUPPORT_MAX_FINGERS];
S16 JT_1stYpoint[FW_SUPPORT_MAX_FINGERS];
S16 JT_dxdy[FW_SUPPORT_MAX_FINGERS];
U8 JT_PointEnter[FW_SUPPORT_MAX_FINGERS];
U8 JT_LockORnot[FW_SUPPORT_MAX_FINGERS];
U8 JT_StaticCounter[FW_SUPPORT_MAX_FINGERS];

void Jitter_IIR_Init(tFingerIDList *FingerID_list_output)
{
    U8 ii;
    for (ii=0; ii<MAX_FINGERS; ii++)
    {
        if (FingerID_list_output->Touch_Info[ii] == 0)
        {
            JT_1stXpoint[ii]=0;
            JT_1stYpoint[ii]=0;
            JT_dxdy[ii]=0;
            JT_PointEnter[ii]=0;
            JT_LockORnot[ii]=1;
            JT_StaticCounter[ii]=0;
        }
    }
}

//JS20130710a copy from Poly's new Jitter_IIR function call
void Jitter_IIR(tFingerIDList *FingerID_list_output)
{
    S16 data  Xpoint,Ypoint; //,IIRXpoint,IIRYpoint;
    U8 ii;
    U16 tmpLOCKTHforChargerOn,tmpUNLOCKTHforChargerOn,tmpStatic;
    U8 iir_ratio;
//    if (charger_on_status == 1)
//    {
//        tmpLOCKTHforChargerOn = (flJitterIIRPara.Lock_TH_Normal * flJitterIIRPara.CHR_TH_Scale_Ratio);        // only for charger
//        tmpUNLOCKTHforChargerOn = (flJitterIIRPara.Unlock_TH_Normal * (flJitterIIRPara.CHR_TH_Scale_Ratio >> 1)); // only helf ratio of LOCK
//        tmpStatic = flJitterIIRPara.StaticCounterTimes >> (flJitterIIRPara.CHR_TH_Scale_Ratio>>1);
//    }
//    else
    {
        tmpLOCKTHforChargerOn = flJitterIIRPara.Lock_TH_Normal;
        tmpUNLOCKTHforChargerOn = flJitterIIRPara.Unlock_TH_Normal;
        tmpStatic = flJitterIIRPara.StaticCounterTimes;
    }

#if BIGFingerProcess
  if ((fgBIGFinger == 1) && (FingerIDList_Output.fingers_number >= 2))
    {
        tmpLOCKTHforChargerOn = BIGFingerLOCK ;       // 150
        tmpUNLOCKTHforChargerOn = BIGFingerUnLOCK;      // 100
        tmpStatic = flJitterIIRPara.StaticCounterTimes >> 2;
    }
#endif

    for (ii=0; ii<MAX_FINGERS; ii++)
    {
//#if !TP_CUSTOMER_PROTOCOL
        if (FingerID_list_output->Touch_Info[ii] == 1)    //standard
//#else
//  if ((FingerID_list_output->Touch_Info[ii] == 1) && (RO_Queue[ii] > RO_DelayLatency))  // lib
//#endif
        {
            Ypoint = FingerID_list_output->Pos[0][ii];
            Xpoint = FingerID_list_output->Pos[1][ii];

            if (JT_PointEnter[ii] >= 1)
            {
                JT_dxdy[ii] = (abs(JT_1stYpoint[ii]-Ypoint)) + (abs(JT_1stXpoint[ii]-Xpoint));    // distance

                if (JT_dxdy[ii] >tmpLOCKTHforChargerOn)
                  {
          JT_LockORnot[ii] = 0;
          iir_ratio = 2;
                  }
    else
      {
          iir_ratio = 4;
      }
      Ypoint = JT_1stYpoint[ii]+((Ypoint-JT_1stYpoint[ii])>>iir_ratio);   // /4
      Xpoint = JT_1stXpoint[ii]+((Xpoint-JT_1stXpoint[ii])>>iir_ratio);   // /4
            }

            if (JT_PointEnter[ii] == 0)
            {
                JT_PointEnter[ii]=1;    // pont enter counter
            }

            if (JT_StaticCounter[ii] > tmpStatic)
            {
                JT_LockORnot[ii] = 1;
                JT_PointEnter[ii] = 1;
                JT_StaticCounter[ii] = 0;
            }
            if (JT_LockORnot[ii]==0)    // unlock mode
            {
                if ( JT_PointEnter[ii] >= 2 )     // 2~6, 5 points.  How many points need to do IIR filter?
                {
                    if (JT_dxdy[ii] >= tmpUNLOCKTHforChargerOn)
                    {
                        JT_StaticCounter[ii] = 0;             
//                     Ypoint = JT_1stYpoint[ii]+((Ypoint-JT_1stYpoint[ii])>>iir_ratio);    // /4
//                      Xpoint = JT_1stXpoint[ii]+((Xpoint-JT_1stXpoint[ii])>>iir_ratio);   // /4
                      // update
                     JT_1stYpoint[ii] = Ypoint;     // 1<- now
                      JT_1stXpoint[ii] = Xpoint;      // 1<- now
                      JT_PointEnter[ii] = 99;
                      }
      else
      {
        JT_StaticCounter[ii]++;
      }
                }
            }
            else              // lock mode
            {
                if (JT_PointEnter[ii] == 1)
                {
                    JT_PointEnter[ii] = 2;
                    JT_1stYpoint[ii] = Ypoint;        // 1<- now
                    JT_1stXpoint[ii] = Xpoint;        // 1<- now
                }
                else
                {
                    // small than jitter
                    Ypoint = JT_1stYpoint[ii];
                    Xpoint = JT_1stXpoint[ii];
                }
            }
            FingerID_list_output->Pos[0][ii] = Ypoint;
            FingerID_list_output->Pos[1][ii] = Xpoint;
        }
    }
}
#endif
//#################################################
#if  PointGitterReject
S16 preXpoint[max_ID]; // array [2] is two figner
S16 preYpoint[max_ID]; // array [2] is two figner
S16 pre2Xpoint[max_ID]; // array [2] is two figner
S16 pre2Ypoint[max_ID]; // array [2] is two figner
U8 AdjustFlag[max_ID];     // [0:2]: Y delta input equal count
//   [3]: output new Y delta
// [4:6]: X delta input equal count
//   [7]: output new X delta
#define point_Threshold 16 //16
#define Equal_Threshold_Y 3
#define Equal_Threshold_X (Equal_Threshold_Y<<4)
void GitterReject_paraReset(tFingerList *finger_list)
{
    U8 data RestFlag[(max_ID>>3)+1];
    U8 ii,_byte,_bit;
    for (ii=0; ii<(max_ID>>3)+1; ii++) RestFlag[ii]=0;
    for (ii=0; ii<finger_list->fingers_number; ii++) // mark finger ID
    {
        if (finger_list->ID[ii]<max_ID)
        {
            _byte=finger_list->ID[ii]>>3;
            _bit=finger_list->ID[ii]&0x7;
            RestFlag[_byte]|=BIT8_MASK[_bit];
        }
    }
    for (ii=0; ii<max_ID; ii++)
    {
        _byte=ii>>3;
        _bit=ii&0x7;
        if ((RestFlag[_byte] & BIT8_MASK[_bit]) ==0) // the ID no finger
        {
            pre2Ypoint[ii]=0;
            pre2Xpoint[ii]=0;
            preYpoint[ii]=0;
            preXpoint[ii]=0;
            AdjustFlag[ii]=0;
        }
    }
    return;
}

void GitterReject(tFingerList *finger_list)
{
    U8 data TempFlag ;
    U8 data pos_ptr,ii;
    S16 data Xpoint,Ypoint;
    for (ii=0; ii<finger_list->fingers_number; ii++)
    {
        Xpoint=finger_list->Pos[1][ii];
        Ypoint=finger_list->Pos[0][ii];
        pos_ptr=finger_list->ID[ii];
        if (pos_ptr>MAX_FINGERS)
        {
            //printf(" %03bu , %03bu\r\n",pos_ptr,finger_list->fingers_number);
            finger_list->ID[ii]=9;
            break;
        }
        // ----- Y -----//
        TempFlag=AdjustFlag[pos_ptr] & 0x0f;                                                                                    //  T0  T1  T2
        if ((preYpoint[pos_ptr]>Ypoint)&&(pre2Ypoint[pos_ptr]>=preYpoint[pos_ptr]))             //  - - -
            TempFlag=8;
        else if ((preYpoint[pos_ptr]<Ypoint)&&(pre2Ypoint[pos_ptr]<=preYpoint[pos_ptr])) // + + +
            TempFlag=8;
        else if (preYpoint[pos_ptr] == Ypoint)
        {
            if (TempFlag < Equal_Threshold_Y) TempFlag++;
        }
        else
            TempFlag=0;
        AdjustFlag[pos_ptr]=(AdjustFlag[pos_ptr] & 0xf0) | TempFlag;
        if ( (abs(preYpoint[pos_ptr]-Ypoint)>point_Threshold) || (TempFlag >= Equal_Threshold_Y)) //  don't process
        {
            pre2Ypoint[pos_ptr]=preYpoint[pos_ptr];
            preYpoint[pos_ptr]=Ypoint;
        }
        else
            Ypoint=preYpoint[pos_ptr] ;   //use preYpoint
        // ----- X -----//
        TempFlag=AdjustFlag[pos_ptr] & 0xf0;
        if (((preXpoint[pos_ptr]>Xpoint)&&(pre2Xpoint[pos_ptr]>=preXpoint[pos_ptr])))
            TempFlag=0x80;
        else if ((preXpoint[pos_ptr]<Xpoint)&&(pre2Xpoint[pos_ptr]<=preXpoint[pos_ptr]))
            TempFlag=0x80;
        else if (preXpoint[pos_ptr]==Xpoint)
        {
            if (TempFlag < Equal_Threshold_X) TempFlag+=0x10;
        }
        else
            TempFlag=0;
        AdjustFlag[pos_ptr]=(AdjustFlag[pos_ptr]&0x0f) | TempFlag;
        if ( (abs(preXpoint[pos_ptr]-Xpoint)>point_Threshold) || (TempFlag >= Equal_Threshold_X))
        {
            pre2Xpoint[pos_ptr]=preXpoint[pos_ptr];
            preXpoint[pos_ptr]=Xpoint;
        }
        else
            Xpoint=preXpoint[pos_ptr] ;
        //
        finger_list->Pos[1][ii]=Xpoint;
        finger_list->Pos[0][ii]=Ypoint;
    }
}

#endif
//-----
#define NormalToFloating_TH 100
#define FloatingToNormal_TH 400
#if FLOATMODE
U8 sim_floating;
U8 nMutualTHFlag1;
U8 MutualTHFlag2;
U8 mutualMode; // 0 : normal , 1: floating mode
U8 mutualModeCount ;
#define NormalMode 0
#define FloatingMode 1
// 5.
void InitMutualDeltaFlag()
{
    nMutualTHFlag1=0;
    MutualTHFlag2=0;
}

void checkMutualDelta(S16 delta)
{
    if (mutualMode == NormalMode) // normal MODE
    {
        if (delta>FloatingPara.Mutual_TH1)      {            nMutualTHFlag1=1;        }
    }
    else                    // floating MODE
    {
        if (delta>FloatingPara.Mutual_TH2)      {            MutualTHFlag2=1;        }
    }
    return;
}

#define sim_toFloating 1
#define sim_toNormal 2
//U8 sim_floating;
void isSwitchMode()
{
    //NoReport = 1;
    if (mutualMode== NormalMode) // normal mode
    {
        if (!(sim_floating&sim_toNormal))
        {
            if (nMutualTHFlag1 == 0 || (sim_floating&sim_toFloating))
            {
                mutualModeCount++;
                NoReport=1;
                if (mutualModeCount == 2)
                {
                    mutualModeCount=0;
                    MutualScanInit(mutu_sequ,FloatingPara.MutualPluseCount,flRawPara.MutualPulseLen,flRawPara.Mutu_fb);
                    mutualMode=FloatingMode;
                    //  reset baseline
                    //TxNumber = 0;
                    bPowerOnMutual[freqIndex] = 1;
                    isMuBaselineRdy=0;
                }
            }
            else
            {
                mutualModeCount=0;
            }
        }
    }
    else    //floating mode
    {
        if (!(sim_floating&sim_toFloating))
        {
            if (MutualTHFlag2 ==1 ||  (sim_floating&sim_toNormal))
            {
                mutualModeCount++;
                NoReport=1;
                if (mutualModeCount == 2)
                {
                    mutualModeCount=0;
                    MutualScanInit(mutu_sequ,flRawPara.MutualPulseCount,flRawPara.MutualPulseLen,flRawPara.Mutu_fb);
                    mutualMode=NormalMode;
                    //  reset baseline
                    //TxNumber = 0;
                    bPowerOnMutual[freqIndex] = 1;
                    isMuBaselineRdy=0;
                }
            }
            else
            {
                mutualModeCount=0;
            }
        }
    }
    return;
}

#endif
#if 1
U8 fnIsOnEdge(U16 Y, U16 X,U8 mode)
{
    /*  mode : 0 => X & Y are peak
     *  mode : 1 => X & Y are point address
     *  --------------------------------
     *      return 0x0 = on center
     *      return 0x1 = on Y0 Edge
     *      return 0x2 = on Y last Edge
     *      return 0x4 = on X0 Edge
     *      return 0x8 = on X last Edge
     */
    U8 uOnEdge=0;
    if (mode==1)
    {
        X/=RES_SCALE;
        Y/=RES_SCALE;
    }
    if (Y==0)
        uOnEdge=0x1;
    else if (Y==(AA_TxLen-1))
        uOnEdge=0x2;
    if (X==0)
        uOnEdge|=0x4;
    else if (X==(AA_RxLen-1))
        uOnEdge|=0x8;
    return uOnEdge;
}

#endif
#if ISOTRI_CANCEL       // Crovax 2013-03-29
S16 CalMutualEdgeAddress( U8 Ypeak, U8 Xpeak, U8 fgAxis )
{
    U8 PeakCh, SumDir, BandDir;
    S8 i, j, idxStr, idxEnd;
    S16 tmpIdx, tmpVal, ReturnPos;
    S16 tmpBand[3];
    if ( fgAxis == 0 )
    {
        PeakCh=Ypeak;
        idxStr=( Xpeak == 0                       ) ? 0 : -1;
        idxEnd = ( Xpeak == AA_RxLen-1 )? 1 :  2;

        if ( Ypeak == 0 ) tmpIdx=Xpeak;
        else        tmpIdx = (Ypeak-2)*Total_RX + Xpeak;

        SumDir=1;
        BandDir = Total_RX;
    }
    else
    {
        PeakCh=Xpeak;
        idxStr=( Ypeak == 0                       ) ? 0 : -1;
        idxEnd = ( Ypeak == AA_TxLen-1 )? 1 :  2;

        if ( Xpeak == 0 ) tmpIdx = Ypeak*Total_RX + Xpeak;
        else        tmpIdx = Ypeak*Total_RX + Xpeak-2;

        SumDir  = Total_RX;
        BandDir=1;
    }
    for ( i=0; i<3; i++ )
    {
        tmpBand[i]=0;
        for ( j=idxStr; j<idxEnd; j++)
        {
            tmpVal=ptrLastQuaDelta[tmpIdx+(j*SumDir)];
            tmpBand[i]=tmpBand[i] + ((tmpVal>0) ? tmpVal : 0);
        }
        tmpIdx+=BandDir;;
    }
    ReturnPos=EdgeWeightingExtend(tmpBand, PeakCh, 1, fgAxis);
    return ReturnPos;
}

#endif
//-------Edge Algorithm start----------------------------
//
//  IsolateTriangle Algorithm with Regrassion Analysis Edge Prediction
//
//      ReturnY = IsolateTriangleRA( InputBand, PeakCh, ChannelNumbers )
//
//      ReturnY         : Output Coordinate
//      InputBand       : Whole Channel Band.
//      PeakCh          : Peak Channel.
//      ChannelNumbers  : Channel Number of Specified Channel.
//      Prediction      : Generated predicted channel;
//      RES_SCALE       : Resolution Scale, normally are 64.
//
//---------------------------------------------------------------------------
//U8 bEdgeReport0       = 0;
#if JITTER_6
U8 JitterFixIdx;
#endif
#if !(SELF_WEIGHTING)
S16 IsolateTrianglePure( S16 *InputBand,U8 PeakCh)  //tim Modify orginal Crovax's code
{
    //S16  ReturnY, TmpVal1, TmpVal2; //JStest
    S16 ReturnY;
    S16 data TmpVal1, TmpVal2; //JStest to data
    if ( InputBand[2] > InputBand[0])
    {
        TmpVal1=(InputBand[1]-InputBand[2])>>1;
        TmpVal2=((InputBand[1]-InputBand[0])>>1) +1  ;
        ReturnY=(PeakCh+1) * RES_SCALE - ( ( RES_SCALE >> 2 ) * TmpVal1 + ( TmpVal2 >> 2 ) ) / (( TmpVal2 >>1 )+1);
    }
    else
    {
        TmpVal1=(InputBand[1]-InputBand[0])>>1 ;
        TmpVal2=(InputBand[1]-InputBand[2]>>1) + 1;
        ReturnY=PeakCh * RES_SCALE + ( ( RES_SCALE >> 2) * TmpVal1 + ( TmpVal2 >> 2 ) ) / (( TmpVal2 >> 1 )+1);
    }
    return ReturnY;
}

#endif
////////////////////////////////////////
// Seg = 0: coeff0 =>  56 ~  (0) ~  7 //
// Seg = 4: coeff1 =>   8 ~ (16) ~ 23 //
// Seg = 8: coeff2 =>  24 ~ (32) ~ 39 //
// Seg =12: coeff3 =>  40 ~ (48) ~ 55 //
////////////////////////////////////////
#if (!ISOTRI_CANCEL)
#if (!LAZY_EDGE )
U8 SegChk( S16 PreAxis ) // Search the position segment to determine coefficient.
{
    U8 Seg;
    Seg=(PreAxis+8) & 0x30;   // 0x3f
    return (Seg>>(4-2));
}

#endif
//-------------------------------------------------------------------------------------------
// The Version of IslateTriangleRA is for Full-Mutual Mode, please don't use it to self mode.
//-------------------------------------------------------------------------------------------
//
// DirSel : Direction of index. For example, DirSel = 3 if we process coordinate Y.
// SideChk  : The value was used to calcule max-two band combine direction.
//
//-------------------------------------------------------------------------------------------
S16 IsolateTriangleRA( S16 *TmpDelta, U8 InputX, U8 InputY, S16 RefPts, U8 fgAxis)
{
    U8 data TarNums, TarEdge, RefNums, RefEdge, i, j, k, EdgeChk;
    S8 data TarPeak, RefPeak, TarDir, RefDir;
    S16 data ReturnPos, TmpIdx;
    S16 data TmpBand[3]; // EdgeTmpBand[3];
    // -- Edge Method Selected
#if LAZY_EDGE
    S16 data TmpIdx2; // TmpFix
#else
    U8 data DiaSeg;
    long *Edgecoeff_ptr;
#endif
    // -- End
    /*
       U8   data PeakCh, RefPeak, ChNums, RefNums, EdgeChk, i, j, k;
       S8 data DirSel, SideChk;
       S8 data TmpIdx, TmpIdx2;
       S16 data ReturnY;
       S16 data TmpBand[3];
     */
    TmpBand[0]=RefPts;
    EdgeChk=fnIsOnEdge( ((U16)InputY), ((U16)InputX), 0);
    //
    if (fgAxis)
    {
        TarPeak=InputX;
        RefPeak=InputY;
        TarNums=AlgorithmParam.h_cols;
        RefNums=AlgorithmParam.v_rows;
        TarDir=1;
        RefDir=3;
        TarEdge=EdgeChk&0xc;
        RefEdge=EdgeChk&0x3;
    }
    else
    {
        TarPeak=InputY;
        RefPeak=InputX;
        TarNums=AlgorithmParam.v_rows;
        RefNums=AlgorithmParam.h_cols;
        TarDir=3;
        RefDir=1;
        TarEdge=EdgeChk&0x3;
        RefEdge=EdgeChk&0xc;
    }
#if !LAZY_EDGE
    Edgecoeff_ptr=EdgecoeffMutual;
#endif
    //--Determine grid position
    //GridX = ( InputX == 0 ) ? 0 : ( ( InputX == AlgorithmParam.h_cols-1) ? 2 : 1 );
    //GridY = ( InputY == 0 ) ? 0 : ( ( InputY == AlgorithmParam.v_rows-1) ? 2 : 1 );
    //TmpIdx  = GridY*3+GridX;
    //--End
    //--Determine grid position
    TmpIdx=4;
    if ( RefPeak == 0 ) TmpIdx-=RefDir;
    else if ( RefPeak == (RefNums-1) ) TmpIdx+=RefDir;
    if ( TarPeak == 0 ) TmpIdx-=TarDir;
    else if ( TarPeak == (TarNums-1) ) TmpIdx+=TarDir;
#if JITTER_6
    JitterFixIdx=TmpIdx;
#endif
    //--End
    if ( TarEdge == 0 )
    {
#if MUTUAL_MAX_TWO
        if ( RefEdge == 0 )
        {
            if ( TmpDelta[TmpIdx-RefDir] > TmpDelta[TmpIdx+RefDir])
                RefDir=-RefDir;
        }
        else // Modified in 20121109
        {
            if ( RefPeak != 0 )
                RefDir=-RefDir;
        }
        TmpBand[0]=(TmpDelta[TmpIdx-TarDir]+TmpDelta[TmpIdx-TarDir+RefDir]+1)>>1;
        TmpBand[1]=(TmpDelta[TmpIdx]               +TmpDelta[TmpIdx               +RefDir]+1)>>1;
        TmpBand[2]=(TmpDelta[TmpIdx+TarDir]+TmpDelta[TmpIdx+TarDir+RefDir]+1)>>1;
#else
        TmpBand[0]=TmpDelta[TmpIdx-TarDir];
        TmpBand[1]=TmpDelta[TmpIdx       ];
        TmpBand[2]=TmpDelta[TmpIdx+TarDir];
#endif
#if ISOTRI_PEAK_ENSURE
        if ( TmpBand[0] > TmpBand[1] ) TmpBand[0]=TmpBand[1]-1;
        if ( TmpBand[2] > TmpBand[1] ) TmpBand[2]=TmpBand[1]-1;
#endif
    }
    else // Edge Area
    {
#if LAZY_EDGE
        RefDir=abs(RefDir);
        if ( RefEdge == 0 )
        {
            if (TmpDelta[TmpIdx-RefDir]>TmpDelta[TmpIdx+RefDir] )
                RefDir=-RefDir;
        }
        else
        {
            if ( RefPeak != 0 )
                RefDir=-RefDir;
        }
        TmpIdx=( TarPeak == 0 ) ? TmpIdx : TmpIdx - (TarDir)*2;
#if 0
        if ( RefEdge == 0 )
        {
            for ( i=0; i < 3; i++ )
            {
                TmpBand[i]=( TmpDelta[TmpIdx+(TarDir*i)]+
                             TmpDelta[TmpIdx+(TarDir*i)-RefDir]+
                             TmpDelta[TmpIdx+(TarDir*i)+RefDir]+ 1 ) /3;
            }
        }
        else
        {
            TmpBand[0]=(TmpDelta[TmpIdx                       ]+TmpDelta[TmpIdx                       +RefDir])/2;
            TmpBand[1]=(TmpDelta[TmpIdx+TarDir        ]+TmpDelta[TmpIdx+TarDir        +RefDir])/2;
            TmpBand[2]=(TmpDelta[TmpIdx+(TarDir*2)]+TmpDelta[TmpIdx+(TarDir*2)+RefDir])/2;
        }
#else
        for ( i=0; i < 3; i++ )
        {
            TmpIdx2=TmpIdx+(TarDir*i);
            TmpBand[i]=((TmpDelta[TmpIdx2]>0) ? TmpDelta[TmpIdx2] : 0);
            TmpIdx2=TmpIdx2+RefDir;
            if ( RefEdge == 0 )
            {
                k=2;
            }
            else
            {
                k=1;
            }
            for ( j=0; j<k; j++)
            {
                if ( TmpDelta[TmpIdx2] > 0)
                {
                    TmpBand[i]+=TmpDelta[TmpIdx2];
                }
                TmpIdx2-=2*RefDir;
            }
            TmpBand[i]=TmpBand[i]>>2;
        }
#endif
        /*
           TmpBand[0]  = (TmpDelta[TmpIdx      ]+TmpDelta[TmpIdx       +SideChk])/2;
           TmpBand[1]  = (TmpDelta[TmpIdx+DirSel   ]+TmpDelta[TmpIdx+DirSel  +SideChk])/2;
           TmpBand[2]  = (TmpDelta[TmpIdx+(DirSel*2)]+TmpDelta[TmpIdx+(DirSel*2)+SideChk])/2;
         */
#else
        TmpIdx=( PeakCh == 0 ) ? TmpIdx : TmpIdx - TarDir*2;
        TmpBand[0]=TmpDelta[TmpIdx                   ];
        TmpBand[1]=TmpDelta[TmpIdx+TarDir    ];
        TmpBand[2]=TmpDelta[TmpIdx+(TarDir*2)];
#endif
#ifdef ISOTRI_PEAK_ENSURE
        if ( TarPeak==0 )
        {
            if (TmpBand[0]<TmpBand[1])
                TmpBand[1]=TmpBand[0]-1;
        }
        else
        {
            if (TmpBand[2]<TmpBand[1])
                TmpBand[1]=TmpBand[2]-1;
        }
#endif
    }
    if ( TarEdge == 0 )
    {
        ReturnPos=IsolateTrianglePure( TmpBand, TarPeak );
#ifdef GAMMA_TABLE
        if (ScanMode==SELF_MODE ) // bending and mutual mode don't enable
        {
            if (fgAxis == 0)
                ReturnPos+=((S16) VGammaTable[ReturnPos&0x3f]);    // Y_Axis
            else
                ReturnPos+=((S16) HGammaTable[ReturnPos&0x3f]);    // X_Axis
        }
#endif
    }
#if LAZY_EDGE // Calculate edge by weighting-sum
    else
    {
#if 0
        // ** Delta Modify  ** Start
        // -- Delta modify, substate delta by TmpFix which calculate by peticuler ration.
        // -- EdgeCoeffWeight was the dividend of ratio which segmented by 64 degree.
        //
        if ( TarPeak == 0 ) TmpFix=( ( (TmpBand[0]+TmpBand[1]) * ( (fgAxis==1) ? EdgeCoeffWeight[0] : EdgeCoeffWeight[2] ) ) >>6  );
        else TmpFix=( ( (TmpBand[2]+TmpBand[2]) * ( (fgAxis==1) ? EdgeCoeffWeight[1] : EdgeCoeffWeight[3] ) ) >>6  );
        if ( TmpFix > 0 )
        {
            TmpBand[0]=TmpBand[0] - TmpFix;
            TmpBand[1]=TmpBand[1] - TmpFix;
            TmpBand[2]=TmpBand[2] - TmpFix;
            if ( TmpBand[0] < 0 ) TmpBand[0]=0;
            if ( TmpBand[1] < 0 ) TmpBand[1]=0;
            if ( TmpBand[2] < 0 ) TmpBand[2]=0;
        }
        // <- Delta Modify -- End
#endif
#if 0
        if ( TarPeak == 0 )
        {
            if ( (TmpBand[0]==0) && (TmpBand[1]==0) )
            {
                ReturnPos=0;
            }
            else
            {
                EdgeTmpBand[0]=( TmpBand[0]+4 ) >> 3;
                EdgeTmpBand[1]=( TmpBand[1]+4 ) >> 3;
                ReturnPos=(( EdgeTmpBand[0] *32 + EdgeTmpBand[1] *96 + ( (EdgeTmpBand[0]+EdgeTmpBand[1])>>1))<<1) / ( EdgeTmpBand[0]+EdgeTmpBand[1] );
                ReturnPos=ReturnPos - RES_SCALE;
            }
        }
        else
        {
            if ( (TmpBand[1]==0) && (TmpBand[2]==0) )
            {
                ReturnPos=RES_SCALE;
            }
            else
            {
                EdgeTmpBand[1]=( TmpBand[1]+4 ) >> 3;
                EdgeTmpBand[2]=( TmpBand[2]+4 ) >> 3;
                ReturnPos=( ( EdgeTmpBand[1] *32 + EdgeTmpBand[2] *96 + ( (EdgeTmpBand[1]+EdgeTmpBand[2])>>1))<<1) / ( EdgeTmpBand[1]+EdgeTmpBand[2] );
                ReturnPos=ReturnPos-(RES_SCALE<<1);
            }
        }
        if ( ReturnPos<0 ) ReturnPos=0;
        else if (ReturnPos>64) ReturnPos=64;
        ReturnPos+=TarPeak*RES_SCALE;
#else
        ReturnPos=EdgeWeightingExtend(TmpBand, TarPeak, 1, fgAxis);
#endif
    }
#else  // Calculate edge by regression
    else if ( !fgAxis ) // Y-Axis
    {
        // -- Corner Enhance
        if ( ( RefPts < KEEP_CORNER ) || ( RefPeak == 0 ) )
            DiaSeg=16;
        else if ( ( RefPts >= (tp_header.RES_X-KEEP_CORNER) ) || ( RefPeak==  AlgorithmParam.h_cols-1) )
            DiaSeg=20;
        else
            DiaSeg=SegChk( RefPts );
        // -- End Corner Enhance
        if ( PeakCh == 0 ) // Do Y-Axis = 0
        {
            ReturnPos=edgepos_calculation(TmpBand, TarPeak, &Edgecoeff_ptr[DiaSeg] );
        }
        else // Do Y-Axis = AlgorithmParam.v_rows-1
        {
            ReturnPos=edgepos_calculation(TmpBand, TarPeak, &(Edgecoeff_ptr[24 + DiaSeg]));
        }
    }
    else    // Do X-Axis
    {
        //if ( ( reference_point < KEEP_CORNER ) || ( reference_peak == 0 ) )
        if ( RefPeak== 0 ) // 2012.08.23 Modified
            DiaSeg=16;
        //else if ( ( reference_point >= 1088 - KEEP_CORNER ) || ( reference_peak == AlgorithmParam.v_rows-1 ) )
        else if ( RefPeak == AlgorithmParam.v_rows-1 )  // 2012.08.23 Modified
            DiaSeg=20;
        else
            DiaSeg=SegChk(RefPts);
        if ( PeakCh == 0 )
        {
            ReturnPos=edgepos_calculation(TmpBand, TarPeak, &(Edgecoeff_ptr[48 + DiaSeg]));
        }
        else // Do Tail Channel
        {
            ReturnPos=edgepos_calculation(TmpBand, TarPeak, &(Edgecoeff_ptr[72+DiaSeg]));
        }
    }
#endif
    return ReturnPos;
}

#endif
#if 1
// CR20130327-01
//-------------------------------------------------------------------------------------------------
// Edge Weighting Extend
//-------------------------------------------------------------------------------------------------
// The function was available to Self and Mutual.
// The function was used to calculate edge area by weighting sum extension.
// ------------------------------------------------------------------------------------------------
// PeakCh   : Input peak.
// InputBand  : Input delta with sub-channel summation.
// ScanMod    : Self: 0, Mutual: 1
// fgAxis   : Y-Axis: 0, X-Axis: 1
// Edit     : Crovax 2013.03.27
//-------------------------------------------------------------------------------------------------
S16 EdgeWeightingExtend(S16* InputBand, U8 PeakCh, U8 ScanMod, U8 fgAxis )
{
    S16 ReturnPos, TmpFix;
    S16 EdgeTmpBand[3];
    EdgeTmpBand[0]=InputBand[0];
    EdgeTmpBand[1]=InputBand[1];
    EdgeTmpBand[2]=InputBand[2];
    // ** Delta Modify  ** Start
    // -- Delta modify, substate delta by TmpFix which calculate by peticuler ration.
    // -- EdgeCoeffWeight was the dividend of ratio which segmented by 64 degree.
    //
    if ( ScanMod == 0 )
        if ( PeakCh == 0 ) TmpFix=( ( (EdgeTmpBand[0]+EdgeTmpBand[1]) * ( (fgAxis==1) ? EdgeCoeffWeightSelf[0] : EdgeCoeffWeightSelf[2] ) ) >>6  );
        else TmpFix=( ( (EdgeTmpBand[1]+EdgeTmpBand[2]) * ( (fgAxis==1) ? EdgeCoeffWeightSelf[1] : EdgeCoeffWeightSelf[3] ) ) >>6  );
    else
        if ( PeakCh == 0 ) TmpFix=( ( (EdgeTmpBand[0]+EdgeTmpBand[1]) * ( (fgAxis==1) ? EdgeCoeffWeightMutual[0] : EdgeCoeffWeightMutual[2] ) ) >>6  );
        else TmpFix=( ( (EdgeTmpBand[1]+EdgeTmpBand[2]) * ( (fgAxis==1) ? EdgeCoeffWeightMutual[1] : EdgeCoeffWeightMutual[3] ) ) >>6  );
    if ( TmpFix > 0 )
    {
        EdgeTmpBand[0]=EdgeTmpBand[0] - TmpFix;
        EdgeTmpBand[1]=EdgeTmpBand[1] - TmpFix;
        EdgeTmpBand[2]=EdgeTmpBand[2] - TmpFix;
        if ( EdgeTmpBand[0] < 0 ) EdgeTmpBand[0]=0;
        if ( EdgeTmpBand[1] < 0 ) EdgeTmpBand[1]=0;
        if ( EdgeTmpBand[2] < 0 ) EdgeTmpBand[2]=0;
    }
    // <- Delta Modify -- End
    // ** Delta Down Scale ** Start
    // -- When InputDelta > 80, down-scale the delta value by bit-shift 3.
    //
    if ( (EdgeTmpBand[0] > 80) || (EdgeTmpBand[2] > 80))
    {
        EdgeTmpBand[0]=( EdgeTmpBand[0]+4 ) >> 3;
        EdgeTmpBand[1]=( EdgeTmpBand[1]+4 ) >> 3;
        EdgeTmpBand[2]=( EdgeTmpBand[2]+4 ) >> 3;
    }
    // <- Delta Down Scale -- End
    if ( PeakCh == 0 )
    {
        if ( (EdgeTmpBand[0]==0) && (EdgeTmpBand[1]==0) )
        {
            ReturnPos=0;
        }
        else
        {
            ReturnPos=(( EdgeTmpBand[0] *32 + EdgeTmpBand[1] *96 + ( (EdgeTmpBand[0]+EdgeTmpBand[1])>>1))<<1) / ( EdgeTmpBand[0]+EdgeTmpBand[1] );
            ReturnPos=ReturnPos - RES_SCALE;
        }
    }
    else
    {
        if ( (EdgeTmpBand[1]==0) && (EdgeTmpBand[2]==0) )
        {
            ReturnPos=RES_SCALE;
        }
        else
        {
            ReturnPos=( ( EdgeTmpBand[1] *32 + EdgeTmpBand[2] *96 + ( (EdgeTmpBand[1]+EdgeTmpBand[2])>>1))<<1) / ( EdgeTmpBand[1]+EdgeTmpBand[2] );
            ReturnPos=ReturnPos-(RES_SCALE<<1);
        }
    }
    if ( ReturnPos<0 ) ReturnPos=0;
    else if (ReturnPos>64) ReturnPos=64;
    ReturnPos+=PeakCh*RES_SCALE;
    return ReturnPos;
}

#endif
#if (!ISOTRI_CANCEL)
#if (!LAZY_EDGE )
S16  edgepos_calculation(S16 *InputBand, U8 PeakCh, long *ptr_Edgecoeff)
{
    S16 data ReturnVal;
#if EDGE_CROSS
    S16 ModTriY;
    S16 TmpBand[3];
#endif
    ReturnVal=( (long) InputBand[0] *ptr_Edgecoeff[0] +
                (long) InputBand[1] *ptr_Edgecoeff[1] +
                (long) InputBand[2] *ptr_Edgecoeff[2] +
                ptr_Edgecoeff[3] + 128 ) >> 8 ;
#if EDGE_CROSS
    // modify return value using tri-angle method
    TmpBand[1]=InputBand[PeakCh];
    if (PeakCh == 0)
    {
        TmpBand[0]=InputBand[PeakCh+1];
        TmpBand[2]=InputBand[PeakCh+2];
    }
    else
    {
        TmpBand[0]=InputBand[PeakCh-1];
        TmpBand[2]=InputBand[PeakCh-2];
    }
    ModTriY=IsolateTrianglePure( TmpBand, 1 );
    if (PeakCh == 0)
    {
        ModTriY=( 96 - ModTriY + 2 )<<1;                // TwoKey
        if ( ( ModTriY > ReturnVal ) && ( ReturnVal > 10 ) ) ReturnVal=ModTriY;
    }
    else
    {
        ModTriY=( ModTriY - RES_SCALE ) << 1;
        if ( ( ModTriY < ReturnVal ) && ( ReturnVal <(RES_SCALE - 10 ) ) ) ReturnVal=ModTriY;
    }
#endif
    if (ReturnVal < 0)
    {
        ReturnVal=0;
    }
    else if (ReturnVal >= RES_SCALE)
    {                                                                                       // Crovax 20120607
        if (PeakCh!=0) ReturnVal=RES_SCALE-1;   // Crovax 20120607
        else ReturnVal=RES_SCALE;                       // Crovax 20120607
    }
    return (PeakCh*RES_SCALE + ReturnVal);
}

#endif
#endif
//-------Edge Algorithm end----------------------------
#if POINT_PREDICT_ON
void Point_Predict(tFingerIDList *InputIDList)
{
    U8 data fgs, tmpID;
    U8 data y, kk;
    U8 Index_last[MAX_PQBUFFER];
    U8 data tmpStatus;
    S16 Vx[3], Vy[3];
    S16 Ax, Ay;
    S16 Px, Py;
    for ( fgs=0; fgs < MAX_FINGERS; fgs++)
    {
        PredictFingerIDList.Touch_Info[fgs]=InputIDList->Touch_Info[fgs];
        PredictFingerIDList.Pos[0][fgs]=InputIDList->Pos[0][fgs];
        PredictFingerIDList.Pos[1][fgs]=InputIDList->Pos[1][fgs];
        //PredictFingerIDList.Z_Area[fgs] = InputIDList->Z_Area[fgs];
        //PredictFingerIDList.Z_Force[fgs]  = InputIDList->Z_Force[fgs];
        tmpStatus=PointsQueReg.Status[fgs]&0x7;
        if ( ( tmpStatus == 0x7 ) ||  ( tmpStatus == 0x3 ) )
        {
            for (y=0, kk=PointsQueReg.Index[fgs]; y< MAX_PQBUFFER; y++, kk++)
                Index_last[y]=kk % MAX_PQBUFFER;
            for ( y=0; y < (tmpStatus/3); y++)
            {
                Vx[y]=PointsQueReg.Point_buf[fgs].Pos[0][Index_last[7-y]]-PointsQueReg.Point_buf[fgs].Pos[0][Index_last[7-y-1]];
                Vy[y]=PointsQueReg.Point_buf[fgs].Pos[1][Index_last[7-y]]-PointsQueReg.Point_buf[fgs].Pos[1][Index_last[7-y-1]];
            }
            /*
               printf("X:%4d,%4d,%4d -- ", PointsQueReg.Point_buf[fgs].Pos[0][Index_last[7]],
                                                        PointsQueReg.Point_buf[fgs].Pos[0][Index_last[6]],
                                                        PointsQueReg.Point_buf[fgs].Pos[0][Index_last[5]]);
             */
            if ( tmpStatus == 0x07)
            {
                Ax=Vx[0] - Vx[1];
                Ay=Vy[0] - Vy[1];
            }
            else
            {
                Ax=0;
                Ay=0;
            }
            //Ax = ( ( Vx[0] - Vx[1] ) + ( Vx[1] - Vx[2] ) )/2;
            //Ay = ( ( Vy[0] - Vy[1] ) + ( Vy[1] - Vy[2] ) )/2;
            //printf("fgs:%bu, I:(%4d, %4d), ", fgs, InputIDList->Pos[0][fgs], InputIDList->Pos[1][fgs] );
            Px=PointsQueReg.Point_buf[fgs].Pos[0][Index_last[7]] + Vx[0] + Ax;
            Py=PointsQueReg.Point_buf[fgs].Pos[1][Index_last[7]] + Vy[0] + Ay;
            PredictFingerIDList.Pos[0][fgs]=(  Px + InputIDList->Pos[0][fgs] + 1 ) / 2;
            PredictFingerIDList.Pos[1][fgs]=(  Py + InputIDList->Pos[1][fgs] + 1 ) / 2;
            /*
               printf(" P:(%4d,%4d), O:(%4d,%4d), V:(%4d,%4d), A:(%4d,%4d)\n\r", Px, Py,
                                                                                                                                        PredictFingerIDList.Pos[0][fgs],
                                                                                                                                        PredictFingerIDList.Pos[1][fgs],
                                                                                                                        Vx[0], Vy[0], Ax, Ay);
             */
        }
    }
}

#endif

void Mureset()
{

                bPowerOnMutual[0] = 1;
                bPowerOnMutual[1] = 1;
                bPowerOnMutual[2] = 1;
}
void selfEnviroment_cal_flagreset()
{
                selfEnviroment_cal_flag[0] = 0;
                selfEnviroment_cal_flag[1] = 0;
              selfEnviroment_cal_flag[2] = 0;
}

void Sereset()
{

                    bPowerOnSelf[1] = 1;
                    bPowerOnSelf[0] = 1;
                    bPowerOnSelf[2] = 1;

}
void SebaseRyreset()
{
  SelfBaseRy[0]=0;
  SelfBaseRy[1]=0;
  SelfBaseRy[2]=0;
}
void MubaseRyreset()
{
  MutuBaseRy[0]=0;
  MutuBaseRy[1]=0;
  MutuBaseRy[2]=0;



}
void SePOBreset()
{
    selfPowerOnBaseLine[0] = 1;
    selfPowerOnBaseLine[1] = 1;
    selfPowerOnBaseLine[2] = 1;


}

void MuPOBreset()
{
    muPowerOnBaseLine[0] = 1;     // Twokey 0801    // [twokey]
    muPowerOnBaseLine[1] = 1;     // Twokey 0801    // [twokey]
    muPowerOnBaseLine[2] = 1;     // Twokey 0801    // [twokey]
}
U8 BaseLine_Hold;
void CalMutualBaseline(U8 Tx) // [twokey]
{
    U8 data k, j;// BaseLine_Hold;
    U8 data jstr, jend; //JStest
    S16  data base_ptr, base_idx;
    U16 data aa; //JStest
  S16 *pMutualBaseLine;
  U16 *pMBaseLine;
  

    base_ptr =0;
    base_idx = 0;

  if( MutuBaseRy[freqIndex]==0)

  {
    for(k=0;k<3;k++)
    {
          MutualScanStart(pMutuTargetAddr);  // -%%---- 36.4 us
        while (!IsEndOfScan()) {};
      SwitchMutuMemory();
      freqbufnow = ((++freqbufnow)%4); 
      freqhist[freqbufnow] = freqIndex;
    }
    pbT3 = MutualBaseLine[freqIndex];
    pbT2 = pbT3;
    pbT1 = pbT3;
    pbT0 = pbT3;



        MutualScanStart(pMutuTargetAddr);  // -%%---- 36.4 us
    

  }
  


    if ((OpMode[1]  & RUN_FULL_MUTUAL) && MutuBaseRy[freqIndex]>0 )
    {
        muEnviroment_cal_flag[freqIndex] = 1;
    }

    peakmax = 0;
    peakmin = 1000;


  
  pMutualBaseLine = MutualBaseLine[freqIndex];  
    pMBaseLine = MBaseLine[freqIndex];
  
    if (muEnviroment_cal_flag[freqIndex]==1)
    {
        for (aa=0;aa<Mu_nodes;aa++)
        {
#if SignDelta
  #if REDUCE_XRAM
    #if RAW_FILTER
      (S16) pMuRaw_buf_T3[aa] = (pMuRaw_buf_T3[aa] >>2) + (pMuRaw_buf_PRE[aa]>>2)*3;; //add 20140114
      pMuRaw_buf_PRE[aa] = pMuRaw_buf_T3[aa];

      ptrLastQuaDelta[aa] = (pMuRaw_buf_T2[aa]+pMuRaw_buf_T3[aa])<<1;
      base_idx = ptrLastQuaDelta[aa]-(S16)( ((U16)pbT2[aa]+(U16)pbT3[aa]+2)>>1 );
    #else
      ptrLastQuaDelta[aa] = (pMuRaw_buf_T2[aa]+pMuRaw_buf_T3[aa])<<1;
      base_idx = ptrLastQuaDelta[aa]-(S16)( ((U16)pbT2[aa]+(U16)pbT3[aa]+2)>>1 );
    #endif
  #else
//      (S16) pMuRaw_buf_T3[aa] = (pMuRaw_buf_T3[aa] >>2) + (pMuRaw_buf_PRE[aa]>>2)*3;; //add 20140114
//      pMuRaw_buf_PRE[aa] = pMuRaw_buf_T3[aa];

      ptrLastQuaDelta[aa] = pMuRaw_buf_T0[aa]+pMuRaw_buf_T1[aa]+pMuRaw_buf_T2[aa]+pMuRaw_buf_T3[aa];
      base_idx = ptrLastQuaDelta[aa]-(S16)(((U16)pbT0[aa]+(U16)pbT1[aa]+(U16)pbT2[aa]+(U16)pbT3[aa]+2)>>2);
  #endif

            if (peakmax<base_idx)
            {
                peakmax = base_idx;
            }
            else if (peakmin > base_idx)
            {
                peakmin = base_idx;
            }
#else
  #if REDUCE_XRAM
            base_idx = ptrLastQuaDelta[aa]-( ((pbT2[aa]>>2)-(pbT3[aa]>>2)) << 1);
  #else
            base_idx = ptrLastQuaDelta[aa]-(pbT0[aa]>>2)-(pbT1[aa]>>2)-(pbT2[aa]>>2)-(pbT3[aa]>>2);
  #endif
  
            base_idx= base_idx > 0 ? base_idx: 0;
            if (peakmax<base_idx)
            {
                peakmax = base_idx;
            }
#endif
        }
        // Delta Range : (muENVIROMENT_CALTHD1, 0); (0, -20)
        //      -> do baseline calibrartion
        //     (muENVIROMENT_CALTHD12, muENVIROMENT_CALTHD1]; [-20, -35)
        //      -> remove this rae frame data
        //    o.w.
        //      ->  reset all count and start baseline calibration


        if ((peakmax<flEnvironmentTHDPara.Mu_CalTHD1) && (peakmin>flEnvironmentTHDPara.Mu_CalTHD3) )
        {
            // Range 1
            if (bPowerOnMutual[freqIndex]==0)   bPowerOnMutual[freqIndex] = 1;

            BaseLine_Hold = 0;
        }
        else if ((peakmax<flEnvironmentTHDPara.Mu_CalTHD2) && (peakmin>flEnvironmentTHDPara.Mu_CalTHD4))
        {
            // Range 2
            BaseLine_Hold = 1;
        }
        else
        {
            // Range 3
            bPowerOnMutual[freqIndex] = 0;
            BaseLine_Hold = 0;
        }




#if PowerOnFinger_Tim
        // to calculate mutuBaLine_NoUpdateCn
        if (flMutuPofPara.EnableFlag == 1)
        {
            if ((bPowerOnMutual[freqIndex]==0)&&(last_finger == 0) && ( last_key==0))
            {
                if (mutuBaLine_NoUpdateCn<flMutuPofPara.PowerOnUpdateCN2)
                {
                    mutuBaLine_NoUpdateCn ++;
                }

            }
            else
            {
                mutuBaLine_NoUpdateCn =0;
            }

        }


#endif
    }
    else
    {
        //for(aa=0;aa<Mu_nodes;aa++)
        //{
        //  pMutualdelta[aa] = 0;
        //}
    }


//-----------------------------------------------------------------------
    if (selfprocess==1)
    {
        CalSelfBaseline(); // update self baseline
        
    }


  if (bPowerOnMutual[freqIndex] == 0)
  { 
    if(MutuBaseRy[freqIndex]==1)
        return;
    else
      bPowerOnMutual[freqIndex] = 1;
    
  }

    if (Tx == 100)
    {
        jstr = 0; jend = AlgorithmParam.v_rows;
    }
    else
    {
        jstr =Tx; jend = Tx+1;
    }

  
    for (j=jstr;j<jend ;j++)
    {
        base_ptr=j * AlgorithmParam.h_cols;
        for (k=0; k < AlgorithmParam.h_cols; k++)
        {
            base_idx = base_ptr+k;
            if (bPowerOnMutual[freqIndex] == 1)
            {
                pMBaseLine[base_idx] = 0;
                if ((muPowerOnBaseLine[freqIndex] == 1) || (Tx != 100))
                {

          if(MutuBaseRy[freqIndex]==0 || freqcounter>4)
          {
          #if REDUCE_XRAM
            pMutualBaseLine[base_idx] = (S16)((pMuRaw_buf_T2[base_idx]+pMuRaw_buf_T3[base_idx]) <<1);
          #else
            pMutualBaseLine[base_idx] = (S16)(pMuRaw_buf_T0[base_idx]+pMuRaw_buf_T1[base_idx]+pMuRaw_buf_T2[base_idx]+pMuRaw_buf_T3[base_idx]);
          #endif
          }
          else
            BaseLine_Hold = 1;

        }
            }
            else if ((bPowerOnMutual[freqIndex]<18) && (BaseLine_Hold==0))
            {
                pMBaseLine[base_idx] +=(((U16) (S16)pMuRaw_buf_T3[base_idx]));

        if (bPowerOnMutual[freqIndex] == 17)
                {

                    pMBaseLine[base_idx] = pMBaseLine[base_idx]>>2;
                    if (muPowerOnBaseLine[freqIndex] == 1)
                    {
                        pMutualBaseLine[base_idx] = (S16) pMBaseLine[base_idx];
  
            
                  }
                    else
                        pMutualBaseLine[base_idx] = (pMutualBaseLine[base_idx] + ((S16) pMBaseLine[base_idx]))/2;
                }
            }
        }
    }

    MutuBaseRy[freqIndex]=1;


    if ((Tx == 100) && (BaseLine_Hold == 0))
    
    {

        if (bPowerOnMutual[freqIndex] == 17)
        {
            bPowerOnMutual[freqIndex] = 0;
      BaseLine_Hold = 0;
            muPowerOnBaseLine[freqIndex] = 0;
        }
        else
        {
            bPowerOnMutual[freqIndex] ++;
        }
    }
}

S16 selfpeakmax,selfpeakmin;
U8 selfBaseLine_Hold;
void CalSelfBaseline(void)
{
    //S16 filter, base_p;

  
#if PowerOnFinger_Tim
    bit data selfPowerOnFlag = 0;
#endif
    U8 data k;
    S16  data buffer;
    S16  data deltaSum=0;
#if 1

  S16 *pSelfBaseLine;
  U16 *pSBaseLine;


  pSelfBaseLine = SelfBaseLine[freqIndex];
  pSBaseLine = SBaseLine[freqIndex];

  if(SelfBaseRy[freqIndex]==1)
    selfEnviroment_cal_flag[freqIndex] =1;


    selfpeakmax = 0;
    selfpeakmin = 1000;

  
    if (selfEnviroment_cal_flag[freqIndex]==1)
    {
        for (k=0;k<(Self_nodes-1);k++)
        {
            buffer = pSelfTargetAddr[k]-pSelfBaseLine[k];
            deltaSum += buffer;
            if (selfpeakmax<buffer)
            {
                selfpeakmax = buffer;
            }
            else if (selfpeakmin > buffer)
            {
                selfpeakmin = buffer;
            }
        }
        // Delta Range : (muENVIROMENT_CALTHD1, 0); (0, -20)
        //      -> do baseline calibrartion
        //     (muENVIROMENT_CALTHD12, muENVIROMENT_CALTHD1]; [-20, -35)
        //      -> remove this rae frame data
        //    o.w.
        //      ->  reset all count and start baseline calibration

    
    

    //if ((selfpeakmax<flEnvironmentTHDPara.Self_CalTHD1)||(deltaSum< flEnvironmentTHDPara.Self_CalSum) ) // && (selfpeakmin>-20) //
    if (( (selfpeakmax<flEnvironmentTHDPara.Self_CalTHD1) && (selfpeakmax > selfENVIROMENT_CALTHD3)) ||(deltaSum< flEnvironmentTHDPara.Self_CalSum) ) // && (selfpeakmin>-20) //
          {                                   // deltaSum to detect power palm
            // Range 1
              if (bPowerOnSelf[freqIndex]==0)   bPowerOnSelf[freqIndex] = 1;
              selfBaseLine_Hold = 0;
          }
          //else if ((selfpeakmax<flEnvironmentTHDPara.Self_CalTHD2) ) // && (selfpeakmin>-35)
          else if ((selfpeakmax<flEnvironmentTHDPara.Self_CalTHD2) && (selfpeakmax > selfENVIROMENT_CALTHD4) ) // && (selfpeakmin>-35)
          {
              // Range 2
              selfBaseLine_Hold = 1;
          }
          else
          {
            // Range 3
              bPowerOnSelf[freqIndex] = 0;
              selfBaseLine_Hold = 0;
          }
    
    
    
#if PowerOnFinger_Tim
        if (flMutuPofPara.EnableFlag == 1)
        {
            //  for power on Finger check
            if ((bPowerOnSelf[freqIndex]>0)) //  &&(deltaSum>selfENVIROMENT_Sum)
            {
                //for(k=0;k<(Self_nodes-flKeyPara.num);k++)
                for (k=0;k<Self_nodes;k++)
                {
                    if (pSelfTargetAddr[k]<flMutuPofPara.self_minRaw)
                    {
                        SelfComOffset[ ChMapping[k] ] -= 2;
                        selfPowerOnFlag =1 ;
                    }
                }

                if (selfPowerOnFlag == 1)
                {
                  
                    selfBaLine_updateCn=0;
                    //re-Update self baseline
          Sereset();
          SePOBreset();
            selfEnviroment_cal_flagreset();
                    return;
                }
            }
            // re-Update mutual baseline for power on finger
            if (bPowerOnSelf[freqIndex]>0)
            {
                if (selfBaLine_updateCn<flMutuPofPara.PowerOnUpdateCN1)
                {
                    selfBaLine_updateCn++;
                }
            }
            else
            {
                selfBaLine_updateCn = 0;
            }
      
            if ((selfBaLine_updateCn==flMutuPofPara.PowerOnUpdateCN1)&&(mutuBaLine_NoUpdateCn==flMutuPofPara.PowerOnUpdateCN2))
            {
                //re-Update mutual baseline
        Mureset();
        MuPOBreset();
        
            }
        }
#endif
        //printf(" %d ",deltaSum);
    }


  
#endif


  if (bPowerOnSelf[freqIndex] == 0)
  {
    if(SelfBaseRy[freqIndex]==1)
      return;
    else
      bPowerOnSelf[freqIndex]=1;
  }




    for (k = 0;k<(Self_nodes);k++)
    {
        if (bPowerOnSelf[freqIndex] == 1)
        {
            pSBaseLine[k] = 0;
            if ((selfPowerOnBaseLine[freqIndex] == 1))
            {
                pSelfBaseLine[k] = pSelfTargetAddr[k];
      } 
        }
        else if ((bPowerOnSelf[freqIndex] < 18)&& (selfBaseLine_Hold==0))
        {
            pSBaseLine[k] +=( ((U16) pSelfTargetAddr[k])>>2);
            if (bPowerOnSelf[freqIndex] == 17)
            {
                pSBaseLine[k] = pSBaseLine[k]>>2;
                if (selfPowerOnBaseLine[freqIndex] == 1)
                {
                    pSelfBaseLine[k] = (S16) pSBaseLine[k];
              
                }
                else
                    pSelfBaseLine[k] = (pSelfBaseLine[k] + ((S16) pSBaseLine[k]))/2;
            }
        }
    }


  SelfBaseRy[freqIndex]=1;

  
    if (selfBaseLine_Hold == 0)
    {
        if (bPowerOnSelf[freqIndex] == 17)
        {
            bPowerOnSelf[freqIndex] = 0;

      selfPowerOnBaseLine[freqIndex] = 0;
        }
        else
        {
            bPowerOnSelf[freqIndex] ++;
        }
    }
}

/*
   void ReadRawData(U8 Mode)
   {
        U8 xxx;
        U16 j;
   // U16 uBuffer_ptr,ii;
        switch(Mode)
        {
                case SELF_MODE:
                        //SelfTargetAddr
                        //for(xxx=0;xxx<Self_nodes;xxx++)
                        //{
                        //  wbuf[xxx] = SelfTargetAddr[xxx];
                        //}
                        break;
                case MUTUAL_MODE:
                        for(j=0;j<Mu_nodes;j++)
                        {
                                Mutu_que.Raw_buf[j][Mutu_que.index] = MutuTargetAddr[j];
                        }
                        break;
                default:
                        break;
        }
   }
 */
/****************************************************************************/
/* Peak Search Methods                                                       */
/****************************************************************************/
//U8 *tar_flags;
//U8 bits_ptr;
/*
   void flags_set(U8 *bits, U8 bits_len)
   //--Enter: bits[] is a flags bit array.
   {
        tar_flags=bits;
        bits_ptr=bits_len-1;
   }
   S8 flags_find_pos(void)
   // Note: The flags must be ordered from left to right = byte0LSB..MSB, byte1LSB..MSB, byte2LSB..MSB....
   //--Enter: Before call it first time, you must call flags_set() to setup the pointer.
   //--Return the first flag position founded which searched from right to left. if didn't find. return -1.
   {
        U8 data sel, bits_mask;
        S8 data byte_ptr;
        byte_ptr = bits_ptr >> 3;
        bits_mask = BIT8_MASK[(bits_ptr & 0x07)];
        do
        {
                sel=tar_flags[byte_ptr];
                if(sel)
                {
                        while(! (sel & bits_mask))
                        {
                                bits_ptr--; // Search from right to left.
                                bits_mask >>= 1;  // Right side is MSB, so shifted toward LSB.
                        }
                        tar_flags[byte_ptr] = sel & (~bits_mask); // Clear this bit.
                        return(bits_ptr);
                }
                --byte_ptr;
                if((byte_ptr==(S8)-1)) break;
                else
                {
                        bits_ptr = (byte_ptr << 3) | 0x07;
                        bits_mask=0x80;
                }
        } while(1);
        return(S8)(-1);
   }
 */
//S16 *last_row_ptr;
//U8 flags_a[MAX_H_CH_ToByte];
//U8 flags_b[MAX_H_CH_ToByte];
//U8 IsFlagsA=0;
//U8 *t0_flags;
//U8 *t1_flags;
U8 peak[2][MaxProcessPeak]; // [0][] y , [1][] x
void start_frame(void)
{
    U8 data r;
    //IsFlagsA=0; // Default t1_flags = flags_b;
    LastAvailableFingerNo=0;
    SwitchScanMode_trig=SwitchScanMode_trig_pre;
    SwitchScanMode_trig_pre=0;
    KeyPress=0;
    TouchFingersList->fingers_number=0;
    for (r=0; r< MAX_FINGERS ; r++)
    {
        TouchFingersList->ID[r]=0xFF;
        TouchFingersList->Pos[0][r]=0;
        TouchFingersList->Pos[1][r]=0;
    }
}

#if palm_solution
/*
   void ReportAreaPressure(S16 X,S16 Y,U8 index) //[ jackson ]
   {
        U8 data j,serial=0;
        S16 data pressure = 0;
        serial = 0;
        X = X>>6;
        Y = Y>>6;
        if(PalmID[(Y *AlgorithmParam.h_cols+X)] != 0)
        {
                for(j=0;j<Mu_nodes;j++)
                {
                        if(PalmID[j] == PalmID[(Y *AlgorithmParam.h_cols+X)])
                                serial++;
                }
                if(serial!=0) //&& AreaCount<MAX_FINGERS
                {
                        if(serial > 32)    //for spec
                                serial = 32;
                        pressure  = pMutualdelta[Y*AlgorithmParam.h_cols+X] - PalmReject.PressureOffset;
                        pressure = pressure > 0 ? pressure : 0;
                        pressure = pressure / PalmReject.PressureDivisor;
                        if(pressure > 7 || serial > 20)
                                pressure = 7;
                        //FingerArea_out[0] = serial;
                        //FingerPressure_out[0] = pressure;
                        TouchFingersList->Z_Area[index] = serial;
                        TouchFingersList->Z_Force[index] = pressure;
                        //TouchFingersList->FingerArea[AreaCount] = serial;
                        //TouchFingersList->FingerPressure[AreaCount++] = pressure;
                }
        }
   }
 */
U8 PalmIDCheck(U8 TX, U8 RX)
{
    //if return 0, maybe is noise
    //if return 1, normal finger
    //if return 2, big finger , can report
    //if return 3, big finger or Plam, remove the point
    U8 data i;
    U8 tmpPlamID=0;
    U8 tmpPlamIndex=0xFF;
    U8 noise_detect;
    U8 tmpfinger_lvl=AlgorithmParam.mutual_finger_lvl/2;
    tmpPlamID=PalmID[TX*AlgorithmParam.h_cols+RX] ;
    //PID_unm[PID_IdxCn] = localPID_unm[m];
    //PID_Idx[PID_IdxCn] = m;
    for (i=0 ; i <PID_IdxCn ; i++)
    {
        if (PID_Idx[i] ==tmpPlamID)
        {
            tmpPlamIndex=i;
            break;
        }
    }
    if (tmpPlamIndex==0xFF) // search more than mutu threshold
    {
        //return 0;
        noise_detect=0;
        if (RX>0)
            if (ptrLastQuaDelta[TX*AlgorithmParam.h_cols+ RX-1] >tmpfinger_lvl) // left
            {
                noise_detect++;
            }
        if (RX<(AlgorithmParam.h_cols-1))
            if (ptrLastQuaDelta[TX*AlgorithmParam.h_cols+ RX+1] >tmpfinger_lvl) // right
            {
                noise_detect++;
            }
        if (TX>0)
            if (ptrLastQuaDelta[(TX-1)*AlgorithmParam.h_cols+ RX] >tmpfinger_lvl) // up
            {
                noise_detect++;
            }
        if (TX<(AlgorithmParam.v_rows-1))
            if (ptrLastQuaDelta[(TX+1)*AlgorithmParam.h_cols+ RX] >tmpfinger_lvl) // down
            {
                noise_detect++;
            }
        if (noise_detect>0)
        {
            //printf(">25 = %03bu ",noise_detect);
            return 1;
        }
        else
        {
            return 0; // maybe is noise
        }
    }
    else
    {
        if (PID_unm[tmpPlamIndex] >=PalmReject.PalmAreaCount) // palm
        {
            return 3 ;
        }
        if (PID_unm[tmpPlamIndex] >=PID_bigFingerTH ) // big finger
        {
            if (PID_report_en[tmpPlamIndex]==0)
            {
                PID_report_en[tmpPlamIndex]=1;
                //printf("Palm Addr %03bu,%03bu \r\n",PID_unm[tmpPlamIndex],gbCNter);
                return 2 ;
            }
            else
            {
                return 3 ;
            }
        }
        else
            return 1;
    }
}

void PalmCalAddr(U8 TX, U8 RX,S16* OutputAddr)
{
//  U8 ii,jj;
    U8 row_id,col_id,y_idx,x_idx;
    U8 tmpPlamID ;
    U16 uWeights;
    U16 data m=0;
    U32 uWeights_y;
    unsigned long X_Weights_Total_up,X_Weights_down;
    unsigned long Y_Weights_Total_up,Y_Weights_down;
    X_Weights_Total_up=0;
    Y_Weights_Total_up=0;
    X_Weights_down=0;
    Y_Weights_down=0;
    //tim modify
    PID_flag=1;
    tmpPlamID=PalmID[TX*AlgorithmParam.h_cols+RX] ;
    m=0;
    y_idx=1;
    for (row_id=0; row_id<AlgorithmParam.v_rows; row_id++)
    {
        x_idx=1; // 2 + 1
        uWeights_y=0;
        for (col_id=0; col_id<AlgorithmParam.h_cols; col_id++)
        {
            if (PalmID[m + col_id] == tmpPlamID)
            {
                uWeights=(ptrLastQuaDelta[m + col_id]);
                X_Weights_down+=uWeights;   //uWeights;
                X_Weights_Total_up+=uWeights * x_idx;
                uWeights_y+=uWeights;
            }
            x_idx+=2;
        }
        Y_Weights_down+=uWeights_y;
        Y_Weights_Total_up+=uWeights_y* y_idx;
        y_idx+=2;
        m+=AlgorithmParam.h_cols;
    }
    OutputAddr[0]=Y_Weights_Total_up *32 /Y_Weights_down ;
    OutputAddr[1]=X_Weights_Total_up *32 /X_Weights_down ;
}

void PalmRejection(void) //1332k bytes
{
    U8 data ID=1,id_temp=0;
    U8 data j,row_id,col_id; // ,y_idx,x_idx;
    U16 *addr_now,*addr_last;
    U8 data serial_flag=0;
    U16 data m=0;
    U16 maxGropID;
    unsigned long X_Weights_Total_up=0,X_Weights_down=0;
    unsigned long Y_Weights_Total_up=0,Y_Weights_down=0;
//  U16 data uWeights;
//  U32 data uWeights_y;
    U8 localPID_unm[MAX_MUTUAL_NODE];
    //U32 Weights[TxLength];
    palm_id_count=0;
    for (j=0; j<MAX_FINGERS; j++)
    {
        //id_rec[j] = 0;
        PID_Idx[j]=0;
        PID_unm[j]=0;
        PID_report_en[j]=0;
    }
    for (m=0; m<Mu_nodes; m++)
    {
        PalmID[m]=0;
        localPID_unm[m]=0;
        if ((ptrLastQuaDelta[m] >PalmReject.PalmThreshold) && ( ((m%AlgorithmParam.h_cols)!=0) || (m == 0) ))
        {
            PalmID[m]=ID;
            serial_flag++;
        }
        else if ((ptrLastQuaDelta[m] > PalmReject.PalmThreshold) && (m%AlgorithmParam.h_cols == 0))
        {
            ID++;
            PalmID[m]=ID;
            serial_flag=1;
        }
        else if (serial_flag != 0)
        {
            ID++;
            serial_flag=0;
        }
    }
    addr_last=PalmID;
    addr_now=PalmID;
    for (row_id=1; row_id<AlgorithmParam.v_rows; row_id++)
    {
        addr_now+=AlgorithmParam.h_cols;
        for (col_id=0; col_id<AlgorithmParam.h_cols; col_id++)
        {
            if ((addr_now[col_id] != addr_last[col_id]) && ((addr_now[col_id] != 0) && (addr_last[col_id] != 0)))
            {
                id_temp=addr_now[col_id];
                ID=addr_last[col_id];
                for (m=0; m<Mu_nodes; m++)
                    //for(j=0;j<AlgorithmParam.h_cols;j++)
                {
                    if (addr_now[m] == id_temp)
                        addr_now[m]=ID;
                }
            }
        }
        addr_last=addr_now;
    }
    /*
       addr_now = PalmID;
       for(row_id=0;row_id<AlgorithmParam.v_rows;row_id++)
       {
        printf("%02bu: ",row_id);
        for(col_id=0;col_id<AlgorithmParam.h_cols;col_id++)
        {
                printf("%u ",addr_now[col_id]);
        }
        printf("\r\n");
        addr_now +=AlgorithmParam.h_cols;
       }
     */
    maxGropID=0;
    for (m=0; m<Mu_nodes; m++)
    {
        if (PalmID[m] > 0 )
        {
            localPID_unm[PalmID[m]]++;
            if (maxGropID<PalmID[m])
            {
                maxGropID=PalmID[m];
            }
        }
    }
    PID_IdxCn=0;
    for (m=0; m<(maxGropID+1); m++)
    {
        if (PID_IdxCn<MAX_FINGERS)
        {
            if (localPID_unm[m]>1)
            {
                PID_unm[PID_IdxCn]=localPID_unm[m];
                PID_Idx[PID_IdxCn]=m;
                PID_IdxCn++;
            }
        }
    }
    if (PID_IdxCn>0)
    {
        //  printf("PID_Idx;");
        //  for(j=0;j<PID_IdxCn;j++)
        //  {
        //    printf("%03bu ",PID_Idx[j]);
        //  }
        //  printf("\r\n");
        //  printf("PID_unm:");
        //  for(j=0;j<PID_IdxCn;j++)
        //  {
        //    printf("%03bu ",PID_unm[j]);
        //  }
        //  printf("\r\n");
    }
    /*
       serial_flag = 0;
       id_temp = 0;
       ID = 0;
       for(m=0;m<Mu_nodes;m++)
       {
           if(PalmID[m] != 0 && PalmID[m] != id_temp)
           {
                         for(j=0;j<palm_id_count;j++)
                         {
                                if(id_rec[j] == PalmID[m])
                                ID = 0xaa;
                         }
                        if(ID!= 0xaa)
                         {
                                 id_temp = PalmID[m];
                                 for(j=0;j<Mu_nodes;j++)
                                 {
                                           if(id_temp == PalmID[j])
                                           serial_flag++;
                                 }
                                 if(serial_flag > PalmReject.PalmAreaCount)
                                        id_rec[palm_id_count++] = id_temp;
                                 serial_flag = 0;
                        }
                        ID = 0;
           }
       }
       if(palm_id_count>0)
       {
        for(m=0;m< palm_id_count;m++)
        {
                printf("%03bu ",id_rec[m]);
        }
        printf("\r\n");
       }
     */
    /////////////////////////////////////////////////////////////////////////////
    /*
        if(palm_id_count > 0 && PalmReject.PalmReport_point == 1) //turn off palm report point
        {
                X_Weights_Total_up =   0;
                Y_Weights_Total_up =  0;
                X_Weights_down = 0;
                Y_Weights_down  = 0;
                //tim modify
                for(j=palm_id_count;j!=0;j--)
                {
                        m=0;
                        y_idx = 1;
                        for(row_id=0;row_id<AlgorithmParam.v_rows;row_id++)
                        {
                                x_idx=1; // 2 + 1
                                uWeights_y =0;
                                for(col_id=0;col_id<AlgorithmParam.h_cols;col_id++)
                                {
                                        if(PalmID[m + col_id] == id_rec[j-1])
                                        {
                                                 uWeights = ptrLastQuaDelta[m + col_id];
                                                 X_Weights_down += uWeights;//uWeights;
                                                 X_Weights_Total_up +=  uWeights * x_idx;
                                                 uWeights_y +=uWeights;
                                        }
                                        x_idx +=2;
                                }
                                Y_Weights_down += uWeights_y;
                                Y_Weights_Total_up += uWeights_y* y_idx;
                                y_idx+=2;
                                m +=AlgorithmParam.h_cols;
                        }
                        break;      //only one palm point needed
                }
                PalmX = X_Weights_Total_up *32 /X_Weights_down ;
                PalmY = Y_Weights_Total_up *32 /Y_Weights_down ;
        }
     */
}

#endif
#if 1
U8 peak_flag[24][MAX_H_CH_ToByte]; //JS20130616a --> modify from 26 to 27
void SearchDeltaPeak(S16 *row_ptr, U8 row_count, U8 row_len,S16 fingerlevel)  // ---------------------3.3 ms
{
    U8 data row_id,col_id,fbyte, b_mask, last_b_mask, last_fbyte;
    U8 fg_bigger,near_bigger;
//  S8 peaks;
    S16 *lastRow_ptr,*nowRow_ptr;
    S16 data now;
    U8 data col_id_add1,col_id_dec1,row_id_dec1;
    S16 fingerlevel_c;
    for (row_id=0 ; row_id< AlgorithmParam.v_rows; row_id++)
        for (fbyte=0 ; fbyte< (MAX_H_CH_ToByte); fbyte++)
            peak_flag[row_id][fbyte]=0; // Clear flags
    // search all frame peak, but don't care fingerlevel
    //peakmax = 0;
    fbyte=0;
    b_mask=0x01;
    nowRow_ptr=row_ptr;
    fingerlevel_c=fingerlevel;
    for (row_id=0 ; row_id< row_count; row_id++)
    {
        row_id_dec1=row_id-1;
        for (col_id=0 ; col_id< row_len; col_id++)
        {
            /*
               if ((row_id>0)&&(row_id<(row_count-1))&&(col_id>0)&&(col_id<(row_len-1)))
               {
                 fingerlevel_c = flRawPara.Mutu_TH;
             #if SZ_Floating_Poly2
                        if (nFloatDynamicTH >= 1)
                                //fingerlevel_c = flRawPara.Mutu_TH*7;
                                //fingerlevel_c = flRawPara.Mutu_TH*4;    // 100 count
                                //fingerlevel_c = flRawPara.Mutu_TH*2;    // 50 count
                                fingerlevel_c = DynamicTH;
             #endif
               }
               else
               {
                 fingerlevel_c = fingerlevel;
             #if SZ_Floating_Poly2
                        if (nFloatDynamicTH >= 1)
                                //fingerlevel_c = flRawPara.Mutu_TH*7;
                                //fingerlevel_c = flRawPara.Mutu_TH*4;    // 100 count
                                //fingerlevel_c = flRawPara.Mutu_TH*2;    // 50 count
                                fingerlevel_c = DynamicTH;
             #endif
               }
             */
            if (nowRow_ptr[col_id]  > fingerlevel_c)
            {
#if 0 // SZ_Floating_DEBUG
                PeakChecksum=0;
                if ((row_id>0) &&(row_id< (row_count-1))&& (col_id>0) && (col_id< (row_len-1)))
                {
                    PeakChecksum=nowRow_ptr[col_id]+nowRow_ptr[col_id+1]+nowRow_ptr[col_id-1]+lastRow_ptr[col_id];
                    if ( (U16)PeakChecksum < ((fingerlevel_c*3)>>1) )
                    {
                        //printf("PeakCHS:%5d  fTH=%5d\n\r",PeakChecksum,fingerlevel_c);
                        break;
                    }
                    //printf("PeakCHS:%5d  fTH=%5d\n\r",PeakChecksum,fingerlevel_c);
                }
#endif
                fg_bigger=0;
                near_bigger=0;
                b_mask=BIT8_MASK[col_id & 0x7];
                fbyte=col_id>>3;
                //-- Start to search the peak
                now=nowRow_ptr[col_id] ;
                col_id_add1=col_id+1;
                col_id_dec1=col_id-1;
                if (row_id>0) //-- It is the first row, so compare only to left bit
                { // When row_id > 0, Compare to upper row too.
                    if (now >= lastRow_ptr[col_id]) // I bigger than up
                    {
                        //clear Upper flag
                        peak_flag[row_id_dec1][fbyte]&=~b_mask;   //@ t0_flag
                        near_bigger++;
                    }
#if NineGrid_SearchPeak
                    if (col_id>0)
                    {
                        if (now >= lastRow_ptr[col_id_dec1]) // I bigger than up left side
                        {
                            peak_flag[row_id_dec1][(col_id_dec1)>>3]&=BIT8_MASK_0[(col_id_dec1)&0x7];   //@ t0_flag
                            near_bigger++;
                        }
                    }
                    else
                        near_bigger++;
                    //
                    if (col_id<row_len-1)
                    {
                        if (now >= lastRow_ptr[col_id_add1]) // I bigger than up right side
                        {
                            peak_flag[row_id_dec1][(col_id_add1)>>3]&=BIT8_MASK_0[(col_id_add1)&0x7];   //@ t0_flag
                            near_bigger++;
                        }
                    }
                    else
                        near_bigger++;
                    if (near_bigger==3)
                        fg_bigger++;
#else
                    if (near_bigger==1)
                        fg_bigger++;
#endif
                }
                else
                { // Force to set bigger at top row.
                    fg_bigger++;
                }
                if (col_id>0)
                {
                    if (now >= nowRow_ptr[col_id_dec1] ) // I am bigger than left
                    {
                        //clear left
                        last_b_mask=BIT8_MASK[(col_id_dec1) & 0x7];
                        last_fbyte=(col_id_dec1)>>3;
                        peak_flag[row_id][last_fbyte]&=~last_b_mask;   //@ t1_flag
                        fg_bigger++;
                    }
                }
                else
                { // Force set to bigger at left border.
                    fg_bigger++;
                }
                //-- Two biggers means biggest.
                if (fg_bigger >=2 )
                {
                    peak_flag[row_id][fbyte]|=b_mask;
                }
                //--------------------------------------------------------------------
            }
        }
        lastRow_ptr=nowRow_ptr;   // Keep remember for next process.
        //nowRow_ptr+=row_len;
        nowRow_ptr+=Total_RX;
    }
}

#else
U8 peak_flag[TxLength][MAX_H_CH_ToByte];
void SearchDeltaPeak(S16 *row_ptr, U8 row_count, U8 row_len)  // ---------------------3.3 ms
{
    U8 data row_id,col_id,fbyte, b_mask, last_b_mask, last_fbyte;
    U8 fg_bigger,near_bigger;
//  S8 peaks;
    S16 *lastRow_ptr,*nowRow_ptr;
    S16 data now;
    U8 data col_id_add1,col_id_dec1,row_id_dec1;
    U8 *peakflag_ptr, *peakflag_ptr1;
    for (row_id=0 ; row_id< AlgorithmParam.v_rows; row_id++)
    {
        peakflag_ptr=peak_flag[row_id];
        for (fbyte=0 ; fbyte< (MAX_H_CH_ToByte); fbyte++)
            peakflag_ptr[fbyte]=0; //peak_flag[row_id_dec1][fbyte] = 0; // Clear flags
    }
    // search all frame peak, but don't care fingerlevel
    //peakmax = 0;
    fbyte=0;
    b_mask=0x01;
    nowRow_ptr=row_ptr;
    for (row_id=0 ; row_id< row_count; row_id++)
    {
        row_id_dec1=row_id-1;
        peakflag_ptr=peak_flag[row_id];
        peakflag_ptr1=peak_flag[row_id_dec1];
        for (col_id=0 ; col_id< row_len; col_id++)
        {
            fg_bigger=0;
            near_bigger=0;
            b_mask=BIT8_MASK[col_id & 0x7];
            fbyte=col_id>>3;
            //-- Start to search the peak
            now=nowRow_ptr[col_id] ;
            col_id_add1=col_id+1;
            col_id_dec1=col_id-1;
            if (row_id>0) //-- It is the first row, so compare only to left bit
            { // When row_id > 0, Compare to upper row too.
                if (now >= lastRow_ptr[col_id]) // I bigger than up
                {
                    //clear Upper flag
                    peakflag_ptr1[fbyte]&=~b_mask;   //@ t0_flag
                    // peak_flag[row_id_dec1][fbyte] &= ~b_mask; //@ t0_flag
                    near_bigger++;
                }
#if NineGrid_SearchPeak
                if (col_id>0)
                {
                    if (now >= lastRow_ptr[col_id_dec1]) // I bigger than up left side
                    {
                        peakflag_ptr1[(col_id_dec1)>>3]&=~BIT8_MASK[(col_id_dec1)&0x7];   //@ t0_flag
                        //peak_flag[row_id_dec1][(col_id_dec1)>>3] &= ~BIT8_MASK[(col_id_dec1)&0x7]; //@ t0_flag
                        near_bigger++;
                    }
                }
                else
                    near_bigger++;
                //
                if (col_id<row_len-1)
                {
                    if (now >= lastRow_ptr[col_id_add1]) // I bigger than up right side
                    {
                        peakflag_ptr1[(col_id_add1)>>3]&=~BIT8_MASK[(col_id_add1)&0x7];   //@ t0_flag
                        //peak_flag[row_id_dec1][(col_id_add1)>>3] &= ~BIT8_MASK[(col_id_add1)&0x7]; //@ t0_flag
                        near_bigger++;
                    }
                }
                else
                    near_bigger++;
                if (near_bigger==3)
                    fg_bigger++;
#else
                if (near_bigger==1)
                    fg_bigger++;
#endif
            }
            else
            { // Force to set bigger at top row.
                fg_bigger++;
            }
            if (col_id>0)
            {
                if (now >= nowRow_ptr[col_id_dec1] ) // I am bigger than left
                {
                    //clear left
                    peakflag_ptr[last_fbyte]&=~last_b_mask;   //@ t1_flag
                    //peak_flag[row_id][last_fbyte] &= ~last_b_mask; //@ t1_flag
                    fg_bigger++;
                }
            }
            else
            { // Force set to bigger at left border.
                fg_bigger++;
            }
            //-- Two biggers means biggest.
            if (fg_bigger >=2 )
            {
                peakflag_ptr[fbyte]|=b_mask;
                //peak_flag[row_id][fbyte] |= b_mask;
                //if(peakmax< nowRow_ptr[col_id])
                //{
                //  peakmax = nowRow_ptr[col_id];
                //}
                //if(searchMode==SELF_MODE)
                //  peaks = 1;
            }
            //--------------------------------------------------------------------
            last_b_mask=b_mask;
            last_fbyte=fbyte;
        }
        lastRow_ptr=nowRow_ptr;   // Keep remember for next process.
        //nowRow_ptr+=row_len;
        nowRow_ptr+=Total_RX;
    }
}

#endif
/*
   void _2FingerPitch_Improve(S16 *row_ptr, U8 row_count, U8 row_len,S16 fingerlevel )
   {
        U8 data  row_id,col_id;
        U8 data  fbyte,fbit;
        U8 data tmpfbyte,tmpfbit;
        S16 *nowRow_ptr,*lastRow_ptr;
        U16   _2FingerTH;
        U8   para1 = 6,para2 = 10;
        S16  data last,last1,now,now1;
        nowRow_ptr = row_ptr;
        for(row_id=0 ; row_id< row_count; row_id++)
        {
                for(col_id=0 ; col_id< row_len; col_id++)
                {
                        //fbyte  = col_id/8;
                        //fbit = col_id%8;
                        fbyte  = col_id>>3;
                        fbit = col_id & 0x7;
                        if((peak_flag[row_id][fbyte] &BIT8_MASK[fbit])>0)
                        {
                                last = lastRow_ptr[col_id];
                                now = nowRow_ptr[col_id];
                                if(now >= fingerlevel )
                                {
                                        if(row_id>0)
                                        {
                                                if(col_id>0 )
                                                {
                                                        tmpfbyte = (col_id-1)>>3;
                                                        tmpfbit = (col_id-1)& 0x7;
                                                        last1 = lastRow_ptr[col_id-1];
                                                        now1 = nowRow_ptr[col_id-1];
                                                        if((peak_flag[row_id-1][tmpfbyte] &BIT8_MASK[tmpfbit])>0)
                                                        {
                                                                _2FingerTH = (now+last1)* para1 / para2;
                                                                if( (now1+last) >_2FingerTH )  // need to remove a peak
                                                                {
                                                                        if(now>=last1)
                                                                        {
                                                                                peak_flag[row_id-1][tmpfbyte] &= ~BIT8_MASK[tmpfbit];
                                                                        }
                                                                        else
                                                                        {
                                                                                peak_flag[row_id][fbyte] &= ~BIT8_MASK[fbit];
                                                                        }
                                                                }
                                                        }
                                                }
                                                if(col_id<(row_len-1) )
                                                {
                                                        tmpfbyte = (col_id +1)>>3;
                                                        tmpfbit = (col_id + 1)& 0x7;
                                                        last1 = lastRow_ptr[col_id+1];
                                                        now1 = nowRow_ptr[col_id+1];
                                                        if((peak_flag[row_id-1][tmpfbyte] &BIT8_MASK[tmpfbit])>0)
                                                        {
                                                                _2FingerTH = (now+last1)* para1 / para2;
                                                                if( (now1+last )>_2FingerTH )  // need remove a peak
                                                                {
                                                                        if(now>=last1)
                                                                        {
                                                                                peak_flag[row_id-1][tmpfbyte] &= ~BIT8_MASK[tmpfbit];
                                                                        }
                                                                        else
                                                                        {
                                                                                peak_flag[row_id][fbyte] &= ~BIT8_MASK[fbit];
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }
                                else
                                {
                                        peak_flag[row_id][fbyte] &= ~BIT8_MASK[fbit];
                                }
                        }
                }
                lastRow_ptr = nowRow_ptr;
                nowRow_ptr += row_len;
        }
   }
 */
//U8 tim_flag= 0;
#if palm_solution
void Coaxial_Check()
{
    //ptrLastQuaDelta[kk]
    //peak[0][peaks] = row_id;
    //peak[1][peaks] = col_id;
    U8 ii,jj;
    U8 tmp;
    U8 peak_remove[MAX_FINGERS];
    U8 peak_removeCN;
    U8 index;
    S16 palmDelta;
    // PID_continueflag
    if (TouchFingersList->fingers_number <2)
        return ;
    for (ii=0; ii<MAX_FINGERS; ii++)
    {
        peak_remove[ii]=0;
    }
    peak_removeCN=0;
    PalmIDCheck(peak[0][ii],peak[1][ii]);
    for (ii=0; ii<TouchFingersList->fingers_number; ii++)
    {
        tmp=PalmIDCheck(peak[0][ii],peak[1][ii]);
        if (tmp>1) // palm occur
        {
            palmDelta=ptrLastQuaDelta[peak[0][ii] *AlgorithmParam.h_cols+peak[1][ii]]>>1 ;
            for (jj=0; jj<TouchFingersList->fingers_number; jj++)
            {
                if ( abs(peak[1][ii] - peak[1][jj]) < 2)
                    if (palmDelta > ptrLastQuaDelta[peak[0][jj] *AlgorithmParam.h_cols+peak[1][jj]]  )
                    {
                        peak_remove[jj]=1;
                        peak_removeCN++;
                        //printf("~~need Remove the point, %03bu %03bu \r\n",peak[0][jj] ,peak[1][jj]);
                    }
            }
        }
    }
    index=0;
    if (peak_removeCN>0 ) // need remove point
    {
        //printf(" ori \r\n");
        //for(ii =0;ii<TouchFingersList->fingers_number;ii++)
        //{
        //  printf("%03bu %03bu \r\n",peak[0][ii],peak[1][ii]);
        //}
        for (ii=0; ii<TouchFingersList->fingers_number; ii++)
        {
            if (peak_remove[ii] ==0 )
            {
                peak[0][index]=peak[0][ii];
                peak[1][index]=peak[1][ii];
                index++;
            }
        }
        LastAvailableFingerNo-=peak_removeCN;
        TouchFingersList->fingers_number-=peak_removeCN;
        //printf(" new \r\n");
        //for(ii =0;ii<TouchFingersList->fingers_number;ii++)
        //{
        //  printf("%03bu %03bu \r\n",peak[0][ii],peak[1][ii]);
        //}
    }
}

#endif
#if (OpenFWNoiseCheck||CoaxialLevelCheck) //JStest
U8 IsSavePoint(U8 row_id,U8 col_id)
{
    //saveflag:
    // 0 => need to remove
    // 1 => need to save
    U8 aa;
    U8 saveflag=0;
    for (aa=0 ; aa< prePeaks; aa++)
    {
        //if((prePeak[0][aa] == row_id) && (prePeak[1][aa] == col_id))
        if ( ( (abs(prePeak[0][aa] - row_id) < 2)  && (prePeak[1][aa] == col_id) ) || ( (abs(prePeak[1][aa] - col_id) < 2)  && (prePeak[0][aa] == row_id))  )
        {
            if ( fnIsOnEdge(row_id, col_id, 0)==0)
            {
                saveflag=1;
#if debug_Charger
                printf("save (%03bu,%03bu)\r\n",row_id,col_id);
#endif
            }
            break;
        }
    }
    return saveflag;
}

#endif //#if (OpenFWNoiseCheck||CoaxialLevelCheck) //JStest
#if CoaxialLevelCheck //JStest
void Coaxial_LevelCheck()
{
    U8 data ii,jj; //JStest
    U16 data kk; //JStest
    U8 peak_reg[MaxProcessPeak];
    U8 data peak_removeCN; //JStest
    U8 data tmpCN; //JStest
    S16 data FW_N_PeakLevel; //,FW_2_PeakLevel;//,Last2Delta_reg; //JStest
    S16 data FW_N_PeakLevel_edge; //JStest
    if (TouchFingersList->fingers_number>1)
    {
        for (ii=0; ii<MaxProcessPeak; ii++)
        {
            peak_reg[ii]=0xFF;
        }
        peak_removeCN=0;
        for (ii=0; ii<TouchFingersList->fingers_number; ii++)
        {
            //if(peak_reg[ii]==0xFF)
            if (peak[0][ii] < 0x80)
            {
                tmpCN=0;
                FW_N_PeakLevel=0;
                FW_N_PeakLevel_edge=0;
                //FW_2_PeakLevel = 0;
                for (jj=0; jj<TouchFingersList->fingers_number; jj++)
                {
                    //if(peak[1][ii]== peak[1][jj]) // Coaxial
                    if (peak[0][jj] < 0x80)
                    {
                        if (abs(peak[1][ii]- peak[1][jj])<2 ) // Coaxial
                        {
                            tmpCN++;
                            peak_reg[jj]=peak[1][ii];
                            kk=peak[0][jj] *Total_RX+ peak[1][jj] ;
                            //Last2Delta_reg =  (  (S16)pMuRaw_buf_T2[kk])- ((MutualBaseLine[kk])>>2);
                            if (FW_N_PeakLevel < ptrLastQuaDelta [kk])
                            {
                                FW_N_PeakLevel=ptrLastQuaDelta[kk];
                            }
                            //if (FW_2_PeakLevel < Last2Delta_reg)
                            //  FW_2_PeakLevel = Last2Delta_reg;
                        }
                    }
                }
                //FW_N_PeakLevel = (FW_N_PeakLevel*3)/5;
                //FW_2_PeakLevel = FW_2_PeakLevel/2;
                //FW_N_PeakLevel = (FW_N_PeakLevel + FW_2_PeakLevel)>>1;
                FW_N_PeakLevel_edge=(FW_N_PeakLevel *3) /5;
                FW_N_PeakLevel=(FW_N_PeakLevel )>>1;
                if (tmpCN>1)
                {
                    for (jj=0; jj<TouchFingersList->fingers_number; jj++)
                    {
                        if ((peak[0][jj] < 0x80) && ( peak[1][ii] == peak_reg[jj]) )
                        {
                            kk=peak[0][jj] *Total_RX + peak[1][jj] ;
                            //Last2Delta_reg =  (  (S16)pMuRaw_buf_T2[kk])- ((MutualBaseLine[kk])>>2);
                            //if(  (ptrLastQuaDelta[kk] +Last2Delta_reg) < FW_N_PeakLevel)
                            if ( ( (peak[0][jj]< 2 )||(peak[0][jj]>(AlgorithmParam.v_rows -3)) )&& (ptrLastQuaDelta[kk] <=FW_N_PeakLevel_edge )   )
                            {
                                peak[0][jj]|=0x80;
                                peak_removeCN++;
                            }
                            else if (  (ptrLastQuaDelta[kk]) <=FW_N_PeakLevel)
                                //if ((ptrLastQuaDelta[kk] < FW_N_PeakLevel) || (Last2Delta_reg< FW_2_PeakLevel))
                            {
                                if (IsSavePoint(peak[0][jj],peak[1][jj])==0)
                                {
#if debug_Charger
                                    printf("Rm (%03bu,%03bu),%03bu\r\n", peak[0][jj], peak[1][jj],gbCNter);
#endif
                                    peak[0][jj]|=0x80;
                                    peak_removeCN++;
                                }
                            }
                        }
                    }
                }
            }
        }
        //
        if (peak_removeCN>0 ) // need remove point
        {
            jj=0;
            for (ii=0; ii<TouchFingersList->fingers_number; ii++)
            {
                if (peak[0][ii] < 0x80 )
                {
                    peak[0][jj ]=peak[0][ii];
                    peak[1][jj ]=peak[1][ii];
                    jj++;
                }
            }
            LastAvailableFingerNo-=peak_removeCN;
            TouchFingersList->fingers_number-=peak_removeCN;
        }
        /*
           if(TouchFingersList->fingers_number> MAX_FINGERS)
           {
                TouchFingersList->fingers_number = MAX_FINGERS;
                LastAvailableFingerNo = MAX_FINGERS;
           }
         */
        //------for AC charger 220V---------------------
#if 0
        if (TouchFingersList->fingers_number>1)
        {
            if ( (peak[0][0]==0) ||(peak[0][0]==1))
            {
                printf("peak delta ");
                for (ii=0; ii<TouchFingersList->fingers_number; ii++)
                {
                    printf("(y%02bu x%02bu,d %03d )",peak[0][ii],peak[1][ii], ptrLastQuaDelta[ peak[0][ii] *12 + peak[1][ii]]);
                }
                printf(",F %05d\r\n\r\n",gbCNter);
                /*
                   printf("%03d %03d %03d\r\n%03d %03d %03d\r\n%03d %03d %03d\r\n"
                                        ,ptrLastQuaDelta[ (peak[0][0]-1)*12 + peak[1][0]-1] , ptrLastQuaDelta[ (peak[0][0]-1)*12 + peak[1][0]] , ptrLastQuaDelta[ (peak[0][0]-1)*12 + peak[1][0] + 1]
                                        ,ptrLastQuaDelta[ peak[0][0]*12 + peak[1][0]-1] , ptrLastQuaDelta[ peak[0][0]*12 + peak[1][0]] , ptrLastQuaDelta[ peak[0][0]*12 + peak[1][0] + 1]
                                        ,ptrLastQuaDelta[ (peak[0][0]+1)*12 + peak[1][0]-1] , ptrLastQuaDelta[ (peak[0][0]+1)*12 + peak[1][0]] , ptrLastQuaDelta[ (peak[0][0]+1)*12 + peak[1][0] + 1]
                        );
                   printf("\r\n");
                 */
            }
        }
#endif
    }
    /*
       if(TouchFingersList->fingers_number>1)
       {
        for(ii =0;ii<TouchFingersList->fingers_number;ii++)
        {
                printf("%4d %4d %4d \r\n",ptrLastQuaDelta[peak[0][ii]*AlgorithmParam.h_cols+peak[1][ii]-1 ]
                                                            ,ptrLastQuaDelta[peak[0][ii]*AlgorithmParam.h_cols+peak[1][ii]     ]
                                                            ,ptrLastQuaDelta[peak[0][ii]*AlgorithmParam.h_cols+peak[1][ii]+1 ]
                        );
        }
        printf("**");
       }
       if(TouchFingersList->fingers_number>0)
        printf("avg =%03d \r\n",OneFrDeltaAvg);
     */
}

#endif //#if CoaxialLevelCheck //JStest


S8 SearchRowPeak(S16 *row_ptr, U8 row_count, U8 row_len, S16 fingerlevel, U8 searchMode)  // ----------~ 7ms
// Call this function after each row delta data received.
//-- Enter :
// row_ptr point to a whole H line raw data. row_id is the row number, range in 0~(v_rows-1).
//-- Return:
// the No.(row_id-2) row touch flags was determind.
// t1_flag[] always point to the current effective peak poistion.
//-- Note:
// call this routine while row_id==v_rows-1, it will automatic finish the touch flags,
// So after call this routine while row_id==v_rows-1, all touch flags were valid.
{
    U8 data row_id,col_id,fbyte,fbit;
    S8 data peaks; //JStest
    S16 *nowRow_ptr;

//  U16 dkey_aa_bound = (Total_TX-1)*Total_RX; 

    S16 data deltaMax;
    S16 peak_TH=0;
#if palm_solution
    U8 PID_result;
#endif
#if Twokey_debug_SearchRowPeak
    S16 temp, maxD, sV, sH;
#endif

U16 tmpu1;
#if LCM_ANALYSIS
       U16 tmpu2;
  S16 tmps1,tmps2;
  bit usedflag;
  U8 tmpcounter;  
  S16 ratio1,ratio2;
  U8 LStart,LEnd,RStart,REnd;
#endif  

    //#if PEAK_IMPROVE //JS20130803 XX
#if SEARCH_NEW_DELTA_PEAK //JS20130803
    S16 data kk;
    //S16 data kk2; //JS20130803
#endif
#if PEAK_EXTEND
    U8 fgNine;
    U8 C1, C2;
    U8 EDir[4]; // Dir[0]=Ydir1 Dir[1]=Ydir2, Dir[2]=Xdir1, Dir[3]=Xdir2
#endif
    S16 data NeiborUp=0,NeiborDn=0,NeiborLf=0,NeiborRt=0; //JStest
#if OpenFWNoiseCheck // [poly]
    U8 data fgFWCheck; //,aa; //JStest
    S16 data NeiborUpDn,NeiborLfRt,FWpos; //JStest
//  S16 NeiborUp,NeiborDn,NeiborLf,NeiborRt;
#endif
#if LARGE_AREA
    U16 addr_idx;
#endif
#if SEARCH_NEW_DELTA_PEAK       // use Mutu_que.Raw_buf to search peaks. Crovax 20120910.
    if ( searchMode == MUTUAL_MODE )
    {
        deltaMax=0;
#if LCM_ANALYSIS// only for 34-ch mi2
  if ( flKeyPara.type==MUTU_RIGHTKEY )
  {
    LStart = 1;
    LEnd = LFTotalChno;
    RStart = (LFTotalChno + 1);
    REnd = (LFTotalChno + RTTotalChno);

        if (charger_on_status == 1)
        {
      ratio1 = ChargerRatio1;
      ratio2 = ChargerRatio2;
        }
    else
    {
      ratio1 = NormalRatio1;
      ratio2 = NormalRatio2;
        }
    }
#endif
        for (kk=0; kk<(Total_TX*Total_RX); kk++) // ---~700us
        {

    tmpu1 = (kk+1)%row_len;

    #if TP_CUSTOMER_PROTOCOL
//        ptrLastQuaDelta[kk] = ((pMuRaw_buf_T0[kk]+pMuRaw_buf_T1[kk]+pMuRaw_buf_T2[kk]+pMuRaw_buf_T3[kk])-
//                              ((S16)(((U16)pbT0[kk]+(U16)pbT1[kk]+(U16)pbT2[kk]+(U16)pbT3[kk]+2)>>2))>>2);  
        //ptrLastQuaDelta[kk] = (pMuRaw_buf_T3[kk] - (pbT3[kk]>>2));
      ptrLastQuaDelta[kk] = ((pMuRaw_buf_T3[kk] + pMuRaw_buf_T2[kk]) - ((pbT3[kk]+pbT2[kk]+2)>>2))>>1;  
//      ptrLastQuaDelta[kk] = (pMuRaw_buf_T3[kk] - (pbT3[kk]>>2));
//      ptrLastQuaDelta[kk] = (pMuRaw_buf_T3[kk] - ((pbT3[kk]+pbT2[kk]+2)>>3));  
    #else

      ptrLastQuaDelta[kk] = pMuRaw_buf_T3[kk]- ((pbT3[kk]+2)>>2);  
#if 0   
                if (charger_on_status == 1)
                {
                #if REDUCE_XRAM
            ptrLastQuaDelta[kk] = ((pMuRaw_buf_T3[kk] + pMuRaw_buf_T2[kk]) - ((pbT3[kk]+pbT2[kk]+2)>>2))>>1;  
          #else
            ptrLastQuaDelta[kk] = ((pMuRaw_buf_T0[kk]+pMuRaw_buf_T1[kk]+pMuRaw_buf_T2[kk]+pMuRaw_buf_T3[kk])-
                                    ((S16)(((U16)pbT0[kk]+(U16)pbT1[kk]+(U16)pbT2[kk]+(U16)pbT3[kk]+2)>>2))>>2);  
          #endif

                }
                else
                {
          ptrLastQuaDelta[kk] = (pMuRaw_buf_T3[kk] - (pbT3[kk]>>2));

                }
#endif
    #endif


#if LCM_ANALYSIS
  if ( flKeyPara.type==MUTU_RIGHTKEY )
  {
    if((tmpu1==LStart) || (tmpu1==RStart))
    {
      usedflag = 0;
      tmps2 = 0;
      tmpcounter = 0;
    }else if(((tmpu1>=(LStart+1)) && (tmpu1<=(LEnd-1))) || ((tmpu1>=(RStart+2)) && (tmpu1<=(REnd-2))))
    {
      tmps1=ptrLastQuaDelta[kk]-ptrLastQuaDelta[kk-1];
      if((tmps1> LCMthrLow) && (tmps1< LCMthrHigh) &&  (ptrLastQuaDelta[kk]< LCMDeltaHiThr)&&  (ptrLastQuaDelta[kk]>LCMDeltaLoThr))
      {
        tmps2 += ptrLastQuaDelta[kk];
        tmpcounter++;
        usedflag = 1;

      }else if(usedflag==1)
      {
        tmps2 -= ptrLastQuaDelta[kk-1];
        tmpcounter--;
        usedflag = 0;
      }

    }else if(tmpu1==LEnd)
    {
      if(tmpcounter>0)
        tmps2 = tmps2/tmpcounter;
      for(tmpu2=0;tmpu2<LFTotalChno;tmpu2++)
      {
        #if IIREnable
          ptrLastQuaDelta[kk-tmpu2]  = ((ratio1*(ptrLastQuaDelta[kk-tmpu2] -tmps2)+(ratio2)*ptrLastQuaDelta_pre[kk-tmpu2])>>2);
        #else
          ptrLastQuaDelta[kk-tmpu2]  = ptrLastQuaDelta[kk-tmpu2] -tmps2;        
        #endif
      }
    }else if(tmpu1==(REnd-1))
    {
      if(tmpcounter>0)
        tmps2 = tmps2/tmpcounter;
      for(tmpu2=0;tmpu2<(RTTotalChno-1);tmpu2++)    // sub key channel
      {
        #if IIREnable
          ptrLastQuaDelta[kk-tmpu2]  =((ratio1*(ptrLastQuaDelta[kk-tmpu2] -tmps2)+(ratio2)*ptrLastQuaDelta_pre[kk-tmpu2])>>2);
        #else
          ptrLastQuaDelta[kk-tmpu2]  = ptrLastQuaDelta[kk-tmpu2] -tmps2;
        #endif

      }
    } 
  }
#endif

      if (flKeyPara.type==MUTU_RIGHTKEY)
      {
        if ((kk+1)%row_len == 0)
          ptrLastQuaDelta[kk] = 0;
      } 
      else if (flKeyPara.type==MUTU_DOWNKEY)
      {
        if ( kk >= ((Total_TX -1) * Total_RX) )
          ptrLastQuaDelta[kk] = 0;
      } 
#if DetectMutualDownKeySetting
      else if ((flKeyPara.type==MUTU_DOWNKEY) && (CoaxisKey==0x11))   // co-tx
      {
        //if(kk>=dkey_aa_bound) ptrLastQuaDelta[kk] = 0;
        if (kk>= (row_len*CoaxisTxCHno))
          ptrLastQuaDelta[kk] = 0;
      }
#endif
    if (ptrLastQuaDelta[kk]>deltaMax)
            {
                deltaMax=ptrLastQuaDelta[kk];
#ifdef JUST_for_MobilePhone_DEBUG
                Debug_MutuDeltaMax = ptrLastQuaDelta[kk];
#endif
            }
        }
#if SZ_Floating_Poly3
        if ((deltaMax) >= (flRawPara.Mutu_TH<<1))
        {
            fgDyDetectLARGEAREA=1;
        }
        else
        {
            fgDyDetectLARGEAREA=0;
        }
#endif
#if 0 // SZ_Floating_Poly2
        if (nFloatDynamicTH == 0)
        {
            if ((deltaMax < (DynamicTH * 2)) || (deltaMax > (DynamicTH * 4)))
            {
                nFloatDynamicTHCounter_LToH++;
                if (nFloatDynamicTHCounter_LToH > 3) // floating -> normal no change
                {
                    nFloatDynamicTH=1;   // enforce to TH_H
                    nFloatDynamicTHCounter_HToL=0;
                }
            }
            else
                nFloatDynamicTHCounter_LToH=0;
        }
        else if ( nFloatDynamicTH == 1 )
        {
            if ( deltaMax > (DynamicTH * 2))
                nFloatDynamicTH=2;
            if ((deltaMax >= (DynamicTH * 2)) && (deltaMax <= (DynamicTH * 4)))
            {
                nFloatDynamicTHCounter_HToL++;
                if (nFloatDynamicTHCounter_HToL >3) // delay issue
                {
                    nFloatDynamicTH=0;   // enforce to TH_L
                    nFloatDynamicTHCounter_LToH=0;
                }
            }
            else
                nFloatDynamicTHCounter_HToL=0;
        }
        else if ( nFloatDynamicTH == 2 )
        {
            if (deltaMax <= (DynamicTH * 2))
                nFloatDynamicTH=1;
        }
#endif
        SearchDeltaPeak(ptrLastQuaDelta,  row_count,  row_len,fingerlevel); // -------------------------------------------------~ 3.3 ms
    }
    else
    {
        SearchDeltaPeak(row_ptr,  row_count,  row_len,fingerlevel);
    }
#else
    SearchDeltaPeak(row_ptr,  row_count,  row_len,fingerlevel); // delta serch peak , no compare fingerlevel
#endif
    if (searchMode==MUTUAL_MODE)
    {
        // ---- add peak algorithm , throw the fake peak ----
        // ---- register peak to buffer , and compare fingerlevel ----
        //#if Twokey_debug_SearchRowPeak
#if 0
        if (prePeaks > 0)
        {
            for (kk=0; kk<prePeaks; kk++)
            {
                temp=peak[0][kk] *AlgorithmParam.h_cols + peak[1][kk];
                printf("*****pD (%bu, %bu):%4d, V(%4d, %4d), H(%4d, %4d)\r\n", peak[1][kk], peak[0][kk], ptrLastQuaDelta[temp], ptrLastQuaDelta[temp-AlgorithmParam.h_cols], ptrLastQuaDelta[temp+AlgorithmParam.h_cols], ptrLastQuaDelta[temp-1], ptrLastQuaDelta[temp+1]);
            }
            printf("\n");
        }
#endif
#if palm_solution
        PalmRejection(); //[ jackson ]
#endif
        peaks=0;
        //_2FingerPitch_Improve(row_ptr,  row_count,  row_len,fingerlevel); // -----------------~ 651us
        nowRow_ptr=ptrLastQuaDelta;
        peaks=0;
        //peakmax = peakmax/2;
#if OpenFWNoiseCheck
        //if (OneFrDeltaAvg>0)
        //  fingerlevel = fingerlevel+OneFrDeltaAvg;
#endif
        peak_TH=deltaMax/3;
        for (row_id=0 ; row_id< row_count; row_id++) //---------------------------------B ~683us
        {
            for (col_id=0 ; col_id< row_len; col_id++)
            {
                fbyte=col_id>>3;
                fbit=col_id & 0x7;
                if (peaks<MaxProcessPeak)
                {
                    if ((peak_flag[row_id][fbyte] &BIT8_MASK[fbit])) //>0
                    {
                        if ( ptrLastQuaDelta[row_id*row_len +col_id] >= peak_TH ) //deltaMax
                        {
#if OpenFWNoiseCheck
#if SZ_Floating_Poly3
                            if (fgDyDetectLARGEAREA == 1) // only normal mode
                            {
                                fgFWCheck=1;
                                if (fnIsOnEdge(row_id, col_id, 0) ==0)
                                    //if( (row_id!=0) && (col_id!=0) && (row_id != (row_count-1)) && (col_id != (row_len-1)) )
                                {
                                    FWpos=row_id*row_len +col_id;
                                    NeiborUp=ptrLastQuaDelta[FWpos-row_len];
                                    NeiborDn=ptrLastQuaDelta[FWpos+row_len];
                                    NeiborLf=ptrLastQuaDelta[FWpos-1];
                                    NeiborRt=ptrLastQuaDelta[FWpos+1];
                                    NeiborUpDn=NeiborUp+ NeiborDn;
                                    NeiborLfRt=NeiborLf + NeiborRt;
                                    if (NeiborUpDn < 0)
                                        NeiborUpDn=0;
                                    if (NeiborLfRt < 0)
                                        NeiborLfRt=0;
                                    if ( NeiborUpDn > (NeiborLfRt*3) || ( NeiborLfRt  > (NeiborUpDn*3)) )
                                        //if (0)
                                    {
                                        fgFWCheck=0;
                                        // --- 2012/11/20
#if OpenFWNoiseCheck_2
                                        if ((NeiborUp >= 0) && (NeiborDn >=0) && (NeiborLf>=0)&&(NeiborRt>=0))
                                        {
                                            fgFWCheck=1;
                                        }
                                        else
                                        {
#if debug_Charger
                                            printf("@@(%02bu,%02bu)\r\n",row_id,col_id);
#endif
                                        }
#endif
                                        //printf("Lost UPDN=%d  LFRT=%d!!\n",NeiborUpDn,NeiborLfRt);
                                        //printf("C: %d, V:(%d, %d)  H:(%d, %d) \r\n",pMutualdelta[FWpos], pMutualdelta[FWpos-row_len],pMutualdelta[FWpos+row_len],pMutualdelta[FWpos-1],pMutualdelta[FWpos+1]);
#if 1
                                        if (fgFWCheck==0)
                                            if (IsSavePoint(row_id,col_id))
                                            {
                                                fgFWCheck=1;
                                            }
#else
                                        for (aa=0 ; aa< prePeaks; aa++)
                                        {
                                            //if((prePeak[0][aa] == row_id) && (prePeak[1][aa] == col_id))
                                            if ((abs(prePeak[0][aa] - row_id) < 2)  && (prePeak[1][aa] == col_id))
                                            {
                                                if ( fnIsOnEdge(row_id, col_id, 0)==0 )
                                                {
                                                    fgFWCheck=1;
#if debug_Charger
                                                    printf("save (%03bu,%03bu)\r\n",row_id,col_id);
#endif
                                                }
                                                break;
                                            }
                                        }
#endif
#if 0
                                        for (ii=-1; ii<2; ii++)
                                        {
                                            for (jj=-1; jj<2; jj++)
                                            {
                                                printf(" %3d,%d,%d ",ptrLastQuaDelta[(row_id+ii)*row_len +(col_id+jj)],(row_id+ii),(col_id+jj));
                                            }
                                            printf("\r\n");
                                        }
                                        printf("\r\n");
#endif
                                    }
                                    else
                                        fgFWCheck=1;
                                }
                            }
#endif
#endif
#if palm_solution
                            PID_result=PalmIDCheck(row_id, col_id);
                            if (PID_result>0&& PID_result<3)
#endif
#if OpenFWNoiseCheck
                                if (fgFWCheck)
#endif
                                {
#if debug_Charger
                                    FWpos=row_id*row_len +col_id;
                                    NeiborLfRt = ((MutualBaseLine[freqIndex][FWpos])>>2);

                                    printf("(%03bu,%03bu),m3=%03d"
                                           , row_id,col_id
                                           ,ptrLastQuaDelta[FWpos]      );
                                    NeiborUpDn=((  (S16)pMuRaw_buf_T2[FWpos])- NeiborLfRt) ;
                                    printf(",m2=%03d",NeiborUpDn);
                                    NeiborUpDn=((  (S16)pMuRaw_buf_T1[FWpos])- NeiborLfRt) ;
                                    printf(",m1=%03d",NeiborUpDn);
                                    NeiborUpDn=((  (S16)pMuRaw_buf_T0[FWpos])- NeiborLfRt) ;
                                    printf(",m0=%03d,%03bu\r\n",NeiborUpDn,gbCNter);
#endif
                                    peak[0][peaks]=row_id;
                                    peak[1][peaks]=col_id;
                                    peaks++;
                                }
                        }
                    }
                }
            }
            //nowRow_ptr+=row_len;
            nowRow_ptr+=Total_RX;
        } //--------------------------------------------------------------B End
#if PEAK_EXTEND // Extend "SEARCH_NEW_DELTA_PEAK" to ensure the peak is the local maximum.
        if ( peaks > 0 ) //--------------------------------------------------C, 5F~ 330us
        {
            for ( fbit=0; fbit < peaks; fbit++ )
            {
                fgNine=0;
                while ( fgNine != ((EDir[1]-EDir[0]+1)*(EDir[3]-EDir[2]+1)) )
                {
                    fgNine=0;
                    EDir[0]=( peak[0][fbit] == 0                                         ) ? peak[0][fbit] : peak[0][fbit]-1;
                    EDir[1]=( peak[0][fbit] == AlgorithmParam.v_rows-1 ) ? peak[0][fbit] : peak[0][fbit]+1;
                    EDir[2]=( peak[1][fbit] == 0                                         ) ? peak[1][fbit] : peak[1][fbit]-1;
                    EDir[3]=( peak[1][fbit] == AlgorithmParam.h_cols-1 ) ? peak[1][fbit] : peak[1][fbit]+1;
                    for ( C1=EDir[0]; C1 < EDir[1]+1; C1++ )
                        for ( C2=EDir[2]; C2 < EDir[3]+1; C2++ )
                            if ( pMutualdelta[C1*AlgorithmParam.h_cols+C2] > pMutualdelta[peak[0][fbit] *AlgorithmParam.h_cols+peak[1][fbit]] )
                            {
                                peak[0][fbit]=C1;
                                peak[1][fbit]=C2;
                            }
                            else
                            {
                                fgNine++;
                            }
                }
            }
        }
#endif
#if Twokey_debug_SearchRowPeak
        if (0)
            //if (peaks > 0)
        {
            maxD=0;
            for (kk=0; kk<peaks; kk++)
            {
                temp=peak[0][kk] *AlgorithmParam.h_cols + peak[1][kk];
                printf("pD (%bu, %bu):%4d, V(%4d, %4d), H(%4d, %4d)\r\n", peak[1][kk], peak[0][kk], ptrLastQuaDelta[temp], ptrLastQuaDelta[temp-AlgorithmParam.h_cols], ptrLastQuaDelta[temp+AlgorithmParam.h_cols], ptrLastQuaDelta[temp-1], ptrLastQuaDelta[temp+1]);
                if (maxD < ptrLastQuaDelta[temp]) maxD=ptrLastQuaDelta[temp];
            }
            printf("\n");
            maxD=maxD *1 /2;
            for (kk=0; kk<peaks; kk++)
            {
                temp=peak[0][kk] *AlgorithmParam.h_cols + peak[1][kk];
                if (maxD >      ptrLastQuaDelta[temp])
                {
                    printf("Revpek-1: (%bu, %bu)->(%4d, %4d)\r\n",peak[1][kk], peak[0][kk],maxD,ptrLastQuaDelta[temp] );
                }
                sV=ptrLastQuaDelta[temp-AlgorithmParam.h_cols] + ptrLastQuaDelta[temp+AlgorithmParam.h_cols];
                sH=ptrLastQuaDelta[temp-1]+ptrLastQuaDelta[temp+1];
                if (((sV > 3*sH) || (sH > 3*sV))&& ((ptrLastQuaDelta[temp-AlgorithmParam.h_cols]<0)||(ptrLastQuaDelta[temp+AlgorithmParam.h_cols]<0)||( ptrLastQuaDelta[temp-1]<0)||( ptrLastQuaDelta[temp+1]<0)))
                    printf("Revpek-2: (%bu, %bu)->(%4d, %4d)\r\n",peak[1][kk], peak[0][kk],sV, sH);
            }
        }
#endif
        //--------------------------------------------------C End
        LastAvailableFingerNo=peaks;
        TouchFingersList->fingers_number=peaks;
#if LARGE_AREA
#if 0 // SZ_Floating_Poly2
        if (nFloatDynamicTH >= 1)
            DynamLEVEL=DynamicTH;   // normal mode
        else
            DynamLEVEL=LARGEAREA_THD;   // floating mode
#endif
        /* Crovax-2103.05.07
         #if SZ_Floating_Poly3
                if (fgDyDetectLARGEAREA == 1)
                        DynamLEVEL_TH = (LARGEAREA_THD<<1);
                else
                        DynamLEVEL_TH = LARGEAREA_THD;
         #endif
         */
        // Crovax 2013.05.07, Change Area Weighting Sum to ratio base with max delta.
        DynamLEVEL_TH=( deltaMax / flLargeAreaPara.LargeArea_thd_ratio );
        for (addr_idx=0, row_id=0; row_id < AlgorithmParam.v_rows; row_id++)
            for ( col_id=0; col_id < AlgorithmParam.h_cols; col_id++, addr_idx++)
                if ((flKeyPara.type == MUTU_RIGHTKEY) && (col_id == (AlgorithmParam.h_cols-1) ))          
                {
        AreaChkFlag[row_id][col_id] = 0;    // key area don't care, clear to '0'.
                }
      else
                    AreaChkFlag[row_id][col_id]= (ptrLastQuaDelta[addr_idx]>=DynamLEVEL_TH)?  1 : 0;
        for ( kk=0; kk<TouchFingersList->fingers_number; kk++ )
            PeakArea[kk]=AreaCheck(peak[0][kk], peak[1][kk], BoundSet[kk]);
        AreaProcess();
#endif
#if CoaxialLevelCheck
        Coaxial_LevelCheck();
#endif
        if (TouchFingersList->fingers_number> MAX_FINGERS)
        {
            TouchFingersList->fingers_number=MAX_FINGERS;
            LastAvailableFingerNo=MAX_FINGERS;
        }
    }
    else // searchMode == SELF_MODE //self
    {
#if Self_Process
        if (bSelfProcessEn) //AndyAdd_0719_0001
        {
            // find delta max
            deltaMax=0;
            for (row_id=0 ; row_id< row_len; row_id++)
            {
                if (row_ptr[row_id]>deltaMax)
                {
                    deltaMax=row_ptr[row_id];
                }
            }
  #ifdef JUST_for_MobilePhone_DEBUG
  Debug_SelfDeltaMax = deltaMax;
  #endif
            deltaMax=deltaMax>>2;   //>>1;

            //
            peaks=0 ;
            for (row_id=0 ; row_id< row_len; row_id++)
            {
                fbyte=row_id>>3;
                fbit=row_id&0x7;
                if (peaks<5)
                {
                    if ((peak_flag[0][fbyte] &BIT8_MASK[fbit])&& (row_ptr[row_id]>=deltaMax) )
                    {
                        // Crovax Foe Edge First Touch -- Star 05/10
                        // -----------------------------------
                        // When peak was in edge and at first touch, the threshold was set to 1.5x self first touch threshold.
                        //
                        if ( (last_finger==0) && ( (row_id==0) || (row_id==(row_len-1))  ) )
                        {
                            if ( row_ptr[row_id] >= ((flRawPara.Self_first_TH*3)>>1) )
                            {
                                ptrSelfPeak[peaks]=row_id;
                                peaks++;
                            }
                        }
                        else
                            // Crovax Foe Edge First Touch -- End
                        {
                            ptrSelfPeak[peaks]=row_id;
                            peaks++;
                        }
                    }
                }
            }
            return peaks;
        }
#endif
#if 0
        peaks=0;
        nowRow_ptr=row_ptr;
        for (row_id=0 ; row_id< row_count; row_id++)
        {
            for (col_id=0 ; col_id< row_len; col_id++)
            {
                fbyte=col_id>>3;
                fbit=col_id&0x7;
                if ((peak_flag[row_id][fbyte] &BIT8_MASK[fbit])>0)
                {
                    if (nowRow_ptr[col_id] >= fingerlevel )
                    {
                        // new added Twokey
#if PEAK_IMPROVE
                        if ((col_id > 0) && (col_id < row_len-2))
                        {
                            for (kk=col_id; kk<col_id+3; kk++)
                            {
                                if (nowRow_ptr[kk] > nowRow_ptr[kk-1])
                                    dir[kk-col_id]=1;
                                else if (nowRow_ptr[kk] == nowRow_ptr[kk-1])
                                    dir[kk-col_id]=0;
                                else if (nowRow_ptr[kk] < nowRow_ptr[kk-1])
                                    dir[kk-col_id]=-1;
                            }
                            if ((dir[0] == dir[2]) && (dir[0] != dir[1]))
                            {
                                // 1 -1 1 or 1 0 1
                                if ((nowRow_ptr[col_id]-nowRow_ptr[col_id+1])>PEAK_DIFF_THD)
                                    peaks=1;
                                else
                                {
                                    peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];  // clear the bit
#if DBG_RPT
                                    printf("**Peak %bu Remove:\t", col_id);
                                    printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id-1],nowRow_ptr[col_id],nowRow_ptr[col_id+1],nowRow_ptr[col_id+2]);
#endif
                                }
                            }
                            else if ( ( (nowRow_ptr[col_id+2]-nowRow_ptr[col_id]) /25 ) > (nowRow_ptr[col_id]-nowRow_ptr[col_id+1]) || ( (nowRow_ptr[col_id-2]-nowRow_ptr[col_id])/25) > (nowRow_ptr[col_id]-nowRow_ptr[col_id-1]) )
                            {
                                peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];
#if DBG_RPT
                                printf("**Peak %bu Remove:\t", col_id);
                                printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id],nowRow_ptr[col_id+1],nowRow_ptr[col_id+2],nowRow_ptr[col_id+3]);
#endif
                            }
                            else
                            {
                                peaks=1;
                            }
                        }
                        else if ( col_id == 0 )
                        {
                            for ( kk=col_id+1; kk < col_id+4; kk++ )
                            {
                                if (nowRow_ptr[kk] > nowRow_ptr[kk-1])
                                    dir[kk-col_id-1]=1;
                                else if (nowRow_ptr[kk] == nowRow_ptr[kk-1])
                                    dir[kk-col_id-1]=0;
                                else if (nowRow_ptr[kk] < nowRow_ptr[kk-1])
                                    dir[kk-col_id-1]=-1;
                            }
                            if ( (dir[1]== dir[2]) && ( dir[1]!=dir[0]) )
                            {
                                if (( nowRow_ptr[col_id]-nowRow_ptr[col_id+1])>PEAK_DIFF_THD)
                                    peaks=1;
                                else
                                {
                                    peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];
#if DBG_RPT
                                    printf("**Peak %bu Remove:\t", col_id);
                                    printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id],nowRow_ptr[col_id+1],nowRow_ptr[col_id+2],nowRow_ptr[col_id+3]);
#endif
                                }
                            }
                            else if ( ((nowRow_ptr[col_id+2]-nowRow_ptr[col_id])/25) > (nowRow_ptr[col_id]-nowRow_ptr[col_id+1]))
                            {
                                peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];
#if DBG_RPT
                                printf("**Peak %bu Remove:\t", col_id);
                                printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id],nowRow_ptr[col_id+1],nowRow_ptr[col_id+2],nowRow_ptr[col_id+3]);
#endif
                            }
                            else
                            {
                                peaks=1;
                            }
                        }
                        else
                        {
                            for ( kk=col_id-2; kk <= col_id; kk++ )
                            {
                                if (nowRow_ptr[kk] > nowRow_ptr[kk-1])
                                    dir[kk-col_id+2]=1;
                                else if (nowRow_ptr[kk] == nowRow_ptr[kk-1])
                                    dir[kk-col_id+2]=0;
                                else if (nowRow_ptr[kk] < nowRow_ptr[kk-1])
                                    dir[kk-col_id+2]=-1;
                            }
                            if ( (dir[0]==dir[1]) && ( dir[1]!=dir[2]) )
                            {
                                if (( nowRow_ptr[col_id]-nowRow_ptr[col_id-1])>PEAK_DIFF_THD )
                                {
                                    peaks=1;
                                }
                                else
                                {
                                    peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];
#if DBG_RPT
                                    printf("**Peak %bu Remove:\t", col_id);
                                    printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id-3],nowRow_ptr[col_id-2],nowRow_ptr[col_id-1],nowRow_ptr[col_id]);
#endif
                                }
                            }
                            else if ( ((nowRow_ptr[col_id-2]-nowRow_ptr[col_id])/25) > (nowRow_ptr[col_id]-nowRow_ptr[col_id-1]))
                            {
                                peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];
#if DBG_RPT
                                printf("**Peak %bu Remove:\t", col_id);
                                printf("%4d, %4d, %4d, %4d\n\r", nowRow_ptr[col_id-3],nowRow_ptr[col_id-2],nowRow_ptr[col_id-1],nowRow_ptr[col_id]);
#endif
                            }
                            else
                            {
                                peaks=1;
                            }
                        }
#else
                        peaks=1;
#endif
                        //printf("peak delta = %04d %04u\r\n",nowRow_ptr[col_id],fingerlevel);
                    }
                    else
                    {
                        peak_flag[row_id][fbyte]&=~BIT8_MASK[fbit];  // clear the bit
                    }
                }
            }
        }
#endif
    }
    prePeaks=TouchFingersList->fingers_number;
    for (row_id=0 ; row_id< prePeaks; row_id++)
    {
        prePeak[0][row_id]=peak[0][row_id];
        prePeak[1][row_id]=peak[1][row_id];
    }
#if 0 // SZ_Floating_DEBUG
    peakCounter=(U8)prePeaks;
    peak_1st=0;
    peak_2nd=0;
    peak_3rd=0;
    peak_4th=0;
    if (peakCounter)
    {
        peak_1st=ptrLastQuaDelta[(peak[0][0] *AlgorithmParam.h_cols + peak[1][0])];
        peakCounter--;
    }
    if (peakCounter)
    {
        peak_2nd=ptrLastQuaDelta[(peak[0][1] *AlgorithmParam.h_cols + peak[1][1])];
        peakCounter--;
    }
    if (peakCounter)
    {
        peak_3rd=ptrLastQuaDelta[(peak[0][2] *AlgorithmParam.h_cols + peak[1][2])];
        peakCounter--;
    }
    if (peakCounter)
    {
        peak_4th=ptrLastQuaDelta[(peak[0][3] *AlgorithmParam.h_cols + peak[1][3])];
        peakCounter--;
    }
#endif
    return peaks;
}

#if Self_Process  //---- for self  tim
#if 0
S16 CalSelfAddress_Edge( S16 *InputBand,U8 PeakCh)  //tim Modify orginal Crovax's code
{
    S16 data TmpBand[3], EdgeTmpBand[3];
    S16 data ReturnPos;
    TmpBand[0]=InputBand[0];
    TmpBand[1]=InputBand[1];
    TmpBand[2]=InputBand[2];
    if ( PeakCh == 0 )
    {
        if ( (TmpBand[0]==0) && (TmpBand[1]==0) )
        {
            ReturnPos=0;
        }
        else
        {
            EdgeTmpBand[0]=( TmpBand[0]+4 ) >> 3;
            EdgeTmpBand[1]=( TmpBand[1]+4 ) >> 3;
            ReturnPos=(( EdgeTmpBand[0] *32 + EdgeTmpBand[1] *96 + ( (EdgeTmpBand[0]+EdgeTmpBand[1])>>1))<<1) / ( EdgeTmpBand[0]+EdgeTmpBand[1] );
            ReturnPos=ReturnPos - RES_SCALE;
        }
    }
    else
    {
        if ( (TmpBand[1]==0) && (TmpBand[2]==0) )
        {
            ReturnPos=RES_SCALE;
        }
        else
        {
            EdgeTmpBand[1]=( TmpBand[1]+4 ) >> 3;
            EdgeTmpBand[2]=( TmpBand[2]+4 ) >> 3;
            ReturnPos=( ( EdgeTmpBand[1] *32 + EdgeTmpBand[2] *96 + ( (EdgeTmpBand[1]+EdgeTmpBand[2])>>1))<<1) / ( EdgeTmpBand[1]+EdgeTmpBand[2] );
            ReturnPos=ReturnPos-(RES_SCALE<<1);
        }
    }
    if ( ReturnPos<0 ) ReturnPos=0;
    else if (ReturnPos>64) ReturnPos=64;
    ReturnPos+=PeakCh*RES_SCALE;
    return ReturnPos;
}

#endif
S16 CalSelfAddress( S16 *InputBand,U8 PeakCh,U8 X1Y0)  //tim Modify orginal Crovax's code
{
    S16 data Addr=0;
    U8 data CHMax ;
    if (X1Y0) // X
    {
//        CHMax=AlgorithmParam.h_cols-1;
  CHMax = AA_RxLen-1;
    }
    else            // Y
    {
//        CHMax=AlgorithmParam.v_rows-1;
  CHMax = AA_TxLen-1;
    }
    if (PeakCh==0)
    {
        //Addr = CalSelfAddress_Edge(&InputBand[PeakCh],PeakCh); // CalSelfAddress_Edge
        Addr=EdgeWeightingExtend(&InputBand[PeakCh], PeakCh,0, X1Y0);
    }
    else if (PeakCh==CHMax )
    {
        //Addr = CalSelfAddress_Edge(&InputBand[PeakCh-2],PeakCh); // CalSelfAddress_Edge
        Addr=EdgeWeightingExtend(&InputBand[PeakCh-2], PeakCh,0, X1Y0);
    }
    else // center area
    {
#if SELF_WEIGHTING
        Addr=WeightingSumSelfMain( &InputBand[0], PeakCh, CHMax );
#else
        Addr=IsolateTrianglePure( &InputBand[PeakCh-1], PeakCh);
#endif
    }
    return Addr;
}

#if !StandardORLibrary
#define SelfToMutualRange
void SelfToMutuBound(U8 *Bound,U8 Peak0, U8 Peak1)
{
  U8 tmpval;

  tmpval = ((Peak0>=2) ? (Peak0-2):(0)); 
  Bound[2]=tmpval;
  tmpval = ((Peak0<(AlgorithmParam.v_rows-2-1)) ? (Peak0+2):(AlgorithmParam.v_rows-1-1)); 
  Bound[3]=tmpval;
  tmpval = ((Peak1>=2) ? (Peak1-2):(0)); 
  Bound[0]=tmpval;
  tmpval = ((Peak1<(AlgorithmParam.h_cols-2)) ? (Peak1+2):(AlgorithmParam.h_cols-1)); 
  Bound[1]=tmpval;
}
#endif

void ComFuncCallToSaveCode()
{
    TouchFingersList->Pos[0][0] = CalSelfAddress( pSelfdelta, peak[0][0],0);
    //TouchFingersList->Pos[1][0] = CalSelfAddress( &pSelfdelta[AA_TxLen], peak[1][0],1);
    TouchFingersList->Pos[1][0] = CalSelfAddress( &pSelfdelta[Total_TX], peak[1][0],1);

    TouchFingersList->Pos[0][1] = CalSelfAddress( pSelfdelta, peak[0][1],0);
    //TouchFingersList->Pos[1][1] = CalSelfAddress( &pSelfdelta[AA_TxLen], peak[1][1],1);
    TouchFingersList->Pos[1][1] = CalSelfAddress( &pSelfdelta[Total_TX], peak[1][1],1);

    TouchFingersList->fingers_number =2;

#if SelfTrick_Poly
    if (((Self_0x13_flag == 1) ||(Self_0x31_flag == 1)) )
    {
    TouchFingersList->Pos[0][2] = CalSelfAddress( pSelfdelta, peak[0][2],0);
    //TouchFingersList->Pos[1][2] = CalSelfAddress( &pSelfdelta[AA_TxLen], peak[1][2],1);
    TouchFingersList->Pos[1][2] = CalSelfAddress( &pSelfdelta[Total_TX], peak[1][2],1);
    TouchFingersList->fingers_number =3;
    }
#endif

//    mutuPoint = 0;
  EnForceSelf = 1;
}

void fnSelf_Process()
{
#if 1
    U8 data ii,Tx_peak=0,Rx_peak=0,peak_index,Reverse; //,tx_Ch,rx_Ch;
    S16 Self_Th;
//  bit self_master_flag = 0; // TouchFingersList->fingers_number <3

#if OriginalSelfModeJudge
    S16 delta_max;
    U8 str,stop,jj;
#endif
//  U16 mutmp01,mutmp02,mutmp03;//,mutmp04;
//  S16 mutmpsum01,mutmpsum02,mutmpsum03;//,mutmpsum04;


//  self_master_flag = ( (fgDyDetectLARGEAREA==0) || (TouchFingersList->fingers_number <3) ) ?   1 : 0 ;

//    tx_Ch = AlgorithmParam.v_rows;
//    rx_Ch = AlgorithmParam.h_cols;

//  if (flKeyPara.type==Mutu_Downkey)
//    rx_Ch -= 1;

#if SelfTrick_Poly
  Self_0x12_flag = 0;
  Self_0x21_flag = 0;
  Self_0x13_flag = 0;
  Self_0x31_flag = 0;
//  Self_0x14_flag = 0;
//  Self_0x41_flag = 0;
#endif

    for (ii =0; ii<(MAX_FINGERS);ii++)
    {
        selfpeak[0][ii] =0;
        selfpeak[1][ii] =0;
    }

  if (charger_on_status == 0)
  {
    Self_Th = (last_finger==0) ?  flRawPara.Self_first_TH : flRawPara.Self_TH; // detect is first touch

  }
  else
    Self_Th = flRawPara.Self_first_TH;

    ptrSelfPeak = selfpeak[0];
    //Tx_peak = SearchRowPeak(pSelfdelta,1,AA_TxLen,Self_Th,SELF_MODE);
    Tx_peak = SearchRowPeak(pSelfdelta,1,Total_TX,Self_Th,SELF_MODE);
    ptrSelfPeak = selfpeak[1];
    //Rx_peak = SearchRowPeak(&pSelfdelta[AA_TxLen],1,AA_RxLen,Self_Th,SELF_MODE);
    Rx_peak = SearchRowPeak(&pSelfdelta[Total_TX],1,AA_RxLen,Self_Th,SELF_MODE);

//  SelfFingerNo = (Tx_peak >= Rx_peak) ? Tx_peak : Rx_peak;

    peak_index = (Tx_peak<<4) | (Rx_peak);



//  if (MutuFingerNo > SelfFingerNo)
//    peak_index = 0xFF;

#ifdef SZ_Floating_DEBUGProtocol_Tim
    //self_peakX = 0;
    //self_peakY = 0;
    //self_peakX = peak_index + 140;
#endif
    switch (peak_index)
    {
    case 0x00: case 0x10: case 0x01:
        TouchFingersList->fingers_number =0;
  #if SelfTrick_Poly
      Coaxis_2Finger = 0;
      Coaxis_3Finger = 0;
  #endif
  EnForceSelf = 1;  
        break;
    case 0x11:
        peak[0][0] = selfpeak[0][0];
        peak[1][0] = selfpeak[1][0];
        TouchFingersList->Pos[0][0] = CalSelfAddress( pSelfdelta, selfpeak[0][0],0);
        //TouchFingersList->Pos[1][0] = CalSelfAddress( &pSelfdelta[AA_TxLen], selfpeak[1][0],1);
        TouchFingersList->Pos[1][0] = CalSelfAddress( &pSelfdelta[Total_TX], selfpeak[1][0],1);
        TouchFingersList->fingers_number =1;

//    printf("SF[0][0]:%bu  SF[1][0]:%bu ===  Y:%d X:%d  \n\r",selfpeak[0][0], selfpeak[1][0], TouchFingersList->Pos[0][0], TouchFingersList->Pos[1][0]);
//        mutuPoint = 0;
  EnForceSelf = 1;
#ifdef SZ_Floating_DEBUGProtocol_Tim
        //self_peakX = pSelfdelta[tx_Ch+selfpeak[1][0]] +100;
        //self_peakY = pSelfdelta[selfpeak[0][0]] + 100;
#endif
  #if SelfTrick_Poly
      Coaxis_2Finger = 0;
      Coaxis_3Finger = 0;
  #endif
        break;

#if 1
    case 0x22:
        Reverse =0;

  if ((MutuFingerNo<3) || (charger_on_status == 1))

        {
            if ((abs(selfpeak[0][1] - selfpeak[0][0])>1) && (abs(selfpeak[1][1] - selfpeak[1][0])>1))
            {

                if ((ptrLastQuaDelta[selfpeak[0][0]*Total_RX + selfpeak[1][0]]+ptrLastQuaDelta[selfpeak[0][1]*Total_RX + selfpeak[1][1]]) <
                    (ptrLastQuaDelta[selfpeak[0][1]*Total_RX + selfpeak[1][0]]+ptrLastQuaDelta[selfpeak[0][0]*Total_RX + selfpeak[1][1]]) )
                {
                      Reverse =1;
                }
      peak[0][0] = selfpeak[0][0];
      peak[0][1] = selfpeak[0][1];    
                if (Reverse )
                {
//                    peak[0][0] = selfpeak[0][0];
                    peak[1][0] = selfpeak[1][1];
//                    peak[0][1] = selfpeak[0][1];
                    peak[1][1] = selfpeak[1][0];
                }
                else
                {
//                    peak[0][0] = selfpeak[0][0];
                    peak[1][0] = selfpeak[1][0];
//                    peak[0][1] = selfpeak[0][1];
                    peak[1][1] = selfpeak[1][1];
                }

    if (charger_on_status == 1)
      ComFuncCallToSaveCode();
    else
    {
#if !StandardORLibrary
      SelfToMutuBound(BoundSet[0],peak[0][0],peak[1][0]);
      SelfToMutuBound(BoundSet[1],peak[0][1],peak[1][1]);
#endif
      EnForceSelf = 0;
      TouchFingersList->fingers_number =2;
    }
              }
        }

        break;

  case 0x12:
  #if SelfTrick_Poly
  {
    if (charger_on_status == 1)
    {
    peak[0][0] = selfpeak[0][0];    // -----
    peak[1][0] = selfpeak[1][0];
    peak[0][1] = selfpeak[0][0];
    peak[1][1] = selfpeak[1][1];

      ComFuncCallToSaveCode();
    }
    else
    {
#if !StandardORLibrary
      SelfToMutuBound(BoundSet[0],peak[0][0],peak[1][0]);
      SelfToMutuBound(BoundSet[1],peak[0][1],peak[1][1]);
#endif
      EnForceSelf = 0;
      TouchFingersList->fingers_number =2;
    }
  }
  #endif
  break;

  case 0x21:
  #if SelfTrick_Poly
  {
    if (charger_on_status == 1)
    {
    peak[0][0] = selfpeak[0][0];
    peak[1][0] = selfpeak[1][0];    // -----
    peak[0][1] = selfpeak[0][1];
    peak[1][1] = selfpeak[1][0];

      ComFuncCallToSaveCode();
    }
    else
    {
#if !StandardORLibrary
      SelfToMutuBound(BoundSet[0],peak[0][0],peak[1][0]);
      SelfToMutuBound(BoundSet[1],peak[0][1],peak[1][1]);
#endif
      EnForceSelf = 0;
      TouchFingersList->fingers_number =2;
    }
  }
  #endif
  break;
#endif

    case 0x13:
    #if SelfTrick_Poly
    if (charger_on_status == 1) 
    {
      {
        peak[0][0] = selfpeak[0][0];    // -----
        peak[1][0] = selfpeak[1][0];
        peak[0][1] = selfpeak[0][0];
        peak[1][1] = selfpeak[1][1];
        peak[0][2] = selfpeak[0][0];
        peak[1][2] = selfpeak[1][2];
        Self_0x13_flag= 1;
        ComFuncCallToSaveCode();
      }
    }
    #endif
    break;

    case 0x31:
    #if SelfTrick_Poly
    if (charger_on_status == 1) 
    {
      {
        peak[0][0] = selfpeak[0][0];
        peak[1][0] = selfpeak[1][0];    // -----
        peak[0][1] = selfpeak[0][1];
        peak[1][1] = selfpeak[1][0];
        peak[0][2] = selfpeak[0][2];
        peak[1][2] = selfpeak[1][0];
        Self_0x31_flag= 1;
        ComFuncCallToSaveCode();
      }
    }
    #endif
  default:
        break;
    }

    //printf("0x%bX\r\n",peak_index);
    //prepeak_index = peak_index;
#endif
}
#endif
//###################################################################################
//extern S16 mutukeyBaseline[]; // tim add mutual key buf
extern S16 mutukeyBaseline[][4];

extern S16 mutuKey_Raw[];
extern S16 Delta_key_LPF[]; //JS20130619a : Modify for FAE real mutual key case by Gavin
#if 0
void TpFillRawdataPacket()
{
    U16 ii=0;
    U8 jj=0;
    //if(ScanMode == SELF_MODE)
    {
        switch (OpMode[1]&0xC0)
        {
        case SELF_ROWDATA:    //0x40
            ptrReportSelfData=(S16*)pSelfTargetAddr;
            if (flKeyPara.type == SELF_KEY)
            {
                ii=AlgorithmParam.v_rows + AlgorithmParam.h_cols;
                for (jj=0; jj<flKeyPara.num; jj++)
                {
                    ptrReportSelfData[ii+jj]=pSelfTargetAddr[ii+jj];
                }
            }
            break;
        case SELF_BASELINE: //0x80
            ptrReportSelfData=(S16*)SelfBaseLine;
            if (flKeyPara.type == SELF_KEY)
            {
                ii=AlgorithmParam.v_rows + AlgorithmParam.h_cols;
                for (jj=0; jj<flKeyPara.num; jj++)
                {
                    ptrReportSelfData[ii+jj]=SelfBaseLine[ii+jj];
                }
            }
            break;
        case SELF_DELTA:  //0xC0
            ptrReportSelfData=(S16*)pSelfdelta;
            if (flKeyPara.type == SELF_KEY)
            {
                ii=AlgorithmParam.v_rows + AlgorithmParam.h_cols;
                for (jj=0; jj<flKeyPara.num; jj++)
                {
                    ptrReportSelfData[ii+jj]=pSelfdelta[ii+jj];
                }
            }
            break;
        default:
            break;
        }
    }
    //else
    {
        switch (OpMode[1]&0x30)
        {
        case MUTUAL_ROWDATA:  // 0x10
//#if REDUCE_XRAM
            ptrReportMutualData=(S16 *)pMuRaw_buf_T3;
//#else
//      ptrReportMutualData=(S16 *)pMuRaw_buf_T3;
//#endif
//        if(flKeyPara.CHS[7]==0xFF)    // D9 - self key
//        {
//          ii = AlgorithmParam.v_rows + AlgorithmParam.h_cols;
//          for(jj=0;jj<flKeyPara.num;jj++)
//          {
//            ptrReportMutualData[jj] = pSelfTargetAddr[ii +flKeyPara.CHS[jj]] ;
//          }
//        }
            // update key Raw data to (Tx * Rx) corresponding cell
            if ((flKeyPara.type == MUTU_DOWNKEY) || (flKeyPara.type == MUTU_RIGHTKEY))
            {
                for (jj=0; jj<flKeyPara.num; jj++)
                {
                    ptrReportMutualData[(flKeyPara.CHS[jj])]=pMuRaw_buf_T3[(flKeyPara.CHS[jj])];
                }
            }
            break;
        case MUTUAL_BASELINE: //0x20
            ptrReportMutualData=MutualBaseLine;
//        if(flKeyPara.CHS[7]==0xFF)    // D9 - self key
//        {
//          ii = AlgorithmParam.v_rows + AlgorithmParam.h_cols;
//          for(jj=0;jj<flKeyPara.num;jj++)
//          {
//            ptrReportMutualData[jj] = SelfBaseLine[ii +flKeyPara.CHS[jj]] ;
//          }
//        }
            // update Baseline data to (Tx * Rx) corresponding cell
            if ((flKeyPara.type == MUTU_DOWNKEY) || (flKeyPara.type == MUTU_RIGHTKEY))
            {
                for (jj=0; jj<flKeyPara.num; jj++)
                {
                    ptrReportMutualData[(flKeyPara.CHS[jj])]=mutukeyBaseline[jj]>>2;
                }
            }
            break;
        case MUTUAL_DELTA:  //0x30
            ptrReportMutualData=(S16 *)ptrLastQuaDelta;
            //if(flKeyPara.CHS[7]==0xFF)    // D9 - self key
            {
                ii = AlgorithmParam.v_rows + AlgorithmParam.h_cols;
                for (jj=0;jj<flKeyPara.num;jj++)
                {
                    ptrReportMutualData[jj] = pSelfdelta[ii +flKeyPara.CHS[jj]] ;
                }
            }
            // update Delta to (Tx * Rx) corresponding cell

            /*
                  if ((flKeyPara.type == MUTU_DOWNKEY) || (flKeyPara.type == MUTU_RIGHTKEY))
                  {
                    for (jj=0; jj<flKeyPara.num; jj++)
                    {
                      //ptrReportMutualData[(flKeyPara.CHS[jj])]=MutukeyDeltaArray[jj];
                      ptrReportMutualData[(flKeyPara.CHS[jj])]=Delta_key_LPF[jj]>>2;
                      //printf("FILL---mutuD[%bu] = %d \n\r",jj,MutukeyDeltaArray[jj]);
                    }
                  }

            */
            break;
        default:
            break;
        }
        ptrReportMutualData_1=&ptrReportMutualData[128];
    }
}
#else
//bit fg_delta=0;
void TpFillRawdataPacket()
{
    //U16 ii=0;
    S16 *dptr;
    U8 jj;
  //fg_delta = 0;
    switch (OpMode[1]&0xC0)
    {
 #if 1
    case SELF_ROWDATA:  //0x40
        ptrReportSelfData=(S16*)pSelfTargetAddr;
        dptr = &pSelfTargetAddr[Self_nodes];
        break;
    case SELF_BASELINE: //0x80
        ptrReportSelfData =  (S16*)SelfBaseLine[freqIndex];
        dptr = &SelfBaseLine[freqIndex][Self_nodes];
        break;
    case SELF_DELTA:  //0xC0
        ptrReportSelfData=(S16*)pSelfdelta;
        dptr = &pSelfdelta[Self_nodes];
    //fg_delta = 1;
        break;
#endif
    default:
        break;
    }


    dptr = NULL;
    switch (OpMode[1]&0x30)
    {
#if 1
    case MUTUAL_ROWDATA: // 0x10
        ptrReportMutualData=(S16 *)pMuRaw_buf_T3;
        break;
    case MUTUAL_BASELINE: //0x20
        ptrReportMutualData =MutualBaseLine[freqIndex];
    //#ifdef POLY_1128LIB
        //  dptr = mutukeyBaseline[freqIndex];
    //#else
          dptr = mutukeyBaseline[freqIndex];
    //#endif
        break;
    case MUTUAL_DELTA: //0x30
        ptrReportMutualData=(S16 *)ptrLastQuaDelta;
//  ptrLastQuaDelta[0]=(UsedCTFreq[freqIndex]<<2);
#if 0
  ptrLastQuaDelta[0]=UsedCTFreq[freqIndex];
  ptrLastQuaDelta[1]=freqIndex;
  ptrLastQuaDelta[2]=sum3reqnoise(0);
  ptrLastQuaDelta[3]=sum3reqnoise(1);
  ptrLastQuaDelta[4]=sum3reqnoise(2);
  if (charger_on_status == 1)
    ptrLastQuaDelta[6]=100;
  else
    ptrLastQuaDelta[6]=50;
  ptrLastQuaDelta[5]= FingerIDList_Output.fingers_number;
  ptrLastQuaDelta[7] = RO_Queue[0];

  ptrLastQuaDelta[8] = Noiselevel[0];
  ptrLastQuaDelta[9] = Noiselevel[1];
  ptrLastQuaDelta[10] = Noiselevel[2];
  ptrLastQuaDelta[11] = Noiselevel[3];
  ptrLastQuaDelta[12] = Noiselevel[4];
  ptrLastQuaDelta[13] = Noiselevel[5];
  ptrLastQuaDelta[14] = Noiselevel[6];
  ptrLastQuaDelta[15] = Noiselevel[7];
  ptrLastQuaDelta[16] = Noiselevel[8];
#endif  
  
#endif
  //ptrReportMutualData[0] = (S16)RO_Continue[0];

#if 0//!TP_CUSTOMER_PROTOCOL
//Hopping debug info
  ptrLastQuaDelta[0]=UsedCTFreq[freqIndex];
  ptrLastQuaDelta[1]=sum3reqnoise(0);
  ptrLastQuaDelta[2]=sum3reqnoise(1);
  ptrLastQuaDelta[3]=sum3reqnoise(2);
  ptrLastQuaDelta[4]=freqcounter;
  ptrLastQuaDelta[5]=checkcorrectfig;
  ptrLastQuaDelta[6]=MutuBaseRy[0];
  ptrLastQuaDelta[7]=MutuBaseRy[1];
  ptrLastQuaDelta[8]=MutuBaseRy[2];
  ptrLastQuaDelta[9]=freqIndex;
  ptrLastQuaDelta[10] = freqbufnow;
  ptrLastQuaDelta[11] = freqhist[freqbufnow];
  ptrLastQuaDelta[12] = NLPulseTime;
#endif  
        dptr = Delta_key_LPF;
        break;
    default:
        break;
    }
#if 1
    //if ((dptr!=NULL) && (flKeyPara.type != SELF_KEY) )
    if ((dptr!=NULL) && ((flKeyPara.type == MUTU_DOWNKEY) || (flKeyPara.type == MUTU_RIGHTKEY)) )
    {
        for (jj=0; jj<flKeyPara.num; jj++)
        {
            ptrReportMutualData[(flKeyPara.CHS[jj])]=dptr[jj];
            //ptrReportMutualData[(flKeyPara.CHS[jj])]=(fg_delta) ? dptr[jj]>>2 : dptr[jj];
        }
    }
#endif
    ptrReportMutualData_1=&ptrReportMutualData[128];

}
#endif

#if (!ISOTRI_CANCEL)
void CalXYAddressMutual( U8 Ypeak, U8 Xpeak, S16* OutputAddr)  //
{
    U8 data i, j, k; //JStest
    U8 data OnEdge=0;  //JStest
    S16 Grid3by3[9];
    S16 data TmpIdx, UpperLim, LowerLim; //TmpVal //JStest
    S16 data Dir[2]; //JStest
    S16 data STmp,STmp1 ;
    OutputAddr[0]=0;
    OutputAddr[1]=0;
    OnEdge=fnIsOnEdge(Ypeak, Xpeak, 0);
//    Dir[0]=( Ypeak == 0 ) ? 0 : ((Ypeak == AlgorithmParam.v_rows-1 ) ? (AlgorithmParam.v_rows-3) : (Ypeak-1));
    Dir[0]=( Ypeak == 0 ) ? 0 : ((Ypeak == AA_TxLen-1 ) ? (AA_TxLen-3) : (Ypeak-1));
//    Dir[1]=( Xpeak == 0 ) ? 0 : ((Xpeak == AlgorithmParam.h_cols-1 ) ? (AlgorithmParam.h_cols-3) : (Xpeak-1));
    Dir[1]=( Xpeak == 0 ) ? 0 : ((Xpeak == AA_RxLen-1 ) ? (AA_RxLen-3) : (Xpeak-1));
    //printf("---\n\rDir[0]=%4d, Dir[1]=%4d, Ypeak=%2bu, Xpeak=%2bu\n\r", Dir[0], Dir[1], Ypeak, Xpeak);
    //k =0;
    for ( i=Dir[0],k=0; i < Dir[0]+3; i++)
    {
        for ( j=Dir[1]; j < Dir[1]+3; j++,k++)
        {
            TmpIdx=i * Total_RX + j;
#if REDUCE_XRAM
            STmp=pMuRaw_buf_T3[TmpIdx];
            UpperLim=(S16)STmp + ( (S16)ptrLastQuaDelta[TmpIdx]>>3 );
            LowerLim=(S16)STmp - ( (S16)ptrLastQuaDelta[TmpIdx]>>3 );
            STmp1=(S16)STmp;
            STmp=pMuRaw_buf_T2[TmpIdx];
            STmp1+=(((S16)STmp) < LowerLim ) ? LowerLim : ( ( ( (S16)STmp ) > UpperLim ) ? UpperLim : ( (S16) STmp) );
            STmp1<<=1;   // Keep 4X ratio.
#else
            STmp=pMuRaw_buf_T3[TmpIdx];
            UpperLim=(S16)STmp + ( (S16)ptrLastQuaDelta[TmpIdx]>>3 );
            LowerLim=(S16)STmp - ( (S16)ptrLastQuaDelta[TmpIdx]>>3 );
            //UpperLim = (S16)STmp*11/10;
            //LowerLim = (S16)STmp*9/10;
            //UpperLim = (S16)(STmp*9)>>3;
            //LowerLim = (S16)(STmp*7)>>3;
            STmp1=(S16)STmp;
            STmp=pMuRaw_buf_T2[TmpIdx];
            STmp1+=(((S16)STmp) < LowerLim ) ? LowerLim : ( ( ( (S16)STmp ) > UpperLim ) ? UpperLim : ( (S16) STmp) );
            STmp=pMuRaw_buf_T1[TmpIdx];
            STmp1+=(((S16)STmp) < LowerLim ) ? LowerLim : ( ( ( (S16)STmp ) > UpperLim ) ? UpperLim : ( (S16) STmp) );
            STmp=pMuRaw_buf_T0[TmpIdx];
            STmp1+=(((S16)STmp) < LowerLim ) ? LowerLim : ( ( ( (S16)STmp ) > UpperLim ) ? UpperLim : ( (S16) STmp) );
#endif
            //JS20130722 : Begin   remove #define Mutu_keyTpye but judge with flKeyPara.type
            //#if Mutu_keyTpye
            //  Grid3by3[k] = STmp1 - MutualBaseLine[TmpIdx];                // MUTU_DOWNKEY
            //#else
            //  Grid3by3[k] = STmp1 - MutualBaseLine[TmpIdx-i];   // MUTU_RIGHTKEY
            //#endif
//            if (flKeyPara.type ==MUTU_DOWNKEY)
                Grid3by3[k]=STmp1 - MutualBaseLine[freqIndex][TmpIdx];   // MUTU_DOWNKEY
//            else
//                Grid3by3[k] = STmp1 - MutualBaseLine[TmpIdx-i];   // MUTU_RIGHTKEY
            //JS20130722 : End   remove #define Mutu_keyTpye but judge with flKeyPara.type
            //k++;
        }
    }
    /*
       printf("--\n\r");
       for ( i = 0; i < 3; i++)
        {
        for ( j = 0; j < 3; j++)
                printf("%8d ", Grid3by3[i*3+j]);
        printf("\n\r");
        }
       printf("\n\r");
     */
    if ((OnEdge==4) || ( OnEdge==8)) // do Y-axis first
    {
        OutputAddr[0]=IsolateTriangleRA(Grid3by3, Xpeak, Ypeak, OutputAddr[1], 0);
        OutputAddr[1]=IsolateTriangleRA(Grid3by3, Xpeak, Ypeak, OutputAddr[0], 1);
    }
    else // do X-axis first
    {
        OutputAddr[1]=IsolateTriangleRA(Grid3by3, Xpeak, Ypeak, OutputAddr[0], 1);
        OutputAddr[0]=IsolateTriangleRA(Grid3by3, Xpeak, Ypeak, OutputAddr[1], 0);
    }
    return ;
}

#endif

//-----------------------------------------------------------------------------------------------
#if 1 // TP_CUSTOMER_PROTOCOL
tFingerPos tmp_Rotate_FingerPos;

// ########################################
void Rotate_Queue(tFingerIDList *fingerID_list_input, tFingerIDList *FingerID_list_output)
{
  U8 x;
  
  FingerID_list_output->fingers_number=0;
  for (x=0; x<MAX_FINGERS; x++)
  {
    if (fingerID_list_input->Touch_Info[x] == 1)
    {
      if (RO_Queue[x] < RO_DelayLatency)
      {
        RO_Queue[x]++ ;
        tmp_Rotate_FingerPos.Touch_Info[x]= 0; //last point
        FingerID_list_output->Touch_Info[x]= 0;       
      }
      else
      {
        RO_Queue[x] = 99;
        FingerID_list_output->Touch_Info[x]=1;
             FingerID_list_output->Pos[0][x] = fingerID_list_input->Pos[0][x];
            FingerID_list_output->Pos[1][x] = fingerID_list_input->Pos[1][x];
        FingerID_list_output->fingers_number++;
        tmp_Rotate_FingerPos.Touch_Info[x]=1; //last point
        tmp_Rotate_FingerPos.Pos[0][x]=fingerID_list_input->Pos[0][x];
        tmp_Rotate_FingerPos.Pos[1][x]=fingerID_list_input->Pos[1][x];
      }
    }     
    else
    {
      if((tmp_Rotate_FingerPos.Touch_Info[x]==1) && (RO_Queue[x] >= 1))
      {
        if(RO_Queue[x] >RO_DelayLatency)
        {
          RO_Queue[x]=RO_DelayLatency-1;
        }else
        {
          RO_Queue[x]--;
        }
        IdFlags |=  F_POS_MASK[x];
          
        FingerID_list_output->Touch_Info[x] = 1;
        FingerID_list_output->Pos[0][x] = tmp_Rotate_FingerPos.Pos[0][x];
        FingerID_list_output->Pos[1][x] = tmp_Rotate_FingerPos.Pos[1][x];
        FingerID_list_output->fingers_number++;
          
      }
      else
      {
        RO_Queue[x] = 0;
        tmp_Rotate_FingerPos.Touch_Info[x]= 0;
        FingerID_list_output->Touch_Info[x] = 0;
      }
    }
  }
}

//#######################################################
#else

#ifndef POINTQUEUE_8
S16 DataAvg(S16 *PQBuf, U8 *BufIdx, U8 BufAvg_Idx)              // [twokey]
{
    S16 DataOut=0;
    U8 kk, DataCnt;
    DataCnt=0;
    for (kk=0; kk<MAX_PQBUFFER; kk++)
    {
        if (BufAvg_Idx & 0x01)
        {
            DataOut+=PQBuf[BufIdx[kk]];
            DataCnt++;
        }
        BufAvg_Idx>>=1;
    }
    DataOut=(DataOut+(DataCnt>>1))/DataCnt;
    return DataOut;
}

#else // POINTQUEUE_8
//DataAvg(PointsQueReg.Point_buf[x].Pos[0], Index_last, BufAvg_YIdx);
S16 DataAvg(S16 *PQBuf, U8 *BufIdx, U8 BufAvg_Idx, U8 Status, U8 *ContinueFlag)         // [twokey]
{
    S16 data DataOut; //JStest
    U8 data DataCnt, Pdir_cnt, Ndir_cnt, FPdir_cnt, FNdir_cnt, SPdir_cnt, SNdir_cnt, DirFlag; //JStest
    S8 data kk, strkk, bufavg_no; //JStest
    S16 data diffData, minData, tmpData; //JStest
    U8 data Status2; //JStest
    Status2=Status & 0x07;
    SPdir_cnt=0;
    SNdir_cnt=0;
    Pdir_cnt=0;
    Ndir_cnt=0;
    FPdir_cnt=0;
    FNdir_cnt=0;
    DataCnt=0;
    if (Status2 == 0)
    {
        DataOut=PQBuf[BufIdx[7]];
    }
    else if ((Status2 == 0x04) || (Status2 == 0x06))
    {
        //DataOut = ((Status2==0x06)||(*ContinueFlag))? ((PQBuf[BufIdx[7]]+PQBuf[BufIdx[6]]+1)>>1) : 4096;
        DataOut=PQBuf[BufIdx[7]] + PQBuf[BufIdx[6]]+ 1;
        if (Status2 == 0x06)
        {
            DataOut=(DataOut + PQBuf[BufIdx[6]] + PQBuf[BufIdx[5]] + 1)>>2;
            for (kk=7; kk>=4; kk--)
            {
                diffData=PQBuf[BufIdx[kk]]-PQBuf[BufIdx[kk-1]];
                if                  (diffData >= DIR_PQBUFTOL_FP) FPdir_cnt++;
                else if (diffData >= DIR_PQBUFTOL_P) Pdir_cnt++;
                else if (diffData >= DIR_PQBUFTOL_SP) SPdir_cnt++;
                else if (diffData <= DIR_PQBUFTOL_FN) FNdir_cnt++;
                else if (diffData <= DIR_PQBUFTOL_N) Ndir_cnt++;
                else if (diffData <= DIR_PQBUFTOL_SN) SNdir_cnt++;
                DataCnt++;
            }
            if ( ((FPdir_cnt>=1)&&((FPdir_cnt+Pdir_cnt+SPdir_cnt)==DataCnt)&&(SPdir_cnt<=2))
                    ||((FNdir_cnt>=1)&&((FNdir_cnt+Ndir_cnt+SNdir_cnt)==DataCnt)&&(SNdir_cnt<=2)) )
                *ContinueFlag=1;
#if Twokey_debug_PointQueue
            //printf(":%2bu,(%2bu,%2bu),(%2bu,%2bu)->%2bu\r\n",DataCnt, FPdir_cnt, Pdir_cnt, FNdir_cnt, Ndir_cnt, *ContinueFlag);
#endif
        }
        else
        {
            DataOut=DataOut>>1;
        }
    }
    else
    {
        //bufavg_no = (BufAvg_Idx<=4)? 4 : (8-BufAvg_Idx);
        bufavg_no=(BufAvg_Idx<=4) ? 5 : (8-BufAvg_Idx);
        minData=4096;
        strkk=(Status == 0x0f) ? 7 : 4;
        for (kk=strkk; kk>=bufavg_no; kk--)
        {
            //search same direction
            diffData=PQBuf[BufIdx[kk+1]]-PQBuf[BufIdx[kk]];
            if ((bufavg_no<3)||(Status==0x0f))
            {
                if          (diffData >= DIR_PQBUFTOL_FP) FPdir_cnt++;
                else if (diffData >= DIR_PQBUFTOL_P) Pdir_cnt++;
                else if (diffData <= DIR_PQBUFTOL_FN) FNdir_cnt++;
                else if (diffData <= DIR_PQBUFTOL_N) Ndir_cnt++;
                DataCnt++;
            }
            // select average position
            diffData=abs(diffData);
            if (minData > diffData)
            {
                tmpData=(PQBuf[BufIdx[kk]]+PQBuf[BufIdx[kk+1]]+1)/2;
                minData=diffData;
            }
        }
        if ( ((FPdir_cnt>1)&&((FPdir_cnt+Pdir_cnt)==DataCnt))||((FNdir_cnt>1)&&((FNdir_cnt+Ndir_cnt)==DataCnt))) DirFlag=1;
        else if ( ((Pdir_cnt>0)&&(Ndir_cnt==0)) || ((Ndir_cnt>0)&&(Pdir_cnt==0)) ) DirFlag=2;
        else DirFlag=0;
        DataOut=0;
        DataCnt=0;
        //strkk = (Status <= 0x1f)? 6 : 5;
        strkk=6;
        if (bufavg_no<2) bufavg_no=2;   // for SUO decrease latency
        for (kk=strkk; kk>=bufavg_no; kk--)
        {
            if ((DirFlag == 1) || (DirFlag == 2))
            {
                diffData=0;
            }
            else
                diffData=abs(tmpData-PQBuf[BufIdx[kk]]);
            //if (diffData <= PQ_NEARPOINTTHD)
            if (1)
            {
                DataOut+=PQBuf[BufIdx[kk]];
                DataCnt++;
            }
        }
        DataOut=(DataOut+(DataCnt>>1))/DataCnt;
        //DataOut = DataOut/DataCnt;
    }
    return DataOut;
}

#endif
#ifndef POINTQUEUE_8
void Points_Queue(tFingerIDList *fingerID_list_input,tFingerIDList *FingerID_list_output)       // [twokey]
{
#if 1
    U8 data x,y, kk, tmpStatus;
    //U8  Index_last1,Index_last2, Index_last3;
    U8 Index_last[8];
    U8 BufAvg_XIdx, BufAvg_YIdx;
    U8 isOnborder;
    S16 dis_x, dis_y, dis_px, dis_py, dis_ix, dis_iy;
    //U8 tmp_flag = 0;
#if PQMODIFY_TWOKEY
    //S16 limitedData[3][2];
    S16 limitedData[3];
#endif
//  U16 ychno_peak, xchno_peak;
    /*
        isOnborder-> bit0=1: peak in Y1
                  -> bit1=1: peak in AlgorithmParam.v_rows
                  -> bit2=1: paek in X1
                  -> bit3=1: peak in AlgorithmParam.h_cols
     */
    //  ----< start queue >----
    FingerID_list_output->fingers_number=0;
    BufAvg_XIdx=0;
    BufAvg_YIdx=0;
    for (x=0; x<MAX_FINGERS; x++)
    {
        isOnborder=fnIsOnEdge( fingerID_list_input->Pos[0][x],fingerID_list_input->Pos[1][x],1);
        //tmpStatus = ((fingerID_list_input->Touch_Info[x] & 0x1) + (PointsQueReg.Status[x] <<1)) & 0xF ;
        tmpStatus=((fingerID_list_input->Touch_Info[x] & 0x1) + (PointsQueReg.Status[x] <<1));
        // fingerID_list_input->Pos[0]->Index_last[0]->Index_last[1]->Index_last[2]->... Index_last[7]
        // Index_last[0]: oldest for current input
        // Index_last[1]: older 2 frame
        // Index_last[7]: newest frame before input
        for (y=0, kk=PointsQueReg.Index[x]; y< MAX_PQBUFFER; y++, kk++)
            Index_last[y]=kk % MAX_PQBUFFER;
        switch (tmpStatus & 0x7) // only use 3 bits
        {
        case 0x0:
        case 0x2: // 000, 010 , no touch , clear all
            if ( PQ_contFlag[x] && (tmpStatus >= 0x38)) // Twokey 0926
            {
                //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                BufAvg_YIdx=4;
                BufAvg_XIdx=4;
                FingerID_list_output->Touch_Info[x]=1;
            }
            else
            {
                // ---<no output> ---
                FingerID_list_output->Pos[0][x]=0;
                FingerID_list_output->Pos[1][x]=0;
                FingerID_list_output->Touch_Info[x]=0;
            }
            PQ_contFlag[x]=0;   // Twokey 0926
            PointsQueReg.Index[x]=0;
            break;
        case 0x1:
        case 0x5: // 001, 101<- first point
            // ---<no output> ---
            // Twokey added 0910 -- start
            if ( ((tmpStatus&0x1f)==0x1d) && (isOnborder==0))
            { // previous is 0x06
                // compensate 1 point
                dis_x=abs(fingerID_list_input->Pos[0][x] - PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]);
                dis_y=abs(fingerID_list_input->Pos[1][x] - PointsQueReg.Point_buf[x].Pos[1][Index_last[2]]);
                if ((dis_x + dis_y) < (4*RES_SCALE))
                {
                    PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]=fingerID_list_input->Pos[0][x];
                    PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]=fingerID_list_input->Pos[1][x];
                    //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                    //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                    BufAvg_YIdx=4;
                    BufAvg_XIdx=4;
                    FingerID_list_output->Touch_Info[x]=1;
                    //FingerID_list_output->fingers_number ++;
                    tmpStatus|=0x02;
                    PointsQueReg.Index[x]=Index_last[1];
#if Twokey_debug_PointQueue
                    printf("PQy: %4d %4d %4d\n\r", PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                    printf("PQx: %4d %4d %4d\n\r", PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]);
                    //printf("Out:(%4d,%4d)\n\r", FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x]);
#endif
                }
                else
                {
                    for (y=0; y<PointQueBufferLength; y++)
                    {
                        PointsQueReg.Point_buf[x].Pos[0][y]=0;
                        PointsQueReg.Point_buf[x].Pos[1][y]=0;
                    }
                    PointsQueReg.Point_buf[x].Pos[0][0]=fingerID_list_input->Pos[0][x];
                    PointsQueReg.Point_buf[x].Pos[1][0]=fingerID_list_input->Pos[1][x];
                    FingerID_list_output->Pos[0][x]=0;
                    FingerID_list_output->Pos[1][x]=0;
                    FingerID_list_output->Touch_Info[x]=0;
                    PointsQueReg.Index[x]=1;
                    tmpStatus&=0x1;  // clear status
                }
            }
            else
            {
                for (y=0; y<PointQueBufferLength; y++)
                {
                    PointsQueReg.Point_buf[x].Pos[0][y]=0;
                    PointsQueReg.Point_buf[x].Pos[1][y]=0;
                }
                PointsQueReg.Index[x]=1;
                PointsQueReg.Point_buf[x].Pos[0][0]=fingerID_list_input->Pos[0][x];
                PointsQueReg.Point_buf[x].Pos[1][0]=fingerID_list_input->Pos[1][x];
                FingerID_list_output->Pos[0][x]=0;
                FingerID_list_output->Pos[1][x]=0;
                FingerID_list_output->Touch_Info[x]=0;
                tmpStatus&=0x1;  // clear status
            }
            // Twokey added 0910 -- end
            PQ_contFlag[x]=0;   // Twokey 0926
            break;
        case 0x3: // 011, <- continue two point
            // ---<output report average> ---
            //distance = abs(FingerID_list_output->Pos[0][x]-PointsQueReg.Point_buf[x].Pos[0][0]) + abs(FingerID_list_output->Pos[1][x]-PointsQueReg.Point_buf[x].Pos[1][0]);
            if ((isOnborder==0)&&(abs(fingerID_list_input->Pos[0][x]-PointsQueReg.Point_buf[x].Pos[0][Index_last[2]])<= PQ_NEARPOINTTHD)&&(abs(fingerID_list_input->Pos[1][x]-PointsQueReg.Point_buf[x].Pos[1][Index_last[2]])<=PQ_NEARPOINTTHD))
            {
                //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                BufAvg_XIdx=4;
                BufAvg_YIdx=4;
                FingerID_list_output->Touch_Info[x]=1;
                //FingerID_list_output->fingers_number ++;
            }
            else
            {
                FingerID_list_output->Pos[0][x]=0;
                FingerID_list_output->Pos[1][x]=0;
                FingerID_list_output->Touch_Info[x]=0;
            }
            PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]=fingerID_list_input->Pos[0][x];
            PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]=fingerID_list_input->Pos[1][x];
            PointsQueReg.Index[x]=Index_last[1];
            PQ_contFlag[x]=0;   // Twokey 0926
            break;
        case 0x7: // 111, <- continue three point
            FingerID_list_output->Touch_Info[x]=1;
            if (isOnborder != 0)
            { // border region
                if ( (tmpStatus & 0xc0)&&((tmpStatus & 0x38)==0x38) ) // 2012.08.23 Modified
                {
#if PQMODIFY_TWOKEY
                    // only Rx axis
                    limitedData[0]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]);
                    limitedData[1]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                    limitedData[2]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                    //limitedData[3] = abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]-fingerID_list_input->Pos[0][x]);
                    if ((limitedData[0]<=PQBUFFER_SMOOTHTHD1)&&(limitedData[1]>PQBUFFER_SMOOTHTHD2)&&(limitedData[2]>PQBUFFER_SMOOTHTHD2))
                    {
                        //tmp_flag =1;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]],0, 2);
                        BufAvg_YIdx=6;
                    }
                    else if ((limitedData[0]>PQBUFFER_SMOOTHTHD2)&&(limitedData[1]<=PQBUFFER_SMOOTHTHD1)&&(limitedData[2]>PQBUFFER_SMOOTHTHD2))
                    {
                        //tmp_flag =2;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]],0, 2);
                        BufAvg_YIdx=3;
                    }
                    else if ((limitedData[0]>PQBUFFER_SMOOTHTHD2)&&(limitedData[1]>PQBUFFER_SMOOTHTHD2)&&(limitedData[2]<=PQBUFFER_SMOOTHTHD1))
                    {
                        //tmp_flag =3;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]],0, 2);
                        BufAvg_YIdx=5;
                    }
                    else
                    {
                        //tmp_flag =0;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], 3);
                        BufAvg_YIdx=7;
                    }
#if 0
                    printf("EDGE id %02bu,%01bu,%03bu (%04d %04d %04d) O %04d,%03bu\r\n",x,BufAvg_YIdx,tmpStatus
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]
                           ,FingerID_list_output->Pos[0][x],gbCNter);
#endif
                    //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], 3);
                    BufAvg_XIdx=7;
#else
                    FingerID_list_output->Pos[0][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], 3);
                    FingerID_list_output->Pos[1][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], 3);
#endif
                }
                //else if (((tmpStatus&0x18) == 0x18) || ((tmpStatus&0x18) == 0x08)  || ((tmpStatus&0x18) == 0x38)))
                else if (tmpStatus & 0x38)
                {
                    if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], 3);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], 3);
                        BufAvg_YIdx=7;
                        BufAvg_XIdx=7;
                    }
                    else if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], 0, 2);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], 0, 2);
                        BufAvg_YIdx=6;
                        BufAvg_XIdx=6;
                    }
                    else
                    {
                        //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                        //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                        BufAvg_YIdx=4;
                        BufAvg_XIdx=4;
                    }
                }
                else
                { // 0x07
                    if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], 0, 2);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], 0, 2);
                        BufAvg_YIdx=6;
                        BufAvg_XIdx=6;
                    }
                    else if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                        //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                        BufAvg_YIdx=4;
                        BufAvg_XIdx=4;
                    }
                    else
                    {
                        FingerID_list_output->Pos[0][x]=0;
                        FingerID_list_output->Pos[1][x]=0;
                        FingerID_list_output->Touch_Info[x]=0;
                    }
                }
            }
            else // center region
            {
                if (tmpStatus & 0x8)
                {
#if PQMODIFY_TWOKEY
                    limitedData[0]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]);
                    limitedData[1]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                    limitedData[2]=abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                    //limitedData[3] = abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]-fingerID_list_input->Pos[0][x]);
                    // don't care the x axis
                    //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], 3);
                    BufAvg_XIdx=7;
                    if ((limitedData[0]<=PQBUFFER_SMOOTHTHD1)&&(limitedData[1]>PQBUFFER_SMOOTHTHD2)&&(limitedData[2]>PQBUFFER_SMOOTHTHD2))
                    {
                        //tmp_flag =1;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]],0, 2);
                        BufAvg_YIdx=6;
                    }
                    else if ((limitedData[0]>PQBUFFER_SMOOTHTHD2)&&(limitedData[1]<=PQBUFFER_SMOOTHTHD1)&&(limitedData[2]>PQBUFFER_SMOOTHTHD2))
                    {
                        //tmp_flag =2;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]],0, 2);
                        BufAvg_YIdx=3;
                    }
                    else if ((limitedData[0]>PQBUFFER_SMOOTHTHD2)&&(limitedData[1]>PQBUFFER_SMOOTHTHD2)&&(limitedData[2]<=PQBUFFER_SMOOTHTHD1))
                    {
                        //tmp_flag =3;
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]],0, 2);
                        BufAvg_YIdx=5;
                    }
                    else
                    {
                        //tmp_flag =0;
                        if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                        {
                            //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], 3);
                            BufAvg_YIdx=7;
                        }
                        else
                        {
                            //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], 0, 2);
                            //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], 0, 2);
                            BufAvg_YIdx=6;
                            BufAvg_XIdx=6;
                        }
                    }
#if 0
                    printf("id %02bu,%01bu,%03bu (%04d %04d %04d) O %04d,%03bu\r\n",x,ufAvg_YIdx,tmpStatus
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]
                           ,PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]
                           ,FingerID_list_output->Pos[0][x],gbCNter);
#endif
#else
                    if ((abs(PointsQueReg.Point_buf[x].Pos[0][y]-fingerID_list_input->Pos[0][x])<=PQ_NEARPOINTTHD) && (abs(PointsQueReg.Point_buf[x].Pos[1][y]-fingerID_list_input->Pos[1][x])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][2], PointsQueReg.Point_buf[x].Pos[0][1], PointsQueReg.Point_buf[x].Pos[0][0], 3);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][2], PointsQueReg.Point_buf[x].Pos[1][1], PointsQueReg.Point_buf[x].Pos[1][0], 3);
                        BufAvg_YIdx=7;
                        BufAvg_XIdx=7;
                    }
                    else
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], 0, 2);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], 0, 2);
                        BufAvg_YIdx=6;
                        BufAvg_XIdx=6;
                    }
#endif
                }
                else
                {
                    // ---<output report average> ---
                    if ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]])<= PQ_NEARPOINTTHD) &&(abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[0]])<=PQ_NEARPOINTTHD))
                    {
                        //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], 0, 2);
                        //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], 0, 2);
                        BufAvg_YIdx=3;
                        BufAvg_XIdx=3;
                    }
                    else
                    {
                        //FingerID_list_output->Pos[0][x] = PointsQueReg.Point_buf[x].Pos[0][Index_last[1]];
                        //FingerID_list_output->Pos[1][x] = PointsQueReg.Point_buf[x].Pos[1][Index_last[1]];
                        BufAvg_YIdx=2;
                        BufAvg_XIdx=2;
                    }
                }
            }
            // Twokey added 0926
            if (tmpStatus >= 0x1f)
            {
                dis_iy=fingerID_list_input->Pos[0][x] - PointsQueReg.Point_buf[x].Pos[0][Index_last[2]];
                dis_y=PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[1]];
                dis_py=PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[0]];
                if ( ((dis_iy>0)&&(dis_y>0)&&(dis_py>0)&&(dis_y>=dis_py)&&(dis_y<=dis_iy<=(dis_y+dis_py*2)))
                        ||((dis_iy>=RES_SCALE)&&(dis_y>RES_SCALE)&&(dis_py>0))
                        ||((dis_iy<0)&&(dis_y<0)&&(dis_py<0)&&(dis_y<=dis_py)&&(dis_y>=dis_iy>=(dis_y+dis_py*2)))
                        ||((dis_iy<=-1*RES_SCALE)&&(dis_y<=-1*RES_SCALE)&&(dis_py<0)) )
                {
                    PQ_contFlag[x]=1;   // Twokey 0926
                }
                else
                {
                    dis_ix=fingerID_list_input->Pos[1][x] - PointsQueReg.Point_buf[x].Pos[1][Index_last[2]];
                    dis_x=PointsQueReg.Point_buf[x].Pos[1][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[1]];
                    dis_px=PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[0]];
                    if ( ((dis_ix>0)&&(dis_x>0)&&(dis_px>0)&&(dis_x>=dis_px)&&(dis_x<=dis_ix<=(dis_x+dis_px*2)))
                            ||((dis_ix>=RES_SCALE)&&(dis_x>=RES_SCALE/2)&&(dis_px>0))
                            ||((dis_ix<0)&&(dis_x<0)&&(dis_px<0)&&(dis_x<=dis_px)&&(dis_x>=dis_ix>=(dis_x+dis_px*2)))
                            ||((dis_ix<-1*RES_SCALE)&&(dis_x<-1*RES_SCALE/2)&&(dis_px<0)) )
                    {
                        PQ_contFlag[x]=1;   // Twokey 0926
                    }
                    else
                    {
                        PQ_contFlag[x]=0;   // Twokey 0926
                    }
                }
            }
            else
            {
                PQ_contFlag[x]=0;   // Twokey 0926
            }
            //if (PQ_contFlag[x] == 0)
            //{
            //
            //  printf("PQX: %4d %4d %4d %4d ->%2bu\n\r", fingerID_list_input->Pos[1][x], PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], PQ_contFlag[x]);
            //  printf("PQX: %4d %4d %4d %4d\n\r", fingerID_list_input->Pos[0][x], PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
            //}
#if Twokey_debug_PointQueue
            if ((tmpStatus&0x1f)==0x1f)
            {
                printf("P7y: %03bu (%4d  %4d %4d %4d)\r\n", x,fingerID_list_input->Pos[0][x], PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                //printf("P7x: %4d %4d %4d\n\r", PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]);
                //printf(" , In:%4d,Out:%4d\r\n", fingerID_list_input->Pos[0][x], FingerID_list_output->Pos[0][x]);
            }
#endif
            PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]=fingerID_list_input->Pos[0][x];
            PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]=fingerID_list_input->Pos[1][x];
            //FingerID_list_output->fingers_number ++;
            PointsQueReg.Index[x]=Index_last[1];
            break;
        case 0x6: // 110, <-last a touch
            // ---<output report average> ---
            if (tmpStatus & 0x8)
            {
                if ((PQ_contFlag[x] ==1) || ((abs(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[1]])<= PQ_NEARPOINTTHD) &&(abs(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[1]])<=PQ_NEARPOINTTHD)) )
                {
                    //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[0]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], 3);
                    //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[0]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], 3);
                    BufAvg_YIdx=7;
                    BufAvg_XIdx=7;
                }
                else
                {
                    //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], 0, 2);
                    //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], 0, 2);
                    BufAvg_YIdx=6;
                    BufAvg_XIdx=6;
                }
            }
            else
            {
                //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], 0, 2);
                //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], 0, 2);
                BufAvg_YIdx=6;
                BufAvg_XIdx=6;
            }
#if Twokey_debug_PointQueue
            if ((tmpStatus&0xf)==0xe)
            {
                printf("P6y: %4d %4d %4d\n\r", PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]);
                printf("P6x: %4d %4d %4d\n\r", PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]);
                printf("Out:(%4d,%4d)\n\r", FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x]);
            }
#endif
            FingerID_list_output->Touch_Info[x]=1;
            //PointsQueReg.Index[x] = Index_last[1];
            //FingerID_list_output->fingers_number ++;
            break;
        case 0x4: // 100, <-The last two touch
            //Twokey added 0926
            if ( PQ_contFlag[x] && (tmpStatus >= 0x3C)) // Twokey 0926
            {
                //FingerID_list_output->Pos[0][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[0][Index_last[1]], PointsQueReg.Point_buf[x].Pos[0][Index_last[2]], 0, 2);
                //FingerID_list_output->Pos[1][x] =DataAvg(PointsQueReg.Point_buf[x].Pos[1][Index_last[1]], PointsQueReg.Point_buf[x].Pos[1][Index_last[2]], 0, 2);
                BufAvg_YIdx=6;
                BufAvg_XIdx=6;
                FingerID_list_output->Touch_Info[x]=1;
            }
            else
            {
                FingerID_list_output->Pos[0][x]=0;
                FingerID_list_output->Pos[1][x]=0;
                FingerID_list_output->Touch_Info[x]=0;
            }
            tmpStatus=0;
            break;
        }
        // output
        if (FingerID_list_output->Touch_Info[x]==1)
        {
            FingerID_list_output->Pos[0][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[0], Index_last, BufAvg_YIdx);
            FingerID_list_output->Pos[1][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[1], Index_last, BufAvg_XIdx);
        }
        //if ((( tmpStatus & 0x38)==0x38 )|| ( tmpStatus & 0x1 ) || ((PQ_contFlag[x]==1) && (FingerID_list_output->Touch_Info[x])))
        //printf("ID:%2bu, St:%2bx, , PQ:%bu, (%4d, %4d), (%4d, %4d)\n\r", x, tmpStatus, PQ_contFlag[x], FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x], fingerID_list_input->Pos[1][x], fingerID_list_input->Pos[0][x]);
#if debug_Charger
        if (  ( tmpStatus & 0x1 ) ||  (FingerID_list_output->Touch_Info[x]))
            //if(FingerID_list_output->Touch_Info[x]==1)
        {
            printf("id %02bu,%01bu,%03bu (%04d %04d %04d) ",x,BufAvg_YIdx,tmpStatus
                   ,PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]
                   ,PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]
                   ,PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]
                  );
            if (FingerID_list_output->Touch_Info[x]==1)
            {
                printf(" Out %04d ",FingerID_list_output->Pos[0][x]);
            }
            printf(",%03bu\r\n",gbCNter);
        }
#endif
        //  ----< final >----
        FingerID_list_output->fingers_number+=FingerID_list_output->Touch_Info[x];
        PointsQueReg.Status[x]=tmpStatus ;    // Twokey 1011
    }
#endif
}

#else   // POINTQUEUE_8
#if JITTER_6
U8 Touch1;
#endif
void Points_Queue(tFingerIDList *fingerID_list_input,tFingerIDList *FingerID_list_output)       // [twokey]
{
    U8 data x,y, kk, tmpStatus;
    U8 data State_LNB, State_HNB; //JStest
    //U8  Index_last1,Index_last2, Index_last3;
    U8 data Index_last[8], tmpIdx; //JStest
    U8 data BufAvg_XIdx, BufAvg_YIdx; //JStest
    U8 data isOnborder; //JStest
    U8 data PQmask; //JStest
#if JITTER_6
    U16 Position_y, Position_x;
#endif
#if   SLASH_6MM
    U8 SlashDetCnt;
    S16 tempX;
    long tempLongY;
#endif
//  S16 dis_x, dis_y, dis_px, dis_py, dis_ix, dis_iy;
    //U8 tmp_flag = 0;
#if PQMODIFY_TWOKEY
    //S16 limitedData[3][2];
    //S16 limitedData[3];
#endif
//  U16 ychno_peak, xchno_peak;
    /*
        isOnborder-> bit0=1: peak in Y1
                  -> bit1=1: peak in AlgorithmParam.v_rows
                  -> bit2=1: paek in X1
                  -> bit3=1: peak in AlgorithmParam.h_cols
     */
    //  ----< start queue >----
    FingerID_list_output->fingers_number=0;
    for (x=0; x<MAX_FINGERS; x++)
    {
        isOnborder=fnIsOnEdge( fingerID_list_input->Pos[0][x],fingerID_list_input->Pos[1][x],1);
#if SwitchLowFreqMode
        if (CheckWkUp == 1)
        {
            PointsQueReg.Index[x]=0x03;
            PointsQueReg.Status[x]=0x07;
            for (y=0; y< PointsQueReg.Index[x]; y++)
            {
                PointsQueReg.Point_buf[x].Pos[0][y]=fingerID_list_input->Pos[0][x];
                PointsQueReg.Point_buf[x].Pos[1][y]=fingerID_list_input->Pos[1][x];
//        printf("(%d, %d)  \n\r", PointsQueReg.Point_buf[x].Pos[0][y], PointsQueReg.Point_buf[x].Pos[1][y]);
            }
        }
#endif
        //tmpStatus = ((fingerID_list_input->Touch_Info[x] & 0x1) + (PointsQueReg.Status[x] <<1)) & 0xF ;
        tmpStatus=((fingerID_list_input->Touch_Info[x] & 0x1) + (PointsQueReg.Status[x]<<1));
        // fingerID_list_input->Pos[0]->Index_last[0]->Index_last[1]->Index_last[2]->... Index_last[7]
        // Index_last[0]: oldest for current input
        // Index_last[1]: older 2 frame
        // Index_last[7]: newest frame before input
        for (y=0, kk=PointsQueReg.Index[x]; y< MAX_PQBUFFER; y++, kk++)
            Index_last[y]=kk % MAX_PQBUFFER;
#if PointsQueueCaseSelection
  State_LNB = tmpStatus & 0x07;
#else
        State_LNB = tmpStatus & 0x0f;
#endif
        State_HNB=tmpStatus>>4;
        switch (State_LNB) // only use 3 bits
        {
        case 0:
        case 2:
        case 4:
        case 5:
#if PointsQueueCaseSelection
        case 6: // 0110
#else
        case 8:
            if ((State_LNB==8) &&(PQ_contFlag[x]))
            {
                FingerID_list_output->Touch_Info[x]=1;
                IdFlags|=F_POS_MASK[x];
            }
            else
#endif
            {
                FingerID_list_output->Touch_Info[x]=0;
                PQ_contFlag[x]=0;
                PointsQueReg.Index[x]=0;   // reset buffer index to fix address
                // clear buffer
                for (y=0; y<  MAX_PQBUFFER; y++)
                {
                    PointsQueReg.Point_buf[x].Pos[0][y]=0;
                    PointsQueReg.Point_buf[x].Pos[1][y]=0;
                }
                Index_last[0]=0;
                tmpStatus=tmpStatus & 0x01;
            }
            break;
        case 1: // 0001
        case 3: // 0011
#if PointsQueueCaseSelection
#else
        case 6: // 0110
#endif
            //case 7: // 0111
            FingerID_list_output->Touch_Info[x]=0;
            IdFlags|=F_POS_MASK[x];
#if JITTER_6
            Touch1=0;
#endif
            break;
#if PointsQueueCaseSelection
#else
        case 9: // 1001
        case 10: // 1010
            if (State_HNB>=0x07)
            {
                FingerID_list_output->Touch_Info[x]=1;
            }
            else
            {
                FingerID_list_output->Touch_Info[x]=0;
            }
            IdFlags|=F_POS_MASK[x];
            break;
        case 11: // 1011
        case 13: // 1101
            if (State_HNB>=0x07)
            {
                tmpStatus|=0x0f;
                FingerID_list_output->Touch_Info[x]=1;
                PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]=(PointsQueReg.Point_buf[x].Pos[0][Index_last[7]] + fingerID_list_input->Pos[0][x] + 1)>>1;
                PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]=(PointsQueReg.Point_buf[x].Pos[1][Index_last[7]] + fingerID_list_input->Pos[1][x] + 1)>>1;
                //for(tmpIdx=Index_last[0], y=0; y<8; y++) Index_last[y]=(y<7)? Index_last[y+1] : tmpIdx; //JS20130205E XX
                //JS20130205E : Start
                tmpIdx=Index_last[0];
                for (y=0; y<7; y++) Index_last[y]=Index_last[y+1];
                Index_last[7]=tmpIdx;
                //JS20130205E : End
            }
            else
            {
                FingerID_list_output->Touch_Info[x]=0;
            }
            break;
        case 12: // 1100
        case 14: // 1110
            if ((State_HNB>=0x03)&&((State_LNB==14)||(PQ_contFlag[x])))
            {
                FingerID_list_output->Touch_Info[x]=1;
            }
            else
            {
                FingerID_list_output->Touch_Info[x]=0;
            }
            IdFlags|=F_POS_MASK[x];
#if JITTER_6
            if (tmpStatus==0xfc) Touch1=0;
#endif
            break;
        case 15:
#endif
        case 7: // 0111
            FingerID_list_output->Touch_Info[x]=1;
#if JITTER_6
            if (tmpStatus==0x0f) Touch1=1;
#endif
            break;
        }
        // Input
        if (fingerID_list_input->Touch_Info[x])
        {
            PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]=fingerID_list_input->Pos[0][x];
            PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]=fingerID_list_input->Pos[1][x];
            tmpIdx=Index_last[0];
            //for (y=0; y<7; y++) Index_last[y]=Index_last[y+1];    // error
            for (y=0; y<7; y++)  Index_last[y] = ((tmpIdx+y+1)%MAX_PQBUFFER);   //revise
            Index_last[7]=tmpIdx;
        }
        // buffer point
        PointsQueReg.Index[x]=Index_last[0];
        //Status
        PointsQueReg.Status[x]=tmpStatus;
        // Output
        if (FingerID_list_output->Touch_Info[x])
        {
            BufAvg_YIdx=0;
            BufAvg_XIdx=0;
            for (PQmask=0x80, y=0; y<8; y++, PQmask>>=1)
            {
                if ((tmpStatus&PQmask) != 0)
                {
                    BufAvg_YIdx++;
                    BufAvg_XIdx++;
                }
            }
            if (State_LNB==14)
            {
                PQ_contFlag[x]=0;
#if 0 //Twokey_debug_PointQueue
                //printf("CHK%bu", x);
#endif
            }
            FingerID_list_output->Pos[0][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[0], Index_last, BufAvg_YIdx, tmpStatus, &(PQ_contFlag[x]));
            //FingerID_list_output->Pos[0][x] = DataAvg(PointsQueReg.Point_buf[x].Pos[0], Index_last, BufAvg_YIdx, tmpStatus, 0);
            //if (fingerID_list_input->Pos[0][x])
            //  FingerID_list_output->Pos[0][x] = fingerID_list_input->Pos[0][x];
#if 0   //Twokey_debug_PointQueue
            if (State_LNB==14)
            {
                printf("CHK%bu", x);
            }
#endif
            FingerID_list_output->Pos[1][x]=DataAvg(PointsQueReg.Point_buf[x].Pos[1], Index_last, BufAvg_XIdx, tmpStatus, &(PQ_contFlag[x]));
            //FingerID_list_output->Pos[1][x] = DataAvg(PointsQueReg.Point_buf[x].Pos[1], Index_last, BufAvg_XIdx, tmpStatus, 0);
            //if (fingerID_list_input->Pos[1][x])
            //  FingerID_list_output->Pos[1][x] = fingerID_list_input->Pos[1][x];
#if  0 // JITTER_6
#ifdef DEBUG_JITTER_6MM
            printf("%bu, %bu, %bu, (%4d %4d),", Touch1, tmpStatus, JitterFixIdx, FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x]);
#endif
            if (Touch1)
            {
                if (tmpStatus>0x3f)
                {
                    Position_y=FingerID_list_output->Pos[0][x];
                    Position_x=FingerID_list_output->Pos[1][x];
                }
                else
                {
                    Position_y=fingerID_list_input->Pos[0][x];
                    Position_x=fingerID_list_input->Pos[1][x];
                }
                if ( (JitterFixIdx!=4)
                        //&&(FingerID_list_output->Pos[1][x]<=TargetPosition_High[JitterFixIdx][1]) && (FingerID_list_output->Pos[1][x]>=TargetPosition_Low[JitterFixIdx][1])
                        //&&(FingerID_list_output->Pos[0][x]<=TargetPosition_High[JitterFixIdx][0]) && (FingerID_list_output->Pos[0][x]>=TargetPosition_Low[JitterFixIdx][0]))
                        &&(Position_x<=PreditPosition_High[JitterFixIdx][1]) && (Position_x>=PreditPosition_Low[JitterFixIdx][1])
                        &&(Position_y<=PreditPosition_High[JitterFixIdx][0]) && (Position_y>=PreditPosition_Low[JitterFixIdx][0]))
                {
                    FingerID_list_output->Pos[1][x]=((FingerID_list_output->Pos[1][x]+PreditPosition_Comp[JitterFixIdx][1])+TargetPosition[JitterFixIdx][1]+1)>>1;
                    FingerID_list_output->Pos[0][x]=((FingerID_list_output->Pos[0][x]+PreditPosition_Comp[JitterFixIdx][0])+TargetPosition[JitterFixIdx][0]+1)>>1;;
                }
                else
                    Touch1=0;
            }
#ifdef DEBUG_JITTER_6MM
            printf("(%4d %4d)\r\n", FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x]);
#endif
#endif
#if 0 // SLASH_6MM
            if (tmpStatus == 0x0f)
            {
                Slash_Flag=0;
                if (FingerID_list_output->Pos[1][x]<SlashCorner[0][1])
                {
                    if      (FingerID_list_output->Pos[0][x]<SlashCorner[0][0]) Slash_Flag=0x0;   //Slash_Flag = 0x10;
                    else if (FingerID_list_output->Pos[0][x]>SlashCorner[1][0]) Slash_Flag=0x0;   //Slash_Flag = 0x20;
                }
                else if (FingerID_list_output->Pos[1][x]>SlashCorner[2][1])
                {
                    if      (FingerID_list_output->Pos[0][x]<SlashCorner[2][0]) Slash_Flag=0x0;   //Slash_Flag = 0x30;
                    else if (FingerID_list_output->Pos[0][x]>SlashCorner[3][0]) Slash_Flag=0x40;
                }
#ifdef DEBUG_SLASH_6MM
                printf("SH(%4d %4d),%bu\r\n", FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x], Slash_Flag);
#endif
                for (y=0; y<4; y++)
                {
                    SlashOutBuf_X[y]=0;
                    SlashOutBuf_Y[y]=0;
                }
            }
            else if ((tmpStatus>0xf) && (Slash_Flag != 0))
            {
                if ((FingerID_list_output->Pos[1][x]<=SLASH_BOUNDARY)||(FingerID_list_output->Pos[1][x]>=LAST2CHANNELPOS))
                {
                    if ((Slash_Flag & 0x0f) == 0)
                    {
                        for (y=0; y<3; y++)
                        {
                            SlashOutBuf_X[y]=SlashOutBuf_X[y+1];
                            SlashOutBuf_Y[y]=SlashOutBuf_Y[y+1];
                        }
                        if (Slash_Flag<0x30)
                        {
                            SlashOutBuf_X[3]=FingerID_list_output->Pos[1][x];
                            SlashOutBuf_Y[3]=FingerID_list_output->Pos[0][x];
                        }
                        else
                        {
                            SlashOutBuf_X[3]=LASTCHANNELPOS - FingerID_list_output->Pos[1][x];
                            SlashOutBuf_Y[3]=896 - FingerID_list_output->Pos[0][x];
                        }
                        SlashDetCnt=0;
                        for (y=3; y>=1; y--)
                        {
                            tempX=SlashOutBuf_X[y]-SlashOutBuf_X[y-1];
                            if ((tempX>0)&&(tempX < 5)
                                    &&(FingerID_list_output->Pos[0][x]>1)&&(FingerID_list_output->Pos[0][x]<895-2))
                                SlashDetCnt++;
                        }
                        if (SlashDetCnt >= 3) Slash_Flag|=0x01;
                    }
                    //printf("Sb1= %bu\r\n",        Slash_Flag);
                    if ( (FingerID_list_output->Pos[1][x]>RES_SCALE/2)&&(FingerID_list_output->Pos[1][x]<(LASTCHANNELPOS-RES_SCALE/2))&&((Slash_Flag&0x0f)==0))
                        Slash_Flag=0;
                    SlashDetCnt=(Slash_Flag&0xf0)>>4;
#ifdef DEBUG_SLASH_6MM
                    printf("Sb=%bu:%(%4d,%4d), (%4d,%4d), (%4d,%4d), (%4d,%4d)\r\n",Slash_Flag, SlashOutBuf_X[3],SlashOutBuf_Y[3], SlashOutBuf_X[2],SlashOutBuf_Y[2], SlashOutBuf_X[1],SlashOutBuf_Y[1], SlashOutBuf_X[0],SlashOutBuf_Y[0]);
#endif
                    if (Slash_Flag&0x01)
                    {
#if 1
                        tempX=FingerID_list_output->Pos[1][x];
                        if ((SlashDetCnt==3)||(SlashDetCnt==4)) tempX=LASTCHANNELPOS-tempX;
                        tempLongY=((((long) tempX)*128 * TX_CHANNELNO + RX_CHANNELNO/2) / RX_CHANNELNO)/128;
                        FingerID_list_output->Pos[0][x]=(S16) tempLongY;
                        if ((SlashDetCnt == 2) || (SlashDetCnt == 4))
                            FingerID_list_output->Pos[0][x]=TX_CHANNELNO*RES_SCALE-1 - FingerID_list_output->Pos[0][x];
#else
                        if (FingerID_list_output->Pos[0][x]<= 108)
                            //tempLongY = ((long) FingerID_list_output->Pos[0][x]);
                            tempLongY=0;
                        else
                        {
                            if ((Slash_Flag&0xf0)== 0x20) //tempLongY = (long) 896- ((long) FingerID_list_output->Pos[0][x]);
                                tempLongY=64-((long) FingerID_list_output->Pos[0][x]);
                            else tempLongY=0;
                        }
                        tempLongY=(tempLongY * 10 + 50)/100;
                        if (FingerID_list_output->Pos[0][x]<= 108)
                            FingerID_list_output->Pos[0][x]=FingerID_list_output->Pos[0][x] + ((S16) tempLongY);
                        else
                            FingerID_list_output->Pos[0][x]=FingerID_list_output->Pos[0][x] + ((S16) tempLongY);
#endif
#ifdef DEBUG_SLASH_6MM
                        printf("Sout=(%4d, %4d):,",FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x]);
#endif
                        if (FingerID_list_output->Pos[0][x] < 0) FingerID_list_output->Pos[0][x]=0;
                        else if (FingerID_list_output->Pos[0][x] > TX_CHANNELNO*RES_SCALE-1) FingerID_list_output->Pos[0][x]=TX_CHANNELNO*RES_SCALE-1;
                        if (FingerID_list_output->Pos[1][x] < 0) FingerID_list_output->Pos[1][x]=0;
                        else if (FingerID_list_output->Pos[1][x] > RX_CHANNELNO*RES_SCALE-1) FingerID_list_output->Pos[1][x]=RX_CHANNELNO*RES_SCALE-1;
                    }
#ifdef DEBUG_SLASH_6MM
                    printf("%bu: (%4d, %4d), %4ld\r\n\n",Slash_Flag, FingerID_list_output->Pos[1][x], FingerID_list_output->Pos[0][x], tempLongY);
#endif
                }
            }
#endif
#if PointsQueueCaseSelection
#else
            if (State_LNB==8)
            {
                PQ_contFlag[x]=0;
                // clear buffer
                for (y=0; y<  MAX_PQBUFFER; y++)
                {
                    PointsQueReg.Point_buf[x].Pos[0][y]=0;
                    PointsQueReg.Point_buf[x].Pos[1][y]=0;
                }
                Index_last[0]=0;
                PointsQueReg.Status[x]=0;
            }
#endif
            if ((FingerID_list_output->Pos[0][x]==4096)||(FingerID_list_output->Pos[1][x]==4096))
            {
                FingerID_list_output->Pos[0][x]=0;
                FingerID_list_output->Pos[1][x]=0;
                FingerID_list_output->Touch_Info[x]=0;
            }
            else
                FingerID_list_output->fingers_number++;
        }
        else
        {
            FingerID_list_output->Pos[0][x]=0;
            FingerID_list_output->Pos[1][x]=0;
        }
        // debug report
#if 0 //Twokey_debug_PointQueue
        if (tmpStatus!= 0)
        {
            // Y
            //printf("idy %02bu,%01bu,%03bu %02bu ->(%04d %04d %04d %04d %04d %04d %04d %04d) ",x,BufAvg_YIdx,tmpStatus, PointsQueReg.Index[x]
            /*printf("idy %02bu,%01bu,%03bu %02bu ->(%04d %04d %04d %04d %04d %04d %04d %04d) ",x,PQ_contFlag[x],tmpStatus, PointsQueReg.Index[x]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[7]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[6]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[5]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[4]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[3]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[2]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[1]]
                                                                  ,PointsQueReg.Point_buf[x].Pos[0][Index_last[0]]
                                                                  );*/
            if ((PointsQueReg.Point_buf[x].Pos[0][Index_last[4]] != 0)
                    &&((PointsQueReg.Point_buf[x].Pos[0][Index_last[7]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[4]] > 64) ||
                       (PointsQueReg.Point_buf[x].Pos[0][Index_last[7]]-PointsQueReg.Point_buf[x].Pos[0][Index_last[4]] <-64)))
            {
                printf("idy %02bu,%01bu,%03bu %02bu ->(%04d %04d %04d %04d) ",x,PQ_contFlag[x],tmpStatus, PointsQueReg.Index[x]
                       ,PointsQueReg.Point_buf[x].Pos[0][Index_last[7]]
                       ,PointsQueReg.Point_buf[x].Pos[0][Index_last[6]]
                       ,PointsQueReg.Point_buf[x].Pos[0][Index_last[5]]
                       ,PointsQueReg.Point_buf[x].Pos[0][Index_last[4]]
                      );
                if (FingerID_list_output->Touch_Info[x]==1) printf(" Out %04d ",FingerID_list_output->Pos[0][x]);
                //if (fingerID_list_input->Touch_Info[x]==1)   printf(" in %04d ",fingerID_list_input->Pos[0][x]);
                printf("\r\n");
            }
            //X
            //printf("idx %02bu,%01bu,%03bu     ->(%04d %04d %04d %04d %04d %04d %04d %04d) ",x,BufAvg_XIdx,tmpStatus
            /*printf("idx %02bu,%01bu,%03bu     ->(%04d %04d %04d %04d %04d %04d %04d %04d) ",x,PQ_contFlag[x],tmpStatus
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[7]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[6]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[5]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[4]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[3]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[2]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[1]]
                                                                          ,PointsQueReg.Point_buf[x].Pos[1][Index_last[0]]
                                                                          );*/
            if ((PointsQueReg.Point_buf[x].Pos[1][Index_last[4]] != 0)
                    &&((PointsQueReg.Point_buf[x].Pos[1][Index_last[7]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[4]] > 64) ||
                       (PointsQueReg.Point_buf[x].Pos[1][Index_last[7]]-PointsQueReg.Point_buf[x].Pos[1][Index_last[4]] <-64)))
            {
                printf("idx %02bu,%01bu,%03bu     ->(%04d %04d %04d %04d) ",x,PQ_contFlag[x],tmpStatus
                       ,PointsQueReg.Point_buf[x].Pos[1][Index_last[7]]
                       ,PointsQueReg.Point_buf[x].Pos[1][Index_last[6]]
                       ,PointsQueReg.Point_buf[x].Pos[1][Index_last[5]]
                       ,PointsQueReg.Point_buf[x].Pos[1][Index_last[4]]
                      );
                if (FingerID_list_output->Touch_Info[x]==1) printf(" Out %04d ",FingerID_list_output->Pos[1][x]);
                //if(fingerID_list_input->Touch_Info[x]==1) printf(" in %04d ",fingerID_list_input->Pos[1][x]);
                printf("\r\n\n");
            }
        }
#endif
    }
}

#endif   // POINTQUEUE_8

#endif

void SortFingerID(tFingerList *finger_list)
{
    U8 data x,tmpID;
#if 0
    FingerIDList.fingers_number=finger_list->fingers_number;
    // clear FingerIDList
    for (x=0; x <MAX_FINGERS; x++)
    {
        FingerIDList.Touch_Info[x]=0;
        FingerIDList.Pos[0][x]=0;
        FingerIDList.Pos[1][x]=0;
        FingerIDList.Z_Area[x]=0;
        FingerIDList.Z_Force[x]=0;
    }
    // update FingerIDList
    for (x=0; x <finger_list->fingers_number; x++)
    {
        tmpID=finger_list->ID[x];
        FingerIDList.Pos[0][tmpID]=finger_list->Pos[0][x];
        FingerIDList.Pos[1][tmpID]=finger_list->Pos[1][x];
        FingerIDList.Touch_Info[tmpID]=1;
        FingerIDList.Z_Area[tmpID]=finger_list->Z_Area[x];
        FingerIDList.Z_Force[tmpID]=finger_list->Z_Force[x];
#if debug_log
        //  printf("ID %02bu ",tmpID);
#endif
    }
#else
    // clear FingerIDList
    p1FingerIDList->fingers_number=0;
    for (x=0; x <MAX_FINGERS; x++)
    {
        p1FingerIDList->Touch_Info[x]=0;
        p1FingerIDList->Pos[0][x]=0;
        p1FingerIDList->Pos[1][x]=0;
        //p1FingerIDList->Z_Area[x] = 0;
        //p1FingerIDList->Z_Force[x] = 0;
    }
    // update FingerIDList
    for (x=0; x <finger_list->fingers_number; x++)
    {
        tmpID=finger_list->ID[x];
        if (tmpID<max_ID)
        {
            p1FingerIDList->Pos[0][tmpID]=finger_list->Pos[0][x];
            p1FingerIDList->Pos[1][tmpID]=finger_list->Pos[1][x];
            p1FingerIDList->Touch_Info[tmpID]=1;
            p1FingerIDList->Z_Area[tmpID]=finger_list->Z_Area[x];
            p1FingerIDList->Z_Force[tmpID]=finger_list->Z_Force[x];
            p1FingerIDList->fingers_number++;
        }
        //printf("Sort (%4d,%4d)\r\n",p1FingerIDList->Pos[0][tmpID],p1FingerIDList->Pos[1][tmpID]);
        //printf("p1(%4d, %4d)\n\r ", p1FingerIDList->Pos[1][tmpID], p1FingerIDList->Pos[0][tmpID]);
        /*
           if( p0FingerIDList->Touch_Info[tmpID] == 1)
           printf("p0(%4d, %4d)", p0FingerIDList->Pos[1][tmpID], p0FingerIDList->Pos[0][tmpID]);
           printf("\n\r");
         */
    }
    /*
       for(x = 0;x <MAX_FINGERS;x++)
       {
        // clear previous frame information
        if ((p1FingerIDList->Touch_Info[x] == 0) && (PQ_contFlag[x]==0))
        {
                p0FingerIDList->Touch_Info[x] = 0;
                touchCount[x] = 0;
        }
        else
        {
                if (touchCount[x]<16)   touchCount[x]++;
                p1FingerIDList->Touch_Info[x] = (touchCount[x]>1)? 1 : 0;
        }
       }
     */
#endif
}

// 0.
void StartTrigScan(void)
{
    if ((OpMode[1] & RUN_FULL_MUTUAL))
    {
        ScanMode=MUTUAL_MODE;
        NextScanMode=MUTUAL_MODE;
        NextScanMode2=MUTUAL_MODE;
        MutualScanStart(pMutuTargetAddr);
        SwitchMemory();
        //---mutual scan
    }
    else
    {
        ScanMode=SELF_MODE;
        NextScanMode=SELF_MODE;
        NextScanMode2=SELF_MODE;
        // ---self scan
        //SelfScanInit(RawcountPara.SelfPulseCount,RawcountPara.SelfPulseLen);
        SelfScanStart(pSelfTargetAddr);
    }
}

//U8 Customer_reg = Customer_byte;
U8 Customer_reg; //JS20130616a
//U8 Project_ID_reg = Project_ID_default;
U8 Project_ID_reg; //JS20130616a
void Struct_Init(void)
{
    Total_TX=flPannel.Y_TX;
    Total_RX=flPannel.X_RX;
    AlgorithmParam.self_finger_lvl=flRawPara.Self_TH; // for up down threshold
    AlgorithmParam.mutual_finger_lvl=flRawPara.Mutu_TH; // for up down threshold
    Customer_reg=flProtocol.Customer;
    Project_ID_reg=flProtocol.Project_ID;
    AlgorithmParam.v_rows=Total_TX;
    AlgorithmParam.h_cols=Total_RX;
/* Andy optimized
    switch (flKeyPara.type)
    {
    case MUTU_DOWNKEY:
//      AlgorithmParam.v_rows=Total_TX-1;
        AlgorithmParam.v_rows=Total_TX;
        AlgorithmParam.h_cols=Total_RX;
        break;
    case MUTU_RIGHTKEY:
        AlgorithmParam.v_rows=Total_TX;
//      AlgorithmParam.h_cols=Total_RX -1;
        AlgorithmParam.h_cols=Total_RX;
        break;
    default:
        AlgorithmParam.v_rows=Total_TX;
        AlgorithmParam.h_cols=Total_RX;
        break;
    }
*/
    AlgorithmParam.frame_delta=1;
#if FLOATMODE
    FloatingPara.Mutual_enable=1;
    FloatingPara.Mutual_TH1=NormalToFloating_TH;
    FloatingPara.Mutual_TH2=FloatingToNormal_TH;
    FloatingPara.MutualPluseCount=250;
    FloatingPara.Self_enable=0;
    FloatingPara.Self_TH1=0;
    FloatingPara.Self_TH2=0;
    FloatingPara.SelfPluseCount=200;
#endif
#if palm_solution
    PalmReject.PalmThreshold=50;   //mutual_finger_th;
    PalmReject.PalmAreaCount=30;
    PalmReject.bigFingerNodeTH=8;
    PalmReject.PressureOffset=150;
    PalmReject.PressureDivisor=20;
    PalmReject.PalmReport_point=PalmReportPoint;
    PID_bigFingerTH=PalmReject.bigFingerNodeTH ;
    PID_flag=0;
#endif
}

/*
   void DSP_Mov(U8 dst,U32 *src)
   {
        U8 * srcptr;
        //U16 *tmpptr;
        //tmpptr = (U16*)src;
        srcptr = (U8 *)src;
        OP4L = *(srcptr+3);
        OP4M = *(srcptr+2);
        OP4H = *(srcptr+1);
        //printf("%03bu %03bu %03bu, %u \r\n",*srcptr,*(srcptr+1),*(srcptr+2),(*tmpptr));
        //if(dst < 4)
        {
                OPCODE  = 0x00|0xC|dst;
                while((DSP_STAT & 0x20) == 0);
        }
   }
   void CALL_DSP(U8 mode,U32* src,U32* dest,U32* result)
   {
   // 0->1 : 50us
        //U32 result ;
        //U8 * srcptr;
        U8 *ptr;
        // ---0
        DSP_Mov(0,src);
        DSP_Mov(1,dest);
        switch(mode)
        {
                case Add: // add
                        OPCODE = 0x20|(0<<2)|(1);
                        break;
                case Sub:
                        OPCODE = 0x30|(0<<2)|(1);
                        break;
                case Mul:
                        OPCODE = 0x40|(0<<2)|(1);
                        break;
                case Div:
                        OPCODE = 0x60|(0<<2)|(1);
                        break;
                default:
                        break;
        }
        // ---1
        while(!(DSP_STAT&0x20)) ;
        // ---2
        ptr = (U8*)result;
 *(ptr+3) = OP2L;
 *(ptr+2) = OP2M;
 *(ptr+1) = OP2H;
        //result = OP2L+(OP2M<<8)+(OP2H<<16)  ;
        //printf("re##~ %bu %bu %bu %d\r\n",OP2L,OP2M,OP2H,result);
        return ;
   }
 */
// 1.---- TpInit ----
void SystemInit(void)
{
    //===================================================
    //----Init HW & protocol->hearder------------
    SetI2CDevAddress(flSystemPara.I2C_DevAddr);
#ifdef ACTIVE_POWER_SAVING              //AD20130725 Add #if for new SetMcuMainClk(..,..)
    SetMcuMainClk(flSystemPara.Clock_rate, (flKeyPara.KeyGain[7] & 0x1e) >> 1);
#else
    SetMcuMainClk(flSystemPara.Clock_rate);
#endif
    ADCInit(flPannel.Y_TX,flPannel.X_RX);

//    HWUART_INIT();                          // Initial Hardware UART

    ProtocolInit();
    SystemTimerInit(1,flSystemPara.Timer_period_Normal);
    SystemTimerStart();     // Enable systemer and go !
    EnableIntrAll();
    //----Init HW & protocol->hearde  End------------
    //===================================================
}

extern U8 MutualForceMCC;
extern bit bPowerOnMutual_Key;

void keycheck_init(void);

U8 ComboKeyCheck(S16 *keyraw, U8 key_num);
extern U8 prev_freqIndex;

      
void keyCheck()
{
    U8 data xxx, i; //JStest
  U16 data mp;
  static U8 cnt4Frames=0;

  if(!(flKeyPara.type & 0x80)) return;
  if(prev_freqIndex != freqIndex)
  { // While jumping to a nre frequency, re-accumulate the key RAW data for 4 frame mode.
    cnt4Frames=0;
    //bPowerOnMutual_Key = 1; //Reset baseline.
  }

  switch(flKeyPara.type)
  {
    case SELF_KEY:
      xxx=AlgorithmParam.v_rows+AlgorithmParam.h_cols;
          for (i=0; i<flKeyPara.num ; i++)
      {
        mutuKey_Raw[i] = pSelfdelta[xxx+i];
      }
      break;
    case SELF_SHARE_KEY:
          for (i=0; i<flKeyPara.num ; i++)
      {
        mutuKey_Raw[i] = pSelfdelta[flKeyPara.CHS[i]];
      }
      break;
    case MUTU_DOWNKEY:
    case MUTU_RIGHTKEY:
            for (xxx=0; xxx<flKeyPara.num ; xxx++)
            {
        mp = flKeyPara.CHS[xxx];
      #if REDUCE_XRAM
        if(cnt4Frames>=4)
        {
                mutuKey_Raw[xxx]=(((S16)(pMuRaw_buf_T2[ mp ]))
                                  +((S16)(pMuRaw_buf_T3[ mp ]))
                                 ) << 1;
        }
        else
        {
          mutuKey_Raw[xxx] =  (S16)(pMuRaw_buf_T3[ mp ])<<2;
          cnt4Frames++;
        }
      #else
        if(cnt4Frames>=4)
        {
                  mutuKey_Raw[xxx]=(((S16)(pMuRaw_buf_T0[ mp ]))
                                    +((S16)(pMuRaw_buf_T1[ mp ]))
                                    +((S16)(pMuRaw_buf_T2[ mp ]))
                                    + ((S16)(pMuRaw_buf_T3[ mp ])));
        }
        else
        {
          mutuKey_Raw[xxx] =  (S16)(pMuRaw_buf_T3[ mp ])<<2;
          cnt4Frames++;
        }
      #endif
            }
      break;
    default: break;
  }
  KeyPress=ComboKeyCheck(mutuKey_Raw, flKeyPara.num);
  prev_freqIndex = freqIndex;
}

U16 sum3reqnoisefirst(U8 freq)
{
  return (Noiselevelfirst[freq-1]+Noiselevelfirst[freq]+Noiselevelfirst[freq+1]);
}



void NLlevelfirst(void)
{
  S16 tmpv1,tmpv2;
  U8 tData3;
  U8 rx_length = flPannel.X_RX;

  tmpv1 = 0;
  //for(tData3=0;tData3<RxLength;tData3++)
  for(tData3=0;tData3<rx_length;tData3++)
  {
    tmpv2 = ((NoiseTargetAddr[tData3]>=NoiseTargetAddr[tData3+rx_length]) ?  (NoiseTargetAddr[tData3]-NoiseTargetAddr[tData3+rx_length]):(NoiseTargetAddr[tData3+rx_length]-NoiseTargetAddr[tData3]));        

    if(tmpv2>tmpv1)
      tmpv1 = tmpv2;  
  } 
  
  tData3 = NLPulseTime - 12;
  if(Noiselevelfirst[tData3]==0)
    Noiselevelfirst[tData3] = tmpv1;
  else
  Noiselevelfirst[tData3] = (Noiselevelfirst[tData3]+tmpv1)>>1;

}

#if DetectMutualDownKeySetting
//U8 Key0Tx,Key1Tx,Key2Tx,Key0Rx,Key1Rx,Key2Rx;
//U8 CoaxisKey; //0: Tx, 1:Rx
void CheckKey_CoTxorRx(void)
{
  Key0Tx = KEY0_TX;
  Key1Tx = KEY1_TX;
  Key2Tx = KEY2_TX;
  Key0Rx = KEY0_RX;
  Key1Rx = KEY1_RX;
  Key2Rx = KEY2_RX;

  if ((Key0Tx = (Key1Tx&Key2Tx)) && (Key1Tx = (Key2Tx&Key0Tx)) && (Key2Tx = (Key0Tx&Key1Tx)))
  {
    CoaxisKey = 0x11;
    CoaxisTxCHno = KEY0_TX;
  }
  else if ((Key0Rx = (Key1Rx&Key2Rx)) && (Key1Rx = (Key2Rx&Key0Rx)) && (Key2Rx = (Key1Rx&Key0Rx)))
  {
    CoaxisKey = 0x22;
    CoaxisRxCHno = KEY0_RX;
  }
  else
    CoaxisKey = 0x00;
  
}
#endif

bit startnoiselisten;
void TpInit(void)
{
    U16 data x,y;
    U8 data tData3 =0;
    U8 tmpv1,tmpv2,tmpv3,tmpv4;
    U16 minnoisel;
    U16 noiselevnow;
    U8 sortflag[2];
    U8 HoppingStartFreq,DisableHopping;
    Struct_Init();
    paraSELF_SING_ALL = fDEBUGSettingPara.paraSELFTrigMode;
  
    bSelfProcessEn=flKeyPara.KeyGain[7] & 0x01;     // Bit 0 enable the normal self mode.
    key_set(flKeyPara.type,flKeyPara.num, flKeyPara.CHS);
    SelfADCEnALLBit=1;              // set self mode En All bit(0x3A) as '1'
#if SwitchLowFreqMode
    CheckWkUp=0;
#endif

    KeyPress=0;
    keycheck_init();
    nbridge_i2c_EN=1;
    fgFrameSync=0;
    ScanMode=0;
    NextScanMode=0;
    NextScanMode2=0;
    startnoiselisten = 0;


    SebaseRyreset();
  
    MubaseRyreset();
    Sereset();
    Mureset();
#if DetectMutualDownKeySetting
    CheckKey_CoTxorRx();
#endif
#ifdef POLY_1128LIB
    bPowerOnMutual_Key[0] = 1;
    bPowerOnMutual_Key[1] = 1;
    bPowerOnMutual_Key[2] = 1;
#endif

    TouchFingersList=&TouchFingersList_0;
    lastTouchFingersList_1=&TouchFingersList_1;
    lastTouchFingersList_2=&TouchFingersList_2;    // Twokey 0912
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
    p0FingerIDList=&FingerIDList;
#endif
    p1FingerIDList=&FingerIDList_1;
    wbuf_T0=SelfTargetAddr[0];
    wbuf=SelfTargetAddr[1];
    pSelfTargetAddr=SelfTargetAddr[2];
    ptrLastQuaDelta=LastQuaDelta;
    ptrLastQuaDelta_pre=LastQuaDelta_1;
#if REDUCE_XRAM
    pMuRaw_buf_T2=MutuTargetAddr[0];
    pMuRaw_buf_T3=MutuTargetAddr[1];
    pMutuTargetAddr=MutuTargetAddr[2];
  #if RAW_FILTER
    pMuRaw_buf_PRE = MutuTargetAddr[3];
  #endif
#else
    pMuRaw_buf_T0=MutuTargetAddr[0];
    pMuRaw_buf_T1=MutuTargetAddr[1];
    pMuRaw_buf_T2=MutuTargetAddr[2];
    pMuRaw_buf_T3=MutuTargetAddr[3];
    pMutuTargetAddr=MutuTargetAddr[4];
//  pMuRaw_buf_PRE = MutuTargetAddr[5];
#endif
    //pMuRaw_buf_Tn1 = MutuTargetAddr[5];
    preOpMode[0]=0;
    preOpMode[1]=ReportInfo_default |Runtime_scamMode_default;
    OpMode[0]=preOpMode[0] ;
    OpMode[1]=preOpMode[1];
    OpMode[2]=0;
    pLast_error_flags=&OpMode[2];
#if UseTrickToDEBLOCKING //JS20130710a merge from Poly
    PowerOnFlagForJitter=0;         // 1: enter jitter processing
#endif
    TouchFingersList_0.fingers_number=0;
    TouchFingersList_1.fingers_number=0;
    TouchFingersList_2.fingers_number=0;
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
    FingerIDList.fingers_number=0;
#endif
    FingerIDList_1.fingers_number=0;
    for (x=0; x< MAX_FINGERS ; x++)
    {
        TouchFingersList_0.ID[x]=0xFF;
        TouchFingersList_0.Pos[0][x]=0;
        TouchFingersList_0.Pos[1][x]=0;
        TouchFingersList_1.ID[x]=0xFF;
        TouchFingersList_1.Pos[0][x]=0;
        TouchFingersList_1.Pos[1][x]=0;
#if 0//!StandardORLibrary
    PointsQueReg.Status[x]=0;
#endif
//    PointsQueReg.Filter[x] = 0;
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
        FingerIDList.Pos[0][x]=0;
        FingerIDList.Pos[1][x]=0;
        FingerIDList.Touch_Info[x]=0;
#endif
        FingerIDList_1.Pos[0][x]=0;
        FingerIDList_1.Pos[1][x]=0;
        FingerIDList_1.Touch_Info[x]=0;
  #if !TP_CUSTOMER_PROTOCOL
        //touchCount[x]=0;   // Twokey 0918
        PQ_contFlag[x]=0;   // Twokey 0926
  #endif
  }
    //for (x=0; x< MAX_MUTU_KEY_NUM ; x++)
    //{
    //    KeyPressCounter[x]=0;
    //}
    Self_nodes=AlgorithmParam.v_rows + AlgorithmParam.h_cols ;
    if ( flKeyPara.type == SELF_KEY) //self mode key
    {
        Self_nodes+=flKeyPara.num;
        //selfprocess =1;
    }
    selfprocess=1;
    Mu_nodes=AlgorithmParam.v_rows * AlgorithmParam.h_cols;
    Mutu_que.index=0;
    Mutu_que.status=0;
    for (x=0; x<(Total_TX*Total_RX); x++)
    {
        
#if WP1004_B
        // MubaselineAddr[x] = (S16)RawcountPara.Mutu_RAWOffset;
#else
        //MubaselineAddr[x] = (U8 )RawcountPara.Mutu_RAWOffset;
#endif
        pMutuTargetAddr[x]=0x00;

        for(y=0;y<3;y++)
          MutualBaseLine[y][x] = 0;


    }

#if 0
  if (fFREQAnalysis.WideScanEnable)
  {
    UsedCTFreq[0] = HoppingFreq0;
    UsedCTFreq[1] = HoppingFreq1;
    UsedCTFreq[2] = HoppingFreq2;
  }
  else
  {
    UsedCTFreq[0] = flRawPara.MutualPulseLen;
    UsedCTFreq[1] = flRawPara.MutualPulseLen;
    UsedCTFreq[2] = flRawPara.MutualPulseLen;
  }
#endif
  
    if (selfprocess==1)     // for self mode
    {
        SelfScanInit(flRawPara.SelfPulseCount,flRawPara.SelfPulseLen,flRawPara.Self_fb);
        SelfPulseTime = UsedCTFreq[2];
        AutoSelfCalibration(pSelfTargetAddr);
    }
    MutualScanInit(flRawPara.MutualPulseCount,flRawPara.MutualPulseLen,flRawPara.Mutu_fb);
    MutuPulseTime = UsedCTFreq[2];
#ifdef AUTOCC_MODULE
  #ifdef MutuCCByNode
      AutoMutuCalibration_byNode(pMutuTargetAddr,flRawPara.MutuCC_MinRaw,flRawPara.Mutu_offsetCC);
  #else
      AutoMutuCalibration(pMutuTargetAddr,flRawPara.MutuCC_MinRaw,flRawPara.Mutu_offsetCC);
  #endif
#else
  /* This section make the force CC and let RKEY CC independent.
  // 13-0915A New.
  */
    for (x = 0; x < flPannel.X_RX; x++)
    {
        { // Use FORCE CC  results.
            MutualComOffset[x] = MutualForceMCC;
        }
    }
  if (flKeyPara.type ==MUTU_RIGHTKEY)
  {
    x--;
    MutualComOffset[x] = flKeyPara.CHS[6] & 0x3f;
  }
#endif
    //NoiseFilterSetting(16, 0,1);
    pSelfdelta=SelfDelta;
    //pMutualdelta = MutualDelta;
    //TxNumber = 0;
    prePeaks=0;
    FrameCount=0;
    //Tri_InputBand[0] = 0;
    //Tri_InputBand[1] = 0;
    //Tri_InputBand[2] = 0;
    SwitchScanMode_trig_pre=0;
    SwitchScanMode_trig=0;
    gbCNter=0;
    isMuBaselineRdy=0;
    NoReport=0;
    //PowerOnFlag = 1;
    //DynamicsTracking_Init();
    //--- need trig first time self scan ---// @@
    start_frame();
#if PointGitterReject
    GitterReject_paraReset(TouchFingersList);
#endif
#if JITTER_for_INNOS_Poly
    Jitter_IIR_Init(&FingerIDList_Output);
#endif
#if FLOATMODE
    sim_floating=0;
    nMutualTHFlag1=0;
    MutualTHFlag2=0;
    mutualMode=NormalMode;
    mutualModeCount=0;
#endif
  SePOBreset(); 
  MuPOBreset();
#if 0//!StandardORLibrary
  #ifdef POINTQUEUE_8
    for (x=0; x<MAX_FINGERS; x++)
    {
        PointsQueReg.Index[x]=0;
    }
  #endif
#endif
    //updateMuBaselineCN = 0;
    //updateMuBaselineCN_pre =0 ;
    //SelfToMutuCNter =0;
    MutuToselfCNter=0;
    Intr_trig=0;
    bridge_i2c_done=1;
    bridge_i2c_done_pre=1;
    preIdFlags=0;

   freqbufnow = 0;
   freqhist[freqbufnow] = 0;
    //----------------------
   for(x=0;x<2;x++)
   {
      MutualScanStart(pMutuTargetAddr);
      while (!IsEndOfScan()) {};
    }

  startnoiselisten = 1;

  HoppingStartFreq = (fFREQAnalysis.WideFreqStart/4);
  DisableHopping = 0;       // enable
  
  if (HoppingStartFreq < 12)  
  {
    HoppingStartFreq = 12;      // for min RC
  }
  else if (fFREQAnalysis.WideFreqStart == 255)
  {
    DisableHopping = 1;
  }

//if (fFREQAnalysis.WideScanEnable)
  if (!DisableHopping)
{
  for(x=0;x<39;x++)
  {
      //tmpv1 = ((((U8)x)%13)+12);
      tmpv1 = ((((U8)x)%13)+HoppingStartFreq);
    NLPulseTime=(U8)tmpv1;
    NoiseListenScanStart();     
      while (!IsEndOfScan()) {};
    NLlevelfirst();
  }
#if 1
  sortflag[0]=0;
  sortflag[1]=0;


  for(tmpv1=0;tmpv1<NonSortFreqNum;tmpv1++)
  {
      tmpv2 = NonSortfreq[tmpv1]-HoppingStartFreq;
    tmpv3=(tmpv2>>3);
    tmpv4=(tmpv2&0x07);
    sortflag[tmpv3]|=BIT8_MASK[tmpv4];
  }


  for(tmpv1=0;tmpv1<SortNum;tmpv1++)
  {
    minnoisel=0xffff;;
    tData3 = 0;
      
    for(tmpv2=1;tmpv2<12;tmpv2++)  //13~23:192K~108K
    {
      tmpv3=(tmpv2>>3);
      tmpv4=(tmpv2&0x07);
      if((sortflag[tmpv3]&BIT8_MASK[tmpv4])==0)
      {
        noiselevnow = sum3reqnoisefirst(tmpv2);
        if(noiselevnow<minnoisel)
        {
          minnoisel = noiselevnow;
          tData3 = tmpv2; 
        }
      }
    }
    if(tData3!=0)
    {
      tmpv3=(tData3>>3);
      tmpv4=(tData3&0x07);

      sortflag[tmpv3]|=BIT8_MASK[tmpv4];
        UsedCTFreq[tmpv1]=tData3+HoppingStartFreq;  
    }     
  }

  //check charger on/off flag
  charger_on_status = 0;  //default
  if (minnoisel > NSL_On_ChargerFlagTH)
  {
    charger_on_status = 1;
  }
  else if (minnoisel <= NSL_Off_ChargerFlagTH)
  {
    charger_on_status = 0;
  }


  for(tmpv1=0;tmpv1<3;tmpv1++)
  {
    tData3 = UsedCTFreq[tmpv1];
      Noiselevel[tmpv1*3]=Noiselevelfirst[tData3-(HoppingStartFreq+1)];
      Noiselevel[tmpv1*3+1]=Noiselevelfirst[tData3-HoppingStartFreq];
      Noiselevel[tmpv1*3+2]=Noiselevelfirst[tData3-(HoppingStartFreq-1)];
  }

#else
  UsedCTFreq[0] =13;
  UsedCTFreq[1] =14;
  UsedCTFreq[2] =15;
#endif
}
  else
  {
    UsedCTFreq[0] =(flRawPara.MutualPulseLen/4);    // 76/4
    UsedCTFreq[1] =(flRawPara.MutualPulseLen/4);    // 76/4
    UsedCTFreq[2] =(flRawPara.MutualPulseLen/4);
  }




  
  //chooisefreq1();
  //startnoiselisten = 1;


  scanfreqIndex = 0;
    freqIndex = scanfreqIndex;
  MutuPulseTime = UsedCTFreq[scanfreqIndex];
   
    StartTrigScan(); // first trig ADC
#if debug_log
    printf("WP1104 Start \r\n");
#endif
#if SLASH_6MM
    Slash_Flag=0;
    for (x=0; x<4; x++) SlashOutBuf_X[x]=0;
#endif
    return;
}

// 3. SelfMainLoop
void SelfMainLoop(void)
{
#if 0
    //S8 r;
    //U8 j;
    U8 data xxx;
    U8 OnEdge=0;   //, OnXEdge =0;
    U8 data xpeak_cn=0,ypeak_cn=0;
    //proximity
    S16 delta;
    // run time detect update baseline flag
#if 1
    if (selfEnviroment_cal_flag  == 1)
    {
        for (xxx=0; xxx<Self_nodes; xxx++)
        {
            if (((wbuf[xxx] - SelfBaseLine[xxx]) < selfENVIROMENT_CALTHD1) && (bPowerOnSelf == 0))
            {
                bPowerOnSelf=1;
            }
            else if ((wbuf[xxx] - SelfBaseLine[xxx]) >= selfENVIROMENT_CALTHD2)
            {
                bPowerOnSelf=0;
                break;
            }
        }
    }
    selfEnviroment_cal_flag=1;
#endif
    //--- update baseline and row data
    CalSelfBaseline();
    //bPowerOnSelf = 0;
#if 0
    printf("%4d\n\r", wbuf[11]);
#endif
    for (xxx=0; xxx<Self_nodes; xxx++)
    {
        delta=(S16)(wbuf[xxx])-(SelfBaseLine[xxx]);
#if 1 //SignDelta
        pSelfdelta[xxx]=delta ;
#else
        pSelfdelta[xxx]=delta > 0 ? delta : 0;
#endif
#if 0
        if (xxx==11)
            printf("%4d, , %4d, %4d\n\r", wbuf[xxx], SelfBaseLine[xxx], pSelfdelta[xxx]);
#endif
    }
    OnEdge=0;
    for (xxx=0; xxx<10; xxx++)
    {
        if (pSelfdelta[xxx]>300)
            OnEdge=0x1;
    }
    for (xxx=10; xxx<27; xxx++)
    {
        if (pSelfdelta[xxx]>300)
            OnEdge|=0x2;
    }
    TpFillRawdataPacket();  //--- search row peak
    ypeak_cn=0;
    xpeak_cn=0;
    if (SearchRowPeak(pSelfdelta, 1, AlgorithmParam.v_rows,AlgorithmParam.self_finger_lvl,ScanMode)) // Check Y side only
    {
        flags_set(peak_flag[0], AlgorithmParam.v_rows);
        while ((xxx=flags_find_pos()) !=(S8)-1) // Found a flag bit was set !
        {
            peak[0][ypeak_cn++]=xxx;
        }
        if (SearchRowPeak(&pSelfdelta[AlgorithmParam.v_rows], 1, AlgorithmParam.h_cols,AlgorithmParam.self_finger_lvl,ScanMode)) // Check X side only
        {
            flags_set(peak_flag[0], AlgorithmParam.h_cols);
            while ((xxx=flags_find_pos())  !=(S8)-1) // Found a flag bit was set !
            {
                peak[1][xpeak_cn++]=xxx;
            }
        }
    }
    //--- is finger?  decide next NextScanMode
    if (((OpMode[1]  & 0xC) ==RUN_FULL_AUTO) ||  (isMuBaselineRdy==0)  ) // @.@
    {
        if (ypeak_cn==0 ||xpeak_cn==0) // no figner
        {
            //#if 1
            SelfToMutuCNter=0;
            if ((OpMode[1]  & 0xC) ==RUN_FULL_AUTO)
            {
                updateMuBaselineCN++;
                //if(b8Test_flag==0)
                //{
                if (updateMuBaselineCN == MuBaselineOnSlefCounter) // change to mutual to update parameter
                {
                    //b8Test_flag =1;
                    //printf("switch mutu");
                    updateMuBaselineCN_pre=updateMuBaselineCN;
                    updateMuBaselineCN=0;
                    // trigger next scan
                    SwitchScanMode_trig_pre=1;
                    Mutu_que.status=0;
                    Mutu_que.index=0;
                    NextScanMode2=MUTUAL_MODE;
                    if ((muPowerOnBaseLine == 0) ||(bPowerOnMutual > 1) )
                        muEnviroment_cal_flag=1;
                }
                //}
                else if (SwitchScanMode_trig_pre == MuBaselineOnSlefCounter)
                {
                    updateMuBaselineCN_pre=updateMuBaselineCN;
                    NextScanMode2=MUTUAL_MODE;
                }
                else
                {
                    updateMuBaselineCN_pre=updateMuBaselineCN;
                    NextScanMode2=SELF_MODE;
                }
            }
            else
                //#endif
            {
                NextScanMode2=SELF_MODE;
            }
        }
        else if ( (ypeak_cn==1 && xpeak_cn==1)|| (isMuBaselineRdy==0)) // 1 figner
        {
            NextScanMode2=SELF_MODE;
            SelfToMutuCNter=0;
        }
        else // >1 figner
        {
            /*
               if(SelfToMutuCNter<1)
               {
                //SwitchScanMode_trig_pre = 1;
                NoReport = 1;
                NextScanMode = SELF_MODE;
                SelfToMutuCNter++;
               }
               else
             */
            {
                SelfToMutuCNter=0;
                SwitchScanMode_trig_pre=1;
                Mutu_que.status=0;
                Mutu_que.index=0;
                NextScanMode2=MUTUAL_MODE;
                muEnviroment_cal_flag=0;
                bPowerOnMutual=0;
            }
        }
    }
    else
    {
        if ((OpMode[1]  & RUN_FULL_MUTUAL)) // @.@
        {
            //printf(" switch mutu 1 , %03bu %03bu\r\n",ypeak_cn,xpeak_cn);
            SwitchScanMode_trig_pre=1;
            Mutu_que.status=0;
            Mutu_que.index=0;
            NextScanMode2=MUTUAL_MODE;
        }
        else
        {
            // ---self scan
            NextScanMode2=SELF_MODE;
        }
    }
    //--- is finger?  calculate point address
    if (xpeak_cn==1 && ypeak_cn==1)
    {
        TouchFingersList->fingers_number=1;
        OnEdge=fnIsOnEdge(peak[0][0], peak[1][0], 0);
        if ((OnEdge==4) || ( OnEdge==8)) // do Y-axis first
        {
            TouchFingersList->Pos[0][0]=IsolateTriangleRA(pSelfdelta,peak[1][0], peak[0][0], TouchFingersList->Pos[1][0], 0);
            TouchFingersList->Pos[1][0]=IsolateTriangleRA(pSelfdelta,peak[1][0], peak[0][0], TouchFingersList->Pos[0][0], 1);
        }
        else                                            // do Y-axis first
        {
            TouchFingersList->Pos[1][0]=IsolateTriangleRA(pSelfdelta,peak[1][0], peak[0][0], TouchFingersList->Pos[0][0], 1);
            TouchFingersList->Pos[0][0]=IsolateTriangleRA(pSelfdelta,peak[1][0], peak[0][0], TouchFingersList->Pos[1][0], 0);
        }
        DynamicsTracking(lastTouchFingersList_1, TouchFingersList);
    }
#endif
}

#ifdef AMPLIFY_SELF_RX
extern U8 gainSelfRx;
extern U8 divSelfRx;
extern U8 gainSelfTx;
extern U8 divSelfTx;
#endif
#ifdef ACTIVE_POWER_SAVING
bit fgBoostingClock=0;
U8 CountPeaksLost=0;
#endif
// 4. MutualMainLoop
void MutualMainLoop(void)
{
  S16 minvalx,minvaly;
  bit negflag;
  
#ifdef AMPLIFY_SELF_RX
    S16 delta;
    S16 *wptr;
#endif
#if !SignDelta
    S16 delta ;
#endif
//#if palm_solution
    S16 tmpAddress[2];
//#endif
//  U16 data j;
    U8 data ii; //,jj
#if palm_solution
    U8 kk;
#endif
#if Crovax_debug_MutualMainLoop
    U8 Ci, Ci2;
#endif
#if Self_Process
U8 data Tx_peak=0,Rx_peak=0;
// S16 Self_Th;
#endif


#if 1
//
    // Twokey debug delta -- 0928  start
    /*
       if (prePeaks >0)
       {
        if (peak[1][0]!=0)
        {
                //peakraw_ptr = &(pMutualdelta[(peak[0][0]-1)*AlgorithmParam.h_cols]);
                printf("D[%bu]:%4d %4d %4d %4d %4d %4d\n\r",peak[1][0]-1, pMutualdelta[peak[1][0]-1], pMutualdelta[AlgorithmParam.h_cols+peak[1][0]-1 ] , pMutualdelta[2*AlgorithmParam.h_cols+peak[1][0]-1], pMutualdelta[3*AlgorithmParam.h_cols+peak[1][0]-1],pMutualdelta[4*AlgorithmParam.h_cols+peak[1][0]-1],pMutualdelta[5*AlgorithmParam.h_cols+peak[1][0]-1]);
        }
        //peakraw_ptr = &(pMutualdelta[(peak[0][0])*AlgorithmParam.h_cols] + );
                printf("D[%bu]:%4d %4d %4d %4d %4d %4d\n\r",peak[1][0], pMutualdelta[peak[1][0]], pMutualdelta[AlgorithmParam.h_cols+peak[1][0]] , pMutualdelta[2*AlgorithmParam.h_cols+peak[1][0]],pMutualdelta[3*AlgorithmParam.h_cols+peak[1][0]],pMutualdelta[4*AlgorithmParam.h_cols+peak[1][0]],pMutualdelta[5*AlgorithmParam.h_cols+peak[1][0]]);
        peakraw_ptr = &(pMutualdelta[(peak[0][0]+1)*AlgorithmParam.h_cols]);
                printf("D[%bu]:%4d %4d %4d %4d %4d %4d\n\r\n",peak[1][0]+1, pMutualdelta[peak[1][0]+1], pMutualdelta[AlgorithmParam.h_cols+peak[1][0]+1 ] , pMutualdelta[2*AlgorithmParam.h_cols+peak[1][0]+1], pMutualdelta[3*AlgorithmParam.h_cols+peak[1][0]+1],pMutualdelta[4*AlgorithmParam.h_cols+peak[1][0]+1],pMutualdelta[5*AlgorithmParam.h_cols+peak[1][0]+1]);
       }
       // Twokey debug delta -- 0928  end
     */
    //printf("min = %d\n",peakmin);
    if (prePeaks==0 )
    {
        CalMutualBaseline(100); // [twokey]  // %% ---------------~ 1.16ms
    }
    else
    {

        bPowerOnMutual[freqhist[(freqIndex+3)%4]] = 0;
        bPowerOnSelf[freqhist[(freqIndex+3)%4]] = 0;
  
      //Mureset();
      //Sereset();
    }
//  printf("peakmax:%5d  nFloDyn:%bu  muEnv:%bu \n\r",peakmax,nFloatDynamicTH,muEnviroment_cal_flag);
#ifdef POWERONFINGER_DEBUG              // [Fox]
    if (PowerOnTimerOut_Status == 1)
    {
        //printf("TimeOut\r\n");
        PowerOnTimerOut_Status=2;
    }
#endif
    isMuBaselineRdy=1;
    //--- search row peak
    AlgorithmParam.mutual_finger_lvl = (prePeaks==0)?  flRawPara.Mutu_first_TH : flRawPara.Mutu_TH;

  if(charger_on_status==0)
    SearchRowPeak(NULL, AlgorithmParam.v_rows, AlgorithmParam.h_cols,AlgorithmParam.mutual_finger_lvl,ScanMode);  // %% ---------~ 4.9ms      pMutualdelta
  else
    SearchRowPeak(NULL, AlgorithmParam.v_rows, AlgorithmParam.h_cols,flRawPara.Mutu_first_TH,ScanMode);  // %% ---------~ 4.9ms     pMutualdelta

  MutuFingerNo = prePeaks;
    if (selfprocess==1)
    {
#if 0
        for (ii=0; ii<(Self_nodes); ii++)
        {
            pSelfdelta[ii]=pSelfTargetAddr[ii] -SelfBaseLine[ii];
        }
#endif    



  minvalx=32676;
  negflag=0;
  for(ii=0; ii< AlgorithmParam.v_rows; ii++)
  {
    
    pSelfdelta[ii] = ((pSelfTargetAddr[ii]-SelfBaseLine[freqIndex][ii]) << 1);    // org Y - axis
    if(pSelfdelta[ii]>0) 
    {
      if(pSelfdelta[ii]<minvalx) 
        minvalx = pSelfdelta[ii];
    }else
      negflag=1;
  }
  minvaly=32676;
  for (ii=AlgorithmParam.v_rows; ii< (AlgorithmParam.v_rows+AlgorithmParam.h_cols); ii++)
  {
    pSelfdelta[ii] = pSelfTargetAddr[ii] -SelfBaseLine[freqIndex][ii];    // X-axis ==  3 /2 -- org
    if(pSelfdelta[ii]>0 ) 
    {
      if(pSelfdelta[ii]<minvaly && ii<(AlgorithmParam.v_rows+AlgorithmParam.h_cols-1)) 
        minvaly = pSelfdelta[ii];
    }else
      negflag=1;
      
  }
  if(negflag==0)
  {
    for (ii=0; ii< AlgorithmParam.v_rows; ii++)
    {
      if(minvalx<pSelfdelta[ii]) 
        pSelfdelta[ii]=pSelfdelta[ii]-minvalx;
      else 
        pSelfdelta[ii]=0;
    }
    for (ii=AlgorithmParam.v_rows; ii< (AlgorithmParam.v_rows+AlgorithmParam.h_cols); ii++)
    {
      if(minvaly<pSelfdelta[ii]) 
        pSelfdelta[ii]=pSelfdelta[ii]-minvaly;
      else 
        pSelfdelta[ii]=0;
    }
    }



#ifdef AMPLIFY_SELF_RX
        //if ( flKeyPara.type ==MUTU_DOWNKEY) wptr = &pSelfdelta[Total_TX-1];
        //else wptr = &pSelfdelta[Total_TX];
        if (gainSelfRx)
        {
            wptr=&pSelfdelta[Total_TX];
            for (ii=0; ii<Total_RX; ii++) // Remove mutual key on slef mode
            {
                delta=(*wptr) * gainSelfRx;
                *wptr++=delta / divSelfRx;
            }
        }
        if (gainSelfTx)
        {
            wptr=&pSelfdelta[0];
            for (ii=0; ii<Total_TX; ii++) // Remove mutual key on slef mode
            {
                delta=(*wptr) * gainSelfTx;
                *wptr++=delta / divSelfTx;
            }
        }
#endif

    if (flKeyPara.type==MUTU_RIGHTKEY)
    {
      pSelfdelta[Self_nodes-1] = 0;
    } else if(flKeyPara.type==MUTU_DOWNKEY)
    {
      pSelfdelta[Total_TX-1] = 0;
    }
  
    }
//Self mode search row peak
#if 0
  if (bSelfProcessEn)
  {
      for (ii=0; ii<(MAX_FINGERS); ii++)
      {
          selfpeak[0][ii]=0;
          selfpeak[1][ii]=0;
      }
      Self_Th=(last_finger==0) ?  flRawPara.Self_first_TH : flRawPara.Self_TH;   // detect is first touch
      ptrSelfPeak=selfpeak[0];
      Tx_peak=SearchRowPeak(pSelfdelta,1,AA_TxLen,Self_Th,SELF_MODE);
      ptrSelfPeak=selfpeak[1];
      Rx_peak=SearchRowPeak(&pSelfdelta[AA_TxLen],1,AlgorithmParam.h_cols,Self_Th,SELF_MODE);
    SELF_MODE_PEAKS_SUM = (Tx_peak + Rx_peak);
    SELF_peak_index = (Tx_peak<<4) | (Rx_peak);
  }

//mutual mode search row peak
  mutuPoint=1;    // do mutual addressing
    AlgorithmParam.mutual_finger_lvl=(prePeaks==0) ?  flRawPara.Mutu_first_TH : flRawPara.Mutu_TH;
    SearchRowPeak(NULL, AlgorithmParam.v_rows, AlgorithmParam.h_cols,AlgorithmParam.mutual_finger_lvl,ScanMode); // %% ---------~ 4.9ms     pMutualdelta
  MUTU_MODE_PEAKS = prePeaks;
  //prePeaks: hiding in searchrowpeak
#endif

  if (bSelfProcessEn)
  {
    #if 0
    //if ( (MUTU_MODE_PEAKS < 3) && (Tx_peak == Rx_peak))   // 1f & (2f + non co-axis)
    if ((SELF_peak_index == 0x00) || (SELF_peak_index == 0x11) || (SELF_peak_index == 0x22) || (SELF_peak_index == 0x10) || (SELF_peak_index == 0x01) )   // 1f & (2f + non co-axis)
    //for andy
    //if ((SELF_peak_index == 0x00) || (SELF_peak_index == 0x11)  || (SELF_peak_index == 0x10) || (SELF_peak_index == 0x01) )   // 1f & (2f + non co-axis)
    {
       fnSelf_Process();
       SELF_MODE_PEAKS = TouchFingersList->fingers_number;
       prePeaks= TouchFingersList->fingers_number;
      mutuPoint=0;    // do self addressing
    }
    #else
          mutuPoint = 0;
    EnForceSelf = 0;
            fnSelf_Process();
    #endif


  if (EnForceSelf == 0)
    mutuPoint = 1;

#if 0 // add 20131206 
        prePeaks = TouchFingersList->fingers_number;
    if(coutCounter==coutCounterThr && prePeaks==0)
    {
      NoReport=1;
            selfBaLine_updateCn=0;
            //re-Update self baseline
            SebaseRyreset();
          Sereset();
      SePOBreset();
      selfEnviroment_cal_flagreset();
    } 
#endif

  }
#ifdef ACTIVE_POWER_SAVING
    if (fgBoostingClock)
    {
        if (!prePeaks) //Touching free?
        {
            CountPeaksLost++;
            if (CountPeaksLost>10) //If continuos 10 frame has not touching, Return to low clocks.
            {
                switch_MCUDIV_FreqMode(((flKeyPara.KeyGain[7] & 0x1e) >> 1)); // Set to normal speed.
                fgBoostingClock=0;
            }
        }
    }
    else
    {
        if ((flKeyPara.KeyGain[7] & 0x20)) //SGP_ENABLE
        {
            if (prePeaks) //Touching ?
            {
                switch_MCUDIV_FreqMode(1); // Set to high speed.
                CountPeaksLost=0;
                fgBoostingClock=1;
            }
        }
    }
#endif
    //TpFillRawdataPacket();
#if FLOATMODE
    if (TouchFingersList->fingers_number>0)
    {
        InitMutualDeltaFlag();
        for (j=0 ; j< TouchFingersList->fingers_number ; j++)
        {
            checkMutualDelta( pMutualdelta[peak[0][j] *AlgorithmParam.h_cols +  peak[1][j] ]);
        }
        isSwitchMode();
    }
#endif
    //--- is finger?  decide next NextScanMode
    //--- need trig self or mutual scan ---// @@
    if (!isMuBaselineRdy) // need to re-update baseline
    {
        NextScanMode2=SELF_MODE;
        //TxNumber = 0;
        // ---self scan
    }
    else
    {
        if (((OpMode[1]  & 0xC) ==RUN_FULL_AUTO))
        {
            if (TouchFingersList->fingers_number>0)
            {
                NextScanMode2=MUTUAL_MODE;
                MutuToselfCNter=0;
            }
            else
            {
                /*
                   if(MutuToselfCNter<1)
                   {
                        NextScanMode = MUTUAL_MODE;
                        MutuToselfCNter++;
                        //printf("CNter %03bu,%03bu\r\n",MutuToselfCNter,gbCNter);
                   }
                   else // 1F or 0 F
                 */
                {
                    MutuToselfCNter=0;
                    //SwitchScanMode_trig_pre = 1;
                    NextScanMode2=SELF_MODE;
                    //SelfScanInit(RawcountPara.SelfPulseCount,RawcountPara.SelfPulseLen);
                    //printf(" switch self %03bu\r\n",gbCNter);
                }
            }
        }
        else
        {
            if ((OpMode[1]  & RUN_FULL_MUTUAL))
            {
                NextScanMode2=MUTUAL_MODE;
            }
            else
            {
                SwitchScanMode_trig_pre=1;
                NextScanMode2=SELF_MODE;
            }
        }
        //--- is finger?  calculate point address
        ii=0;
        //PID_flag = 0;
        if (TouchFingersList->fingers_number>0)
        {
            //if(TouchFingersList->fingers_number>=2)
            //{
            //printf(" 1.(%03bu %03bu) 2.(%03bu %03bu), p1 %03d , p2% 03d\r\n",peak[0][0],peak[1][0],peak[0][1],peak[1][1] , ptrLastQuaDelta[peak[0][0]*AlgorithmParam.h_cols+peak[1][0]],ptrLastQuaDelta[peak[0][1]*AlgorithmParam.h_cols+peak[1][1]]  );
            //}
#if palm_solution
            PID_flag=0;
            Coaxial_Check(); // tim add
#endif
            for (ii=0; ii<TouchFingersList->fingers_number; ii++)  // %% --------- 2F : 200 us , 4F:400 us
            {
                //  tim add
#ifdef Twokey_LARGEFINGER
                AreaCheck(peak[0][ii],peak[1][ii]);
#endif
#if palm_solution
                kk=PalmIDCheck(peak[0][ii],peak[1][ii]);
                if (kk>1)
                {
                    PalmCalAddr(peak[0][ii],peak[1][ii],tmpAddress);
                    //printf("[%03bu]%04d %04d,%03bu %03bu\r\n",gbCNter,tmpAddress[0],tmpAddress[1] , peak[0][ii] ,peak[1][ii]);
                    TouchFingersList->Pos[0][ii]=tmpAddress[0];
                    TouchFingersList->Pos[1][ii]=tmpAddress[1];
                    //printf("palm occur %03bu\r\n",gbCNter);
                }
                else
#endif
                {
                    //printf("normal finger %03bu \r\n",gbCNter);
                    //#if Self_Process
                    if (mutuPoint==1 || !bSelfProcessEn) //Andy modify 0719-0001
                        //#endif
                    {
#if LARGE_AREA
                        if ( ( PeakArea[ii] >= flLargeAreaPara.LargeArea_fig ) && ( fnIsOnEdge(peak[0][ii],peak[1][ii],0)==0 ))
                            //IsOnEdge(peak[0][ii],peak[1][ii],0);
                            //if ( PeakArea[ii] >= flLargeAreaPara.LargeArea_fig )
                        {
                            TouchFingersList->Pos[0][ii]=WeightingSumAreaMain(BoundSet[ii],0);
                            TouchFingersList->Pos[1][ii]=WeightingSumAreaMain(BoundSet[ii],1);
                        }
                        else
                        {
#if ISOTRI_CANCEL
                            if ( ( fnIsOnEdge(peak[0][ii], peak[1][ii], 0)&0x03)== 0 ) TouchFingersList->Pos[0][ii]=WeightingSumAreaMain(BoundSet[ii],0);
                            else TouchFingersList->Pos[0][ii]=CalMutualEdgeAddress(peak[0][ii], peak[1][ii], 0);
                            if ( ( fnIsOnEdge(peak[0][ii], peak[1][ii], 0)&0x0c)== 0 ) TouchFingersList->Pos[1][ii]=WeightingSumAreaMain(BoundSet[ii],1);
                            else TouchFingersList->Pos[1][ii]=CalMutualEdgeAddress(peak[0][ii], peak[1][ii], 1);
#else
                            CalXYAddressMutual(peak[0][ii],peak[1][ii],tmpAddress);
                            TouchFingersList->Pos[0][ii]=tmpAddress[0];
                            TouchFingersList->Pos[1][ii]=tmpAddress[1];
                            if ( ( fnIsOnEdge(peak[0][ii], peak[1][ii], 0)&0x03)== 0 ) TouchFingersList->Pos[0][ii]=WeightingSumAreaMain(BoundSet[ii],0);
                            if ( ( fnIsOnEdge(peak[0][ii], peak[1][ii], 0)&0x0c)== 0 ) TouchFingersList->Pos[1][ii]=WeightingSumAreaMain(BoundSet[ii],1);
#endif
                        }
#else
                        CalXYAddressMutual(peak[0][ii],peak[1][ii],tmpAddress); //
                        TouchFingersList->Pos[0][ii]=tmpAddress[0];
                        TouchFingersList->Pos[1][ii]=tmpAddress[1];
#endif
                    }
#if palm_solution
                    //ReportAreaPressure(TouchFingersList->Pos[1][ii],TouchFingersList->Pos[0][ii],ii); //[ jackson ]
#endif
                }
            }
#ifdef Twokey_LARGEFINGER
            printf("********\r\n");
#endif
#if palm_solution
            if (PID_flag==1)
            {
                PID_bigFingerTH=PalmReject.bigFingerNodeTH -1;
            }
            else
            {
                PID_bigFingerTH=PalmReject.bigFingerNodeTH ;
            }
            //printf("bigFingerTH = %03bu,%03bu ",PID_bigFingerTH,PID_flag);
#endif
        }
        /*
         #if palm_solution
                // --- for palm report point---
                if(palm_id_count > 0 &&  ii != MAX_FINGERS && PalmReject.PalmReport_point  == 1)   //[ jackson ]
                {
                        TouchFingersList->Pos[0][ii] = PalmY;
                        TouchFingersList->Pos[1][ii] = PalmX;
                        TouchFingersList->fingers_number++;
                        //ReportAreaPressure(TouchFingersList->Pos[1][ii],TouchFingersList->Pos[0][ii],ii); //[ jackson ]
                }
         #endif
         */
        /*
           if ( TouchFingersList->fingers_number > 0 )
           {
                printf("PMPeaks %bu:\n\r", TouchFingersList->fingers_number );
                for ( Ci = 0; Ci < TouchFingersList->fingers_number; Ci++ )
                {
                        printf("(%d\t%d) ", TouchFingersList->Pos[1][Ci], TouchFingersList->Pos[0][Ci]);
                        printf("Px = %bu, Py = %bu\t", peak[1][Ci], peak[0][Ci]);
                        printf("Peak = %4d", pMutualdelta[ peak[0][Ci] * AlgorithmParam.h_cols +  peak[1][Ci] ] );
                        printf("ID = %bu, \t", TouchFingersList->ID[Ci]);
                        for ( Ci2 = Mutu_que.index+1; Ci2 < Mutu_que.index+5; Ci2++ )
                        {
                                printf("%4d, ", ((S16)(Mutu_que.Raw_buf[peak[0][Ci] * AlgorithmParam.h_cols +  peak[1][Ci]][Ci2%4])- (S16)(MutualBaseLine[peak[0][Ci] * AlgorithmParam.h_cols +  peak[1][Ci]]/4) ));
                        }
                printf("\n\r");
                }
           }
         */
        DynamicsTracking(lastTouchFingersList_1, TouchFingersList); // %% -------- 5F :700us
    }
#endif
#if Crovax_debug_MutualMainLoop
    if ( TouchFingersList->fingers_number > 0 )
    {
        printf("Mutual Peaks %bu:\n\r", TouchFingersList->fingers_number );
        for ( Ci=0; Ci < TouchFingersList->fingers_number; Ci++ )
        {
            tmpAddress[0]=peak[0][Ci] *AlgorithmParam.h_cols +  peak[1][Ci];
            printf("(%d\t%d) ", TouchFingersList->Pos[1][Ci], TouchFingersList->Pos[0][Ci]);
            printf("Px = %bu, Py = %bu\t", peak[1][Ci], peak[0][Ci]);
            //printf("Peak Delta = %4d (%4d), ", pMutualdelta[ tmpAddress[0] ], ptrLastQuaDelta[ tmpAddress[0] ] );
            printf("ID = %bu, \t", TouchFingersList->ID[Ci]);
            for ( Ci2=Mutu_que.index+1; Ci2 < Mutu_que.index+3; Ci2++ )
            {
                //printf("%4d, ", ((S16)(Mutu_que.Raw_buf[Ci2%2][tmpAddress[0]])- (S16)(MutualBaseLine[tmpAddress[0]]/4) ));
                //printf("%4d, ", ((S16)(Mutu_que.Raw_buf[peak[0][Ci] * AlgorithmParam.h_cols +  peak[1][Ci]][Ci2%4]) ) );
            }
            printf("\n\r");
        }
        //printf("\n\r");
        if ( TouchFingersList->fingers_number > 21 )
        {
            for ( Ci=0; Ci < AlgorithmParam.v_rows; Ci++ )
            {
                for ( Ci2=0; Ci2 < AlgorithmParam.h_cols; Ci2++)
                    printf("%4d\t", pMutualdelta[ Ci * AlgorithmParam.h_cols + Ci2 ] );
                printf("\n\r");
            }
            printf("\n\n\r");
        }
    }
#endif
}

//#ifdef Twokey_LARGEFINGER
#if LARGE_AREA
U8 AreaCheck(U8 ypeak, U8 xpeak, U8 *Bound)
//U8 AreaCheck(U8 ypeak, U8 xpeak)
{
    //U16 addr_idx;
    //U8 yaddr_idx, xaddr_idx, peak_idx;
    S8 yaddr_idx;
    U8 areacnt;
    U16 peakareacnt;
    U8 xpeak_leftbound, xpeak_rightbound;
    /*
       for(addr_idx = 0, yaddr_idx = 0; yaddr_idx<TxLength; yaddr_idx++)
        for(xaddr_idx = 0; xaddr_idx<RxLength; xaddr_idx++, addr_idx++)
                AreaChkFlag[yaddr_idx][xaddr_idx]= (ptrLastQuaDelta[addr_idx]>=LARGEAREA_THD)?  1 : 0;
     */
    /*
       for(addr_idx = 0, yaddr_idx = 0; yaddr_idx<AlgorithmParam.v_rows; yaddr_idx++)
        for(xaddr_idx = 0; xaddr_idx<AlgorithmParam.h_cols; xaddr_idx++, addr_idx++)
                AreaChkFlag[yaddr_idx][xaddr_idx]= (ptrLastQuaDelta[addr_idx]>=LARGEAREA_THD)?  1 : 0;
     */
    peakareacnt=0;
    yaddr_idx=ypeak;
    leftbound=xpeak;
    rightbound=xpeak;
    Bound[0]=xpeak;
    Bound[1]=xpeak;
    Bound[2]=ypeak;
    Bound[3]=ypeak;
    areacnt=1;
    while ((areacnt!=0)&&(yaddr_idx>=0))
    {
        //areacnt = area_ydirection(yaddr_idx, xpeak, &Bound);
        areacnt=area_ydirection(yaddr_idx, xpeak);
        peakareacnt+=((U16) areacnt);
        if ( leftbound  < Bound[0] ) Bound[0]=leftbound;
        if ( rightbound > Bound[1] ) Bound[1]=rightbound;
        Bound[2]=(U8)yaddr_idx;
        if (yaddr_idx == ypeak)
        {
            xpeak_leftbound=leftbound;
            xpeak_rightbound=rightbound;
            //xpeak_leftbound = Bound[0];
            //xpeak_rightbound= Bound[1];
        }
        yaddr_idx--;
    }
    yaddr_idx=ypeak+1;
    if (peakareacnt)
    {
        leftbound=xpeak_leftbound;
        rightbound=xpeak_rightbound;
        //Bound[0] = xpeak_leftbound;
        //Bound[1] = xpeak_rightbound;
    }
    areacnt=1;
    //while((areacnt!=0)&&(yaddr_idx<TxLength))
    while ((areacnt!=0)&&(yaddr_idx<AlgorithmParam.v_rows))
    {
        //areacnt = area_ydirection(yaddr_idx, xpeak, &Bound);
        areacnt=area_ydirection(yaddr_idx, xpeak);
        if ( leftbound  < Bound[0] ) Bound[0]=leftbound;
        if ( rightbound > Bound[1] ) Bound[1]=rightbound;
        Bound[3]=(U8)yaddr_idx;
        peakareacnt+=((U16) areacnt);
        yaddr_idx++;
    }
    //printf("P->{(%2bu,%2bu): %04d},%04d\r\n", ypeak, xpeak, peakareacnt, ptrLastQuaDelta[ypeak*RxLength+xpeak]);
    //printf("P->{(%2bu,%2bu): %04d},%04d\r\n", ypeak, xpeak, peakareacnt, ptrLastQuaDelta[ypeak*AlgorithmParam.h_cols+xpeak]);
    return peakareacnt;
}

//U8 area_ydirection(U8 ydir, U8 xpeak, U8 *Bound)
U8 area_ydirection(U8 ydir, U8 xpeak)
{
    U8 i, areacnt, xaddr_idx;
    xaddr_idx=xpeak;
    while ((AreaChkFlag[ydir][xaddr_idx]==0)&&(xaddr_idx>=leftbound)) xaddr_idx--;
    if (xaddr_idx>=leftbound) xpeak=xaddr_idx;
    else
    {
        xaddr_idx=xpeak+1;
        while ((AreaChkFlag[ydir][xaddr_idx]==0)&&(xaddr_idx<=rightbound)) xaddr_idx++;
        if (xaddr_idx<=rightbound) xpeak=xaddr_idx;
        else return 0;
    }
    areacnt=0;
    for (i=0; i<2; i++)
    {
        xaddr_idx=xpeak+i;
        //while((AreaChkFlag[ydir][xaddr_idx]==1)&&(xaddr_idx>=0)&&(xaddr_idx<RxLength))
        while ((AreaChkFlag[ydir][xaddr_idx]==1)&&(xaddr_idx>=0)&&(xaddr_idx<AlgorithmParam.h_cols))
        {
            areacnt++;
            if (i==0)
            {
                leftbound=xaddr_idx;
                xaddr_idx--;
            }
            else
            {
                rightbound=xaddr_idx;
                xaddr_idx++;
            }
        }
    }
//  printf("  %2bu, (%2bu,%2bu,%2bu): %2bu\r\n", ydir, leftbound, xpeak, rightbound,areacnt);
    return areacnt;
}

void AreaProcess()
{
    U8 i, j, k, m;
    for ( i=0; i<TouchFingersList->fingers_number; i++)
    {
        if ( PeakArea[i] >= flLargeAreaPara.LargeArea_rej )
            //----------------------------
            //Palm Reject
            //----------------------------
        {
            //printf("RM:(%2bu,%2bu,%3bu)\n\r", peak[1][i], peak[0][i], PeakArea[i]);
            for ( j=i; j<TouchFingersList->fingers_number; j++)
            {
                peak[0][j]=peak[0][j+1];
                peak[1][j]=peak[1][j+1];
                PeakArea[j]=PeakArea[j+1];
                for ( k=0; k<4; k++) BoundSet[j][k]=BoundSet[j+1][k];
            }
            TouchFingersList->fingers_number--;
            i--;
        }
        else if ( PeakArea[i] >= flLargeAreaPara.LargeArea_fig )
            //----------------------------
            // Check large finger
            //----------------------------
        {
            //fgLargeAreaAlert = 1;
#if BIGFingerProcess
            fgBIGFinger=0;   // initialize
#endif
            for ( j=i+1; j<TouchFingersList->fingers_number; j++)
            {
#if BIGFingerProcess
                if ( ( (PeakArea[j] == PeakArea[i]) && (BoundSet[j][0]==BoundSet[i][0]) && (BoundSet[j][1]==BoundSet[i][1]) && (BoundSet[j][2]==BoundSet[i][2]) && (BoundSet[j][3]==BoundSet[i][3]))
                        || ( ((abs(peak[0][i]-peak[0][j]) <= BIGFingerTolerance) && ( abs(peak[1][i]-peak[1][j] <= BIGFingerTolerance))) && ((PeakArea[i] >= BigFingerAreaNumber) || (PeakArea[j] >= BigFingerAreaNumber))    ))
#else
                if ( (PeakArea[j] == PeakArea[i]) && (BoundSet[j][0]==BoundSet[i][0]) && (BoundSet[j][1]==BoundSet[i][1]) && (BoundSet[j][2]==BoundSet[i][2]) && (BoundSet[j][3]==BoundSet[i][3]))
#endif
                {
                    for ( k=j ; k<TouchFingersList->fingers_number; k++)
                    {
                        peak[0][k]=peak[0][k+1];
                        peak[1][k]=peak[1][k+1];
                        PeakArea[k]=PeakArea[k+1];
                        for ( m=0; m<4; m++) BoundSet[k][m]=BoundSet[k+1][m];
                    }
                    TouchFingersList->fingers_number--;
                    j--;
#if BIGFingerProcess
                    fgBIGFinger=1;
#endif
                }
            }
        }
    }
    /*
       for ( i= 0; i<TouchFingersList->fingers_number; i++)
       {
        if( PeakArea[i] >= flLargeAreaPara.LargeArea_fig )
        {
                fgLargeAreaAlert = 1;
                for ( j=i+1; j<TouchFingersList->fingers_number; j++)
                {
                        if( (PeakArea[j] == PeakArea[i]) && (BoundSet[j][0]==BoundSet[i][0]) && (BoundSet[j][1]==BoundSet[i][1]) && (BoundSet[j][2]==BoundSet[i][2]) && (BoundSet[j][3]==BoundSet[i][3]))
                        {
                                for ( k=j ; k<TouchFingersList->fingers_number; k++)
                                {
                                        peak[0][k] = peak[0][k+1];
                                        peak[1][k] = peak[1][k+1];
                                        PeakArea[k] = PeakArea[k+1];
                                        for ( m=0; m<4; m++) BoundSet[k][m] = BoundSet[k+1][m];
                                }
                                TouchFingersList->fingers_number--;
                                j--;
                        }
                }
        }
       }
     */
}

S16 WeightingSumAreaMain(U8 *BoundInfo, U8 fgAxis)
{
    long tmpVal[3];
    U8 idx1, idx2;
    U8 BdIdx[4];
    U16 tmpIdx;
    S16 Coordinate;
    tmpVal[1]=0;
    tmpVal[2]=0;
    /*
       if ( fgAxis == 1 ) for ( i=0; i<4; i++ ) BdIdx[i] = BoundInfo[i];
       else       for ( i=0; i<4; i++ ) BdIdx[i] = BoundInfo[(i+2)%4];
     */
    if ( fgAxis == 1 )
    {
        BdIdx[0]=BoundInfo[0];
        BdIdx[1]=BoundInfo[1];
        BdIdx[2]=BoundInfo[2];
        BdIdx[3]=BoundInfo[3];
        if ( BdIdx[1] - BdIdx[0] < 1 )
        {
            if ( BdIdx[0] != 0 ) BdIdx[0]--;
            if ( BdIdx[1] != AlgorithmParam.h_cols-1 ) BdIdx[1]++;
        }
        if ( BdIdx[3] - BdIdx[2] < 1 )
        {
            if ( BdIdx[2] != 0 ) BdIdx[2]--;
            if ( BdIdx[3] != AlgorithmParam.v_rows-1 ) BdIdx[3]++;
        }
    }
    else
    {
        BdIdx[0]=BoundInfo[2];
        BdIdx[1]=BoundInfo[3];
        BdIdx[2]=BoundInfo[0];
        BdIdx[3]=BoundInfo[1];
        if ( BdIdx[1] - BdIdx[0] < 1 )
        {
            if ( BdIdx[0] != 0 ) BdIdx[0]--;
            if ( BdIdx[1] != AlgorithmParam.v_rows-1) BdIdx[1]++;
        }
        if ( BdIdx[3] - BdIdx[2] < 1 )
        {
            if ( BdIdx[2] != 0 ) BdIdx[2]--;
            if ( BdIdx[3] != AlgorithmParam.h_cols-1) BdIdx[3]++;
        }
    }
    //for ( idx1 = BoundInfo[2-fgAxis*2]; idx1 <= BoundInfo[3-fgAxis*2]; idx1++)
    for ( idx1=BdIdx[0]; idx1 <= BdIdx[1]; idx1++)
    {
        tmpVal[0]=0;
        //for ( idx2 = BoundInfo[0+fgAxis*2]; idx2 <= BoundInfo[1+fgAxis*2]; idx2++ )
        for ( idx2=BdIdx[2]; idx2 <= BdIdx[3]; idx2++)
        {
            //tmpIdx = ( AlgorithmParam.h_cols*( (1-fgAxis)*idx1 + idx2*fgAxis) ) + ( (1-fgAxis)*idx2 + idx1*fgAxis );
            if ( fgAxis == 1 ) tmpIdx=AlgorithmParam.h_cols * idx2 + idx1;
            else tmpIdx=AlgorithmParam.h_cols * idx1 + idx2;
            if ( ptrLastQuaDelta[tmpIdx] > flLargeAreaPara.LargeArea_weight_thd )
                tmpVal[0]+=ptrLastQuaDelta[tmpIdx];
        }
        tmpVal[0]=tmpVal[0] >> 2;
        tmpVal[1]+=tmpVal[0] *( idx1*RES_SCALE + (RES_SCALE >> 1));
        tmpVal[2]+=tmpVal[0];
    }
    Coordinate=( tmpVal[1]+(tmpVal[2]>>1) ) / tmpVal[2];
    return Coordinate;
}

#endif
#if SELF_WEIGHTING
S16 WeightingSumSelfMain( S16* InputDelta, U8 TarPeak, U8 AxisLength_Minus)
{
    S8 i, Idx, SearchDir;
    S16 ReturnPos, tmpVal, preVal;
    long DeltaSum=0, WDeltaSum=0;
    for ( i=0; i<2; i++ )
    {
        Idx=((S8)TarPeak)-i;
        SearchDir=(i==0) ? 1 : -1;
        preVal=InputDelta[Idx];
        while ( ( InputDelta[Idx] < (preVal*6/5) ) && (Idx <= (AxisLength_Minus)) && (Idx >= 0 ) )
        {
            preVal=(InputDelta[Idx]>0) ? InputDelta[Idx] : 0;
            tmpVal=(preVal>16) ? ( (preVal+8)>>4) : 0;
            DeltaSum+=(long)tmpVal;
            WDeltaSum+=(long)((Idx*RES_SCALE+(RES_SCALE>>1))*tmpVal);
            Idx+=SearchDir;
        }
    }
    ReturnPos=(S16)( (WDeltaSum+(DeltaSum>>1))/DeltaSum);
    return ReturnPos;
}

#endif

#if 0   // remark for tempory shrink code size

//U8 MKeyBaseLine[MAX_MUTU_KEY_NUM];
U8 MKeyBaseLineCounter[3];
U8 CalKeyBaselineOK[3];
U8 KeyLockCounter = 0;
U16 key16buf = 0;
void CalMutuKeyBaseline() // [twokey]
{
    U8 xxx;

    if (bPowerOnMutual_Key[freqIndex])
    {
        bPowerOnMutual_Key[freqIndex] = 0;
        for (xxx = 0; xxx<flKeyPara.num ; xxx++)
        {
            mutukeyBaseline[freqIndex][xxx] = mutuKey_Raw[xxx];
        }
    MKeyBaseLineCounter[freqIndex] = 0;
    CalKeyBaselineOK[freqIndex] = 0;
    } 
    else if (CalKeyBaselineOK[freqIndex] == 0)
    {

        for (xxx=0;xxx<flKeyPara.num;xxx++)
        {
            if (((mutuKey_Raw[xxx] - mutukeyBaseline[freqIndex][xxx]) < 80) ) //40
            {
                mukeyBaseline_en = 1;
            }
            else if ((mutuKey_Raw[xxx] - mutukeyBaseline[freqIndex][xxx]) >= 110) //50              // 110
            {
                mukeyBaseline_en = 0;
//    KeyLockCounter++;
                break;
            }
        }


        if ((mukeyBaseline_en==1) && (MKeyBaseLineCounter[freqIndex] < 17) )
        {
          MKeyBaseLineCounter[freqIndex]++ ;
      
    for (xxx = 0; xxx<flKeyPara.num; xxx++)
    {
        key16buf = ((mutukeyBaseline[freqIndex][xxx] + mutuKey_Raw[xxx]) >>1);    // 16 frame sum
      mutukeyBaseline[freqIndex][xxx]  = key16buf;
    }

    if (MKeyBaseLineCounter[freqIndex] == 17)
    {
      CalKeyBaselineOK[freqIndex]= 1;
          }
  }
    
    }
}

void keyCheck()
{
    U8 data xxx; //JStest
    S16 data buf;
  U16 tmpkey;
  S16 *pbkeyT0,*pbkeyT1,*pbkeyT2,*pbkeyT3;

    if (flKeyPara.type&0x80)
    {
      KeyPress = 0;
        {
      //if(TouchFingersList->fingers_number==0)
            {
                for (xxx = 0; xxx<flKeyPara.num ; xxx++)
                {
            tmpkey = flKeyPara.CHS[xxx];
            
            if(freqcounter>=4)
              mutuKey_Raw[xxx] =  (S16)(pMuRaw_buf_T0[ tmpkey ]+pMuRaw_buf_T1[ tmpkey ]+pMuRaw_buf_T2[ tmpkey ]+pMuRaw_buf_T3[ tmpkey ]);

            else
              mutuKey_Raw[xxx] =  (S16)(pMuRaw_buf_T3[ tmpkey ])<<2;

        
                }
                //printf("%05u %05u\r\n",mutuKey_Raw[0],mutuKey_Raw[1]);
                CalMutuKeyBaseline();
        pbkeyT3 = mutukeyBaseline[freqIndex];
        pbkeyT2 = mutukeyBaseline[freqhist[(freqbufnow+3)%4]];
        pbkeyT1 = mutukeyBaseline[freqhist[(freqbufnow+2)%4]];
        pbkeyT0 = mutukeyBaseline[freqhist[(freqbufnow+1)%4]];
        
                for (xxx = 0; xxx<flKeyPara.num ; xxx++)
                {
                  tmpkey = flKeyPara.CHS[xxx];
          //buf = ((mutuKey_Raw[xxx] -mutukeyBaseline[freqIndex][xxx]) * flKeyPara.tKeyGain[xxx]); //4// 4x * 2
          //buf = pMuRaw_buf_T3[tmpkey ]-(pbkeyT3[xxx]>>2)+
          //    pMuRaw_buf_T2[tmpkey]-(pbkeyT2[xxx]>>2)+
          //    pMuRaw_buf_T1[tmpkey]-(pbkeyT1[xxx]>>2)+
          //    pMuRaw_buf_T0[tmpkey]-(pbkeyT0[xxx]>>2);


          buf = (pMuRaw_buf_T3[tmpkey]+pMuRaw_buf_T2[tmpkey]+pMuRaw_buf_T1[tmpkey]+pMuRaw_buf_T0[tmpkey])-
                (S16)  (((U16)pbkeyT3[xxx]+(U16)pbkeyT2[xxx]+(U16)pbkeyT1[xxx]+(U16)pbkeyT0[xxx]+2)>>2);

//      printf("< xxx:%bu  buf:%d >      ", xxx, buf);

//          buf = buf * flKeyPara.tKeyGain[xxx];      // temp remove by poly 20131203

          
          MutuKeyBufferDelta[xxx] = buf;    // 4x * 2     
          //printf("%d :%d\n\r",(int)xxx,(int)buf);
                    //@@@
      if (buf > flKeyPara.KeyThreshold)
                    {

#if SwitchLowFreqMode
                        if (fgLowfFreq == 1)    // 0: high, 1:low
                            KeyPressCounter[xxx] = flKeyPara.keydebounce;
                        else
#endif
                            KeyPressCounter[xxx]++;
                    }
                    else
                        KeyPressCounter[xxx]=0;

                    if (KeyPressCounter[xxx]>=flKeyPara.keydebounce)
                    {
                        //KeyPress |= 0x1<<xxx;
                        //KeyPress = KeyPress | (1<<xxx);
                        KeyPress  = BIT8_MASK[xxx];   // allow 1 key press on.
                        KeyPressCounter[xxx] = flKeyPara.keydebounce;
                    }
                }
            }
        }   
    }
}

#endif


U16 sum3reqnoise(U8 freq)
{
  return (Noiselevel[freq*3]+Noiselevel[freq*3+1]+Noiselevel[freq*3+2]);
}











void measureNLlevel(void)
{
  S16 tmpv1,tmpv2;
  U8 tData3;
  U8 data rx_length = flPannel.X_RX;

  tmpv1 = 0;
  for(tData3=0;tData3<rx_length;tData3++)
  {
    tmpv2 = ((NoiseTargetAddr[tData3]>=NoiseTargetAddr[tData3+rx_length]) ?  (NoiseTargetAddr[tData3]-NoiseTargetAddr[tData3+rx_length]):(NoiseTargetAddr[tData3+rx_length]-NoiseTargetAddr[tData3]));        

    if(tmpv2>tmpv1)
      tmpv1 = tmpv2;  
  } 

/*
  for(tData3=0;tData3<RxLength;tData3++)
  {
    if(NoiseTargetAddr[tData3]>=NoiseTargetAddr[tData3+RxLength])
      tmpv2 = NoiseTargetAddr[tData3]-NoiseTargetAddr[tData3+RxLength];
    else
      tmpv2 = NoiseTargetAddr[tData3+RxLength]-NoiseTargetAddr[tData3];
    if(tmpv2>tmpv1)
      tmpv1 = tmpv2;  
  } 
*/
  Noiselevel[NLIndex] = (Noiselevel[NLIndex]*7+tmpv1)>>3;
    
}


void chooisefreq1(void)
{
  U16 tmpv1,tmpv2;
  U8 tData3;

  tmpv1 = sum3reqnoise(0);//Noiselevel[0]+Noiselevel[1]+Noiselevel[2];
  scanfreqIndex = 0;
  for(tData3=1;tData3<3;tData3++)
  {
    tmpv2 =sum3reqnoise(tData3);// Noiselevel[tData3*3]+Noiselevel[tData3*3+1]+Noiselevel[tData3*3+2];
    if(tmpv2<tmpv1)
    {
      tmpv1 = tmpv2;
      scanfreqIndex = tData3; 
    } 
  }
  
  if (tmpv1 > NSL_On_ChargerFlagTH)
    charger_on_status = 1;
  else if (tmpv1 <= NSL_Off_ChargerFlagTH)
    charger_on_status = 0;


}

void chooisefreq2(void)
{
  U16 tmpv1,tmpv2;
  U8 tData3,i;
  
  tmpv1 = sum3reqnoise(freqIndex);
  sumnoiselev1=tmpv1;
  
  scanfreqIndex = freqIndex;



  
  for(i=1;i<3;i++)
  {
    tData3 = ((freqIndex+i)%3);
    if(MutuBaseRy[tData3]==1)
    {
      tmpv2 =sum3reqnoise(tData3);
      if(tmpv2<tmpv1)
      {
        tmpv1 = tmpv2;
        scanfreqIndex = tData3;         
        sumnoiselev2=tmpv1;   //best
      }
    } 
  }
  
  if (tmpv1 > NSL_On_ChargerFlagTH)
    charger_on_status = 1;
  else if (tmpv1 <= NSL_Off_ChargerFlagTH)
    charger_on_status = 0;
}

//#endif
// 2. TpMainLoop
//---- Main Loop Start -----------------------------------
//U8 flag = 1;
U8 chargerdebounce=0;

S8 TpMainLoop(void)
{
    S8 retv=1;
    U8 data tData3=0;
    U8 tmpv1,tmpv2;
#if SwitchLowFreqMode
    if (AP_ActiveFlag == 1)
    {
        MCUCounter=0;
        fgLowfFreq=0;
    }
    else if (MCUCounter > flSystemPara.Timer_period_Idle) //JS20130708a
    {
        //switch_MCUDIV_FreqMode(10,1);   // MCU /15 = 30/15 = 2 MHz & ADC power down
        switch_MCUDIV_FreqMode(10);     // MCU /15 = 30/15 = 2 MHz
        fgLowfFreq=1;                                   // enter low freq mode
    }
#endif

    if (IsEndOfScan()&fgFrameSync& bridge_i2c_done)
    {
        if (Intr_trig)
        {
            report_TxOut();// SET INTO LOW
            Intr_trig=0;
        }

/* This IF statement eliminate the RAW data unstable of a RIGHTKEY sensor. */
        if(flKeyPara.type==MUTU_RIGHTKEY)
        {
            U16 node_ptr=Total_RX-1;
            U8 x;
            for(x=0; x<Total_TX;x++)
            {
               if(pMuRaw_buf_T3[node_ptr] > 4000) 
                  pMuRaw_buf_T3[node_ptr] = 1200;  //Fix the unconnected node to 1000.
               node_ptr += Total_RX;
            }
        }
        else if(flKeyPara.type==MUTU_DOWNKEY)
        {
            U16 node_ptr=(Total_TX-1)*Total_RX;
            U8 x;
            for(x=0; x<Total_RX;x++)
            {
                if(pMuRaw_buf_T3[node_ptr] > 4000) 
                    pMuRaw_buf_T3[node_ptr]=1200;  //Fix the unconnected node to 1000.
                node_ptr ++;
            }
        }


        bridge_i2c_done=bridge_i2c_done_pre;
        bridge_i2c_done_pre=0;
        
#if NEWSysTimerReload_poly
        TF0=0;
        SystemTimerReload();    // may remark
        SystemTimerStart();
#endif

        if (NextScanMode == SELF_MODE)
        {
            SelfScanStart(pSelfTargetAddr);
        }
        else
        {
            if (selfprocess==1)
            {
                SelfPulseTime = UsedCTFreq[freqIndex];
                SelfScanStart(pSelfTargetAddr);
                while (!ADCScan_Done) {};
            }
            
            measureNLlevel();
            if(lastTouchFingersList_1->fingers_number!=0 || last_key!=0 || checkcorrectfig!=0)        
            {     
                chooisefreq2();             

                if((charger_on_status == 1) && (sumnoiselev1<2*sumnoiselev2))
                    scanfreqIndex=freqIndex;
                if(scanfreqIndex!=freqIndex)
                    freqcounter=0;
                else if(freqcounter<FrameHiThr)
                    freqcounter++;
              
            }
            else
            {
                chargerdebounce=0;
                if (freqcounter<FrameHiThr)
                {
                    freqcounter++;
                    scanfreqIndex =freqIndex;
                }
                else
                {
                    tmpv1=(freqIndex+1)%3;
                    tmpv2=(freqIndex+2)%3;
                    if(sum3reqnoise(tmpv1)<NoiseHiThr)
                    {
                        scanfreqIndex = tmpv1;
                        if ( sum3reqnoise(tmpv2) < NSL_Off_ChargerFlagTH)
                            charger_on_status = 0;
                    }
                    else if(sum3reqnoise(tmpv2)<NoiseHiThr)
                    {
                        scanfreqIndex = tmpv2;
                        if ( sum3reqnoise(tmpv2) < NSL_Off_ChargerFlagTH)
                          charger_on_status = 0;
                    }
                    else
                        chooisefreq1();
   
                    if(scanfreqIndex!=freqIndex)
                        freqcounter=0;
                    else if(freqcounter<FrameHiThr)
                        freqcounter++;
                }
             }
             MutuPulseTime = UsedCTFreq[scanfreqIndex];      
             MutualScanStart(pMutuTargetAddr);  // -%%---- 36.4 us
        }
        fgFrameSync=0;
        gbCNter++;
        start_frame();
        //--- read wp1001 raw data
        //ReadRawData(ScanMode);
        //--- start trigger Next Scan
        if (ScanMode == SELF_MODE)
        {
            SelfMainLoop();
            preOpMode[1]&=~(0x2) ;
            if ((preOpMode[1]&0xC0)==0) // no read Raw data
                bridge_i2c_done_pre=1;
        }
        else // ScanMode == MUTUAL_MODE
        {
            //---mutual slide winsow---
            if (Mutu_que.status<( Raw_buf_len))
            {
                Mutu_que.status++;
                NoReport=1;
                NextScanMode2=MUTUAL_MODE;
            }
            else
            {
                MutualMainLoop(); // %%---~ 8.7 ms @ 30MHz
                if ((preOpMode[1]&0xF0)==0) // no read Raw data
                    bridge_i2c_done_pre=1;
            }
            Mutu_que.index=(Mutu_que.index+1)%Raw_buf_len;
            preOpMode[1]|=0x2 ;
        }
#if SwitchLowFreqMode
        if (fgLowfFreq == 0)
        {
            if (prePeaks == 0)
            {
                MCUCounter++; // counter for MCU stop mode
                AP_ActiveFlag=0 ;
            }
            else
            {
                MCUCounter=0;
                CheckWkUp=0;
            }
        }
        else    // fgLowfFreq == 1
        {
            if ((prePeaks != 0) || (last_key != 0))
            {
                switch_MCUDIV_FreqMode(1); // MCU_DIV = 1
                MCUCounter=0;
                fgLowfFreq=0;
                CheckWkUp=1;
#if 0 // MeasureIDLEBackTime
                printf("s \n\r");
#endif
            }
        }
#endif
        if (NoReport | SwitchScanMode_trig_pre) // %%------------------------------------EE , 1F:326us , 2F:403
        {
            retv=0;
            bridge_i2c_done_pre=1;
            if(checkcorrectfig ==1) 
                checkcorrectfig =0;
        }
        else
        {
            keyCheck();  // %%  --- 26us
            if(TouchFingersList->fingers_number>1)
            {
                if((TouchFingersList->fingers_number+KeyPress) > 0 && (lastTouchFingersList_1->fingers_number+last_key)==0 && checkcorrectfig ==0)
                {
                      start_frame();
                      checkcorrectfig =1;
                      IdFlags = 0;
                      while (!ADCScan_Done) {};
                      measureNLlevel();
                      chooisefreq2();
                      freqcounter=0;        
                      MutuPulseTime = UsedCTFreq[scanfreqIndex];
                      MutualScanStart(pMutuTargetAddr);  // -%%---- 36.4 us
                }
                else
                    checkcorrectfig =0;
            }     
#if PointGitterReject
            GitterReject(TouchFingersList); // by two key newTD
            GitterReject_paraReset(TouchFingersList);
#endif

#if POINT_PREDICT_ON
            //Point_Predict(TouchFingersList);
            SortFingerID(TouchFingersList);                                         // %% ---- 3F ~100us
            Point_Predict(p1FingerIDList);
            Points_Queue(&PredictFingerIDList, &FingerIDList_Output); // %%  --- 3F ~217us            
#else
            SortFingerID(TouchFingersList);                                         // %% ---- 3F ~100us
  #if 0 // !TP_CUSTOMER_PROTOCOL
            Points_Queue(p1FingerIDList, &FingerIDList_Output); // %%  --- 3F ~217us
       #else
            Rotate_Queue(p1FingerIDList, &FingerIDList_Output);
  #endif
#endif

#if JITTER_for_INNOS_Poly
            Jitter_IIR(&FingerIDList_Output);
            Jitter_IIR_Init(&FingerIDList_Output);
#endif
            /*
               if(FingerIDList_Output.fingers_number>0)
               {
                for(ii=0;ii<MAX_FINGERS;ii++)
                {
                        if(FingerIDList_Output.Touch_Info[ii]>0)
                        {
                                printf("out %02bu,(%04d,%04d),%03bu,%03bu\r\n",ii,FingerIDList_Output.Pos[0][ii],FingerIDList_Output.Pos[1][ii],PointsQueReg.Status[ii],gbCNter);
                        }
                }
                printf("\r\n");
               }
             */
            //Report Finger info----
            Protocol.Customer=preProtocol.Customer;
            Protocol.Project_ID=preProtocol.Project_ID;
/*      #if debug_Self_Rawdata  // Tim for debug self  //AD20130723a: kill.

            if ((fDEBUGSettingPara.Self_Enable == 123) || (OpMode[1]&0x01) == 1) //AD20130723a: add for JACK AP to read self data.
                //if(Self_counter>150)   //AD20130723a: kill.
            {
                if ((OpMode[1]&0xC0)==0)
                {
                    preOpMode[1]|=fDEBUGSettingPara.Self_DataSwitch;   // SELF_ROWDATA SELF_BASELINE SELF_DELTA  //JS20130710a merge from Poly
                }
                preOpMode[1]&=~(0x2) ;
#if SwitchLowFreqMode
                AP_ActiveFlag=1;
#endif
            }
      #endif  //AD20130723a: kill.
*/
            TpFillRawdataPacket();
            ReportSelectProtocol();
        }
//#ifdef SUO_LIB
//    I2C_Send_Enable = 0; //jackson20121106
//#endif
        freqIndex = scanfreqIndex;  
        freqbufnow =((++freqbufnow)%4); S
        freqhist[freqbufnow] = freqIndex;


        pbT3 = MutualBaseLine[freqIndex];
        pbT2 = MutualBaseLine[freqhist[(freqbufnow+3)%4]];
        pbT1 = MutualBaseLine[freqhist[(freqbufnow+2)%4]];
        pbT0 = MutualBaseLine[freqhist[(freqbufnow+1)%4]];
        last_finger=FingerIDList_Output.fingers_number;
        last_key=KeyPress;
        SwitchMemory();
        bridge_i2c_done|=nbridge_i2c_EN;
        NoReport=0;
        NextScanMode2=MUTUAL_MODE;   //  always  is MUTUAL_MODE.
        NextScanMode=NextScanMode2;
        ScanMode=NextScanMode;
        OpMode[0]=preOpMode[0] ;
        OpMode[1]=preOpMode[1];
        preIdFlags=IdFlags;
        // %%---------------------------------------------------------------------------EE  End
    }
    WatchDog_Clear();
#if DebugForAPKTest
    report_DebugInfo_UserDefine_Clear();            //clear APK buffer
#endif
    return retv;
}

#if DebugForAPKTest
void Debug_Function()
{
    //user debug 1
    report_DebugInfo_UserDefine1(0,(S16)(gbCNter));
    report_DebugInfo_UserDefine1(1,(S16)(charger_on_status));
    report_DebugInfo_UserDefine1(2,0x5555);
    report_DebugInfo_UserDefine1(3,0x1AAA);
    report_DebugInfo_UserDefine1(4,0x5555);
    report_DebugInfo_UserDefine1(5,0x1AAA);
    report_DebugInfo_UserDefine1(6,0x5555);
    report_DebugInfo_UserDefine1(7,0x1AAA);
    report_DebugInfo_UserDefine1(8,0x5555);
    report_DebugInfo_UserDefine1(9,0x1AAA);
    report_DebugInfo_UserDefine1(10,0x5555);
    report_DebugInfo_UserDefine1(11,0x1AAA);
    report_DebugInfo_UserDefine1(12,0x5555);
    report_DebugInfo_UserDefine1(13,0x1AAA);
    report_DebugInfo_UserDefine1(14,0x5555);
    report_DebugInfo_UserDefine1(15,0x1AAA);
    //user debug 2
    report_DebugInfo_UserDefine2(0,0x2AAA);
    report_DebugInfo_UserDefine2(1,(S16)(charger_on_status));
    report_DebugInfo_UserDefine2(2,0x2AAA);
    report_DebugInfo_UserDefine2(3,0x5555);
    report_DebugInfo_UserDefine2(4,0x2AAA);
    report_DebugInfo_UserDefine2(5,0x5555);
    report_DebugInfo_UserDefine2(6,0x2AAA);
    report_DebugInfo_UserDefine2(7,0x5555);
    report_DebugInfo_UserDefine2(8,0x2AAA);
    report_DebugInfo_UserDefine2(9,0x5555);
    report_DebugInfo_UserDefine2(10,0x2AAA);
    report_DebugInfo_UserDefine2(11,0x5555);
    report_DebugInfo_UserDefine2(12,0x2AAA);
    report_DebugInfo_UserDefine2(13,0x5555);
    report_DebugInfo_UserDefine2(14,0x2AAA);
    report_DebugInfo_UserDefine2(15,0x5555);
//  printf("CHa: %bu \n\r",charger_on_status);
//  printf("gbc: %d \n\r",gbCNter);
}

#endif
void ReportSelectProtocol()
{
    if (Protocol.Customer==Winpower)
    {
        report_Reset();
        report_SendPoint();
        report_SendKey(KeyPress);
// @@@
#if DebugForAPKTest
        Debug_Function();               // fill bugger and than set INT pin = 0
#endif
// @@@
#if WinpowerCalReportRate
        Intr_trig=1;
#else
        if ((preOpMode[1]&0xF0)==0) // no read Raw data
        {
            if (FingerIDList_Output.fingers_number>0 || KeyPress > 0)
            {
                Intr_trig=1;
            }
            else if (last_finger > 0 ||  last_key>0)
            {
                Intr_trig=1;
            }
        }
        else
        {
            Intr_trig=1;
        }
#endif
    }
    else if ((Protocol.Customer==Customer_reg)&&(Protocol.Project_ID==Project_ID_reg ))
    {
#if SZ_Floating_DEBUG
        DEBUG_mutuRAWOneCell=(pMuRaw_buf_T3[94]);       // near 1500
#endif
       #if TP_CUSTOMER_PROTOCOL // Protocol written by customer //JS20130616a XX : For library-ize
        if (tp_customer_protocol) //JS20130616a : For library-ize
        {
            I2C_Send_Enable=0; //Clear flag at beggining. don't kill it.
            report_SendKey_user(KeyPress); // Must encode key first.
            report_SendPoint_user();
        }
        else //JS20130616a : For library-ize
        {
        }
        #endif //JS20130616a XX : For library-ize
        if ((preOpMode[1]&0x30)==0) // no read Raw data
        {
            if (I2C_Send_Enable == 1) //jackson20121106
            {
                //#ifdef TP_CUSTOMER_PROTOCOL // Protocol written by customer //JS20130616a XX : For library-ize
                if (tp_customer_protocol) //JS20130616a
                {
                    Intr_trig=1;
                }
                //#else
                else //JS20130616a
                {
                    if (FingerIDList_Output.fingers_number>0 || KeyPress > 0)
                    {
                        Intr_trig=1;
                    }
                    else if (last_finger > 0 ||  last_key>0)
                    {
                        Intr_trig=1;
                    }
                }
                //#endif //JS20130616a XX : for library-ize
            }
        }
        else
        {
            Intr_trig=1;
        }

    }
    }

void SwitchMutuMemory(void)
{
    U8* ptr;

#if REDUCE_XRAM
  #if RAW_FILTER
    ptr = (U8 *) pMuRaw_buf_T2;
    pMuRaw_buf_T2 = pMuRaw_buf_T3;
    pMuRaw_buf_T3 = pMuRaw_buf_PRE;
    pMuRaw_buf_PRE = pMutuTargetAddr;
    pMutuTargetAddr =   (S16 *)ptr;
  #else
    ptr = (U8 *) pMuRaw_buf_T2;
    pMuRaw_buf_T2 = pMuRaw_buf_T3;
    pMuRaw_buf_T3 = pMutuTargetAddr;
    pMutuTargetAddr =   (S16 *)ptr;
  #endif
#else
    ptr=(U8 *) pMuRaw_buf_T0;
    pMuRaw_buf_T0=pMuRaw_buf_T1;
    pMuRaw_buf_T1=pMuRaw_buf_T2;
    pMuRaw_buf_T2=pMuRaw_buf_T3;
    pMuRaw_buf_T3 = pMutuTargetAddr;
    pMutuTargetAddr =   (S16 *)ptr;
#endif

}

void SwitchMemory(void)
{
    //tFingerList *TmpList;
    U8* ptr;
    if ((++FrameCount) & 0x01)
    {
        //wbuf = SelfRaw1;
        //mubuf = MutualRaw1;
        pSelfdelta=SelfDelta_1;
        //pMutualdelta = MutualDelta1;
    }
    else
    {
        //wbuf = SelfRaw0;
        //mubuf = MutualRaw0;
        pSelfdelta=SelfDelta;
        //pMutualdelta = MutualDelta;
    }
    //ptr = (U8 *)mubuf_last;
    //mubuf_last = mubuf;
    //mubuf = (S16*)ptr;
    ptr=(U8 *)lastTouchFingersList_2;
    lastTouchFingersList_2=lastTouchFingersList_1;   // Twokey 0912
    lastTouchFingersList_1=TouchFingersList;
    TouchFingersList=(tFingerList *)ptr;
#if ENABLE_DUAL_FINGER_ID_LIST //JS20130205A
    ptr=(U8 *)p0FingerIDList;
    p0FingerIDList=p1FingerIDList;
    p1FingerIDList=(tFingerIDList*)ptr;
#endif
//  ptr = (U8 *)pMuRaw_buf_Tn1;
//  pMuRaw_buf_Tn1 = pMuRaw_buf_T0;

  SwitchMutuMemory();

    ptr=(U8 *)wbuf_T0;
    wbuf_T0=wbuf;
    wbuf=pSelfTargetAddr;
    pSelfTargetAddr=(S16*)ptr;
    ptr=(U8 *)ptrLastQuaDelta;
    ptrLastQuaDelta=ptrLastQuaDelta_pre;
    ptrLastQuaDelta_pre=(S16 *)ptr;
    return;
}
#if !TP_CUSTOMER_PROTOCOL
void TpTestModeLoop(void)
{
    U8 *ptr;
    U8 data aa; //JStest
    if (fgFrameSync)
    {
        SelfADCEnALLBit=0;
        fgFrameSync=0;
        OpMode[1]|=SELF_ROWDATA;
        OpMode[1]&=~(0x2) ;
#if NEWSysTimerReload_poly
        TF0=0;
        SystemTimerReload();    // may remark
        SystemTimerStart();
#endif
        //SelfScanInit(flRawPara.SelfPulseCount,flRawPara.SelfPulseLen,flRawPara.Self_fb);
        //SelfScanInit(150,flRawPara.SelfPulseLen,flRawPara.Self_fb);
        SelfScanInit(fDEBUGSettingPara.TestMode_SelfRawCount, fDEBUGSettingPara.TestMode_SelfRawPulse, fDEBUGSettingPara.TestMode_SelfFb);      //D3 version
        if (TestModeFlag==0)
        {

            if (!fDEBUGSettingPara.TestMode_SelfCC) // If this value ==0, then use AUTO CC for Jack's AP
                AutoSelfCalibration(pSelfTargetAddr);

            else  // Otherwise set as this value for Andy's AP.

                for (aa=0; aa<48; aa++) SelfComOffset[aa]=fDEBUGSettingPara.TestMode_SelfCC; // Force testmode CC as fixed value.
            TestModeFlag=1;
            MutualScanInit(flRawPara.MutualPulseCount,flRawPara.MutualPulseLen,flRawPara.Mutu_fb);
#ifdef AUTOCC_MODULE
            AutoMutuCalibration_byNode(pMutuTargetAddr,flRawPara.MutuCC_MinRaw,flRawPara.Mutu_offsetCC);
#endif
        }
        SelfScanStart(pSelfTargetAddr);
        while (!IsEndOfScan()) ;
        // fill Rowdata
        //for(aa=22;aa<34;aa++) // Remove mutual key on slef mode
        //printf("%04d %04d %04d\r\n",pSelfTargetAddr[19],pSelfTargetAddr[20],pSelfTargetAddr[21]);
#if 0 // shrink-DK
        if ( flKeyPara.type ==MUTU_DOWNKEY)
        {
            for (aa=Total_TX; aa<(Total_TX+Total_RX); aa++) // Remove mutual key on slef mode
            {
                pSelfTargetAddr[aa-1]=pSelfTargetAddr[aa];
            }
        }
#endif
        ptrReportSelfData=(S16*)pSelfTargetAddr;
        FingerIDList_Output.fingers_number=0;
        ReportSelectProtocol();
        report_TxOut();
        ptr=(U8 *)wbuf_T0;
        wbuf_T0=wbuf;
        wbuf=pSelfTargetAddr;
        pSelfTargetAddr=(S16*)ptr;
        WatchDog_Clear();
    }
    return;
}
#endif
U8 fgGotoSleep=0;
void Sleep(void)
{
    fgGotoSleep=1;
}

void WakeUp(void)
{
    fgGotoSleep=3;
}

#define LockPort_OPEN(){P0_1=0; }
#define LockPort_CLOSE(){P0_1=1; }
void TpLoop()
{
    U8 x;
    bSelfProcessEn=flKeyPara.KeyGain[7] & 0x01;     // Bit 0 enable the normal self mode.
#if !TP_CUSTOMER_PROTOCOL
    if ((TestModeFlag==1)||(OpMode[1] & RUN_FULL_SELF))
    {
        TpTestModeLoop();
    }
    else
#endif
    {
        //TpMainLoop();   //AD130724: kill for snooze mode.
        //AD130724: Snooze mode begin
        switch (fgGotoSleep)
        {
        case 0:
            TpMainLoop();
            break;
        case 1:
            //AD20130725: Shutdown ADC --> Kill none-scanning powered ADC consume 2.0 mA.
            LockPort_OPEN();
            MCU_ADDR=ADC_EN_SETTING;
            MCU_DATA=0x70;
            LockPort_CLOSE();
            //: -- Switch MCU to clock to around 4MHZ --> Digital Consumption 0.3mA
            //x = (flSystemPara.Clock_rate/4);  // Minimum stabe clock = 4MHZ.
#ifdef ACTIVE_POWER_SAVING              //AD20130725 Add #if for new SetMcuMainClk(..,..)
            SetMcuMainClk(20, 1);
#else
            SetMcuMainClk(20);
#endif
            x=(McuMainClock/4);   // Minimum stabe clock = 4MHZ.
            if (x>15) x=15; //Max register value = 15.
            switch_MCUDIV_FreqMode(x); // Set current MCU frequency to 4MHz.
            fgGotoSleep=2;
            break;
        case 2: //Stay here while in sleep mode.
            WatchDog_Clear();
            break;
        case 3: // Resume stage.
            //: -- Switch MCU to same as OSC_CLOCK.
#ifdef ACTIVE_POWER_SAVING              //AD20130725 Add #if for new SetMcuMainClk(..,..)
            SetMcuMainClk(flSystemPara.Clock_rate, (flKeyPara.KeyGain[7] & 0x1e) >> 1);
            fgBoostingClock=0;
#else
            SetMcuMainClk(flSystemPara.Clock_rate);
#endif
            //--------------
            //AD20130725: Power up ADC
            LockPort_OPEN();
            MCU_ADDR=ADC_EN_SETTING;  //0x3A
            MCU_DATA=0x01;                  // power on analog block
            LockPort_CLOSE();
            //--------------
            //fgLowfFreq =0;
            //CheckWkUp=1;  // Set to 1 to reduce dirty touch to 1 after wakeup.
            //AD20130725:  Re-initial ADC after power up ADC.
            if (selfprocess==1) // for self mode
            {
                SelfScanInit(flRawPara.SelfPulseCount,flRawPara.SelfPulseLen,flRawPara.Self_fb);
                AutoSelfCalibration(pSelfTargetAddr);
            }
            MutualScanInit(flRawPara.MutualPulseCount,flRawPara.MutualPulseLen,flRawPara.Mutu_fb);
            //--------------
            fgFrameSync=0;
            SystemTimerReload();    // may remark
            SystemTimerStart();
            fgGotoSleep=0;
            break;
        default:
            fgGotoSleep=3;
            break;
        }
        //AD130724: Snooze mode end
    }
}
//==================Terry write code begin====================//
void ResetAllBaseline(void)
{
    Sereset();
    SePOBreset();
    selfEnviroment_cal_flagreset();
    Mureset();
    MuPOBreset();
    SebaseRyreset();
    MubaseRyreset();
    Sereset();
    Mureset();
}
//==================Terry write code End====================//

