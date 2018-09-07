#ifndef _COM_GLOBAL_H
#define _COM_GLOBAL_H


#include "wp100f12.h"
#include "signal.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
#include "drv_uart.h"
#include "knl_protocol.h"
#include "drv_scan.h"
#include "knl_touch.h"
#include "flash_para.h"

#include "com_funcdef.h"
#include "com_global_type.h"
#include "ctm_user_protocol.h"



#define LoByte(AA)  ((U8) (AA))
#define HiByte(AA)  ((U8) (((U16) (AA)) >> 8))
#define B2W(AA,BB)	(U16)(((U16)AA <<8)|(BB))
#define SET_BIT(x,y)    (x|=(1<<y))
#define CLR_BIT(x,y)    (x&=~(1<<y))


#define SELF_MODE				0x0001
#define MUTUAL_MODE				0x0002
#define SELF_MODE_AUTO_CC		0X0004
#define MUTUAL_MODE_AUTO_CC		0x0008
#define NL_MODE					0x0010

#define SELF_ALL				0
#define SELF_SINGLE				1
#define I2C_STATE_RNW        	0x01
#define I2C_STATE_DNA        	0x02
#define I2C_STATE_STOP       	0x04
//bit definition of ScanStatus
#define bOPEN_SHORT_TEST		0x1000

#define	bNEED_TRIGGER_NL		0x0001
#define	bNL_FINISHED			0x0002

#define bNEED_TRIGGER_SELF		0x0010
#define bSELF_FINISHED			0x0020
#define bNEED_AUTO_SELF_CC		0x0040
/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/


#define	bNEED_TRIGGER_MUTUAL	0x0100
#define	bMUTUAL_FINISHED		0x0200
#define bNEED_AUTO_MUTUAL_CC	0x0400

#if PROTOCOL_V20
#define SLOT_TEMP_BUF_SIZE				128
#endif



extern U8 CFGMEM_NL_MOD4_TXEN_TABLE[6]									__attribute__ ((section(".noinit")));
//////////////////////////////////////////////////

extern U16 NL_FREQ_BASELINE[MAX_NL_FREQ_NUM][MAX_NL_NODE_NUM]			__attribute__ ((section(".noinit")));
extern U16 NL_RX_NOISE_LEVEL[MAX_NL_NODE_NUM] 							__attribute__ ((section(".noinit")));
extern U16 NL_FREQ_LEVEL[MAX_NL_FREQ_NUM] 								__attribute__ ((section(".noinit")));
extern U16 PRE_NL_FREQ_LEVEL[MAX_NL_FREQ_NUM] 							__attribute__ ((section(".noinit")));
#if !PROTOCOL_V20
extern U8 AP_MutualCC[MAX_MUTUAL_AP_NODE_NUM]							__attribute__ ((section(".noinit")));
extern U8 SelfRawAdjCC[MAX_SELF_AP_CH_NUM] 								__attribute__ ((section(".noinit")));
extern U8 AP_SelfCC[MAX_SELF_AP_CH_NUM] 								__attribute__ ((section(".noinit")));
#else
extern U8 volatile SlotTempBuffer[];
extern tParamAddresses ParamAddresses ;
#endif
extern U8 MutualAutoCC[CFGMEM_MUTUAL_CCPOL_SIZE]						__attribute__ ((section(".noinit")));
extern U8 SelfAutoCC[CFGMEM_SELF_CCPOL_SIZE]							__attribute__ ((section(".noinit")));
extern U16 NLRawDataBuffer[2][MAX_NL_NODE_NUM] 							__attribute__ ((section(".noinit")));
extern U16 MutualRawDataBuffer[2][CFGMEM_MUTUAL_CCPOL_SIZE]				__attribute__ ((section(".noinit")));
extern U16 SelfRawDataBuffer[2][CFGMEM_SELF___CCPOL_SIZE]				__attribute__ ((section(".noinit")));

extern U8 CFGMEM_MUTUAL_RXEN_TABLE[CFGMEM_MUTUAL_RXEN_LENGTH] 			__attribute__ ((section(".noinit")));
extern U8 CFGMEM_MUTUAL_TXEN_TABLE[CFGMEM_MUTUAL_TXEN_LENGTH] 			__attribute__ ((section(".noinit")));
extern U8 CFGMEM_SELF_RXEN_TABLE[CFGMEM_SELF_RXEN_LENGTH] 				__attribute__ ((section(".noinit")));
extern vU32 ScanStatus													__attribute__ ((section(".noinit")));
extern vU8 CPTrimValue													__attribute__ ((section(".noinit")));
//////////////////////////////////////////////////
extern U16 *pNLTargetRawData;
extern U16 *pMutualTargetRawData;
extern U16 *pSelfTargetRawData;

extern S16 peakmax;
extern S16 peakmin;
extern U8 POF_SelfRecover;

//bios.c
#define IsEndOfScan() (ADCScan_Done==1)
#define IsEndOfFrameTimer() (bFrameTimerSync == 1)
#if !PROTOCOL_V20
#define IsEndOfAPRead() (KnlProtocolCtrl & bBRIDGE_I2C_DONE)
#else
#define IsEndOfAPRead() (ProtocolStateIs.end_of_fetch)
#endif
#define BitSetFrameTimerSync() {bFrameTimerSync = 1;}
#define ClearTimer0Count() {Timer0_ReportRateCount = 0;}

#define ClearEndOfScan() {ADCScan_Done = 0;}
#define ClearEndOfAPread() {KnlProtocolCtrl &= ~bBRIDGE_I2C_DONE;}
#define ClearFrameTimerSync() {bFrameTimerSync = 0;}


extern const U8 BIT8_MASK[8];
extern U8 TxNum;
extern U8 RxNum;
extern vU8 Bios_ScanMode;
extern vU8 ADCScan_Done;
extern U8 ADCNumTotal;
extern U8 SelfActualDataNum;
extern U8 SelfTotalADCDataNum;
extern U16 MutualActualDataNum;
extern U16 MutualModulationADCDataNum;
extern vU16 MutualAutoCC_H_INT_Times;
extern vU16 SelfAutoCC_H_INT_Times;
extern U16 Timer0_ReportRateBound;
extern U16 Timer0_ReportRateCount;
extern U8 bFrameTimerSync;
extern vU16 FRAME_Counter;
extern U16 FRAME_ReadyEnter;
extern U8 IDLE_MODE_STATUS;
extern vU8 WDT_INTR_HAPPENED;
extern vU16 WDT64_WakeUpcount;
#if !PROTOCOL_V20
extern U8 OpenShortTestMode;
#endif
extern vU8 NOW_HoppingBestFrequencyIndex;
extern vU8 NOW_BestFreqTableIndex;
extern vU8 HoppingNowIndex;
extern vU8 HoppingNowTableIndex;
extern U8 DoNoiseListeningCounter;
extern U8 NLHopNoReport;


extern vU8 NLInitBaseCounter;
extern vU8 PowerOnInitialProcess;


//com_service.c
#define Enable_SYNC_Timer		TACTL |= 0x0020	//Set bit4 and bit5 to "01b"
#define Disable_SYNC_Timer		TACTL &= 0xFFCF	//Clear bit4 and bit5 of the TACTL
#define Clear_SYNC_Timer_Flag	TACTL &= ~TAIFG;

#define Disable_WDT_Timer   	WDTCTL = (WDTPW+WDTHOLD+WDTCNTCL+ WDTSSEL  + WDTIS1  + WDTIS0)  		//stop and clear the wdt counter
#define Disable_WDT_Timer_512  	WDTCTL = (WDTPW+WDTHOLD+WDTCNTCL   +WDTSSEL+WDTIS1+0)  					//stop and clear the wdt counter
#define WDT64_Counter_Clear		WDTCTL = (WDTPW + WDTCNTCL + WDTSSEL + WDTIS1  + WDTIS0)
#define WDT512_Counter_Clear	WDTCTL = (WDTPW + WDTCNTCL + WDTSSEL + WDTIS1  +    0  )
#define TimerA_CounterClear		TAR = 0x0000										//Clear timer counter.

#define Enable_WDT_32768    	WDTCTL = (WDTPW + WDTSSEL)							//reset time:0.25ms*32768   interrupt time:0.25ms*16384
#define Enable_WDT_8192     	WDTCTL = (WDTPW + WDTSSEL  +    0    + WDTIS0 )		//reset time:0.25ms*8192    interrupt time:0.25ms*4096 
#define Enable_WDT_512      	WDTCTL = (WDTPW + WDTSSEL  + WDTIS1  +   0    )		//reset time:0.25ms*512     interrupt time:0.25ms*256
#define Enable_WDT_64       	WDTCTL = (WDTPW + WDTSSEL  + WDTIS1  + WDTIS0 )		//reset time:0.25ms*64  interrupt time:0.25ms*32
	// 00:
	// 01: 1000 ms
	// 02: 64   ms
	// 03: 8.54 ms


//scan.c
extern SIG_FRAME SigFrame;
extern BASE_FRAME BaseFrame;
extern U8 MutualBaseAbnormalCounter;
extern U8 SelfBaseAbnormalCounter;
extern U8 SelfModePowerOnFlag;
extern U8 MutualModePowerOnFlag;
extern U8 TouchKeyPowerOnFlag;
#if !SHRINK_FLASH_SIZE__TOUCHKEY
extern U16 TouchKeyRawdata[4];
extern U16 TouchKeyBaseline[4];
extern S16 TouchKeyDelta[4];
extern U8 KeyPressCounter[4];
#endif
extern U16 KeyNode[4];
extern vU8 SelfDummyScan;


//protocol.c
extern vU16 KnlDriverOffset;		//
extern vU8 KnlGetCcMode;			//
extern vU8 KnlGetCcPacketLength;	//
extern vU8 KnlGetCcPacketSize;		//
extern vU16 KnlGetCcByteIndex;		//
extern vU8 KnlGetCcPacketIndex;		//
extern U8 KnlPreBridgeDone;
#if !PROTOCOL_V20
extern vU8 KnlReportSize;
#endif
extern U8 KnlOpMode[3];
extern U8 KnlPreOpMode[3];
extern U16* KnlDataPtr0;
extern U16* KnlDataPtr1;
extern PROTOCOL KnlProtocol;
extern TP_HEADER KnlTpHeader;
extern vU8 PreKeyPress;
extern vU8 PrePointNum;
extern vU8 PreOutPointNum;

#if PROTOCOL_V20
extern S16 DebugDumpVar[16];
#endif

//i2c
extern vU8 I2cRegKeyEnable;
extern vU8 I2cUnLockMaches;
extern const U8 ProtocolUnLockKey[6];

#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
extern U8 BigThumbFirstIn[KNL_MAX_FINGER_NUM];
extern U8 BigThumb;
extern U8 FingerFrameCounter[KNL_MAX_FINGER_NUM];
extern POINT_OUTPUT FingersPool[AVERAGE_LARGE_FRAME];
#endif

//touch.c
extern S16 TxSigBuffer[MAX_TX_NUM+1];
extern S16 RxSigBuffer[MAX_RX_NUM+1];
extern TRACKING_BUFFER PointTrackingBuffer[KNL_MAX_POINT_NUM * KNL_MAX_FINGER_NUM];
extern RX_RANGE RxRangeRecord;
extern RANGE_BUFFER PreRangeBuffer;
extern RANGE_BUFFER PointRange;
extern POINT_LIST PointList;
extern ROTATE_QUEUE RotateQueue;
extern FORCE_POINT_FILTER enForcePointFilter;
extern U16 PointStartMove;
extern U8 TxViewAreaPortNum;
extern U8 RxViewAreaPortNum;
extern U8 TxViewAreaMaxPort;
extern U8 RxViewAreaMaxPort;
extern U8 TxPortNum;
extern U8 RxPortNum;
extern S16 SigLocalMax;
extern U16 TxScalingParameter;
extern U16 RxScalingParameter;
extern U8 TxStartScalingDeno;
extern U8 TxEndScalingDeno;
extern U8 TxScalingNumrator;
extern U8 RxStartScalingDeno;
extern U8 RxEndScalingDeno;
extern U8 RxScalingNumrator;
extern U16 TxMinEdgeExtendCH;
extern U16 TxMaxEdgeExtendCH;
extern U16 RxMinEdgeExtendCH;
extern U16 RxMaxEdgeExtendCH;
#if !SELF_ASSISTANCE
extern U16 MutualOnlyPofCounter;
extern U8 MutualOnlyPofReCalibrateBaselineFrame;
extern U16 NowPosNode;
extern U16 PrePosNode;
#endif
extern S16 MutualDynamicThr;

// SELF one finger addressing
extern U8 SelfPeakTx[KNL_MAX_FINGER_NUM];
extern U8 SelfPeakRx[KNL_MAX_FINGER_NUM];
extern U8 DoSelfReport;
extern U8 SelfJudgePalmStatus;
extern U8 SelfTxAddrNum;
extern U8 SelfRxAddrNum;
extern vU8 SelfNeedReCalibrateCC;
extern vU8 SelfNeddReTryDone;
// SELF one finger addressing


#define UART_INT_ENABLE						0x01	//bit0
#define TIMER0_INT_ENABLE					0x02	//bit1
#define TIMER1_INT_ENABLE					0x04	//bit2
#define I2C_KEY_RISING_INT_ENABLE			0x10	//bit4
#define I2C_KEY_FALLING_INT_ENABLE			0x20	//bit5
#define ID_TRACKING_FINISH_INT_ENABLE		0X40	//bit6

#define FRAME_CAL_FINISH_INT_ENABLE			0X01	//bit0
#define WATCH_DOG_INT_ENABLE				0X04	//bit2
#define GPIO0_INT_ENABLE					0X08	//bit3
#define TCON_INT_ENABLE						0X10	//bit4
#define IF_INT_ENABLE						0X20	//bit5


#endif	//_COM_GLOBAL_H
