#ifndef _USER_PROTOVOLH_
#define _USER_PROTOVOLH_

#include "globaltype.h"

//------Don't change above codes ----------//
//===== This is For PD5 RKEY ======================================
//================ 0724R Region Begin ===========================================
//------ BASIC parameters AREA BEGIN---------------------------------------
// TPM information
#define VENDER_ID         0x511A
#define PRODUCT_ID        0x4886
#define HW_VERSION        0x0B
#define FW_VERSION        0x00
#define YEAR_MONTH_DAY    0x20140303
#define Co_FWAP_Protocol     0x00D3
#define FW_ReleaseVersion    0x0000

// TPM Structure
#define I2C_DEVADDR                   0xDC  




#define TX_LENGTH                     12 // 11 // 11 // 11//12 // Include Mkey TX, if exist.
#define RX_LENGTH                     22// 21 // 21 // 21//22
#define KEY_NUMBER                    0 // 3
#define KEY_TYPE                      0 // MUTU_DOWNKEY //0x00// 0x81 = self , 0x82 = MUTU_DOWNKEY , 0x83 = MUTU_RIGHTKEY , 0 = none

#define KEY0_TX 21//13
#define KEY0_RX 0   //20
#define KEY1_TX 21//13 // 1
#define KEY1_RX 1   //20
#define KEY2_TX 5//13 // 2    
#define KEY2_RX 2   //20
#define KEY3_TX 0
#define KEY3_RX 0

//:---Auxiliary subroutines that help your job. See global_vars.c for the detail.
#define DETECTING_KEY_CHANGE    0// Set to 1 if you wants to call DetectKeyChanging(key_mask).
#define DETECTING_POINT_CHANGE  0// Set to 1 if you wants to call DetectPointChanging().
//:--- Auxiliary subroutines end.

#define USER_MAX_FINGERS 5

// Normal mode Self mode parameters. Recommanded SELF delta around 500.
#define SELF_PROCESS     0// 1: Enable self mode to detect fingers. 0:Disable, pure mutual.

#define SELF_PULSE_COUNT  50    //Adjust it to match delta = 500 +/- 50
#define SELF_PULSE_LEN    76
#define SELF_FB            8 // 6    //JS20130616a, Adjust to match delta = 500 +/- 50.  Usually don't less then 5.
//13-0705 Start: Self gain adjust
#define GAIN_SELF_RX    3   // 0=Diable RX amplify. 1~255=N=Let Rx delta mutiply by N.
#define DIV_SELF_RX     1   // 1~255=Let Rx delta divided by N.
#define GAIN_SELF_TX    0   // 0=Diable TX amplify. 1~255=N=Let Tx delta mutiply by N.
#define DIV_SELF_TX     1   // 1~255=Let Tx delta divided by N.
//13-0705 End: Self gain adjust

// Normal Mutual mode parameters, Recommanded MUTUAL delta around 250.
#define MUTUAL_FORCE_CC   10 //9 //12 // 9:mi2 //10  // 0=Calibrate by auto-mode. 11~31: manually calbrated by this value. 1~10:WARNING !! May make ADC stop. 
#define MUTU_PULSE_COUNT  60 // 75 // 75//90//85  //Adjust it to match delta = 250 +/- 20
#define MUTU_PULSE_LEN    76 //60//48   // Adjust it to make abs(Raw(Xn, Y0)-Raw(Xn,Ylast)) <5%
#define MUTU_FB       4 // 7 //JS20130616a, Adjust to match delta = 250 +/- 20. Usually don't less then 5.

// Detecting Keys parameters
#define KEY_THRESHOLD       120 // 70 //60 //Threshold of key delta. 1x.
#define KEY_DEBOUNCE        3  //JS20130618a
//JS20130619e : Start :: Add for key handler
#define KEY_GAIN0     2 //2  // It must >= 1.
#define KEY_GAIN1     2 //2
#define KEY_GAIN2     2 //5
#define KEY_GAIN3     1
#define KEY_IIR_RATIO   2   // It must >= 1.
#define KEY_CEILING         200 //110 //Set to 110% of key peak delta, 1x.
#define KEY_BASELINE_THRESHOLD1  (KEY_THRESHOLD-70) //(KEY_THRESHOLD-20)  // Set it bigger than water delta. 
#define KEY_BASELINE_THRESHOLD2  50 // Not used now !
//JS20130619e : End :: Add for key handler
//------ BASIC parameters AREA END---------------------------------------

//------ ADVANCED parameters AREA BEGIN---------------------------------------
#define OCS_CLOCK 60  // Must be times of 10MHz. Valid from 20,30,...90.
// Normal mode Self mode Finger detection parameters
#define SELF_FINGER_TH          50 // 70   //JS20130616a, Set it as max((40% of peak delta), (Max.Noise+10%)).
#define SELF_FIRST_TOUCH_THD    (SELF_FINGER_TH+20) // 85   //JS20130616a, Set it as max((50% of peak delta), (SELF_FINGER_TH+10%)).
#define SELF_ENVIROMENT_CALTHD1 (SELF_FINGER_TH-1)    // Set to min((10% of self peak delta), 50) 
#define SELF_ENVIROMENT_CALTHD2 SELF_FINGER_TH    // Set to min((10% of self peak delta), 50)
// Mutual mode Finger detection parameters
#define MUTUAL_FINGER_TH        40 // 35  //35 //28    //JS20130616a, Set it as max((40% of peak delta), (Max.Noise+10)).
#define MUTUAL_FIRST_TOUCH_THD  50  //(MUTUAL_FINGER_TH+15) //28    //JS20130616a, Set it as max((50% of peak delta), (MUTUAL_FINGER_TH+15)).

#define MUTU_ENVIROMENT_CALTHD1 20 // (MUTUAL_FINGER_TH-20)     // 1x. Region1 high thrshold. Set to min((20% of mutual peak delta)). when peak<it, base update. 
#define MUTU_ENVIROMENT_CALTHD3 -15 //-500//(-(MUTU_ENVIROMENT_CALTHD1-5))    // 1x. Region1 low thrshold. 

#define MUTU_ENVIROMENT_CALTHD2 28 // (MUTUAL_FINGER_TH-5)    // 1x. Region2 high thrshold. Set to min((50% of mutual peak delta)). when peak>it, base hold.
#define MUTU_ENVIROMENT_CALTHD4 -20 // (-MUTU_ENVIROMENT_CALTHD1)   // 1x. Region2 low thrshold. 

//------ ADVANCED parameters AREA END---------------------------------------

//------ EXPERT parameters AREA BEGIN---------------------------------------
#define NORMAL_TIMER 9100
#define IDLE_TIMER 10000
#define SELF_ENVIROMENT_SUM -3000
//------ EXPERT parameters AREA END---------------------------------------
//================ 0724R Region End===========================================


//================ 0725R Region Begin=========================================
#define OSC_CLOCK   OCS_CLOCK       //Correcting mistyping
#define SGP_ENABLE  0 //AD20130725 1=Enable Smart Green Power. 0=Disable. This feature works only for OSC_CLOCK != CPU_CLOCK.
#define CPU_CLOCK   (OSC_CLOCK/1) //Change the constant only. CPU_CLK must be 1/N of OSC_CLOCK. N=1~15. And CPU_CLOCK must >= 4MHz. 
//================ 0725R Region End===========================================

//------Don't change below codes----------//

//================ 0904R Region Begin=========================================
#define TESTMODE_SELF_PULSECOUNT  50 // 150
#define TESTMODE_SELF_PULSELEN  48 // 100
#define TESTMODE_SELF_FB      6 // 31   //20130819 update
#define TESTMODE_SELF_CC      0 // 24   //0: Auto, 1: force CC; Adjust this value to let  SEL RAW around 300~2400.
//================ 0904R Region End=========================================

//================ 1113R Region Begin=========================================
#define MUTUAL_FORCE_RKEY_CC      10 //12 // 10//12:mi2 //28
//================ 1113R Region End===========================================


void i2c_rcv_ISR1_user(U8 *i2c_buf, U8 rcv_len);
void report_SendKey_user(U8 key_mask);
void report_SendPoint_user(void);
void i2c_rcv_ISR1_read_user(void);

#endif

