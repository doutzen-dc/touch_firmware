/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _SYSTEM_H_
#define _SYSTEM_H_


void WatchDog_Init(U8);
void com_service_sfr_init(void);
void com_service_variable_init(void);
void drv_i2c_init(void);
void drv_mcu_timer(U8, vU16);
void drv_mcu_uart_init(U8);
void ChipSensorSetting(void);

void _OPEN_SHORT_TEST_MODE(void);
void _FRAME_FLAG_INITIALIZE(void);
U8 _MUTUAL_GET_TOUCH_STATUS(void);
U8 _SELF___GET_TOUCH_STATUS(void);
void _SELF___GET_PEAK_INFO(U8);
void _SELF_MUTUAL_CHECK_TOUCH_STATUS(U8,U8);
void _SELF___UPDATE_BASELINE(U8);
void _MUTUAL_UPDATE_BASELINE(U8);
void _TOUCH_KEY_CHECK(void);
U8 _COORD_XY_ADDRESSING(U8,U8);
void _COORD_XY_LINEARITY_SMOOTHING(void);
void _COORD_XY_REPORT_TO_HOST(void);
void _TRIGGER_INTR(void);
void _SAVE_FRAME_INFO(U8);
#if IDLE_MODE_FUNCTION
void _POWER_MODE_SWITCH(void);
#endif

void STARTING_TRIGGER_SCAN_NORMAL(void);
void STARTING_TRIGGER_SCAN_TESTMODE(void);
#endif	//_SYSTEM_H