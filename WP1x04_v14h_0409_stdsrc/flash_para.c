#include "globaltype.h"
#include "user_protocol.h"
//#include "bios.h"

#define SELF_KEY 0x81                                         //JS20130616a : Don't change the define
#define MUTU_DOWNKEY 0x82 //Mutu_key is at TP "Bottom" side.  //JS20130616a : Don't change the define
#define MUTU_RIGHTKEY 0x83 //Mutu_key is at TP "Right" side.  //JS20130616a : Don't change the define
//------ Don't change above codes  ----------------------------------


//------ Don't change BELOW codes  ----------------------------------

//JS20130616a : Start :: Move to flash_para.c
// customer setting
enum //If modify it, please inform FW team. Thanks a lot.
{
    // ---Customer
    Winpower = 0,
    Wintek  = 1,
    SUO = 2,
    // ---Project_ID
    Inch_4_7 = 1,
};



//#define Flash_data_en_Key 0xAA
#define FLASH_DATA_EN_KEY     0xAA //JS20130616a
#define MUTUAL_CC_MIN_RAW   1000       //JS20130616a
#define MUTUAL_OFFSET_CC    5          //JS20130616a

#define SELF_EN_ALL   1     // 1:En-ALL[default]; 0: single[O/S]

// 20130606 add by Jackson //JS20130618a
#define POF_ENABLE_FLAG     1
#define SELF_POWER_ON_UPDATE_CN 20  // for self
#define MUTU_POWER_ON_UPDATE_CN 40  // for mutu
#define SELF_MIN_RAW_DATA     200

#define LARGEAREA_THD       45  //35
#define LARGEAREA_THD_RATIO   2   // Chose area threshold base on ratio of max delta. 2013.05.07
#define LARGEAREA_FIG       1   //6 // Big finger, report // 8
#define LARGEAREA_REJ       25  //16  // Palm, reject     //16
#define LARGEAREA_WEIGHT_THD    10  //10  // Counted weighting-Sum threshold for calculate coordinate.

//JS20130710a modify by Poly
//#define JT_ENABLE_FLAG      1
#define JT_CHR_TH_SCALE_RATIO   2  // 2x Normal TH --> 2 * 32 = 64
#define JT_LOCK_TH_NORMAL     10 // 32 // 10 // 32 //24 // 10 // 48 // dx+dy: 10/5 too low
#define JT_UNLOCK_TH_NORMAL   8 // 10 // 8 // 16 //16 // 8 // 5
#define JT_IIR_POINT          6
#define JT_UNLOCK_SCREEN      16
#define JT_STATIC_COUNTER_TIMES 1 // 10 // 1//30

//fixed
#define HAUTO_SCAN_ENABLE 1
#define HAUTO_SCAN_START  48  //48 //12   // 48 /4 = 12, 48 = 4.8us = 208KHz
#define HAUTO_FREQ_RANGE  9   // fixed
//#define HCHOOSE_FREQ_NUM  3   // fixed
#define HDO_TIMES     4   // fixed

#if TP_CUSTOMER_PROTOCOL
  bit tp_customer_protocol = 1;
  #define CUSTOMER_DEFAULT SUO
  #define CUSTOMER_BYTE SUO
#else //TP_CUSTOMER_PROTOCOL
  bit tp_customer_protocol = 0;
  #define CUSTOMER_DEFAULT Winpower
  #define CUSTOMER_BYTE   Winpower
#endif //TP_CUSTOMER_PROTOCOL


#define PROJECT_ID_DEFAULT Inch_4_7

//#define TAPPING_DISTANCE_CELLX2 9 //5//9
//#define TAPPING_DISTANCE (RES_SCALE/8 * TAPPING_DISTANCE_CELLX2) * (RES_SCALE/8 * TAPPING_DISTANCE_CELLX2)
#define TAPPING_DISTANCE    5184 //Type it manual (Unit: one cell pitch == 64; Format (dx/4)^2+(dy/4)^2)
#define CUSTOMER_RATIO      64 //JS20130616a
#define SUPPORT_MAX_FINGERS 5 //JS20130616a : For Library-ize
//JS20130616a : End :: Move to flash_para.c



//code U8 Flash_data_en={FLASH_DATA_EN_KEY};  //JS20130616a
//code U8 AP_para[4] = {0,0,0,0};

U8 MutualForceMCC=MUTUAL_FORCE_CC;

U8 gainSelfRx = GAIN_SELF_RX;
U8 divSelfRx  = DIV_SELF_RX;
U8 gainSelfTx = GAIN_SELF_TX;
U8 divSelfTx  = DIV_SELF_TX;

code tFlash fFlash =
{
    0xAA,         //U8, 0xAA = Enable FW Parameter function, otherwise = Disable
    0,          //U8, 0; Customer_ID
    0,          //U8, 0; Project_ID
    0,          //U8, 0; Version_Main
    0,          //U8, 0; Version_Sub
    0,          //U16, 0; Parameter_Checksum
    0,          //U16, 0; FW_Checksum
    Co_FWAP_Protocol, //U16, 0; Co_FWAP_Protocol
    FW_ReleaseVersion,  //U16, 0; FW_ReleaseVersion
    0x011A,       //U16   // from this position to add 0x011A --> PID
    0x010F,       //U16   // from this position to add 0x010F --> FW version
    0,      //U8
    0,      //U8
};

// fixed ADC address location
#ifdef flashpara
code U8 flSelf_sequ = 3;
code U8 flMutu_sequ = 2;

code U8 ChMapping[48] =
{
  29,28,27,26,25,24,17,16,15,14,13,12,  //TX Channel No =12
  11,10,9,8,7,6,5,4,3,2,1,37,38,39,40,41,42,43,44,45,46,47, //RX Channel No =22
  0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

code U8 AdcCycSet[12][6] =  // =AdcCycSet [0x40 ~ 0x4B] [byte]
{
  {0x76,0x00,0x00,0x00,0x70,0x00},  //addr = 0x40 
  {0x67,0x04,0x00,0x00,0x74,0x00},  //addr = 0x41 
  {0x74,0x06,0x00,0x00,0x74,0x00},  //addr = 0x42 
  {0x46,0x07,0x00,0x00,0x74,0x00},  //addr = 0x43 
  {0x67,0x04,0x00,0x00,0x74,0x00},  //addr = 0x44 
  {0x74,0x06,0x00,0x00,0x74,0x00},  //addr = 0x45 
  {0x45,0x07,0x00,0x00,0x74,0x00},  //addr = 0x46 
  {0x57,0x04,0x00,0x00,0x74,0x00},  //addr = 0x47 
  {0x74,0x05,0x00,0x00,0x74,0x00},  //addr = 0x48 
  {0x45,0x07,0x00,0x00,0x74,0x00},  //addr = 0x49 
  {0x57,0x04,0x00,0x00,0x74,0x00},  //addr = 0x4A 
  {0x04,0x05,0x00,0x00,0x04,0x00},  //addr = 0x4B
};
code U8 SelfAdcOffset_0x54[48] =
{
  0x05,0x20,0x15,0x02,0x1D,0x12,0x06,0x1A,
  0x0F,0x09,0x17,0x0C,0x21,0x04,0x1F,0x14,
  0x01,0x1C,0x11,0x07,0x19,0x0E,0x0A,0x16,
  0x03,0x1E,0x13,0x00,0x1B,0x10,0x08,0x18,
  0x0D,0x0B,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

code U8 MutuAdcOffset_0x55[48] =
{
  0x0A,0x09,0x08,0x07,0x06,0x05,0x04,0x03,
  0x02,0x01,0x00,0x15,0x14,0x13,0x12,0x11,
  0x10,0x0F,0x0E,0x0D,0x0C,0x0B,0x00,0x00,
  00,00,00,00,00,00,00,00,
  00,00,00,00,00,00,00,00,
  00,00,00,00,00,00,00,00,
};
code U8 MutuTxSeq_0x4C[32] =
{
  0x1D,0x1C,0x1B,0x1A,0x19,0x18,0x11,0x10,
  0x0F,0x0E,0x0D,0x0C,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};
// fixed ADC address location
#endif

code tProtocol flProtocol =
{
    CUSTOMER_BYTE,      //U8 Customer;        //JS20130616a
    PROJECT_ID_DEFAULT,   //U8 Project_ID;    //JS20130616a
    CUSTOMER_DEFAULT,   //U8 CustomerDefault; //JS20130616a
};
code tSystemPara flSystemPara =
{
    OSC_CLOCK,          //U8 Clock_rate;
    I2C_DEVADDR,        //U8 I2C_DevAddr;
    NORMAL_TIMER,       //U16 Timer_period_Normal;
    IDLE_TIMER,         //U16 Timer_period_Idle;
    0,                  //U8 ReportPoint_para;  // bit0 : Y inverse enable , bit1 : X inverse enable , bit2 : X Y change
    TAPPING_DISTANCE,   //U16 Tapping_distance;
};
code tPannel flPannel =
{
    SUPPORT_MAX_FINGERS,    //U8 SUPPORT_FINGERS;
    TX_LENGTH,              //U8 Y_TX;
    RX_LENGTH,              //U8 X_RX;
    CUSTOMER_RATIO,         //U8 scale; //64 .48  //Please keep it as 64
    HW_VERSION,             //U8 HW_VER;
    FW_VERSION,             //U8 FW_VER;
    YEAR_MONTH_DAY,         //U32 SERIAL_NO;
    0,                      //U8 u8Rev1;
    0,                      //U8 u8Rev2;
    VENDER_ID,                   //U16 V_ID;
    PRODUCT_ID,                   //U16 P_ID;
};
code tRawcountPara flRawPara =
{
    SELF_PULSE_COUNT,   //U16 SelfPulseCount;
    SELF_PULSE_LEN,       //U16 SelfPulseLen;
    MUTU_PULSE_COUNT,     //U16 MutualPulseCount;
    MUTU_PULSE_LEN,     //U16 MutualPulseLen;
    SELF_FIRST_TOUCH_THD,   //U16 Self_first_TH
    SELF_FB,                //U8 Self_fb;
    MUTU_FB,                //U8 Mutu_fb;
    SELF_FINGER_TH,         //U16 Self_TH;
    MUTUAL_FINGER_TH,       //U16 Mutu_TH;
    MUTUAL_FIRST_TOUCH_THD, //U16 Mutu_first_TH ;
    MUTUAL_CC_MIN_RAW,      //U16 MutuCC_MinRaw ;
    MUTUAL_OFFSET_CC,       //U8 Mutu_offsetCC ;
    //U8 CC_byRx[32] ;
};
code tKeyPara flKeyPara =
{
    KEY_TYPE,                   //U8 type; // 0x81:self , 0x82 : MUTU_DOWNKEY ,0x83: MUTU_RIGHTKEY ,  0 : none
    KEY_NUMBER,                 //U8 num;
    KEY0_TX*RX_LENGTH+KEY0_RX,  //U16 CHS[0];153
    KEY1_TX*RX_LENGTH+KEY1_RX,  //U16 CHS[1];131
    KEY2_TX*RX_LENGTH+KEY2_RX,  //U16 CHS[2];109
    KEY3_TX*RX_LENGTH+KEY3_RX,  //U16 CHS[3];0
    0,                          //U16 CHS[4];
    0,                          //U16 CHS[5];
    MUTUAL_FORCE_RKEY_CC,       //U16 CHS[6];  // 13-0915A changed
    0,                  //U16 CHS[7]; // if = ff, send key data to mutual data
    KEY_THRESHOLD*4,            //S16 KeyThreshold;
    KEY_DEBOUNCE,               //U8  keydebounce;  new add 20130605
    KEY_GAIN0,                  //U8  KeyGain[0]   //JS20130619e : Add for key handler
    KEY_GAIN1,                  //U8  KeyGain[1]   //JS20130619e : Add for key handler
    KEY_GAIN2,                  //U8  KeyGain[2]   //JS20130619e : Add for key handler
    KEY_GAIN3,                  //U8  KeyGain[3]   //JS20130619e : Add for key handler
    0,                          //U8  KeyGain[4]   //JS20130619e : Add for key handler
    0,                          //U8  KeyGain[5]   //JS20130619e : Add for key handler
    0,                          //U8  KeyGain[6]   //JS20130619e : Add for key handler
    (SGP_ENABLE<<5) | ((OSC_CLOCK/CPU_CLOCK)<<1) | SELF_PROCESS,               //U8  KeyGain[7]   //JS20130619e : Add for key handler
    // bit0: SELF_PROCESS
    // bit1~4: CPU_CLOCK_FACTOR
    // bit5~7: 0. reserved.
    KEY_IIR_RATIO,              //U8  KeyIIRRate   //JS20130619e : Add for key handler
    KEY_CEILING*4,                //S16 KeyCeiling   //JS20130619e : Add for key handler
    KEY_BASELINE_THRESHOLD1*4,    //S16 KeyBaseTH1   //JS20130619e : Add for key handler
    KEY_BASELINE_THRESHOLD2*4,    //S16 KeyBaseTH2   //JS20130619e : Add for key handler
};


//20130618a merge : Start
//20130531JACKSON
code tMutuPofPara flMutuPofPara =  //power on finger parameter
{
    POF_ENABLE_FLAG,          //U8 EnableFlag
    SELF_POWER_ON_UPDATE_CN,   //S16 PowerOnUpdateCN1
    MUTU_POWER_ON_UPDATE_CN,  //S16 PowerOnUpdateCN2
    SELF_MIN_RAW_DATA,          //S16 self_minRaw
};

code tLargeAreaPara flLargeAreaPara =
{
    0,      //LARGEAREA_THD,//U16 not used any more.//AD130828 fromPoly
    LARGEAREA_THD_RATIO,    //U8 LargeArea_thd_ratio
    LARGEAREA_FIG,        //U8 LargeArea_fig
    LARGEAREA_REJ,        //U8 LargeArea_rej
    LARGEAREA_WEIGHT_THD,   //U8 LargeArea_weight_thd
};


//JS20130710a modify from Poly
code tJitterIIRPara flJitterIIRPara =
{
    0,              //u8 u8Rev1   //20130701
    JT_CHR_TH_SCALE_RATIO,    //U8 CHR_TH_Scale_Ratio
    JT_LOCK_TH_NORMAL,      //U16 Lock_TH_Normal
    JT_UNLOCK_TH_NORMAL,    //U16 Unlock_TH_Normal
    JT_IIR_POINT,       //U8  IIR_Point   //20130701
    JT_UNLOCK_SCREEN,     //U16 Unlock_Screen //20130701
    JT_STATIC_COUNTER_TIMES //U16 StaticCounterTimes  //20130701
};

code tEnvironmentTHDPara flEnvironmentTHDPara =
{
    SELF_ENVIROMENT_CALTHD1,    //S16 Self_CalTHD1
    SELF_ENVIROMENT_CALTHD2,    //S16 Self_CalTHD2
    SELF_ENVIROMENT_SUM,      //S16 Self_CalSum
    MUTU_ENVIROMENT_CALTHD1*4,    //S16 Mu_CalTHD1
    MUTU_ENVIROMENT_CALTHD2*4,    //S16 Mu_CalTHD2
    MUTU_ENVIROMENT_CALTHD3*4,    //S16 Mu_CalTHD3
    MUTU_ENVIROMENT_CALTHD4*4,    //S16 Mu_CalTHD4
};
//20130531JACKSON
//20130618a merge : End

//JS20130710a : merge new parameter struct
code tDEBUGSettingPara fDEBUGSettingPara =
{
    0,              //U8 reserved
    0,              //U8 reserved
    SELF_EN_ALL,        //U8 -- @20131217 -- self mode reigger non-sensing channel (single or all trigger)
    0,              //U8 u8Rev2
    TESTMODE_SELF_CC,     //U8 u8Rev3
    TESTMODE_SELF_FB,       //U8 u8Rev4     //AD130828 fromPoly
    TESTMODE_SELF_PULSECOUNT,   //U16 u16Rev1   //AD130828 fromPoly
    TESTMODE_SELF_PULSELEN,   //U16 u16Rev2   //AD130828 fromPoly
};

code tFREQAnalysis fFREQAnalysis = 
{
  HAUTO_SCAN_ENABLE,    // 0 or 1; enable or disable
  HAUTO_SCAN_START, // 
  0,    //U16
  0,    //U16
  0,    //U16
  HAUTO_FREQ_RANGE,
  HDO_TIMES,
};


