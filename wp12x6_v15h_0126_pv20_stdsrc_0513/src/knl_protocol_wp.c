/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"


sfrb(IF_DMA_ADDR_L       ,__IF_DMA_ADDR_L);
sfrb(IF_DMA_ADDR_H       ,__IF_DMA_ADDR_H);

#if PROTOCOL_V20

#define HAS_USER_PROTOCOL 1
#define MSG_WRITE_SLOT 		0x08
#define MSG_READ_SLOT 		0x09
#define MSG_EXECUTE_ROPS 	0x05
#define MSG_REQ_ACK			0x06

#define GET_U8P_WORD(p) (((U16)(*((p)+1))<<8) | (U16)(*(p)))		// Get U16 from a U8 pointer. This method avoid MCU un-aligned access problem.
#define make_INTO_high() {P1OUT |= 0x01;}
#define make_INTO_low()  {P1OUT &= 0xFE;}


/*
typedef struct
{
	U16 skip_i2c_first_ch	: 1;	//1=Skip the 1st char of the I2C write.
	U16 redirect_i2cw		: 1;	//0=Put the I2C write bytes to default I2cBuffer[]. 1=protocol defined.
	U16 user_protocol		: 1;
	U16 debug_mode			: 1;	//0=Frame free run, 1=Host control frame run.
	U16 end_of_fetch		: 1;
	U16 link_ok				: 1;
	U16 req_ack				: 1;
} tProtocolState;
*/

enum
{
	SLOT_DRAM=0,
	SLOT_MRAW,
	SLOT_MBASE,
	SLOT_MDELTA,
	SLOT_MCC,
	SLOT_SRAW,
	SLOT_SBASE,
	SLOT_SDELTA,
	SLOT_SCC,
	SLOT_DEBUG_VARS,
	SLOT_REQUEST,
	SLOT_STATE,
	/* Add new define here. */
	SLOT_PARAM_ADDRESS,
	SLOT_INDIRECT,

	SLOT_END	// SLOT_END is ending mark, must be at final position.
};


volatile tProtocolState ProtocolStateIs;
volatile tVirtualRegister HostVirtualRegister;
volatile tVirtualRegister CommandVirtualRegister;

S16 DebugDumpVar[16];
volatile U8 *pI2cBuffer;	// Cannot has init value, otherwise will make large BIN file.
volatile U8 LastCommand;
U16 SlotPointer[SLOT_END - SLOT_DRAM];
const U8 PasswordKey[]={0x06, 0xff, 0xa5, 0xb1, 0xc6, 0x54};

#endif

void knl_protocol_init(void)
{
#if USER_PROTOCOL
	#if PROTOCOL_V20
	ProtocolStateIs.user_protocol = TRUE;
	#else
	KnlProtocol.Customer = USER;
	#endif

#else
	#if PROTOCOL_V20
	ProtocolStateIs.user_protocol = FALSE;
	#else
	KnlProtocol.Customer = WINPOWER;
	#endif    
#endif
	KnlProtocol.ProjectID = 0x00;
	KnlProtocol.CustomerDefault = WINPOWER;

	KnlTpHeader.ProtVer = HwSetting.ProtVer;					// byte 1
	KnlTpHeader.Id = HwSetting.Id;								// byte 2
	KnlTpHeader.HwVer = HwSetting.HwVer;						// byte 3
	KnlTpHeader.FwVer = HwSetting.FwVer;						// byte 4
	KnlTpHeader.SerialNo = HwSetting.SerialNum;					// byte 5,6,7,8
	KnlTpHeader.Vid  = HwSetting.Vid;							// byte 9,10
	KnlTpHeader.Pid = HwSetting.Pid;							// byte 11,12
#if n_T_m_R_MUTUAL_ONLY
	#if (N_DRIVING_T == 1)		//	1TmR
		KnlTpHeader.XRx = (SensorDefine.Mutual_RxNumbers>>1);		// byte 13
		KnlTpHeader.YTx = (SensorDefine.Mutual_TxNumbers<<1);		// byte 14
	#elif (M_SENSING_R == 1)	//	nT1R
		KnlTpHeader.XRx = (SensorDefine.Mutual_RxNumbers<<1);		// byte 13
		KnlTpHeader.YTx = (SensorDefine.Mutual_TxNumbers>>1);		// byte 14
	#else
		KnlTpHeader.XRx = SensorDefine.Mutual_RxNumbers;			// byte 13
		KnlTpHeader.YTx = SensorDefine.Mutual_TxNumbers;			// byte 14	
	#endif
#else
	KnlTpHeader.XRx = SensorDefine.Mutual_RxNumbers;			// byte 13
	KnlTpHeader.YTx = SensorDefine.Mutual_TxNumbers;			// byte 14
#endif
	if(FwSetting.PosMappingFunction & bXY_REVERSE)
	{
		KnlTpHeader.ResX = FwSetting.TxTargetResolution;		// byte 15,16
		KnlTpHeader.ResY = FwSetting.RxTargetResolution;		// byte 17,18
	}
	else
	{
		KnlTpHeader.ResX = FwSetting.RxTargetResolution;		// byte 15,16
		KnlTpHeader.ResY = FwSetting.TxTargetResolution;		// byte 17,18
	}
	KnlTpHeader.SupportFingers = KNL_MAX_FINGER_NUM;								// byte 19
	KnlTpHeader.KeyNum = SensorDefine.Key_Numbers;				// byte 20
#if !PROTOCOL_V20_REPORT_20
		//--- Report format keep 4-bytes point format and 2-bytes key format.
	KnlTpHeader.MaxRptLength = 60;								// byte 21	
	KnlTpHeader.Cap1 = SensorDefine.Key_Types & 0x07;			// byte 22
#else		// Report format changed to 6-bytes point format and 1-byte key format.
	KnlTpHeader.MaxRptLength = 64;								// byte 21
	//KnlTpHeader.Cap1 = 0x40 | (SensorDefine.Key_Types & 0x07);	// byte 22, b6:1=new point&key format.. 0=old point&key format.
	KnlTpHeader.Cap1 =  (SensorDefine.Key_Types & 0x07);	// byte 22, b6:1=new point&key format.. 0=old point&key format.
#endif
	KnlTpHeader.Cap2 = 0xC4;									// byte 23

	KnlPreBridgeDone = 0x01;
#if !PROTOCOL_V20
	KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
	KnlReportSize = 0x00;
#else
	KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
	HostVirtualRegister = (tVirtualRegister) {0};
	CommandVirtualRegister = (tVirtualRegister) {0};
	ProtocolStateIs = (tProtocolState) {0};
	ProtocolStateIs.end_of_fetch = TRUE;

	SlotPointer[SLOT_MBASE] = (U16)&BaseFrame.Mbase.Mutual[0];
	SlotPointer[SLOT_MDELTA] = (U16)&SigFrame.Msig.Mutual[0];
	SlotPointer[SLOT_MCC] = (U16)&MutualAutoCC[0];	//AP_MutualCC[0];

	SlotPointer[SLOT_SBASE] = (U16)&BaseFrame.Sbase.Self[0];
	SlotPointer[SLOT_SDELTA] = (U16)&SigFrame.Ssig.Self[0];
	SlotPointer[SLOT_SCC] = (U16)&SelfAutoCC[0];	//AP_SelfCC[0];

	SlotPointer[SLOT_DEBUG_VARS] = (U16)DebugDumpVar;
	SlotPointer[SLOT_REQUEST] = (U16)&HostVirtualRegister;
	SlotPointer[SLOT_PARAM_ADDRESS] = (U16)&ParamAddresses;
	SlotPointer[SLOT_INDIRECT] = (U16)SlotTempBuffer;

#endif

	KnlPreOpMode[0] = 0x00;
	KnlPreOpMode[1] = 0x08;
	KnlPreOpMode[2] = 0x00;

	KnlOpMode[0] = KnlPreOpMode[0];
	KnlOpMode[1] = KnlPreOpMode[1];
	KnlOpMode[2] = KnlPreOpMode[2];

	KnlDriverOffset = 0x0000;


}


void knl_protocol_update_opmode(void)
{
	KnlOpMode[0] = KnlPreOpMode[0];
	KnlOpMode[1] = KnlPreOpMode[1];
	KnlOpMode[2] = KnlPreOpMode[2];

	if((KnlOpMode[1] & 0xF0) != 0x00)
	{
		KnlProtocolCtrl |= bINTR_TRIG;
		KnlPreBridgeDone = 0x00; 
	}

    if((KnlOpMode[1] & 0x04) == 0x04)
    {
        KnlOpMode[1] &= ~0x02;
        ScanStatus |= bOPEN_SHORT_TEST;
    } 
}

void knl_protocol_report_out(void)
{
#if	!PROTOCOL_V20_REPORT_20
	I2cReportBuff[0x00] = (KnlReportSize + 0x01);
#endif

	P1OUT &= 0xFE;

#if PROTOCOL_V20
	ProtocolStateIs.end_of_fetch = FALSE;
#endif
}

#if !PROTOCOL_V20_REPORT_20

void knl_protocol_set_report(void)
{
    U8 i,data_count,z_force,x_idx,y_idx;
	U16 bit_n;

    z_force = 0xC0;
	data_count = 0x01;
    
    KnlReportSize = 0x00;
    
    if(FwSetting.PosMappingFunction  & bXY_REVERSE)
    {
        x_idx = TX_IDX;
        y_idx = RX_IDX;
    }
    else
    {
        x_idx = RX_IDX;
        y_idx = TX_IDX;
    }

    if(PointOutput.PointNum != 0x00)
    {
        for(i=0; i<KNL_MAX_FINGER_NUM; i++)
        {   
            bit_n = (0x0001 << i);

            if(PointOutput.TouchInfo & bit_n)
            {
			  	I2cReportBuff[data_count] = (((i&0x0F) << 0x03) | (PointOutput.Position[x_idx][i] >> 0x08) | 0x80);
				data_count++;
				I2cReportBuff[data_count] = ((U8)(PointOutput.Position[x_idx][i] & 0xFF));
				data_count++;
				I2cReportBuff[data_count] = ((z_force & 0xE0) | (PointOutput.Position[y_idx][i] >> 0x08));
				data_count++;
				I2cReportBuff[data_count] = ((U8)(PointOutput.Position[y_idx][i] & 0xFF));
				data_count++;	
			}
		}
		
		KnlReportSize = (PointOutput.PointNum << 0x02); 
	}
	
    I2cReportBuff[data_count] = 0x20;
    data_count++;
    I2cReportBuff[data_count] = KeyPress;
    data_count++; 
    KnlReportSize += 0x02;
    
}
#else

U8 Report_size_tobe_read=0;
void knl_protocol_set_report(void)
{
    U8 x_idx,y_idx;
    U8 i;
    U8 *rptr;
	U16 bit_n;

    //z_force = 0xC0;
	//data_count = 0x01;

    //KnlReportSize = 0x00;

    if(FwSetting.PosMappingFunction  & bXY_REVERSE)
    {
        x_idx = TX_IDX;
        y_idx = RX_IDX;
    }
    else
    {
        x_idx = RX_IDX;
        y_idx = TX_IDX;
    }

	rptr = (void *)&I2cReportBuff[1];
    if((PointOutput.PointNum>0))
    {
		bit_n = 0x0001;
        for(i=0; i<KNL_MAX_FINGER_NUM; i++)
        {
        	if(PointOutput.TouchInfo & bit_n)
        	{
				rptr[0] = 0x80 | i ;	// down=1. Set hover bit here.
				rptr[1] = PointOutput.Position[x_idx][i] & 0xff;	// Low byte X position.
				rptr[2] = PointOutput.Position[x_idx][i] >> 8;	// High byte X position.
				rptr[3] = PointOutput.Position[y_idx][i] & 0xff;	// Low byte X position.
				rptr[4] = PointOutput.Position[y_idx][i] >> 8;	// High byte X position.
				rptr[5] = 0x30;									// Area. For valid finger, set it to none-zero.
				rptr += 6;
				//KnlReportSize +=6;
        	}
            bit_n <<= 1;
		}
	}

    if(KeyPress)	// Don't report BLANK key if no key pressed.
    {
    	//*rptr++ = 0x20;
    	*rptr++ = 0x20 | (KeyPress & 0x0f);
    	//KnlReportSize+=1;
    }
    //KnlReportSize = (U16)rptr - (U16)I2cReportBuff;	// This is report size. Minimum = 1.
    //KnlReportSize-- ;
    I2cReportBuff[0] = (U16)rptr - (U16)I2cReportBuff;
    Report_size_tobe_read = I2cReportBuff[0];
}
#endif

#if !PROTOCOL_V20
void knl_protocol_ap_read_data(void)
{
	#if !SHRINK_FLASH_SIZE__TOUCHKEY
	U8 m;
	U8 keynum;
	keynum = SensorDefine.Key_Numbers;
	#endif



	while(!IsEndOfScan()) {}; ////////////

	

	switch(KnlOpMode[1] & 0xC0)
	{
	case I2C_SELF_RAW:
		KnlDataPtr0 = (U16 *)(SelfRawDataBuffer[SelfReady]);
	break;

	case I2C_SELF_BASE:
		KnlDataPtr0 = (U16 *)(&BaseFrame.Sbase.Self[0]);
	break;

	case I2C_SELF_DELTA:
		KnlDataPtr0 = (U16 *)(&SigFrame.Ssig.Self[0]);
	#if DBG_NL
		KnlDataPtr0 = (U16 *)(&PRE_NL_FREQ_LEVEL[0]);
		KnlDataPtr0[MAX_NL_FREQ_NUM] = (NOW_HoppingBestFrequencyIndex+1)*100;
		KnlDataPtr0[MAX_NL_FREQ_NUM+1] = NLHopNoReport *100;
		KnlDataPtr0[MAX_NL_FREQ_NUM+2] = HoppingNowIndex * 100;
	#endif
	break;

	default:
	break;
	}


	switch(KnlOpMode[1] & 0x30)
	{	  	  
	case I2C_MUTUAL_RAW:
		KnlDataPtr1 = (U16 *)(MutualRawDataBuffer[MutualReady]);
	break;

	case I2C_MUTUAL_BASE:
		KnlDataPtr1 = (U16 *)(&BaseFrame.Mbase.Mutual[0]) ;

	#if 0 // !SHRINK_FLASH_SIZE__TOUCHKEY
		if ((SensorDefine.Key_Types == MUTUAL_RIGHT_KEY) || (SensorDefine.Key_Types == MUTUAL_DOWN_KEY))
		{
			for ( m = 0 ; m < keynum ; m++ )
			{
				*(KnlDataPtr1 + ((U16)KeyNode[m]) ) = TouchKeyBaseline[m];
			}
		}
	#endif
	break;

	case I2C_MUTUAL_DELTA:
		KnlDataPtr1 = (U16 *)(U16)(&SigFrame.Msig.Mutual[0]);

	#if !SHRINK_FLASH_SIZE__TOUCHKEY
		if ((SensorDefine.Key_Types == MUTUAL_RIGHT_KEY) || (SensorDefine.Key_Types == MUTUAL_DOWN_KEY))
		{
			for ( m = 0 ; m < keynum ; m++ )
			{
				*(KnlDataPtr1 + ((U16)KeyNode[m]) ) = TouchKeyDelta[m];
			}
		}
	#endif
	#if DBG_NL
		KnlDataPtr1[0] = NOW_HoppingBestFrequencyIndex*100;
	#endif
//		KnlDataPtr1[0] = NOW_HoppingBestFrequencyIndex*100;		// debug: show best freq
//		KnlDataPtr1[1] = HoppingNowIndex*100;					// debug: show best freq
	#if GESTURE_REMOTE_CONTROL
		KnlDataPtr1[1] = (U16)GestureRecognized;
	#endif
		
	break;


	default:
	break;
	}
}
#else
//-------------------------------------------------------------------------------------------------------------
void knl_protocol_ap_read_data(void)
{
	SlotPointer[SLOT_MRAW] = (U16)MutualRawDataBuffer[MutualReady];
	SlotPointer[SLOT_SRAW] = (U16)SelfRawDataBuffer[SelfReady];
}
#endif

#if !PROTOCOL_V20

void ctm_user_protocol_DMA_ptr(void)
{
   	U16 j;

	if(KnlProtocolCtrl & bNOTLOAD_RCV_PTR)  // no point
	{
		KnlProtocolCtrl &= ~bNOTLOAD_RCV_PTR;
	}
	else
	{
		j = (U16)(I2cDmaPtr-0x0400);
		IF_DMA_ADDR_L = (U8)(j & 0x00FF);
		IF_DMA_ADDR_H = (U8)(j >> 0x08);
	}
}

void knl_protocol_WP(void)
{
	#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_W
	U16 i;
	#endif
	U16 j;


	switch(I2cDataBuff[0])
	{
		case MSG_GET_REPORTS:
			I2cDmaPtr = (U8 *)(&I2cReportBuff[0]);
			P1OUT |= INT_HIGH;
			//KnlProtocolCtrl |= bFORBID_UPDATE_REPORT;
		break;

		case MSG_SET_MODE:
			KnlPreOpMode[0] = I2cDataBuff[1];
			KnlPreOpMode[1] = I2cDataBuff[2];
			KnlPreOpMode[2] = 0x00;	//I2cDataBuff[3];
			KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
			KnlPreBridgeDone = 0x01;
		break;

		case MSG_GET_HEADER:
			I2cDmaPtr = (U8 *)(&KnlTpHeader.ProtVer);

			if((I2cDataBuff[1] > 0x00) && (I2cDataIdx>1))
				KnlProtocolCtrl |= bDRIVER_ALIVE;
		break;

		case MSG_GET_STATUS:
			I2cDmaPtr = (U8 *)(&KnlOpMode[0]);
		break;

		case I2C_READ_RAW_SELF:
			I2cDmaPtr = (U8 *)(KnlDataPtr0);
		break;

		case I2C_READ_RAW_MUTUAL:
			I2cDmaPtr = (U8 *)(KnlDataPtr1);
		break;

		case MSG_SET_ENG_Mode:
			KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
		break;

		case MSG_Sleep_In:
			com_service_enter_sleep();
		break;

		case MSG_GET_PROTOCOL:
			I2cDmaPtr = (U8 *)(&KnlProtocol.Customer);
		break;

		case MSG_ENTER_SCAN_MODE:
			KnlGetCcMode = I2cDataBuff[0x01];
			if((KnlGetCcMode == 0x80) || (KnlGetCcMode == 0x82) || (KnlGetCcMode == 0x84))
			{
				KnlProtocolCtrl |= bAP_ACTIVE_CC;
			}
			else
			{
				KnlProtocolCtrl &= ~bBRIDGE_I2C_DONE;
				KnlPreBridgeDone = 0x00;
			}
			KnlProtocolCtrl |= bAP_ACTIVE_FLAG;
		break;

		case MSG_READ_CCvalue:
			if((KnlGetCcMode == 0x80) || (KnlGetCcMode == 0x84))    //mutual
			{
				I2cDmaPtr = (U8 *)(&AP_MutualCC[0]);
			}
			else if(KnlGetCcMode == 0x82)   //self
			{
				I2cDmaPtr = (U8 *)(&AP_SelfCC[0]);
			}
			KnlProtocolCtrl &= ~bAP_ACTIVE_FLAG;
		break;

	#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_W
		case MSG_WRITE_CCvalue:
			KnlProtocolCtrl &= ~bBRIDGE_I2C_DONE;
			KnlPreBridgeDone = 0x00;

			KnlGetCcPacketLength = I2cDataBuff[0x01];

			KnlGetCcByteIndex = (KnlGetCcPacketIndex*(I2C_BUFF_LENGTH-0x02));

			if(KnlGetCcPacketLength < (I2C_BUFF_LENGTH-0x02))
			{
				KnlGetCcPacketIndex = 0x00;
				KnlGetCcPacketSize = KnlGetCcPacketLength;
			}
			else
			{
				KnlGetCcPacketIndex++;
				KnlGetCcPacketSize = (I2C_BUFF_LENGTH-0x02);
			}

			if(KnlGetCcPacketLength > 0x00)
			{
				if((KnlGetCcMode == 0x81) || (KnlGetCcMode == 0x85))
				{
					for(i=0x00; i<KnlGetCcPacketSize; i++)
					{
						AP_MutualCC[KnlGetCcByteIndex+i] = I2cDataBuff[i+0x02];
					}
				}
				else if(KnlGetCcMode == 0x83)
				{
					for(i=0x00; i<KnlGetCcPacketSize; i++)
					{
						AP_SelfCC[KnlGetCcByteIndex+i] = I2cDataBuff[i+0x02];
					}
				}
			}

			if(KnlGetCcPacketIndex == 0x00)
			{
				KnlProtocolCtrl |= (bAP_ACTIVE_CC+bBRIDGE_I2C_DONE);
				KnlPreBridgeDone = 0x01;
				KnlProtocolCtrl &= ~bAP_ACTIVE_FLAG;
			}
		break;
	#endif

		default:
			KnlProtocolCtrl |= bNOTLOAD_RCV_PTR;
		break;
	}

	if(KnlProtocolCtrl & bNOTLOAD_RCV_PTR)
	{
		KnlProtocolCtrl &= ~bNOTLOAD_RCV_PTR;
	}
	else
	{
		if(KnlProtocolCtrl & bDRIVER_ALIVE)
		{
			KnlDriverOffset = (I2cDataBuff[1] << 0x03);
		}
		else
		{
			KnlDriverOffset = 0x0000;
		}

		j = (U16)(I2cDmaPtr-0x0400);
		j += KnlDriverOffset;
		IF_DMA_ADDR_L = (U8)(j & 0x00FF);
		IF_DMA_ADDR_H = (U8)(j >> 0x08);
	}
}
#else

#define SLOTWR_WRITE_BITS 	0x00
#define SLOTWR_CLR_BITS 	0x40
#define SLOTWR_SET_BITS 	0x80
#define SLOTWR_TOGGLE_BITS 	0xC0
//-------------------------------------------------------------------------------------------------------------
void knl_protocol_WP(void)
{
	U8 *mem_ptr;
	U16 offset;
	U8 register slot_id;

	//--- Check if it is password to force ACK and switch to WP protocol.
	if((I2cDataBuff[0]==MSG_REQ_ACK))
	{
	#if HAS_USER_PROTOCOL
		if(I2cDataIdx==sizeof(PasswordKey))
		{
			int x;
			for(x=0; x<sizeof(PasswordKey); x++)
			{
				if(I2cDataBuff[x] != PasswordKey[x]) break;	// If any char un-match, then break the loop
			}
			if(x>=sizeof(PasswordKey))	// If x bigger or equal to the size, means all char match.
			{		//--- Switch back to WP protocol as well as request a INTO low.
				make_INTO_high();		// No matter original INTO status, force it to high.
				ProtocolStateIs.req_ack = TRUE;
				ProtocolStateIs.user_protocol = FALSE;
				//putchar('1');
			}
		}
	#else
		make_INTO_high();		// No matter original INTO status, force it to high.
		ProtocolStateIs.req_ack = TRUE;
		//putchar('Q');
	#endif
	}


#if HAS_USER_PROTOCOL
	if(ProtocolStateIs.user_protocol)
	{
		//--- Call User protocol.
		ctm_user_protocol_rcv_write(I2cDataBuff, I2cDataIdx);
	}
	else
#endif
	{
		if(ProtocolStateIs.redirect_i2cw)
		{
			ProtocolStateIs.redirect_i2cw=FALSE;
			pI2cBuffer = I2cDataBuff;	// Restore pI2cBuffer.
			if(LastCommand==MSG_WRITE_SLOT)
			{
				slot_id = I2cDataBuff[1] & 0x3f;
				if(slot_id==SLOT_MCC)  ProtocolStateIs.writeback_mcc = TRUE;
				else if(slot_id==SLOT_SCC)  ProtocolStateIs.writeback_scc = TRUE;
			}
			return;
		}
		LastCommand = I2cDataBuff[0];
		mem_ptr = (U8 *)0xffff;
		switch(I2cDataBuff[0])
		{
			case MSG_GET_REPORTS:
				mem_ptr = (void *)I2cReportBuff;
				//KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
				break;
			case MSG_GET_HEADER:
				mem_ptr = (U8 *)&KnlTpHeader.ProtVer;
				//HostVirtualRegister.debug_mode = TRUE;
				break;
			case MSG_WRITE_SLOT:
				mem_ptr = (U8 *)SlotPointer[(I2cDataBuff[1]&0x3f)];
				offset  = GET_U8P_WORD(&I2cDataBuff[2]);	//[2..3] is 16 bits offset.
				mem_ptr  = &mem_ptr[offset];
				if(I2cDataIdx>4)
				{
					int x;
					int len=I2cDataIdx-4;
					U8 *src = &I2cDataBuff[4];
					switch(I2cDataBuff[1] & 0xC0)
					{
						case SLOTWR_WRITE_BITS:		// 0=Overwrite
							memcpy(mem_ptr, src, len);
							//putchar('B');
							break;
						case SLOTWR_CLR_BITS:		// AND
							for(x=0; x<len; x++) *mem_ptr++ &= ~(*src++);
							break;
						case SLOTWR_SET_BITS:		// OR
							for(x=0; x<len; x++) *mem_ptr++ |= (*src++);
							break;
						default:	// SLOTWR_TOGGLE_BITS
							for(x=0; x<len; x++) *mem_ptr++ ^= (*src++);
							break;
					}
				}
				else
				{
					if((I2cDataBuff[1]==SLOT_INDIRECT) && (offset >= 0xb000))
					{
						memcpy((void *)SlotTempBuffer, (void *)offset, SLOT_TEMP_BUF_SIZE);
					}
					else
					{
						pI2cBuffer = mem_ptr;	// Redirect next I2Cwrite to this address.
						ProtocolStateIs.redirect_i2cw = TRUE;
						ProtocolStateIs.skip_i2c_first_ch = TRUE;	// Let the 1st byte of next WRITE skipped.
					}
				}
				break;

			case MSG_READ_SLOT:
				offset  = GET_U8P_WORD(&I2cDataBuff[2]);	//[2..3] is 16 bits offset.
				mem_ptr = (U8 *)SlotPointer[I2cDataBuff[1]];
				mem_ptr = &mem_ptr[offset];
					//--- While reading debug data, always disable IDLE mode.
				FRAME_Counter = 0;		// never enter IDLE mode
				IDLE_MODE_STATUS = 0;	// never enter IDLE mode
				break;
			default:
				//KnlProtocolCtrl |= bNOTLOAD_RCV_PTR;
				break;
		}
		if((U16)mem_ptr!=0xffff)
		{
			mem_ptr -= 0x400;	// mapping to HW IFDMA range address. After mapping, valid range 0~0x5FFF.
			SetI2cReadDMA(mem_ptr);
		}
	}
}
//-------------------------------------------------------------------------------------------------------------
void I2cReadStopProcess(int rd_len)	// Call by ISR.
{
#if HAS_USER_PROTOCOL
	if(ProtocolStateIs.user_protocol)
	{
			//--- Call User protocol.
		ctm_user_protocol_rcv_read(rd_len);
	}
	else
#endif
	{
		if((LastCommand==MSG_GET_REPORTS) && (rd_len>=Report_size_tobe_read))	// if rd_len > report size, means complete of reading report.
		{
			ProtocolStateIs.end_of_fetch = TRUE;
			make_INTO_high();
		}
		else
		{
			if(LastCommand==MSG_GET_HEADER)	ProtocolStateIs.link_ok = TRUE;
		}
	}
}
//-------------------------------------------------------------------------------------------------------------

void ExecuteHostRequest(void)
{
	CommandVirtualRegister = HostVirtualRegister;
	/*
	if(ProtocolStateIs.writeback_mcc)
	{
		ProtocolStateIs.writeback_mcc = FALSE;
		//drv_scan_set_mutual_cc();
	}
	if(ProtocolStateIs.writeback_scc)
	{
		ProtocolStateIs.writeback_scc = FALSE;
		//drv_scan_set_self_cc();
	}
	*/
}

#endif
//-------------------------------------------------------------------------------------------------------------

// ####### M A I N.c Re-direct function #######
#if !PROTOCOL_V20
void _TRIGGER_INTR(void)
{
	if(KnlProtocolCtrl & bINTR_TRIG)
	{
		KnlProtocolCtrl &= ~bINTR_TRIG;
		knl_protocol_report_out();		//INTO LOW
	}
}
#else
void _TRIGGER_INTR(void)
{
	while ((CommandVirtualRegister.debug_mode) && (!IsEndOfScan()))
	{}

	if((ProtocolStateIs.link_ok && ((KnlProtocolCtrl & bINTR_TRIG) || CommandVirtualRegister.debug_mode)) || ProtocolStateIs.req_ack)
	{
		ProtocolStateIs.req_ack = FALSE;
		KnlProtocolCtrl &= ~bINTR_TRIG;
		knl_protocol_report_out();		//INTO LOW
	}
}
#endif

void _WINPOWER_GUI_PROCESSING(void)
{
	knl_protocol_update_opmode();
	knl_protocol_ap_read_data();

	if(((KnlOpMode[1] & 0xF0) != 0x00) || (KnlProtocolCtrl & bAP_ACTIVE_CC))
	{
		FRAME_Counter = 0;		// never enter IDLE mode
		IDLE_MODE_STATUS = 0;	// never enter IDLE mode

#if !SHRINK_FLASH_SIZE__SELF_MUTUAL_RW
		if(KnlProtocolCtrl & bAP_ACTIVE_CC)
		{
		#if !PROTOCOL_V20
				if((KnlGetCcMode == 0x80) || (KnlGetCcMode == 0x84))
				{
					drv_scan_read_mutual_cc();
					KnlProtocolCtrl |= bINTR_TRIG;
				}
				else if(KnlGetCcMode == 0x82)
				{
					drv_scan_read_self_cc();
					KnlProtocolCtrl |= bINTR_TRIG;
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
				KnlProtocolCtrl &= ~bAP_ACTIVE_CC;
		#endif //PROTOCOL_V20
		}
#endif
		if((KnlOpMode[1] & 0xF0) != 0x00)
		{
			KnlProtocolCtrl |= bINTR_TRIG;
		}               
	}
	else
	{
		if ( ((PreOutPointNum != 0x00) || (PointOutput.PointNum != 0x00) || (KeyPress != 0x00) || (PreKeyPress != 0x00)) && (NLHopNoReport == 0x00) )	// Hopping
		//if ( ((PreOutPointNum != 0x00) || (PointOutput.PointNum != 0x00) || (KeyPress != 0x00) || (PreKeyPress != 0x00)) )
		{
			KnlProtocolCtrl |= bINTR_TRIG;
		}
	}

#if !PROTOCOL_V20
	if(KnlPreBridgeDone == 0x00)
	{
		KnlProtocolCtrl &= ~bBRIDGE_I2C_DONE;
	}
	else
	{
		KnlProtocolCtrl |= bBRIDGE_I2C_DONE;
	}
#endif
}


void _COORD_XY_REPORT_TO_HOST(void)
{
	#if PROTOCOL_V20
	if (ProtocolStateIs.user_protocol == FALSE)
	#else
	if(KnlProtocol.Customer == WINPOWER)
	#endif
	{
		if(P1OUT & 0x01)		// wait INTR high
		{
			knl_protocol_set_report();
		}
	}

	#if PROTOCOL_V20
	if (ProtocolStateIs.user_protocol == TRUE)
	#else
	if(KnlProtocol.Customer == USER)
	#endif   
    {
	    if(P1OUT & 0x01)
	   	{
			ctm_user_protocol_set_report();
	    }
            
        if(I2cSendEnable)
        {
            KnlProtocolCtrl |= bINTR_TRIG;
        }
    }
}


void _SAVE_FRAME_INFO(U8 point_num)
{
	PrePointNum = point_num;
	PreKeyPress = KeyPress;
	PreOutPointNum = PointOutput.PointNum;
	PointList.LegalFlag = 0x0000;
	PointList.CurrSetFlag[0] = 0x0000; 
	PointList.CurrSetFlag[1] = 0x0000;
}
// ####### M A I N.c Re-direct function #######

