/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/


#include "ctm_user_protocol.h"
#include "com_global_type.h"
#include "com_global.h"
#include "flash_para.h"
#include "string.h"
sfrb(IF_DMA_ADDR_L       ,__IF_DMA_ADDR_L);
sfrb(IF_DMA_ADDR_H       ,__IF_DMA_ADDR_H);

#define SetI2cDMARRead(ptr) {IF_DMA_ADDR_L = (U8)((U16)ptr & 0x00FF); IF_DMA_ADDR_H = ((U8)((U16)ptr >> 0x08)-0x04);}
#define	SetINTPinLow() {P1OUT &= 0xfe;}


//########################################################################
//############## Modify Customer protocol from here !!!!! ################
//####### Keep the function name, and only modify conttests of it. #######
//########################################################################
// SetINTPinHigh();  --> Pull INT pin to high level.
//
// I2cSendEnable:1 -> Info Firmware to pull INT low state & do some process.
//
// I2cSendEnable:0 -> Default state.
//
// LastTouch: 1 -> Set 1 while points or key on.
//
// KeyPress: Touchkey status, bit map. eg: 0x00, 0x01, 0x02, 0x04,etc.
//#########################################################################

/*	Project : Lenovo A385e
 * 	Sensor type : 1T2R ?
 *  AA key.
 *  Key1=x64, y798
 *  Key2=x264, y808
 *  Key3=x410, y820
 *  VA resolution: x480, y800?
 *  Report out resolution: x=480, y=854. (Write by host)
 *
 *
 *
 *
 */

//U8 Buf_8047[256];
//U8 *Ptr8047;
int CharIndex=0;
int AddressIndex=2;

U8 *ReportPtr;
U8 *PointPtr;

U16 DataIndex=0;
U16 RegAddr;
U8 Buffer8000[512];	// Mapping to virtual 0x8000~0x81ff;
U8 Buffer4220[]={0x00,0x06,0x90,0x00};
U8 Var41E4=0xBE;
//U8 Cnt814A;
U8 fgSleeping=0;
U8 fgReporting=0;


const U8 data8140[]={
		0x39,0x36,0x38,0x00,0x6E,0x10,0x00,0x00,	// 0~7
		0x00,0x00,0x02,0x00,0x00,0x00,0x80,0x00	// 8~15
};
void ctm_user_protocol_init(void)
{

    I2cSendEnable = 0x00;
	//ProtocolStateIs.redirect_i2cw = true;
	//pI2cBuffer = I2cDataBuff;
    SetINTPinHigh();
    //Ptr8047 = &Buf_8047[2];
    //Cnt814A=0;
    Buffer8000[0x047] = 0x41;	// Chip version.
    memcpy(&Buffer8000[0x140], data8140, sizeof(data8140));
    ReportPtr = &Buffer8000[0x14E];
    //DebugDumpVar[3]=sizeof(TGXPOINT);
    *(U16 *)&Buffer8000[0x146] = RX_TARGET_RESOLUTION;	// X resolution.
    *(U16 *)&Buffer8000[0x148] = TX_TARGET_RESOLUTION;	// Y resolution.
    //DebugDumpVar[3]=*(U16 *)&Buffer8000[0x146];
    //DebugDumpVar[4]=*(U16 *)&Buffer8000[0x148];
}

const U16 x_pos_array[]={64, 264, 410};
void ctm_user_protocol_set_report(void)
{
	int i;
	U16 bit_mask;
	U16 x_pos, y_pos;
	//U8 key_flag=0;
	U8 points_cnt=0;

	//return;
    Buffer8000[0x047] = 0x41;	// Chip version.

    if(fgReporting>0)
	{
    	--fgReporting;
    	ReportPtr[0] = 0x80;	// Set for last point release.
	}

	PointPtr = &ReportPtr[1];
#if 1	// H_WORM
    y_pos = 830;
    if(KeyPress)
    {
   		bit_mask = 0x0001;
    	for(i=0; i<3; ++i)
    	{
    		if(KeyPress & bit_mask)
    		{
    			x_pos = x_pos_array[i];
				PointPtr[0] = i;	// track id
				PointPtr[1] = x_pos & 0xff;		// x_cord low
				PointPtr[2] = x_pos >> 8;			// x_cord high
				PointPtr[3] = y_pos & 0xff; 		// y_cord low
				PointPtr[4] = y_pos >> 8; 			// y_cord high
				PointPtr[5] = 0x02;		// size low
				PointPtr[6] = 0x00;		// size high
				PointPtr[7] = 0x00;		// reserved
            	++points_cnt;
				PointPtr += 8;	 // Shift pointer to next point.
    		}
    		bit_mask <<=1 ;
     	}
    }
#endif
    if (PointOutput.TouchInfo)
    {
    	bit_mask = 0x0001;

        for (i=0; i<KNL_MAX_FINGER_NUM; ++i)
        {
            if (PointOutput.TouchInfo & bit_mask)
            {
        		x_pos = PointOutput.Position[1][i];
        		y_pos = PointOutput.Position[0][i];

#if 0	// S3 key
        		//--- Detect VA key
            	if(y_pos > 825) // Assume Y > 800 : KEY ?
            	{
            		if((x_pos>38) && (x_pos<98))
            		{
            			key_flag = 0x01;
            			//x_pos = 64;
            		} else if((x_pos>234) && (x_pos<294))
            		{
            			key_flag = 0x02;
            			//x_pos = 264;
            		} else if((x_pos>384) && (x_pos<444))
            		{
            			key_flag = 0x04;
            			//x_pos = 410;
            		}
            	}
#endif
				PointPtr[0] = i;	// track id
				PointPtr[1] = x_pos & 0xff;		// x_cord low
				PointPtr[2] = x_pos >> 8;			// x_cord high
				PointPtr[3] = y_pos & 0xff; 		// y_cord low
				PointPtr[4] = y_pos >> 8; 			// y_cord high
				PointPtr[5] = 0x14;		// size low
				PointPtr[6] = 0x00;		// size high
				PointPtr[7] = 0x00;		// reserved
            	++points_cnt;
				PointPtr += 8;	 // Shift pointer to next point.
            }
            bit_mask <<=1 ;
    	}
    }

    if(points_cnt)
    {
        *PointPtr = KeyPress;	// Append trail 0.
    	ReportPtr[0] = (KeyPress ? 0x90 : 0x80) | points_cnt;	// Set buffer valid flag. Cleared by the host.
    	fgReporting = 3;	// Force 3 times 0x80 after all fingers up.
    }
    else
	{
		ReportPtr[9]=0;	// Clear previous key flag.
	}
    if(ReportPtr[0] & 0x80) SetINTPinLow();
}

void ctm_user_protocol_char_handler(int c)
{
	//return;
	if(AddressIndex>0)
	{
		((U8 *)&RegAddr)[--AddressIndex] = (U8) c;
		if(AddressIndex==0 && (RegAddr&0x8000)) DataIndex = RegAddr - 0x8000;
		else DataIndex=0;
	}
	else	// Got address, and this c is data.
	{
		Buffer8000[DataIndex++] = c;
	}
}



void ctm_user_protocol_rcv_write(U8 *rcv_data, int rcv_len)
{
    U8 *memptr;

    //return;

    AddressIndex = 2;	// Reset to accept next I2C cycle.
    memptr = &Buffer8000[DataIndex];	// Prepare DMA address for coming I2C read.

    switch(RegAddr)
    {
    	case 0x41E4:
    		memptr = &Var41E4;
    		break;
    	case 0x4220:
    		memptr = Buffer4220;
    		break;
    	case 0x814E:
    		if(DataIndex>0x14E)	// Clear report command ?
    		{		//--- End of reading report.
    			SetINTPinHigh();
    		}
			if(rcv_len>2) ReportPtr[0]=0; 
    		break;
    	case 0x8040:
    		if(Buffer8000[0x040]==0x05)	// Sleep ?
    		{
    			fgSleeping = 1;
    		}
    		break;
    	case 0x8047:
    		if(DataIndex>0x080)	// Write 240 bytes ?
    		{
    			Buffer8000[0x047] = 0x41;
    		}
    		break;
    	default:
    		break;
    }

    SetI2cDMARRead(memptr);
}

void ctm_user_protocol_rcv_read(int rd_len)
{
  
}

