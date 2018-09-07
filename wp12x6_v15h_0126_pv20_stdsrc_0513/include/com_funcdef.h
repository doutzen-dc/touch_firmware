/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _FUNCDEF_H
#define _FUNCDEF_H

extern void com_service_enter_sleep(void);
#if !PROTOCOL_V20
extern void drv_scan_read_self_cc(void);
extern void drv_scan_set_self_cc(void);
extern void drv_scan_read_mutual_cc(void);
extern void drv_scan_set_mutual_cc(void);
#endif
extern void knl_protocol_init(void);
extern void ctm_user_protocol_init(void);
extern void _WINPOWER_GUI_PROCESSING(void);
extern void _FEEDBACK_GUI_SELF_MUTUAL_MODE_CC(void);
extern void _SELF_MUTUAL_AUTO_CALIBRATION(void);

extern void drv_scan_get_noise_level(U16 *nl_raw_mem, U16 *nl_base, U8 hop_idx ,U16 *rx_level, U16 node);
extern void drv_scan_noiselistening_baseline(U16 *nl_raw_mem, U16 *nl_base, vU8 hop_idx, U16 *rx_level, U16 node);
extern void drv_scan_trigger_mutual(void);
extern void AutoSelfModeCalibration(U16 raw_min, U16 raw_max, U8 offset);
extern void AutoMutualModeCalibration_ByNode(U16 MinRaw, U8 offset);

extern void drv_scan_get_mutual_raw_data(U16 *mutu_raw_mem, U16 *m_base, U16 node);
extern void drv_scan_get_self_raw_data(U16 *self_raw_mem, U16 *s_base, U8 node);
extern void drv_scan_trigger_self(U8 fgshort);
extern void SelfModeReCalibrateCCToTargetRawData(void);

//Gesture
extern void knl_touch_finger_gesture(POINT_OUTPUT *fgesture);
extern S8 GestureMatchProcess(U8 ucCurrentPointNum,U8 ucLastPointNum,S16 curx,S16 cury);
extern void GestureVariableInitialFunction(void);
#endif	//_FUNCDEF_H
