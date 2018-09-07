/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"



void ChipConfigSettingReload()
{
	U16 idx,node;

	node = SensorDefine.Self_Effective_SizeOf_RxEn;
	for ( idx = 0 ; idx < node ; idx++ )
	{
		CFGMEM_SELF_RXEN_TABLE[idx] = SensorDefine.Self_CFGMEM_RxEn_Table[idx];
	}

	node = SensorDefine.Mutual_Effective_SizeOf_TxEn;
	for ( idx = 0 ; idx < node ; idx++ )
	{
		CFGMEM_MUTUAL_TXEN_TABLE[idx] = SensorDefine.Mutual_CFGMEM_TxEn_Table[idx];
	}

	node = SensorDefine.Mutual_Effective_SizeOf_RxEn;
	for ( idx = 0 ; idx < node ; idx++ )
	{
		CFGMEM_MUTUAL_RXEN_TABLE[idx] = SensorDefine.Mutual_CFGMEM_RxEn_Table[idx];
	}

	for ( idx = 0 ; idx < CFGMEM_MOD4_TXEN_FIXED_6BYTES ; idx++ )	// for MOD4
	{
		CFGMEM_NL_MOD4_TXEN_TABLE[idx] = 0x00;
	}
}

#if !SHRINK_FLASH_SIZE__TOUCHKEY
void Get_TouchKey_Node(void)
{
	if (SensorDefine.Key0_Tx || SensorDefine.Key0_Rx)
	{
		KeyNode[0] = (SensorDefine.Key0_Tx * RxNum + SensorDefine.Key0_Rx);
	}

	if (SensorDefine.Key1_Tx || SensorDefine.Key1_Rx)
	{
		KeyNode[1] = (SensorDefine.Key1_Tx * RxNum + SensorDefine.Key1_Rx);
	}

	if (SensorDefine.Key2_Tx || SensorDefine.Key2_Rx)
	{
		KeyNode[2] = (SensorDefine.Key2_Tx * RxNum + SensorDefine.Key2_Rx);
	}

	if (SensorDefine.Key3_Tx || SensorDefine.Key3_Rx)
	{
		KeyNode[3] = (SensorDefine.Key3_Tx * RxNum + SensorDefine.Key3_Rx);
	}
}
#endif

void com_service_variable_init(void)
{
#if WHOLE_VA_EXTEND
	U32 tmp;
#endif
	U8 idx;

	ScanStatus = 0x0000;

	bFrameTimerSync = 1;				// initialize for Timer0 
	Timer0_ReportRateCount = 0;			// initialize for Timer0 

	FRAME_Counter = 0;
	FRAME_ReadyEnter = ( FwSetting.IdleModeTimeToEnter * HwSetting.ReportRateBound );
										// if 10sec
										// (10 * 1000 ms )/(1000ms/RR:80) = 800 count
								
	IDLE_MODE_STATUS = 0;				// 0: Normal mode, 1: idle mode
	WDT_INTR_HAPPENED = 0;
	WDT64_WakeUpcount = 0;
#if !PROTOCOL_V20
	OpenShortTestMode = 0;
#endif
	pMutualTargetRawData = MutualRawDataBuffer[0];	// temp pointer
	pSelfTargetRawData = SelfRawDataBuffer[0];		// temp pointer
	pNLTargetRawData = NLRawDataBuffer[0];			// temp pointer

	for ( idx = 0 ; idx < MAX_NL_FREQ_NUM ; idx++ )
	{
		NL_FREQ_LEVEL[idx] = 0x00;		// initialize
		PRE_NL_FREQ_LEVEL[idx] = 0x00;
	}
#if !PROTOCOL_V20
	for ( idx = 0 ; idx < MAX_SELF_AP_CH_NUM ; idx++ )
	{
		SelfRawAdjCC[idx] = 0x00;
	}
	SelfNeedReCalibrateCC = 0x00;
	SelfNeddReTryDone = 1;		// 1: done, 0: need re-try
#endif
	SelfDummyScan = 0x00;	// if even scan, need it!
	
	ADCNumTotal = 14;
	TxNum = SensorDefine.Mutual_TxNumbers;
	RxNum = SensorDefine.Mutual_RxNumbers;
	SelfActualDataNum = (TxNum+RxNum);
#if !SHRINK_FLASH_SIZE__TOUCHKEY
	if (SensorDefine.Key_Types == SELF_KEY)
	{
		SelfActualDataNum = (TxNum+RxNum+SensorDefine.Key_Numbers);
	}
#endif
	SelfTotalADCDataNum = (SensorDefine.Self_Passes * ADCNumTotal);
	MutualActualDataNum = (TxNum*RxNum);
	MutualModulationADCDataNum = (SensorDefine.Mutual_TxModulationMode * SensorDefine.Mutual_ModulationTxSequence * SensorDefine.Mutual_Passes * ADCNumTotal);

#if n_T_m_R_MUTUAL_ONLY
	#if (N_DRIVING_T == 1)		//	1TmR
	TxNum = (SensorDefine.Mutual_TxNumbers<<1);
	RxNum = (SensorDefine.Mutual_RxNumbers>>1);
	#elif ((M_SENSING_R == 1))						//	nT1R
	TxNum = (SensorDefine.Mutual_TxNumbers>>1);
	RxNum = (SensorDefine.Mutual_RxNumbers<<1);
	#endif 
#endif

	DoSelfReport = 0;			//[1f]-- 0: Mutual Report, 1: Self Report
	SelfJudgePalmStatus = 0;	// Use Self To judge if Palm.
	SelfModePowerOnFlag = 1;
	
	MutualModePowerOnFlag = 1;
	TouchKeyPowerOnFlag = 1;
	KeyPress = 0;
	KnlProtocolCtrl = 0x00;
	//NL
	DoNoiseListeningCounter = 0x00;
	NLHopNoReport = 0x00;

//20141015
	HoppingNowIndex = 0x00;
	NOW_HoppingBestFrequencyIndex = 0x00;
	NOW_BestFreqTableIndex = 0x00;
	HoppingNowTableIndex = 0x00;

#if !SHRINK_FLASH_SIZE__LARGEAREA_SMOOTHING
	for (idx = 0; idx < KNL_MAX_FINGER_NUM; idx++)
	{
		BigThumbFirstIn[idx] = 0x00;
	}
	BigThumb = 0x00;
#endif

	TxViewAreaPortNum = TxNum;
	RxViewAreaPortNum = RxNum;

#if !SHRINK_FLASH_SIZE__TOUCHKEY
	if ((SensorDefine.Key_Types == MUTUAL_RIGHT_KEY) || (SensorDefine.Key_Types == MUTUAL_DOWN_KEY))
	{
		Get_TouchKey_Node();
		if (SensorDefine.Key_Types == MUTUAL_RIGHT_KEY)
		{
			RxViewAreaPortNum = (RxNum-1);
		}
		else if (SensorDefine.Key_Types == MUTUAL_DOWN_KEY)
		{
			TxViewAreaPortNum = (TxNum-1);
		}
	}
#endif

	TxViewAreaMaxPort = (TxViewAreaPortNum-0x01);
	RxViewAreaMaxPort = (RxViewAreaPortNum-0x01);  
#if WHOLE_VA_EXTEND
	tmp = FwSetting.TxTargetResolution;
	tmp <<= 0x0A;
	TxScalingParameter = ((U16)(tmp / (TxViewAreaMaxPort*(FwSetting.TxInterpolationNum))));

	tmp = FwSetting.RxTargetResolution;
	tmp <<= 0x0A;
	RxScalingParameter = ((U16)(tmp / (RxViewAreaMaxPort*(FwSetting.RxInterpolationNum))));;
#else
	TxStartScalingDeno = ((4*FwSetting.TxEdgeExtendch) - (FwSetting.TxStartShortage >> 4));		// denominator
	TxEndScalingDeno = ((4*FwSetting.TxEdgeExtendch) - (FwSetting.TxEndShortage >> 4));			// denominator
	TxScalingNumrator = (4*FwSetting.TxEdgeExtendch);
	RxStartScalingDeno = ((4*FwSetting.RxEdgeExtendch) - (FwSetting.RxStartShortage >> 4));		// denominator
	RxEndScalingDeno = ((4*FwSetting.RxEdgeExtendch) - (FwSetting.RxEndShortage >> 4));			// denominator
	RxScalingNumrator = (4*FwSetting.RxEdgeExtendch);

	TxMinEdgeExtendCH = (FwSetting.TxEdgeExtendch * FwSetting.TxInterpolationNum);
	RxMinEdgeExtendCH = (FwSetting.RxEdgeExtendch * FwSetting.RxInterpolationNum);
	TxMaxEdgeExtendCH = ( (TxViewAreaPortNum - FwSetting.TxEdgeExtendch) * FwSetting.TxInterpolationNum );
	RxMaxEdgeExtendCH = ( (RxViewAreaPortNum - FwSetting.RxEdgeExtendch) * FwSetting.RxInterpolationNum );
#endif
	PreOutPointNum = 0;
	MutualAutoCC_H_INT_Times = 0;
	SelfAutoCC_H_INT_Times = 0;


	ChipConfigSettingReload();

    PrePointNum = 0x00;
    PointList.LegalFlag = 0x0000;
    PointList.SetFlag = 0x0000;
    PointList.CurrSetFlag[0] = 0x0000;
    PointList.CurrSetFlag[1] = 0x0000;

	knl_protocol_init();
#if USER_PROTOCOL
	ctm_user_protocol_init();
    KnlProtocol.ProjectID = USER;
	#if PROTOCOL_V20
	ProtocolStateIs.user_protocol = TRUE;
	#else
    KnlProtocol.Customer = USER;
	#endif
#endif

#if GESTURE_REMOTE_CONTROL
	GestureRecognized = 0x00;	// initialize
	GestureVariableInitialFunction();
#endif

#if !SELF_ASSISTANCE
	MutualOnlyPofCounter = 0x00;		// mutual only power on finger
	MutualOnlyPofReCalibrateBaselineFrame = 10;
	NowPosNode = 0xffff;
	PrePosNode = 0xffff;
#endif

	PowerOnInitialProcess = 0x01;		// 1 for Power on mode
	MutualDynamicThr = FwSetting.MutualFingerTHR;

}




void _WATCH_DOG_INIT(U8 onoff)
{
	if(onoff)
	{
		//WDT Initial
		IE1 |= WDTIE;
		Enable_WDT_64;
	}
	else
	{
		//WDT Disable
		IE1 &= ~WDTIE;
		Disable_WDT_Timer;
	}
}


void _FRAME_FLAG_INITIALIZE(void)
{
	if (HwSetting.SystemTimer0)
	{
		ClearFrameTimerSync();		// bit clear
	}
	else
	{
		BitSetFrameTimerSync();		// bit set = 1
	}
	ClearTimer0Count();
	ClearEndOfScan();
	ClearEndOfAPread();
}
