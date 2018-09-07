/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _COM_GLOBAL_TYPE_H
#define _COM_GLOBAL_TYPE_H

#include "iomacros.h"

sfrb(P1OUT       ,__P1OUT);			// Keep it, do not modify !!!!

#define ENABLE		1
#define DISABLE		0
#define TRUE		1
#define FALSE		0
#define ON			1
#define OFF			0
#define	SelfReady		1
#define	MutualReady		1
#define	NLReady			1
#define	SelfWorking		0
#define	MutualWorking	0
#define	NLWorking		0
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef signed char S8;
typedef signed short S16;
typedef signed long S32;
typedef volatile unsigned char vU8;
typedef volatile unsigned short vU16;
typedef volatile unsigned long vU32;
typedef volatile signed char vS8;
typedef volatile signed short vS16;
typedef volatile signed long vS32;


#define	SetINTPinHigh() {P1OUT |= 0x01;}

#define TX_IDX 							0
#define RX_IDX 							1
#define NO_KEY							0x00
#define SELF_KEY						0x81
#define MUTUAL_DOWN_KEY					0x82
#define MUTUAL_RIGHT_KEY				0x83
#define SELF_SHARE_KEY					0x84

#define KNL_MAX_FINGER_NUM				5
#define KNL_MAX_POINT_NUM				(KNL_MAX_FINGER_NUM * 2)
#define RV_G_THDIFF		                2560

//CFGMEM
#define CFGMEM_SELF_RXEN_LENGTH			36		// 6byte * 6
#define CFGMEM_SELF_RAWOFFSET_LENGTH	48		// 1byte * 48
#define CFGMEM_SELF_CCPOL_SIZE			84		// 6pass * 14 ADC
#define CFGMEM_MUTUAL_TXEN_LENGTH		48		// SD.h V273+; 48 = 6byte * (32Tx/4mod),only support 4x MOD;20140919 		// 6byte * 30Tx = 180
#define CFGMEM_MUTUAL_RXEN_LENGTH		18		// 6byte * 3 PASS
#define	CFGMEM_MOD4_TXEN_FIXED_6BYTES	6
#define FW_RAWOFFSET_PER_PASS			84		// 6pass * 14 ADC 
#define MUTUAL_REG54_LENGTH				30		// MAX 30 Rx
#define MUTUAL_VCOM_RX_LENGTH			48		// VCom RX size
#define MUTUAL_VCOM_RX_PER_PASS			3		// support 3 passes
#define MUTUAL_VERTICAL_ID_LENGTH		30		// MAX 30 Tx

#define I2C_BUFF_LENGTH					32
#define KNL_REPORT_LENGTH				64
#define bNOTLOAD_RCV_PTR				0x80

#define KNL_MUTUAL_TX_LARGE_BOUNDARY    0x80
#define KNL_MUTUAL_TX_LITTLE_BOUNDARY   0x40
#define KNL_MUTUAL_RX_LARGE_BOUNDARY    0x20
#define KNL_MUTUAL_RX_LITTLE_BOUNDARY   0x10
#define MAX_TX_NUM						48
#define MAX_RX_NUM						48

//Bit definition of the FwSetting.PosMappingFunction
#define bXY_REVERSE						0x0004      //default:Rx=>X, Tx=>Y Bit set:Rx=>Y, Tx=>X
#define bRX_POSITION_REVERSE			0x0002
#define bTX_POSITION_REVERSE			0x0001


//////////////////@@@@@@@@@@ A D V A N C E   S E T T I N G @@@@@@@@@@//////////////////
#define		AUTO_FREQ_SEARCH_SPECTRUM	ENABLE
#define		AUTO_FREQ_SELECTION_TABLE	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0xFF, 0xFF,}	// Support max. 10 freq. Table; max 0x0F.(0~15).
			// Freq. table value(10 slots): strart from "0x00" to "0x0F". If disable, fill into "0xFF".

#define		AUTO_FREQ_SWITCHING_LEVEL	50				// fixed																										// If no use freq., fill in 0xff.
#define 	MUTUAL_AUTOCC_START			1				// Max: 0x3F
#define		SELF___AUTOCC_START			1				// Max: 0x3F
#define 	TX_INTERPOLATION_VALUE		64
#define 	RX_INTERPOLATION_VALUE		64
#define 	MIN_MERGE_DISTANCE			((TX_INTERPOLATION_VALUE * 3)/2)				// dx + dy
#define 	TAPPING_DISTANCE			((TX_INTERPOLATION_VALUE * 9)/2)				// tapping
#define 	ROTATE_QUEUE_LATENCY		2
#define 	BIG_THUMB_NODE_THR			16				// parameterize

#define		DE_COMMON_MODE_NOISE		ENABLE //DISABLE			//Default: DISABLE; parameterize
#define		LCM_DIFF_HI					150
#define		LCM_DIFF_LO					-150
#define		LCM_ABS_HI					500
#define		LCM_ABS_LO					0 //0

#define 	SELF_TARGET_RAW_MIN			((SELF_PULSE_LENGTH-20) * SELF_PULSE_COUNT)		//(SELF_PULSE_COUNT * 17)		//15
#define 	SELF_TARGET_RAW_MAX			((SELF_PULSE_LENGTH-15) * SELF_PULSE_COUNT)		//(SELF_PULSE_COUNT * 30)		//17
#define 	MUTU_TARGET_RAW_MIN			((MUTU_PULSE_LENGTH * MUTU_PULSE_COUNT)/2)		//(MUTU_PULSE_COUNT * 27)		//20
#define 	TIMER0						ENABLE
#define 	MUTUAL_ENCLOSURE			(MUTUAL_FINGER_THR >> 3)		//10
#define		MANUAL_FORCE_MUTUAL_CC		DISABLE			// if >0, forece Mutual mode CC
#define		MAMUAL_FORCE_SELF___CC		DISABLE			// if >0, force Self mode CC
#define		IDLE_MODE_SLP2_FRAME		10				// frame no.; 8.5ms * frame no.

#define 	SELF_BASE_TRACE_STEP		2
#define		SELF_BASE_TRACE_QUICK		30
#define 	MUTUAL_BASE_TRACE_STEP		2
#define 	KEY_BASE_TRACE_STEP			2
#define 	AVERAGE_LARGE_FRAME         16
#define		STEPLESS_FAST_FILTER		128
#define		STEPLESS_SLOW_FILTER		16

#define 	DELTA_DENOISE_BIT			4

#define		GESTURE_REMOTE_CONTROL		DISABLE			// SRC
#define 	GESTURE_STYLE 				4				// writing style
#define 	GESTURE_DATA_LEN 			16				// information collection



#define 	PROTOCOL_V20  		 		ENABLE
#if PROTOCOL_V20
	#define	PROTOCOL_VER				0x02
	#define PROTOCOL_V20_REPORT_20		ENABLE
	#define SetI2cReadDMA(ptr) {IF_DMA_ADDR_L = (U8)((U16)ptr & 0x00FF); IF_DMA_ADDR_H = (U8)((U16)ptr >> 0x08);}
#else
	#define		PROTOCOL_VER			0x01
	#define 	PROTOCOL_V20_REPORT_20	DISABLE	// Fixed.
#endif


///////////////////@@@@@@@@@@ S P E C I A L  P A T T E R N  -  n T m R @@@@@@@@@@////////////////////
#define		n_T_m_R_MUTUAL_ONLY			DISABLE
#if n_T_m_R_MUTUAL_ONLY
#define		N_DRIVING_T					1	// 1T
#define		M_SENSING_R					2	// 2R
#define		ORG_nTmR_TX_NUMBER			6
#define		ORG_nTmR_RX_NUMBER			20
#endif
///////////////////@@@@@@@@@@ S P E C I A L  P A T T E R N  -  n T m R @@@@@@@@@@////////////////////



////////////////////////////@@@@@@@@@@ D O  N O T  M O D I F Y @@@@@@@@@@/////////////////////////////
#define		DBG_NL									DISABLE		// LIB
#define		REVERSE_MUTU_DELTA						DISABLE		// SRC
#define		EXTRA_TX_PROCESS						DISABLE		// LIB
////////////////////////////@@@@@@@@@@ D O  N O T  M O D I F Y @@@@@@@@@@/////////////////////////////


//////////////////////////// ########## Shrink FLASH CODE SIZE ########## ////////////////////////////
#define		SHRINK_FLASH_SIZE__DEVCOM				DISABLE		// ---LIB: 352B,ENABLE:mask code, DISABLE: available
#define		SHRINK_FLASH_SIZE__TOUCHKEY				DISABLE		// ---LIB: 936B
#define		SHRINK_FLASH_SIZE__SELF_MUTUAL_RW		DISABLE		// ---LIB: 694B
#define		SHRINK_FLASH_SIZE__SELF_MUTUAL_W		ENABLE		// ---LIB: 502B
#define		SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING	DISABLE		// ---LIB: 334B
#define		SHRINK_FLASH_SIZE__AUTOCC_CODE			DISABLE		// ---SRC: 796B
#define		SHRINK_FLASH_SIZE__AUTO_FREQ			DISABLE		// ---SRC: 770B

#define		CPUMP									ON
#define		SELF_FORCE_VDDA							ON						// 
#define		SELECT_CVDDH							0x06					// SETTING: 1~6;	-- [1]:3.795v, [2]:3.96v, [3]:4.125v, [4]:4.29v, [5]:4.455v, [6]:4.62v,
#define		PUMP_MODE_B_18V							ENABLE					// 
#define 	HW_VER									(0xA0 | SELECT_CVDDH)	// info
//////////////////////////// ########## Shrink FLASH CODE SIZE ########## ////////////////////////////

typedef struct _POINT_OUTPUT
{
    vU16 TouchInfo;
    vU8 PointNum;
    vU16 Position[2][KNL_MAX_FINGER_NUM];
}POINT_OUTPUT;

#if PROTOCOL_V20
typedef struct
{
 U16 debug_mode  : 1; // Request FW goto debug mode.
 U16 freeze_base  : 1; // Request FW no to update baseline any more.
 U16 sleep_mode   : 1; // Request FW to go to sleep mode until next I2C command.
 U16 disable_idle : 1; // Not allowing FW go to IDLE mode automatically.
 U16 enable_hover : 1; // 1=Enable hover detection function. 0=Disable.
 U16 enable_proximity: 1;
 U16 enable_gest_wake: 1;
 U16 enable_hopping : 1;
 U16 force_copy_base : 1;
 U16 reserved  : 7;
} tVirtualRegister;
#endif

typedef struct _HW_PARAMETER
{
	U32 SerialNum;
	U16 Vid;
	U16 Pid;
	U16 UartBR;
	U16 SelfTargetRawMin;
	U16 SelfTargetRawMax;
	U16 MutualTargetRaw;
	U16 SelfPulseCount;
	U16 MutualPulseCount;
	U16 ReportRateBound;
	U8 ProtVer;
	U8 Id;
	U8 HwVer;
	U8 FwVer;
	U8 I2CEnableAll;	
	U8 SelfCcClk;
	U8 SelfCfb;
	U8 SelfPulseLenth;
	U8 MutualCcClk;
	U8 MutualCfb;
	U8 MutualPulseLenth;
	U8 MutualAutoCCStart;
	U8 SelfAutoCCStart;
	U8 CPump;
	U8 CPump_CVDDH;
	U8 CPump_SelfForce33v;
	U8 CPump_Mode_B_18v;
	U8 SystemTimer0;
	U8 OpenShortCcClk;
	U8 OpenShortCfb;
	U8 OpenShortPulseLenth;
	U8 OpenShortPulseCount;
	
	U8 ReservedForHwSetting[4];
}HW_PARAMETER;


typedef	struct _SENSOR_DEFINE
{
	U8 Self_Passes;
	U8 Self_CFGMEM_RxEn_Table[CFGMEM_SELF_RXEN_LENGTH];
	U8 Self_Effective_SizeOf_RxEn;
	U8 Self_RawOffset_Table[CFGMEM_SELF_RAWOFFSET_LENGTH];
	U8 Self_Effective_Sizeof_RawOffset;
	U8 FirmwareRawOffsetPerPass[FW_RAWOFFSET_PER_PASS];
	U8 Mutual_Passes;
	U8 Mutual_CFGMEM_RxEn_Table[CFGMEM_MUTUAL_RXEN_LENGTH];
	U8 Mutual_Effective_SizeOf_RxEn;
	U8 Mutual_REG54_Horizontal_Offset_Table[MUTUAL_REG54_LENGTH];
	U8 Mutual_Effective_SizeOf_REG54;
	U8 Mutual_VCom_Offset_Table[MUTUAL_VCOM_RX_LENGTH];
	U8 Mutual_VCom_Effective_SizeOf_LCM_Table;
	U8 Mutual_VCom_Number_Per_Pass_Table[MUTUAL_VCOM_RX_PER_PASS];
	U8 Mutual_TxModulationMode;
	U8 Mutual_ModulationTypeOf_REG_C7_2;
	U8 Mutual_ModulationTxSequence;
	U8 Mutual_CFGMEM_TxEn_Table[CFGMEM_MUTUAL_TXEN_LENGTH];
	U8 Mutual_Effective_SizeOf_TxEn;
	U8 Mutual_REG53_Vertical_ID_Table[MUTUAL_VERTICAL_ID_LENGTH];
	U8 Mutual_Effective_SizeOf_REG53;
	U8 I2C_Device_Address;
	U8 Mutual_TxNumbers;
	U8 Mutual_RxNumbers;
	U8 Key_Numbers;
	U8 Key_Types;
	U8 Key0_Tx;
	U8 Key0_Rx;
	U8 Key1_Tx;
	U8 Key1_Rx;
	U8 Key2_Tx;
	U8 Key2_Rx;
	U8 Key3_Tx;
	U8 Key3_Rx;
	
	U8 ReservedForSensorSetting[14];
}SENSOR_DEFINE;


typedef struct _FW_PARAMETER
{
	S16 Self_TxPeakBoundary;
	S16 Self_RxPeakBoundary;
	S16 MutualBaseTraceUpperBound;
	S16 MutualBaseTraceLowerBound;
	S16 SelfFingerTHR;
	S16 MutualFingerTHR;
	S16 SelfFingerFirstTHR;
	S16 MutualFingerFirstTHR;
	U16 TxInterpolationNum;
	U16 RxInterpolationNum;
	U16 PalmAreaNum;
	U16 MutualAreaTh;
	U16 MutualPalmTh;
	U16 MinMergeDTxDRx;
	U16 PosMappingFunction;
	U16 TxTargetResolution;
	U16 RxTargetResolution;
	U16 TappingDistance;
	U16 IdleModeTimeToEnter;
	U16 JitterFrozen;
	U16 TxStartShortage;
	U16 TxEndShortage;
	U16 RxStartShortage;
	U16 RxEndShortage;
	U16 AutoFreqSwitchingLevel;
	U16 AutoFreqSelectionTable[10];
	U16 SteplessFastFollowFilter;
	U16 SteplessSlowFollowFilter;
	S16 SelfTxNegativeThreshold;
	S16 SelfRxNegativeThreshold;
	S16 KeyThreshold;
	S16 DeLcmDiffHighTHR;
	S16 DeLcmDiffLoTHR;
	S16 DeLcmAbsHiTHR;
	S16 DeLcmAbsLoTHR;
	S16 MutualModeOnlyPOFThreshold;
	S8 SelfBaseTraceStep;
	S8 SelfBaseTraceQuickStep;
	S8 MutualBaseTraceStep;
	U8 RotateQueueLatencyFrame;
	U8 PalmTxRange;
	U8 PalmRxRange;
	U8 AreaMergeRatioBit;
	U8 KeyDebounceTime;
	U8 KeyBaseTraceStep;
	U8 SelfModeAssistance;		// reserved
	U8 IdleModeSLP2Frame;
	U8 TxEdgeExtendch;
	U8 RxEdgeExtendch;
	U8 DeVComNoiseFunction;
	U8 AutoFreqHopFunction;
	U8 ForceMutualModeCC;
	U8 ForceSelfModeCC;
	U8 GestureWritingStyle;
	U8 GestureDataLength;
	U8 BigThumbNodeTHR;
	U8 SelfTxNegativeNode;
	U8 SelfRxNegativeNode;
	U8 SelfPOFReCalibrateBaseline;
	U8 MutualModeOnlyAbnormalFrame;

	U8 ReservedForFwSetting[0];
}FW_PARAMETER;

typedef struct
{
	U16 mutual_rxen_table_addr;	//Mutual_CFGMEM_RxEn_Table
	U16 mutual_reg54_h_offset_table_addr;
	U16 mutual_reg53_v_id_table_addr;
	U16 self_rxen_table_addr;
	U16 self_raw_offset_table_addr;
	U8 mutual_rxen_length;	//CFGMEM_MUTUAL_RXEN_LENGTH;
	U8 mutual_reg54_length;	//MUTUAL_REG54_LENGTH;
	U8 mutual_vertical_id_length;	//MUTUAL_VERTICAL_ID_LENGTH;
	U8 self_rxen_length;	//CFGMEM_SELF_RXEN_LENGTH;
	U8 self_raw_offset_length;	//CFGMEM_SELF_RAWOFFSET_LENGTH
	U8 tx_modulations;
} tParamAddresses;

extern HW_PARAMETER HwSetting;
extern FW_PARAMETER FwSetting;
extern SENSOR_DEFINE SensorDefine;
extern POINT_OUTPUT PointOutput;
#if PROTOCOL_V20
extern volatile tVirtualRegister HostVirtualRegister;
extern volatile tVirtualRegister CommandVirtualRegister;
#endif
extern U8 KeyPress;
extern U8 I2cSendEnable;
extern U8 LastTouch;

extern U8 I2cStatus;
extern int I2cDataIdx;
extern U8 *I2cDmaPtr;
extern U8 I2cDataBuff[I2C_BUFF_LENGTH];
extern vU8 I2cReportBuff[KNL_REPORT_LENGTH];
extern vU8 KnlProtocolCtrl;
#define ctm_user_protocol_rcv_write_default() {KnlProtocolCtrl |= bNOTLOAD_RCV_PTR;}



#if GESTURE_REMOTE_CONTROL
extern vU8 GestureRecognized;
extern const S8 gs_character[GESTURE_STYLE];
extern const S8 gs_simulation[GESTURE_STYLE*2][GESTURE_DATA_LEN];

#endif


#endif	//_COM_GLOBAL_TYPE_H

