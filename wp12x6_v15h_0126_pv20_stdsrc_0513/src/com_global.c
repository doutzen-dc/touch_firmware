/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"


U8 CFGMEM_NL_MOD4_TXEN_TABLE[6]								__attribute__ ((section(".noinit")));
//////////////////////////////////////////////////
//re-direct

U16 NL_FREQ_BASELINE[MAX_NL_FREQ_NUM][MAX_NL_NODE_NUM] 		__attribute__ ((section(".noinit")));
U16 NL_RX_NOISE_LEVEL[MAX_NL_NODE_NUM] 						__attribute__ ((section(".noinit")));
U16 NL_FREQ_LEVEL[MAX_NL_FREQ_NUM] 							__attribute__ ((section(".noinit")));
U16 PRE_NL_FREQ_LEVEL[MAX_NL_FREQ_NUM] 						__attribute__ ((section(".noinit")));
#if !PROTOCOL_V20
U8 AP_MutualCC[MAX_MUTUAL_AP_NODE_NUM] 						__attribute__ ((section(".noinit")));
U8 SelfRawAdjCC[MAX_SELF_AP_CH_NUM] 						__attribute__ ((section(".noinit")));
U8 AP_SelfCC[MAX_SELF_AP_CH_NUM] 							__attribute__ ((section(".noinit")));
#else
//U8 SlotTempBuffer[SLOT_TEMP_BUF_SIZE]						__attribute__ ((section(".noinit")));
U8 volatile SlotTempBuffer[SLOT_TEMP_BUF_SIZE];
#endif
U8 SelfRawAdjCC[MAX_SELF_AP_CH_NUM] 						__attribute__ ((section(".noinit")));
U8 MutualAutoCC[CFGMEM_MUTUAL_CCPOL_SIZE]					__attribute__ ((section(".noinit")));
U8 SelfAutoCC[CFGMEM_SELF_CCPOL_SIZE]						__attribute__ ((section(".noinit")));
U16 NLRawDataBuffer[2][MAX_NL_NODE_NUM] 					__attribute__ ((section(".noinit")));
U16 MutualRawDataBuffer[2][CFGMEM_MUTUAL_CCPOL_SIZE]		__attribute__ ((section(".noinit")));
U16 SelfRawDataBuffer[2][CFGMEM_SELF___CCPOL_SIZE] 			__attribute__ ((section(".noinit")));

U8 CFGMEM_MUTUAL_RXEN_TABLE[CFGMEM_MUTUAL_RXEN_LENGTH] 		__attribute__ ((section(".noinit")));
U8 CFGMEM_MUTUAL_TXEN_TABLE[CFGMEM_MUTUAL_TXEN_LENGTH] 		__attribute__ ((section(".noinit")));
U8 CFGMEM_SELF_RXEN_TABLE[CFGMEM_SELF_RXEN_LENGTH] 			__attribute__ ((section(".noinit")));
vU32 ScanStatus												__attribute__ ((section(".noinit")));
vU8 CPTrimValue												__attribute__ ((section(".noinit")));
//////////////////////////////////////////////////
U16 *pNLTargetRawData;
U16 *pMutualTargetRawData;
U16 *pSelfTargetRawData;

S16 peakmax;
S16 peakmin;
U8 POF_SelfRecover;

//bios.c
const U8 BIT8_MASK[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
U8 TxNum;
U8 RxNum;
U8 ADCNumTotal;
U8 SelfTotalADCDataNum;
U16 MutualModulationADCDataNum;
U8 SelfActualDataNum;
U16 MutualActualDataNum;
vU16 MutualAutoCC_H_INT_Times;
vU16 SelfAutoCC_H_INT_Times;
U16 Timer0_ReportRateBound;
U16 Timer0_ReportRateCount;
U8 bFrameTimerSync;
vU8 Bios_ScanMode;
vU8 ADCScan_Done;
vU16 FRAME_Counter;
U16 FRAME_ReadyEnter;
U8 IDLE_MODE_STATUS;
vU8 WDT_INTR_HAPPENED;
vU16 WDT64_WakeUpcount;
#if !PROTOCOL_V20
U8 OpenShortTestMode;
#endif
vU8 NOW_HoppingBestFrequencyIndex;
vU8 NOW_BestFreqTableIndex;
vU8 HoppingNowIndex;
vU8 HoppingNowTableIndex;
U8 DoNoiseListeningCounter;
U8 NLHopNoReport;
vU8 PowerOnInitialProcess;


//scan.c
SIG_FRAME SigFrame;
BASE_FRAME BaseFrame;
U8 MutualBaseAbnormalCounter;
U8 SelfBaseAbnormalCounter;
U8 SelfModePowerOnFlag;
U8 MutualModePowerOnFlag;
U8 TouchKeyPowerOnFlag;
#if !SHRINK_FLASH_SIZE__TOUCHKEY
U16 TouchKeyRawdata[4];
U16 TouchKeyBaseline[4];
S16 TouchKeyDelta[4];
U8 KeyPressCounter[4];
#endif
U16 KeyNode[4];
U8 KeyPress;
vU8 SelfDummyScan;

//protocol.c
vU8 KnlProtocolCtrl;
vU16 KnlDriverOffset;		// not
vU8 KnlGetCcMode;			// not
vU8 KnlGetCcPacketLength;	// not
vU8 KnlGetCcPacketSize;		// not
vU16 KnlGetCcByteIndex;		// not
vU8 KnlGetCcPacketIndex;	// not
U8 KnlPreBridgeDone;
#if !PROTOCOL_V20
vU8 KnlReportSize;
#endif
U8 KnlOpMode[3];
U8 KnlPreOpMode[3];
U16 *KnlDataPtr0;
U16 *KnlDataPtr1;
PROTOCOL KnlProtocol;
TP_HEADER KnlTpHeader;
vU8 PreKeyPress;
vU8 PrePointNum;
vU8 PreOutPointNum;


// i2c
U8 I2cStatus;
int I2cDataIdx;
U8 *I2cDmaPtr;
U8 I2cDataBuff[I2C_BUFF_LENGTH];
vU8 I2cReportBuff[KNL_REPORT_LENGTH];
vU8 I2cRegKeyEnable;
vU8 I2cUnLockMaches;
const U8 ProtocolUnLockKey[6] = {MSG_GOTO_STATE, 0xFF, 0xA5, 0xB1, 0xC6, 0x54};


//touch.c
U16 PointStartMove;
S16 TxSigBuffer[MAX_TX_NUM+1];
S16 RxSigBuffer[MAX_RX_NUM+1];
TRACKING_BUFFER PointTrackingBuffer[KNL_MAX_POINT_NUM * KNL_MAX_FINGER_NUM];
RANGE_BUFFER PreRangeBuffer;
RANGE_BUFFER PointRange;
RX_RANGE RxRangeRecord;
POINT_LIST PointList;
ROTATE_QUEUE RotateQueue;
POINT_OUTPUT PointOutput;
FORCE_POINT_FILTER enForcePointFilter;
U8 TxViewAreaPortNum;
U8 RxViewAreaPortNum;
U8 TxViewAreaMaxPort;
U8 RxViewAreaMaxPort;
U8 TxPortNum;
U8 RxPortNum;
S16 SigLocalMax;
U16 TxScalingParameter;
U16 RxScalingParameter;
U8 TxStartScalingDeno;
U8 TxEndScalingDeno;
U8 TxScalingNumrator;
U8 RxStartScalingDeno;
U8 RxEndScalingDeno;
U8 RxScalingNumrator;
U16 TxMinEdgeExtendCH;
U16 TxMaxEdgeExtendCH;
U16 RxMinEdgeExtendCH;
U16 RxMaxEdgeExtendCH;
#if !SELF_ASSISTANCE
U16 MutualOnlyPofCounter;
U8 MutualOnlyPofReCalibrateBaselineFrame;
U16 NowPosNode;
U16 PrePosNode;
#endif

S16 MutualDynamicThr;

#if GESTURE_REMOTE_CONTROL
//Gesture
vU8 GestureRecognized;


const S8 gs_character[GESTURE_STYLE] = {
'3','3','3','3'
};

const S8 gs_simulation[GESTURE_STYLE*2][GESTURE_DATA_LEN] = {
{0,22,28,18,12,20,28,36,46,56,62,54,42,32,24,18},  //3_2.txt:#Gesture 3:
{0,2,10,18,22,22,22,22,24,28,40,46,52,58,60,64},  //
{0,2,-8,-20,-34,-24,-14,-2,12,22,16,4,-8,-20,-32,-38},  //3_3.txt:#Gesture 3:
{0,20,24,26,30,30,30,30,36,46,52,56,58,62,62,62},  //
{0,24,28,16,2,6,20,34,44,40,32,20,12,-2,-10,-16},  //3_5.txt:#Gesture 3:
{0,2,12,20,26,26,26,30,38,48,54,56,60,60,62,62},  //
{0,38,44,32,16,2,-14,-2,12,30,42,38,20,6,-10,-16},  //3a.log
{0,0,10,16,24,28,32,32,32,32,36,48,52,56,60,60},  //
};

#endif
// SELF one finger addressing
U8 SelfPeakTx[KNL_MAX_FINGER_NUM];
U8 SelfPeakRx[KNL_MAX_FINGER_NUM];
U8 DoSelfReport;
U8 SelfJudgePalmStatus;
U8 SelfTxAddrNum;
U8 SelfRxAddrNum;
vU8 SelfNeedReCalibrateCC;
vU8 SelfNeddReTryDone;
// SELF one finger addressing

U8 I2cSendEnable;
U8 LastTouch;

#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
U8 BigThumb;
U8 BigThumbFirstIn[KNL_MAX_FINGER_NUM];
U8 FingerFrameCounter[KNL_MAX_FINGER_NUM];
POINT_OUTPUT FingersPool[AVERAGE_LARGE_FRAME];
#endif

