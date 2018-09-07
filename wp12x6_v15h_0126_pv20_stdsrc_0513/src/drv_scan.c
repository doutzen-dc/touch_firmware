/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"
#include "T1007_reg.h"


void drv_scan_get_self_raw_data(U16 *self_raw_mem, U16 *s_base, U8 node)
{
	U16 idx;
	U16 *src_hwptr;
#if !PROTOCOL_V20
	U16 rawtmp;
#endif
	U16 raw_sorting[84];		// 14 * 6 = 84
	U16 idxtmp;

	src_hwptr = (U16 *)(&SelfRawDataBuffer[SelfWorking]);
	memcpy(self_raw_mem,src_hwptr,(node*2));				// copy working to ready

	idxtmp = 0x00;
	for (idx = 0; idx < node ; idx++)
	{
		idxtmp = SensorDefine.Self_RawOffset_Table[idx];
		(*(raw_sorting+(idxtmp))) = (*(self_raw_mem+idx));

		// Re-K CC
#if !PROTOCOL_V20
		rawtmp = (*(self_raw_mem+idx));
		if (rawtmp < (HwSetting.SelfTargetRawMin>>1) )
		{
			SelfRawAdjCC[idxtmp] |= 0x10;	
			SelfNeedReCalibrateCC = 1;
			SelfNeddReTryDone = 0x00;
		}
#endif
	}

	// re-assignment
	for (idx = 0; idx < node ; idx++)
	{
		//SelfRawDataBuffer[SelfReady][idx] = (*(raw_sorting+idx));
		(*(self_raw_mem+idx)) = (*(raw_sorting+idx));
	}

	// power on to initialize baseline
	if (SelfModePowerOnFlag == 1)
	{
		for (idx = 0; idx < node ; idx++)
		{
			*(s_base+idx) = *(raw_sorting+idx);
		}
		SelfModePowerOnFlag = 0;
	}
}


#if SELF_ASSISTANCE
U8 drv_scan_get_self_signal(U16 *s_raw,U16 *s_base,S16 *s_sig, S16 th, U8 data_num)
{
	U8 idx,flag;

	S16 max;
	U16 negative_node;
	U8 wait_axis;

	flag = 0x00;
	max = 0x0000;
	negative_node = 0x00;
	wait_axis = 0x00;

	if (SelfModePowerOnFlag == 0)
	{
		for(idx=0x00; idx< (data_num&0x7F); idx++)
		{
			(*(s_sig+idx)) = (*(s_raw+idx)) - (*(s_base+idx));

			if((*(s_sig+idx)) > max)
			{
				max = (*(s_sig+idx));
			}

			if ( ((*(s_sig+idx)) < (FwSetting.SelfTxNegativeThreshold)) && ((data_num&0x80) == 0x00) )		// tx
			{
				negative_node++;
				if (negative_node >= (FwSetting.SelfTxNegativeNode))
				{
					wait_axis |= 0x01;
				}
			}
			else if ( ((*(s_sig+idx)) < (FwSetting.SelfRxNegativeThreshold)) && ((data_num&0x80) == 0x80) )	// rx
			{
				negative_node++;
				if (negative_node >= (FwSetting.SelfRxNegativeNode))
				{
					wait_axis |= 0x80;
				}				
			}
		}

		if ( (wait_axis&0x81) && ((data_num&0x80) == 0x80) )		// if > 0
		{
			SelfModePowerOnFlag = 1;
			POF_SelfRecover = FwSetting.SelfPOFReCalibrateBaseline;
		}  
	}
	else
	{
		for(idx=0x00; idx<(data_num&0x7F); idx++)
		{
			(*(s_base+idx)) = (*(s_raw+idx));
		}
		if (data_num & 0x80)
		{
			SelfModePowerOnFlag = 0;
		}
	}

	if(max > th)
	{
		flag = 0x01;
	}

	return flag;
}



#endif

#if SELF_MODE_REPORT_FINGER
U8 SelfModeSearchPeak(S16 *s_sig, S16 th, U8 *peakPos, U8 *palmstatus, U8 data_num, U8 axis)
{
	U8 idx,peaks;
	S16 sf_deltaRight,sf_deltaLeft;

	peaks = 0;
	for ( idx = 0 ; idx < data_num ; idx++ )
	{
		if ( (*(s_sig+idx)) > th )
		{
			if (peaks < KNL_MAX_FINGER_NUM )
			{
				if ( idx == 0 )					// min
				{
					sf_deltaRight = (*(s_sig+idx+1));
					if ( (*(s_sig+idx)) >= sf_deltaRight )
					{
						(*(peakPos+peaks)) = idx;
						peaks++;
					}
				}
				else if ( idx == (data_num-1) )	// max
				{
					sf_deltaLeft = (*(s_sig+idx - 1));
					if ( (*(s_sig+idx)) >= sf_deltaLeft )
					{
						(*(peakPos+peaks)) = idx;
						peaks++;
					}
				}
				else							// center
				{
					sf_deltaLeft = (*(s_sig+idx - 1));
					sf_deltaRight = (*(s_sig+idx + 1));
					if ( ((*(s_sig+idx)) >= sf_deltaLeft) && ((*(s_sig+idx)) >= sf_deltaRight) )
					{
						(*(peakPos+peaks)) = idx;
						peaks++;
					}
				}
			}
		}
	}
	return peaks;
}
#endif


void drv_scan_get_mutual_raw_data(U16 *mutu_raw_mem, U16 *m_base, U16 node)
{
	U16 idx;
	U16 *src_hwptr;

#if n_T_m_R_MUTUAL_ONLY
	U16 m,shift,nRnode,rawnTmR[ORG_nTmR_TX_NUMBER*ORG_nTmR_RX_NUMBER];
	#if (M_SENSING_R == 1)
	U8 model;

	model = 0x00;
	#endif
	nRnode = ORG_nTmR_TX_NUMBER * ORG_nTmR_RX_NUMBER;
#endif

	src_hwptr = (U16 *)(&MutualRawDataBuffer[MutualWorking]);
	memcpy(mutu_raw_mem,src_hwptr,(node*2));				// copy working to ready

#if n_T_m_R_MUTUAL_ONLY
	for ( m = 0 ; m < nRnode ;m++ )
	{
		(*(rawnTmR + m)) = (*(mutu_raw_mem + m));
	}

	#if (N_DRIVING_T == 1)
		for ( idx = 0 ; idx < ORG_nTmR_TX_NUMBER ; idx++ )
		{
			shift = (idx * ORG_nTmR_RX_NUMBER);

			for ( m = 0 ; m < ORG_nTmR_RX_NUMBER  ; m += M_SENSING_R )
			{
				(*(mutu_raw_mem + shift + (m/M_SENSING_R))) = (*(rawnTmR + shift + m));
				(*(mutu_raw_mem + shift + (m/M_SENSING_R) + (ORG_nTmR_RX_NUMBER/M_SENSING_R))) = (*(rawnTmR + shift + m + 1));
			}
		}
	#elif (M_SENSING_R == 1)
		for ( idx = 0 ; idx < ORG_nTmR_TX_NUMBER ; idx += N_DRIVING_T )
		{
			shift = (idx * ORG_nTmR_RX_NUMBER);
			model = (idx%N_DRIVING_T);

			for ( m = 0 ; m < ORG_nTmR_RX_NUMBER ; m++ )
			{
				if ( model == 0)
				{
					(*(mutu_raw_mem + shift + (m*N_DRIVING_T) )) = (*(rawnTmR + shift + m));
				}
				else if ( model == 1)
				{
					(*(mutu_raw_mem + shift + (m*N_DRIVING_T) + (m*ORG_nTmR_RX_NUMBER) )) = (*(rawnTmR + shift + m));
				}
			}
		}
	#endif
#endif

	if (MutualModePowerOnFlag == 1)
	{
		for (idx = 0; idx < node ; idx++)
		{
			(*(m_base+idx)) = (*(mutu_raw_mem+idx));
		}
		MutualModePowerOnFlag = 0;
	}


#if 0
	printf(" MRaw \n\r");
	for (idx = 0 ; idx < node ; idx++)
	{
		printf("[%3d]:%4d ",idx,(*(mutu_raw_mem+idx)));
		if ((idx+1)%24 == 0)
			printf("\n\r");
	}
	printf("\n\r");
#endif
}



U8 knl_de_common_mode_noise_process(S16 *m_sig,S16 upper, S16 lower, S16 th)
{
	U8 idx_tx,idx_rx,txtmp,rxtmp;
	U16 idx;
	S16 max,min,sig_temp;
	U8 flag;
	U8 totalrx;
#if !SELF_ASSISTANCE
	S16 pofth;
	U16 tmppos;
#endif	

#if !SHRINK_FLASH_SIZE__DEVCOM
	S16 sig_temp0,sig_temp1;
	U8 seqno,seqcnt,seqsub;
	S16 seqsum,seqavg;
	U8 seq0num,seq1num;
#endif

#if EXTRA_TX_PROCESS
	U16 start,end;
	S16 etx_avg;
#endif

	flag = 0x00;
	max = 0x0000;
	min = 0x7fff;
	txtmp = TxNum;
	rxtmp = RxNum;
	totalrx = RxNum;

#if !SELF_ASSISTANCE
	pofth = (FwSetting.MutualModeOnlyPOFThreshold);
	tmppos = 0xffff;
#endif

#if !SHRINK_FLASH_SIZE__DEVCOM
if ((FwSetting.DeVComNoiseFunction) && (SensorDefine.Mutual_VCom_Number_Per_Pass_Table[1] != 0))
{
	seqsum = 0x00;
	seqcnt = 0x00;
	seqavg = 0x00;
	seq0num = SensorDefine.Mutual_VCom_Number_Per_Pass_Table[0];
	seq1num = SensorDefine.Mutual_VCom_Number_Per_Pass_Table[1];
	
	for ( idx_tx = 0 ; idx_tx < txtmp ; idx_tx++ )
	{
		seqno = 0;

		while (seqno < rxtmp)
		{
			if ( seqno < seq0num )
			{
				sig_temp0 = (*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seqno])));
				sig_temp1 = (*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seqno+1])));

				if (seqno == 0)
				{
					seqsum = 0x00;
					seqcnt = 0x00;
					seqavg = 0x00;
				}
				else if (seqno < (seq0num - 1))
				{
					if ( ((sig_temp1-sig_temp0) < FwSetting.DeLcmDiffHighTHR) && ((sig_temp1-sig_temp0) > FwSetting.DeLcmDiffLoTHR) && (sig_temp0 < FwSetting.DeLcmAbsHiTHR) && (sig_temp0 > FwSetting.DeLcmAbsLoTHR) )
					{
						seqsum += sig_temp0;
						seqcnt++;
					}
				}
				else if (seqno == (seq0num - 1))
				{
					seqavg = seqsum/seqcnt;					
					for ( seqsub = 0 ; seqsub < seq0num ; seqsub++ )
					{
						(*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seqsub]))) -= seqavg;
					}
				}
			}
			else if ((seqno >= seq0num) && ((seqno < (seq0num+seq1num))))
			{
				sig_temp0 = (*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seqno])));
				sig_temp1 = (*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seqno+1])));
				
				if (seqno == seq0num)
				{
					seqsum = 0x00;
					seqcnt = 0x00;
					seqavg = 0x00;
				}
				else if (seqno < (seq0num+seq1num - 1))
				{
					if ( ((sig_temp1-sig_temp0) < FwSetting.DeLcmDiffHighTHR) && ((sig_temp1-sig_temp0) > FwSetting.DeLcmDiffLoTHR) && (sig_temp0 < FwSetting.DeLcmAbsHiTHR) && (sig_temp0 > FwSetting.DeLcmAbsLoTHR) )
					{
						seqsum += sig_temp0;
						seqcnt++;
					}
				}
				else if (seqno == (seq0num+seq1num - 1))
				{
					seqavg = seqsum/seqcnt;					
					for ( seqsub = 0 ; seqsub < seq1num ; seqsub++ )
					{
						(*(m_sig+((idx_tx * totalrx) + SensorDefine.Mutual_VCom_Offset_Table[seq0num+seqsub]))) -= seqavg;
					}
				}
			}
			seqno++;
		}
	}
}
#endif

#if EXTRA_TX_PROCESS
	start = 0x00;
	end = 0x00;
	etx_avg = 0x00;
	for ( idx_tx = 0 ; idx_tx < txtmp ; idx_tx++ )
	{
		start = idx_tx * totalrx;
		end = ((idx_tx+1) * totalrx) - 1;

		if ( (*(m_sig+start) < (2*th)) && (*(m_sig+end) < (2*th)) )
		{
			etx_avg = ((*(m_sig+end) - (*(m_sig+start)))/ totalrx);
			for ( idx_rx = 0 ; idx_rx < rxtmp ; idx_rx++ )
			{
				idx = idx_tx * totalrx + idx_rx;
				if ( etx_avg*idx_rx > 0 )
				{
					(*(m_sig+idx)) -= (etx_avg*idx_rx);			// offset
				}
			}
		}
	}
#endif

	for ( idx_tx = 0 ; idx_tx < txtmp ; idx_tx++ )
	{
		for ( idx_rx = 0 ; idx_rx < rxtmp ; idx_rx++ )
		{
			idx = idx_tx * totalrx + idx_rx;

			sig_temp = (*(m_sig+idx));

			if (sig_temp > max)
			{
				max = sig_temp;
			}

			if (sig_temp < min)
			{
				min = sig_temp;
				#if !SELF_ASSISTANCE
				tmppos = idx;		// pos
				#endif
			}
		}
	}
	peakmax = max;	// frame max
	peakmin = min;	// frame min

	#if !SELF_ASSISTANCE
	if (min < pofth)
	{
		NowPosNode = tmppos;

		if (PrePosNode == 0xffff)
		{
			PrePosNode = NowPosNode;			// backup
			MutualOnlyPofCounter++;				// continuous
		}
		else
		{
			if (NowPosNode == PrePosNode)
			{
				MutualOnlyPofCounter++;			// continuous
			}
			else
			{
				MutualOnlyPofCounter = 0x00;	// cut down
				PrePosNode = tmppos;
			}
		}
	}
	
	else
	{
		MutualOnlyPofCounter = 0x00;	// reset counter to '0'
		NowPosNode = 0xffff;
	}

	if (MutualOnlyPofCounter >= FwSetting.MutualModeOnlyAbnormalFrame)
	{
		MutualOnlyPofReCalibrateBaselineFrame = 10;		// re-k frame number setting
	}
	#endif


// dynamic threshold
	if (max > th)
	{
		flag = 0x01;
		MutualDynamicThr = (max/3);
	}
	else if (min < lower)
	{
		flag = 0x04;
	}

	return flag;
}

void knl_get_mutual_pre_delta(U16 *m_raw, U16 *m_base, S16 *m_sig, U16 node)
{
	U16 idx;
	U8 idx_tx,idx_rx;
	U8 rxtmp,txtmp;
	U8 totalrx;

	idx = 0x00;
	txtmp = TxNum;
	rxtmp = RxNum;
	totalrx = RxNum;


	if (MutualModePowerOnFlag == 0)
	{
		for ( idx_tx = 0 ; idx_tx < txtmp ; idx_tx++ )
		{
			for ( idx_rx = 0 ; idx_rx < rxtmp ; idx_rx++ )
			{
				idx = idx_tx * totalrx + idx_rx;
			#if REVERSE_MUTU_DELTA
				(*(m_sig+(idx))) = (S16)( (*(m_base+idx)) - (*(m_raw+idx)) );
			#else
				(*(m_sig+(idx))) = (S16)( (*(m_raw+idx)) - (*(m_base+idx)) );
			#endif
			#if 0
				if ((SensorDefine.Key_Types == MUTUAL_RIGHT_KEY) && (idx_rx == (rxtmp-1)))
				{
					(*(m_sig+(idx))) = 0x00;
				}
				else if ((SensorDefine.Key_Types == MUTUAL_DOWN_KEY) && (idx_tx == (txtmp-1)))
				{
					(*(m_sig+(idx))) = 0x00;
				}
			#endif
			}
		}
	}
	else
	{
		for (idx = 0; idx < node ; idx++)
		{
			(*(m_base+idx)) = (*(m_raw+idx));
		}
		MutualModePowerOnFlag = 0;
	}
}


#if !SHRINK_FLASH_SIZE__TOUCHKEY
U8 Get_Mutual_TouchKey_Signal(S16 thr)
{
	U8 m,keynum,kflag;
	S16 max;
	U16 keypos;
	U16 *mdbuffer,*kraw,*kbase,*knode;
	S16 *kdelta;

	mdbuffer = (U16 *)(&MutualRawDataBuffer[MutualReady]);
	kraw = (U16 *)(&TouchKeyRawdata);
	kbase = (U16 *)(&TouchKeyBaseline);
	kdelta = (S16 *)(&TouchKeyDelta);
	knode = (U16 *)(&KeyNode);
	keynum = SensorDefine.Key_Numbers;

	max = 0x0000;
	kflag = 0x00;
	keypos = 0x00;
	
	if (keynum)	
	{
		if (TouchKeyPowerOnFlag == 0)
		{
			for (m = 0 ; m < keynum ; m++)
			{
				keypos = (U16)(knode[m]);
				kraw[m]	= mdbuffer[keypos];
				kdelta[m] = kraw[m] - kbase[m];
				if (kdelta[m] > max)
				{
					max = kdelta[m];
				}			
			}
		}
		else
		{
			for (m = 0 ; m < keynum ; m++)
			{
				keypos = (U16)(knode[m]);
				kraw[m]	= (mdbuffer[keypos]);
				kbase[m]	= (mdbuffer[keypos]);
				kdelta[m] = 0x00;
			}
			TouchKeyPowerOnFlag = 0;
		}

		for (m = 0 ; m < keynum ; m++)
		{
			keypos = knode[m];
			MutualRawDataBuffer[MutualReady][keypos] = kraw[m];
			BaseFrame.Mbase.Mutual[keypos] = kbase[m];
			SigFrame.Msig.Mutual[keypos] = kdelta[m];
		}

		if (max > thr)
		{
			kflag = 1;
		}
	}
	return kflag;
}

U8 Get_Self_TouchKey_Signal(S16 thr)
{
	U16 *sraw,*k_raw,*k_base;
	S16 max,*k_sig;
	U8 idx,keystart,k_flag,keyno;

	sraw = (U16 *)(&SelfRawDataBuffer[SelfReady]);
	k_raw = (U16 *)(&TouchKeyRawdata);
	k_base = (U16 *)(&TouchKeyBaseline);
	k_sig = (S16 *)(&TouchKeyDelta);
	keyno = SensorDefine.Key_Numbers;
	keystart = TxNum+RxNum;

	max = 0x0000;
	k_flag = 0x00;

	if (keyno)
	{
		if (TouchKeyPowerOnFlag == 0)
		{
			for (idx = 0 ; idx < keyno ; idx++)
			{
				k_raw[idx] = sraw[keystart+idx];
				k_sig[idx] = k_raw[idx] - k_base[idx];

				if (k_sig[idx] > max)
				{
					max = k_sig[idx];
				}
			}
		}
		else
		{
			for (idx = 0 ; idx < keyno ; idx++)
			{
				k_raw[idx] = sraw[keystart+idx];
				k_base[idx] = sraw[keystart+idx];		// copy to base
				k_sig[idx] = 0x00;
			}
			TouchKeyPowerOnFlag = 0;
		}

		for (idx = 0 ; idx < keyno ; idx++)
		{
			SelfRawDataBuffer[SelfReady][(keystart+idx)] = k_raw[idx];
			BaseFrame.Sbase.Self[(keystart+idx)] = k_base[idx];
			SigFrame.Ssig.Self[(keystart+idx)] = k_sig[idx];
		}

		if (max > thr)
		{
			k_flag = 1;
		}
	}

	return k_flag;
}

void Get_TouchKey_Status(U16 *k_raw, U16 *k_base, S16 *k_sig, S16 thr)
{
	U8 m,keynum,kpc,kdebounce,bit_mask;
	S16 buf;

	bit_mask = 0x01;
	keynum = SensorDefine.Key_Numbers;
	kdebounce = FwSetting.KeyDebounceTime;


	for (m = 0 ; m < keynum ; m++ )
	{
	#if REVERSE_MUTU_DELTA
		buf = k_base[m] - k_raw[m];
	#else
		buf = k_raw[m] - k_base[m];
	#endif

		kpc = KeyPressCounter[m];
		if ( buf > thr )
		{
			kpc++;
			if (kpc > kdebounce)
			{
				KeyPress |= bit_mask;
			}
			else
			{
				KeyPress &= ~bit_mask;
			}
		}
		else
		{
			KeyPress &= ~bit_mask;
			kpc = 0;
		}
		k_sig[m] = buf;
		KeyPressCounter[m] = kpc;
		bit_mask <<= 1;
	}
}

void KeyCalibrationBaseline(U16 *kbase, S16 *kdelta, S8 kstep)
{
	U8 m,keynum;
	
	keynum = SensorDefine.Key_Numbers;

	for (m = 0 ; m < keynum ; m++ )
	{
		if ( (*(kdelta + m)) > kstep )
		{
			(*(kbase + m)) += kstep;
		}
		else if ( (*(kdelta + m)) < (-kstep) )
		{
			(*(kbase + m)) -= kstep;
		}
	}
}

void Classify_Touch_Key(U8 type)
{
	U8 kflag;

	if (!(type & 0x80)) 
		return;

	switch(type)
	{
		case SELF_KEY:
			kflag = Get_Self_TouchKey_Signal(FwSetting.KeyThreshold);
			if (kflag == 0)
			{
				KeyCalibrationBaseline(&TouchKeyBaseline[0],&TouchKeyDelta[0],FwSetting.KeyBaseTraceStep);
			}
			Get_TouchKey_Status((&TouchKeyRawdata[0]),(&TouchKeyBaseline[0]),(&TouchKeyDelta[0]),FwSetting.KeyThreshold);
			break;

//		case SELF_SHARE_KEY:
//			break;

		case MUTUAL_RIGHT_KEY:
		case MUTUAL_DOWN_KEY:
			kflag = Get_Mutual_TouchKey_Signal(FwSetting.KeyThreshold);
			if (kflag == 0)
			{
				KeyCalibrationBaseline(&TouchKeyBaseline[0],&TouchKeyDelta[0],FwSetting.KeyBaseTraceStep);
			}
			Get_TouchKey_Status((&TouchKeyRawdata[0]),(&TouchKeyBaseline[0]),(&TouchKeyDelta[0]),FwSetting.KeyThreshold);
			break;

		default:
			break;
	}
}
#endif


#if !PROTOCOL_V20

#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_RW
void drv_scan_read_self_cc(void)
{
	U8 i;

	for ( i = 0 ; i < SelfTotalADCDataNum ; i++ )
	{
		if ( i < (2 * ADCNumTotal) )	// Rx part
		{
			if ( SensorDefine.FirmwareRawOffsetPerPass[i] != 0xFF )
			{
				AP_SelfCC[SensorDefine.FirmwareRawOffsetPerPass[i]] = SelfAutoCC[i];
			}
		}
		else							// Tx part
		{
			if ( SensorDefine.FirmwareRawOffsetPerPass[i] != 0xFF )
			{
				AP_SelfCC[SensorDefine.FirmwareRawOffsetPerPass[i]] = SelfAutoCC[i];
			}
		}
	}
#if 0
	printf("SRC---  \n\r");
	for ( i = 0 ; i < (4*14) ; i++)
	{
		printf("S[%2d]:0x%2x  ",i,SelfAutoCC[i] );
		if ((i+1)% 14 == 0)
			printf("\n\r");
	}
	printf("\n\r");


	printf("DIST---  \n\r");
	for ( i = 0 ; i < (4*14) ; i++)
	{
		printf("D[%2d]:0x%2x  ",i,AP_SelfCC[i] );
		if ((i+1)% 14 == 0)
			printf("\n\r");
	}
	printf("\n\r");
#endif
}

void drv_scan_set_self_cc(void)
{
	U8 i,j;

	for ( i = 0 ; i < SelfActualDataNum ; i++ )
	{
		for ( j = 0 ; j < SelfTotalADCDataNum ; j++ )
		{
			if (SensorDefine.FirmwareRawOffsetPerPass[j] == i)
			{
				SelfAutoCC[j] = AP_SelfCC[i];
			}
		}
	}

#if 0
	printf("SRC---  \n\r");
	for ( i = 0 ; i < (4*14) ; i++)
	{
		printf("S[%2d]:0x%2x  ",i,SelfAutoCC[i] );
		if ((i+1)% 14 == 0)
			printf("\n\r");
	}
	printf("\n\r");


	printf("DIST---  \n\r");
	for ( i = 0 ; i < (4*14) ; i++)
	{
		printf("D[%2d]:0x%2x  ",i,AP_SelfCC[i] );
		if ((i+1)% 14 == 0)
			printf("\n\r");
	}
	printf("\n\r");
#endif
}

void drv_scan_read_mutual_cc(void)	// no support 3 passes !!!!!!!!!!
{
	U16 i,m;
	U16 txmod_no,rx_node;
	U16 ap_node,cc_node;
	U8 totalrx;

	txmod_no = SensorDefine.Mutual_TxModulationMode * SensorDefine.Mutual_ModulationTxSequence;
	rx_node = SensorDefine.Mutual_Passes * ADCNumTotal;
	totalrx = RxNum;

	ap_node = 0x0000;
	cc_node = 0x0000;

	for ( i = 0 ; i < txmod_no ; i++ )
	{
		if (SensorDefine.Mutual_VCom_Number_Per_Pass_Table[1] == 0)		// one pass
		{
			for ( m = 0 ; m < rx_node ; m++ )
			{
				if (SensorDefine.FirmwareRawOffsetPerPass[m] != 0xFF )
				{
					ap_node = (((i * totalrx) + SensorDefine.FirmwareRawOffsetPerPass[m]) - (2 * ADCNumTotal));
					cc_node = (i * rx_node + m);
					AP_MutualCC[ap_node] = MutualAutoCC[cc_node];
				}
			}

			for ( m = ADCNumTotal ; m < (ADCNumTotal + rx_node) ; m++ )
			{
				if (SensorDefine.FirmwareRawOffsetPerPass[m] != 0xFF )
				{
					ap_node = (((i * totalrx) + SensorDefine.FirmwareRawOffsetPerPass[m]) - (2 * ADCNumTotal));
					cc_node = (i * rx_node + m);
					AP_MutualCC[ap_node] = MutualAutoCC[cc_node];
				}
			}
		}
		else
		{
			for ( m = 0 ; m < rx_node ; m++ )
			{			
				if (SensorDefine.FirmwareRawOffsetPerPass[m] != 0xFF )
				{
					ap_node = (((i * totalrx) + SensorDefine.FirmwareRawOffsetPerPass[m]) - totalrx);
					cc_node = (i * rx_node + m);
					AP_MutualCC[ap_node] = MutualAutoCC[cc_node];
				}
			}
		}
	}


#if 0
	printf("SRC  \n\r");
	for ( i = 0 ; i < (txmod_no*rx_node) ; i++ )
	{
		printf("[%3d]:0x%2x, ",i,MutualAutoCC[i]);
		if ((i+1)%28 == 0)
			printf("\n\r");
	}
	printf("\n\r");


	printf("AP  \n\r");
	for ( i = 0 ; i < (txmod_no*rx_node) ; i++ )
	{
		printf("[%3d]:0x%2x, ",i,AP_MutualCC[i]);
		if ((i+1)%RxNum == 0)
			printf("\n\r");
	}
	printf("\n\r");
#endif
}


#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_W
void drv_scan_set_mutual_cc(void)		// no support 1 pass & 3 pass
{
	U16 i,j,k,m;
	U16 txmod_no,rx_node;
	U16 ap_node,cc_node;
	U8 totalrx,totaltx;

	txmod_no = SensorDefine.Mutual_TxModulationMode * SensorDefine.Mutual_ModulationTxSequence;
	rx_node = SensorDefine.Mutual_Passes * ADCNumTotal;
	totalrx = RxNum;
	totaltx = TxNum;

	for ( i = 0 ; i < txmod_no ; i++ )
	{
		for ( j = 0 ; j < totalrx ; j++ )
		{
			for ( k = 0 ; k < rx_node ; k++ )
			{
				if ( (SensorDefine.FirmwareRawOffsetPerPass[k] - totaltx)  == j )
				{
					if ( i < totaltx)
					{
						ap_node = (i*totalrx+j);
					}
					else
					{
						m = (totaltx-1);
						ap_node = (m*totalrx+j);
					}					
					cc_node = (i*rx_node+k);
					MutualAutoCC[cc_node] = AP_MutualCC[ap_node];
				}
			}
		}
	}


#if 0
	printf("WRITE SRC  \n\r");
	for ( i = 0 ; i < (txmod_no*rx_node) ; i++ )
	{
		printf("[%3d]:0x%2x, ",i,MutualAutoCC[i]);
		if ((i+1)%28 == 0)
			printf("\n\r");
	}
	printf("\n\r");


	printf("WRITE AP  \n\r");
	for ( i = 0 ; i < (txmod_no*rx_node) ; i++ )
	{
		printf("[%3d]:0x%2x, ",i,AP_MutualCC[i]);
		if ((i+1)%RxNum == 0)
			printf("\n\r");
	}
	printf("\n\r");
#endif
}
#endif	// -W
#endif	// -RW



void SelfModeReCalibrateCCToTargetRawData(void)
{
#if  !SHRINK_FLASH_SIZE__SELF_MUTUAL_RW

	U8 idx;
	U8 selfnum;

	selfnum = SelfActualDataNum;

	if (SelfNeddReTryDone == 0)	
	{
		drv_scan_read_self_cc();				// get AP_SelfCC[xx]
		for ( idx = 0 ; idx < selfnum; idx++ )
		{
			if ( (SelfRawAdjCC[idx] & 0x10) == 0x10 )
			{
				if (AP_SelfCC[idx] >= 131)					// cc over 3
				{
					AP_SelfCC[idx] -= 2;					// offset 2
					if ((AP_SelfCC[idx] - 2) < 129)
					{
						AP_SelfCC[idx] = 129;
						SelfRawAdjCC[idx] |= 0x80;			// done bit
					}
					SelfRawAdjCC[idx] &= (0xEF);			// mask re-k bit
				}
				else if ((AP_SelfCC[idx] == 0x82) || (AP_SelfCC[idx] == 0x81))		// cc = 1 or 2
				{
					SelfRawAdjCC[idx] &= 0xEF;				// mask re-k bit
					SelfRawAdjCC[idx] |= 0x80;				// done bit
				}
				else
				{
					SelfRawAdjCC[idx] |= 0x80;				// done bit
				}
			}
			else
			{
				SelfRawAdjCC[idx] |= 0x80;					// done bit
			}
		}
		SelfNeedReCalibrateCC = 0;
		drv_scan_set_self_cc();


		// decide re-try or not?
		for ( idx = 0 ; idx < selfnum; idx++ )
		{
			if ((SelfRawAdjCC[idx] & 0x80) == 0x80)
			{
				SelfNeddReTryDone = 1;
			}
			else
			{
				SelfNeddReTryDone = 0x00;
			}
		}

		if (SelfNeddReTryDone == 0)		// 0: need re-k
		{
			SelfModePowerOnFlag = 1;
		}
	}	

#endif		// -RW
}

#endif	// !PROTOCOL_V20


void _FEEDBACK_GUI_SELF_MUTUAL_MODE_CC(void)
{
#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_RW	
		if(KnlProtocolCtrl & bAP_ACTIVE_CC)
		{
		#if  !PROTOCOL_V20
			if((KnlGetCcMode == 0x80) || (KnlGetCcMode == 0x84))
			{
				drv_scan_read_mutual_cc();
			}
			else if(KnlGetCcMode == 0x82)
			{
				drv_scan_read_self_cc();
			}
		#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_W
			else if((KnlGetCcMode == 0x81) || (KnlGetCcMode == 0x85))
			{
				drv_scan_set_mutual_cc();
			}
			else if(KnlGetCcMode == 0x83)
			{
				drv_scan_set_self_cc();
			}
		#endif
		#endif	//!PROTOCOL_V20
			KnlProtocolCtrl &= ~bAP_ACTIVE_CC; 
			KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
		}
		else
		{
			//pull INT low
			P1OUT &= 0xFE;
		}
#endif
}


// ####### M A I N.c Re-direct function #######
void _SELF_MUTUAL_CHECK_TOUCH_STATUS(U8 m_flag, U8 s_flag)
{
#if SELF_ASSISTANCE
	U8 SM_index;

	SM_index = (((m_flag&0x0f)<<4) | (s_flag&0x0f));		// calculate SELF & Mutual mode touch status
	switch(SM_index)
	{
		case 0x10:
		case 0x40:
//		case 0x01:
//		case 0x41:
//			SelfBaseAbnormalCounter++;
			MutualBaseAbnormalCounter++;
			break;

		default:			// trace, follow up
			break;
	}

//printf("m_flag:%2d  node:%2d  Fr:%2d  \n\r",m_flag,MutuPOFNodeCounter,MutualPOFFrameCounter);

	if (MutualBaseAbnormalCounter > 5)
	{
		MutualModePowerOnFlag = 1;		// MUTUAL
		MutualBaseAbnormalCounter = 0;
	}
#else
	if (MutualOnlyPofReCalibrateBaselineFrame > 0)
	{
		MutualOnlyPofReCalibrateBaselineFrame--;
		MutualModePowerOnFlag = 1;		// MUTUAL -- force to re-calibrate baseline
	}
#endif
}


U8 _MUTUAL_GET_TOUCH_STATUS(void)
{
	U8 mflag;
	knl_get_mutual_pre_delta( (&MutualRawDataBuffer[MutualReady][0]),(&BaseFrame.Mbase.Mutual[0]),(&SigFrame.Msig.Mutual[0]),MutualActualDataNum );
	mflag = knl_de_common_mode_noise_process((&SigFrame.Msig.Mutual[0]),FwSetting.MutualBaseTraceUpperBound,FwSetting.MutualBaseTraceLowerBound,FwSetting.MutualFingerTHR);

	return 	mflag;
}

U8 _SELF___GET_TOUCH_STATUS(void)
{
#if SELF_ASSISTANCE
	U8 stx_flag,srx_flag,sflag;

	sflag = 0x00;
	stx_flag = drv_scan_get_self_signal((&SelfRawDataBuffer[SelfReady][0]),(&BaseFrame.Sbase.Self[0]),(&SigFrame.Ssig.Self[0]),FwSetting.Self_TxPeakBoundary,TxNum);
	srx_flag = drv_scan_get_self_signal((&SelfRawDataBuffer[SelfReady][TxNum]),(&BaseFrame.Sbase.Self[TxNum]),(&SigFrame.Ssig.Self[TxNum]),FwSetting.Self_RxPeakBoundary,(0x80|RxNum));
	sflag = (stx_flag & srx_flag);

	if (POF_SelfRecover > 0)
	{
		POF_SelfRecover--;
		SelfModePowerOnFlag = 1;
		sflag = 0;
	}//Power on finger function
		

	return sflag;
#else
	return 1;
#endif
}


void _SELF___GET_PEAK_INFO(U8 s_flag)
{
#if SELF_MODE_REPORT_FINGER
	U8 idx,tx_pknum,rx_pknum,pk_info;

	pk_info = 0xff;
	
	for ( idx = 0 ; idx < KNL_MAX_FINGER_NUM ; idx++ )
	{
		SelfPeakTx[idx] = 0xff;
		SelfPeakRx[idx] = 0xff;
	}

	if (s_flag)
	{
		tx_pknum = 0xff;
		rx_pknum = 0xff;
		tx_pknum = SelfModeSearchPeak((&SigFrame.Ssig.Self[0]),FwSetting.Self_TxPeakBoundary,(&SelfPeakTx[0]),(&SelfJudgePalmStatus),TxNum,TX_IDX);
		rx_pknum = SelfModeSearchPeak((&SigFrame.Ssig.Self[TxNum]),FwSetting.Self_RxPeakBoundary,(&SelfPeakRx[0]),(&SelfJudgePalmStatus),RxNum,RX_IDX);
		pk_info = ( (tx_pknum <<4) + rx_pknum);
	}

	if (pk_info == 0x11)		// one finger except key
	{
		if ((SensorDefine.Key_Types == MUTUAL_RIGHT_KEY) && (SelfPeakRx[0] == (RxNum-1)))
		{
			DoSelfReport = 0;
		}
		else if ((SensorDefine.Key_Types == MUTUAL_DOWN_KEY) && (SelfPeakTx[0] == (TxNum-1)))
		{
			DoSelfReport = 0;
		}
		else
		{
//			if (SelfJudgePalmStatus == 0x11)	// if "1", is Palm.
//			{
//				DoSelfReport = 0;		// Mutual Report
//			}
//			else
			{		
				DoSelfReport = 1;		// SELF Report
			}
		}
	}
	else
	{
		DoSelfReport = 0;
		SelfJudgePalmStatus = 0;
	}	
#else
	DoSelfReport = 0;
#endif
}

void _TOUCH_KEY_CHECK(void)
{
#if !SHRINK_FLASH_SIZE__TOUCHKEY
	Classify_Touch_Key(SensorDefine.Key_Types);
#endif
}


void _SELF_MUTUAL_AUTO_CALIBRATION(void)
{
	AutoSelfModeCalibration(HwSetting.SelfTargetRawMin,HwSetting.SelfTargetRawMax,HwSetting.SelfAutoCCStart);
	AutoMutualModeCalibration_ByNode(HwSetting.MutualTargetRaw,HwSetting.MutualAutoCCStart);
}








// #####################################
void STARTING_TRIGGER_SCAN_NORMAL(void)
{
#if !SHRINK_FLASH_SIZE__AUTO_FREQ
	Bios_ScanMode = NL_MODE;
	drv_scan_trigger_mutual();
#else
	#if SELF_ASSISTANCE
		Bios_ScanMode = SELF_MODE;
		drv_scan_trigger_self(SELF_ALL);
	#else
		Bios_ScanMode = MUTUAL_MODE;
		drv_scan_trigger_mutual();
	#endif
#endif
}

// ####### M A I N.c Re-direct function #######

