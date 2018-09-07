/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _FLASH_PARA_H
#define _FLASH_PARA_H

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// If use user protocol, please enable the definition
//
#define USER_PROTOCOL						ENABLE	//	ENABLE, DISABLE
#define I2C_SLAVE_ADDRESS					0x28
//
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// ####### P A N E L Information #######
#define 	PROJECT_ID						0x96	// identification for 12xx -> 96/86/76/56/46 ...
#define 	FW_VER							0xA2	// A2: C/T vs Tcon
#define 	SERIAL_NO						0x20150126
#define 	PROJECT_VID						0x4886
#define 	PROJECT_PID						0x000D


// ####### S E L F mode #######
#define 	SELF_CC_CLOCK					0
#define 	SELF_CFB						2
#define 	SELF_PULSE_LENGTH				88		// A2: min:60(suggestion, by external loading; step 4) 60->6.0us->167KHz
#define 	SELF_PULSE_COUNT				30
#define 	SELF_FINGER_THR					0xFF	// no use
#define		SELF_TX_PEAK_THR				150
#define		SELF_RX_PEAK_THR				60
#define		SELF_FINGER_FIRST_TOUCH			0xFF	// no use
#define		SELF_TX_NEGATIVE_THR			(-SELF_TX_PEAK_THR)
#define		SELF_TX_NEGATIVE_NODE			2
#define		SELF_RX_NEGATIVE_THR			(-SELF_RX_PEAK_THR)
#define		SELF_RX_NEGATIVE_NODE			2
#define		POWER_ON_FINGER_DEBOUNCE_TIME	3

// ####### M U T U A L mode #######
#define 	MUTU_CC_CLOCK					5
#define 	MUTU_CFB						3
#define 	MUTU_PULSE_LENGTH				88		// A2: min:60(suggestion, by external loading; step 4) 60->6.0us->167KHz
#define 	MUTU_PULSE_COUNT				30
#define 	MUTUAL_BASE_TRACE_UPPERBOUND	(MUTUAL_FINGER_THR-(MUTUAL_FINGER_THR>>3))
#define 	MUTUAL_BASE_TRACE_LOWERBOUND	-(MUTUAL_FINGER_THR-((MUTUAL_FINGER_THR>>3)))
#define 	MUTUAL_FINGER_THR				120 // 120
#define		MUTUAL_FINGER_FIRST_TOUCH		120 //(MUTUAL_FINGER_THR+(MUTUAL_FINGER_THR>>2))

// ####### K E Y / I N F O R M A T I O N #######
#define		KEY_THRESHOLD					200
#define		KEY_DEBOUNCE					3


// ####### O P E N / S H O R T  Test mode #######
#define		OPEN_SHORT_CC_CLOCK				3
#define		OPEN_SHORT_CFB					10
#define		OPEN_SHORT_PULSE_LEN			100
#define		OPEN_SHORT_PULSE_COUNT			50


// ####### F I R M W A R E  Parameters Setting #######
#define 	PALM_OVERNODE					32
#define 	MUTUAL_PALM_TH					MUTUAL_FINGER_THR
#define 	PALM_TX_RANGE					5
#define 	PALM_RX_RANGE					5
#define		JITTER_RANGE					8		// dx+dy
#define 	AREA_MERGE_RATIO_BIT			0x12	// -> 1/2


//####### M A P P I N G #######
#define 	POSITION_REMAPPING				0		// 0x01(Tx),0x02(Rx),0x04(XY reverse);	default:Rx=>X, Tx=>Y Bit set:Rx=>Y, Tx=>X
#define		WHOLE_VA_EXTEND					ENABLE
#define 	TX_TARGET_RESOLUTION			852
#define 	RX_TARGET_RESOLUTION			480

#define		SET__DISABLE__TO_GET_SHORTAGE	ENABLE	//default: Disable, After training, enable it.
#define		TX_EDGE_EXTEND					3		// channel
#define		RX_EDGE_EXTEND					3		// channel
#define		TX_EDGE_START_SHORTAGE			32		// pixels
#define		TX_EDGE_END___SHORTAGE			32		// pixels
#define		RX_EDGE_START_SHORTAGE			64		// pixels
#define		RX_EDGE_END___SHORTAGE			64		// pixels


//####### O T H E R S #######
#define		SELF_ASSISTANCE					DISABLE	// ---502B, LIB
#define		SELF_MODE_REPORT_FINGER			DISABLE	// ---660B, LIB

#define		MUTUAL_ONLY_POF_THR				-(MUTUAL_FINGER_THR * 3)		// usually: x1, x2, x3. by actual signal.
#define		MUTUAL_ONLY_POF_ABNORMAL_FRAME	10

#define		REPORT_RATE						70		// 100Hz
#define		IDLE_MODE_FUNCTION				DISABLE	// ---324B, SRC
#define 	IDLE_MODE_TIME_TO_ENTER			30		// sec.
#define 	UART_BAUDRATE					0x0094
#if USER_PROTOCOL
#define 	I2C_ENABLEALLBIT				1		// 1; USER protocol MUST be "1"
#else
#define 	I2C_ENABLEALLBIT				0		// 0; Winpower Protocol
#endif

#endif	//  _FLASH_PARA_H
