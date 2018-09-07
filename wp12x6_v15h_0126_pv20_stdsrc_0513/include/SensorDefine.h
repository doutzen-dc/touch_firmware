/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef SENSOR_DEFINE_H_
#define SENSOR_DEFINE_H_

//=== IC Name: ¤uµ{«~48 ===
//=== Project: Sensor T13R24K0 ===
#define CHIPCONFIG_VER 275	// Who generated this sensor define .H file.
#define SELF_PASSES 4	// Total passes.Support max. 6 passes
#define SELF_RX_PASSES 2	// Passes for RX side.
#define CFGMEM_SELF_RX_EN_TABLE  { \
	0xA0,0xAA,0x00,0x00,0x50,0x55,	\
	0x40,0x55,0x01,0x00,0xA0,0xAA,	\
	0x00,0x00,0x50,0xA5,0x00,0x00,	\
	0x00,0x00,0x28,0x5A,0x01,0x00,	\
	0x00,0x00,0x00,0x00,0x00,0x00,	\
	0x00,0x00,0x00,0x00,0x00,0x00,	\
}	// Support max. 6 passes. Table shall be U8[6][6].
#define EFFECTIVE_SIZE_OF_CFGMEM_SELF_RX_EN_TABLE 24

#define FW_SELF_RAW_OFFSET_TABLE  { \
	 26, 13, 15, 36, 17, 34, 19, 32, 21, 30,	\
	 23, 28, 27, 25, 14, 16, 35, 18, 33, 20,	\
	 31, 22, 29, 24,  0,  2,  9,  4, 11,  6,	\
	  3,  1,  8, 10,  5, 12,  7,255,255,255,	\
	255,255,255,255,255,255,255,255,	\
}	// Raw offset start from 0. Table shall be U8[48].
#define EFFECTIVE_SIZE_OF_FW_SELF_RAW_OFFSET_TABLE 37

#define FW_SELF_RAW_OFFSET_LIST_PER_PASS  { \
	0xff,  26,  13,0xff,  15,  36,  17,  34,  19,  32,  21,  30,  23,  28,	\
	  27,0xff,  25,  14,0xff,  16,  35,  18,  33,  20,  31,  22,  29,  24,	\
	0xff,0xff,0xff,0xff,0xff,0xff,   0,0xff,   2,   9,   4,  11,   6,0xff,	\
	0xff,0xff,0xff,0xff,0xff,   3,0xff,   1,   8,0xff,  10,   5,  12,   7,	\
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,	\
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,	\
}	// Support max. 6 passes. Table shall be U8[6][14].

#define MUTUAL_PASSES 2
#define CFGMEM_MUTUAL_RX_EN_TABLE  { \
	0xA0,0xAA,0x00,0x00,0x50,0x55,	\
	0x40,0x55,0x01,0x00,0xA0,0xAA,	\
	0x00,0x00,0x00,0x00,0x00,0x00,	\
}	// Support max. 3 passes. Table shall be U8[3][6]
#define EFFECTIVE_SIZE_OF_CFGMEM_MUTUAL_RX_EN_TABLE 12

#define REG54_MUTUAL_HORIZONTAL_RAW_OFFSET_TABLE  { \
	 13,  0,  2, 23,  4, 21,  6, 19,  8, 17,	\
	 10, 15, 14, 12,  1,  3, 22,  5, 20,  7,	\
	 18,  9, 16, 11,255,255,255,255,255,255,	\
}	// Support max. 30 mutual RX channels. Raw offset start from 0. Table shall be U8[30].
#define EFFECTIVE_SIZE_OF_REG54_MUTUAL_HORIZONTAL_RAW_OFFSET_TABLE 24

#define FW_MUTUAL_HORIZONTAL_ASCENDING_RAW_OFFSET_TABLE  { \
	  0,  2,  4,  6,  8, 10, 13, 15, 17, 19,	\
	 21, 23,  1,  3,  5,  7,  9, 11, 12, 14,	\
	 16, 18, 20, 22,255,255,255,255,255,255,	\
	255,255,255,255,255,255,255,255,255,255,	\
	255,255,255,255,255,255,255,255,}	// Support max. 30 mutual RX channels. Raw offset start from 0. Table shall be U8[30].
#define EFFECTIVE_SIZE_OF_FW_MUTUAL_HORIZONTAL_ASCENDING_RAW_OFFSET_TABLE 24

#define FW_MUTUAL_RXS_PER_PASS_TABLE  {  12, 12,  0,}	// Support max. 3 passes. Table shall be U8[3].
#define TX_MODULATION 4
#define REG_C7_2 0x11
#define TOTAL_MODULATION_TX_SEQUENCE 4
#define CFGMEM_MUTUAL_TX_EN_TABLE  { \
	0x00,0x00,0x78,0x00,0x00,0x00,	\
	0x00,0x00,0x00,0x0F,0x00,0x00,	\
	0x00,0x00,0x00,0xF0,0x00,0x00,	\
	0x00,0x00,0x00,0xE0,0x01,0x00,	\
	   0,   0,   0,   0,   0,   0,	\
	   0,   0,   0,   0,   0,   0,	\
	   0,   0,   0,   0,   0,   0,	\
	   0,   0,   0,   0,   0,   0,	\
}	// Support max. 30 mutual TX channels. Table shall be U8[8][6].
#define EFFECTIVE_SIZE_OF_CFGMEM_MUTUAL_TX_EN_TABLE 24

	//----- The repetitive sampled line offset bit 7 will be marked to 1.
#define REG53_MUTUAL_VERTICAL_ROW_ID_TABLE  { \
	   3,   0,   1,   2,   4,   5,   6,   7,   8,   9,	\
	  10,  11,0x0D,0x0E,0x0F,  12, 255, 255, 255, 255,	\
	 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,	\
}	// Support max. 30 mutual TX channels. Line offset start from 0. Table shall be U8[30].
#define EFFECTIVE_SIZE_OF_REG53_MUTUAL_VERTICAL_ROW_ID_TABLE 16

#define MUTUAL_TX_NUMBERS 13
#define MUTUAL_RX_NUMBERS 24
#define KEY_NUMBERS 3
#define KEY_TYPE    MUTUAL_RIGHT_KEY	// SELF_KEY, SELF_SHARE_KEY, MUTUAL_DOWN_KEY, MUTUAL_RIGHT_KEY

#define KEY0_TX  2
#define KEY0_RX 23
#define KEY1_TX  6
#define KEY1_RX 23
#define KEY2_TX 10
#define KEY2_RX 23
#define KEY3_TX  0
#define KEY3_RX  0

// End Of Sensor Mapping Definition.
#endif
