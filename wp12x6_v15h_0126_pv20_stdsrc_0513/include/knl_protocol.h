/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _KNL_PROTOCOL_H
#define _KNL_PROTOCOL_H

#include "com_global_type.h"


#define WINPOWER	0x00
#define WINTEK		0x01
#define USER		0x02

#define INT_HIGH	0X01
#define INT_LOW		0X00

typedef struct _PROTOCOL
{
	U8 Customer;
	U8 ProjectID;
	U8 CustomerDefault;
}PROTOCOL;

// 23 bytes
typedef struct _TP_HEADER
{
	U8 ProtVer;
	U8 Id;
	U8 HwVer;
	U8 FwVer;
	U32 SerialNo;
	U16 Vid;
	U16 Pid;
	U16 ResX;
	U16 ResY;
	U8 XRx;
	U8 YTx;
	U8 SupportFingers;
	U8 KeyNum;
	U8 MaxRptLength;
	U8 Cap1;
	U8 Cap2;
}TP_HEADER;

#if PROTOCOL_V20

typedef struct
{
	U16 skip_i2c_first_ch	: 1;	//1=Skip the 1st char of the I2C write.
	U16 redirect_i2cw		: 1;	//0=Put the I2C write bytes to default I2cBuffer[]. 1=protocol defined.
	U16 user_protocol		: 1;
	//U16 debug_mode			: 1;	//0=Frame free run, 1=Host control frame run.
	U16 end_of_fetch		: 1;
	U16 link_ok				: 1;
	U16 req_ack				: 1;
	U16 writeback_mcc		: 1;
	U16 writeback_scc		: 1;
} tProtocolState;

/*
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
*/
extern volatile tProtocolState ProtocolStateIs;
//extern volatile tVirtualRegister HostVirtualRegister;
//extern volatile tVirtualRegister CommandVirtualRegister;
extern U8 volatile *pI2cBuffer;	// Cannot has init value, otherwise will make large BIN file.
void I2cReadStopProcess(int rd_len);	// Call by ISR.

void ExecuteHostRequest(void);

#endif

//protocol define==========
#define MSG_GET_HEADER					0x01
#define MSG_GET_STATUS					0x02
#define MSG_GET_REPORTS					0x03
#define MSG_SET_MODE					0x04
#define MSG_CLEAR_REPORT				0x05
#define MSG_GOTO_STATE					0x06
#define MSG_RESET_TP					0x07
#define MSG_SET_SFR						0x08
#define MSG_SET_BLANK					0x09
#define MSG_SET_TIME_TO_IDLE			0x0B
#define MSG_READ_CCvalue				0x0E
#define MSG_WRITE_CCvalue				0x0F
#define I2C_READ_RAW_SELF				0x10
#define I2C_READ_RAW_MUTUAL				0x11
#define I2C_READ_RAW_KEY				0x12
#define MSG_UNFORCE_REPORTING			0x13
#define MSG_FORCE_REPORTING 			0x14
#define I2C_READ_RAW_MUTUAL2			0x15
#define MSG_ENTER_SCAN_MODE				0x16

#define MSG_SET_ENG_Mode				0x18
#define MSG_GET_MuCCvalue				0x1D
#define MSG_GET_PROTOCOL				0x1E
#define MSG_SET_PROTOCOL				0x1F
#define MSG_Sleep_In					0x20


//=========================

//Bit definition of the KnlProtocolCtrl
//#define bNOTLOAD_RCV_PTR				0x80
#define bNBRIDGE_I2C_EN					0x40
#define bBRIDGE_I2C_DONE				0x20
#define bINTR_TRIG						0x10
#define bDRIVER_ALIVE					0x08
#define bAP_ACTIVE_CC					0x04
#define bAP_ACTIVE_FLAG					0x02
//#define bFORBID_UPDATE_REPORT			0x01	/////
#define bREQ_WRITE_BACK_CC				0x01


//Bit definition of the KnlOpMode[2]
#define bOPM2_SHORT_TEST				0x01

//AP read data
#define I2C_SELF_RAW					0x40
#define I2C_SELF_BASE					0x80
#define I2C_SELF_DELTA					0xC0
#define I2C_MUTUAL_RAW					0x10
#define I2C_MUTUAL_BASE					0x20
#define I2C_MUTUAL_DELTA				0x30


void knl_protocol_WP(void);
void ctm_user_protocol_DMA_ptr(void);

#endif	//_KNL_PROTOCOL_H
