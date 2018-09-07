#ifndef _BIOS_H_
#define _BIOS_H_

#include "globaltype.h"
//#include "WP1001HW.h"
#include "WP1004HW.h"


#define ACTIVE_POWER_SAVING 1       //AD20130725 1=With SGP module.

//JS20130616a XX : Start :: Move to globaltype.h and set globaltype.h as read only file 
//#define _8051_    // Enable this only for 8051.
//
//#if defined(_8051_)
//  #define xdata xdata
//  typedef unsigned char U8;
//  typedef unsigned int U16;
//  typedef unsigned long U32;
//  typedef char S8;
//  typedef int S16;
//  typedef bit FLAG;
//  #define lobyte(AA)  ((unsigned char) (AA))
//  #define hibyte(AA)  ((unsigned char) (((unsigned int) (AA)) >> 8))
//#elif defined(_X86_)
//  typedef unsigned char U8;
//  typedef unsigned short U16;
//  typedef unsigned int U32;
//  typedef char S8;
//  typedef short S16;
//  typedef unsigned char FLAG;
//  #define xdata
//#endif
//JS20130616a XX : End :: Move to globaltype.h and set globaltype.h as read only file 
//#define MutuCCByNode  //----------------------MutuCCByNode

#define WP1004_B 1
//#define AUTOCC_MODULE   //AD130722: Control to reduce code size.
#define EnableIntrAll() {EA=1;}
#define IsEndOfScan() (ADCScan_Done==1)
//---- System Related functions
extern bit timer0_fgEvent;
//extern U8 SamplingCntr;
//extern U8  SelfComOffset[];
extern bit ADCScan_Done;

extern U8 Tcounter;
extern bit STflag;
extern U8 cmdSELF_SING_ALL;
extern U8 paraSELF_SING_ALL;
//extern U8 WDTflag;

#define SystemTimerStart() {TR0=1;}
#define SystemTimerStop() {TR0=0;}
//#define IsSystemTimerEvent() (timer0_fgEvent)     // For polling mode used.
//#define SystemTimerClearEvent()  {timer0_fgEvent=0;}
//#define SystemTimerSetEvent()  {timer0_fgEvent=1;}
void SystemTimerInit(U8 intr_mode, U16 period); // period: 1 unit = 1ms.
  // a fixed period for internal timer counting
void SystemTimerReload();
  // Reload counter value and start.

//void switch_MCUDIV_FreqMode(U8 paraMCU_DIV, U8 PowerDnOn);
void switch_MCUDIV_FreqMode(U8 paraMCU_DIV);

//---- External 500KHz timer functions
void ExtTimerInit(U8 mode, U16 period); // period : unit in 1/500KHz = 2 us.
  // mode b7=1: using interrupt mode. b7=0: polling mode
void ExtTimerStart(void);
void ExtTimerStop(void);

//---- To host interface
#define DeActiveHostInt() {INTO=1;}
void SetI2CDevAddress(U8 dev_adr);  // 7 bits Aligned to MSB. LSB shall be 0. Default 0xDC.
void I2cInit(void);


//---- System functions
extern U16 McuMainClock;

#ifdef ACTIVE_POWER_SAVING
extern U8 CpuClock; //Default MCU_DIV=1.

void SetMcuMainClk(U8 osc_clk, U8 cpu_clk_factor);  // OCS_CLK; CPU_clock=OCS_CLK / cpu_clk_factor.
#else
void SetMcuMainClk(U8 clk_id);
#endif

//---- UART Functions
void HWUART_INIT(void);     // Hardware UART
void Uart0Start(unsigned baudrate); // Call it only after SetMcuMainClk().
unsigned char uart_putchar (char c);
char putchar(char c);
unsigned char uart_getchar(void);
char _getkey(void);
  
//void SetSelfScanChannels(U8 adc_id, U8 *channels, U8 channel_numbers);
  // Enter: assign the A/D converter with the targeted scanning channels. adc_id = 0 or 1 for WP1001.
  //    channels[channel_numbers] contains the expected active channel physical IDs.
  //    Channels[0] is the first scanned channel, Channels[1] is the second scanned channels,
  //    the scan activity will be continued until all channel_numbers channels are scanned then auto stopped.
  // Note: Call this routine will not start the scan process.
  //    If use dual ADC conversion, call this routine twice with different adc_id and channels[].
  
//void TriggerSelfScan(U8 cycle_id, U8 cycles); //from where, no.
  // Enter: Start self-scanning with scan order #id. id from 0 to channel_numbers-1.
  // Note: After calling SetSelfScanChannels(...), calling this routine will start the self scan activity.
  //  The ADC0 & ADC1 will be started  in synchronized.
  
//U8 IsSelfScanStopped(void);
  // Return: Return 0 if the whole self scan activity is still on going. Return non-zero if all channels were scanned.
  
//---- CAP ADC related functions
void ADCInit(U8 Tx,U8 Rx);
//Initialize ADC setting such as power on/off and enable/disable.
void key_set(U8 mode,U8 key_num,U16 *mukeyArray);
void I2cInit(void);
//Enable I2c communication.

void SetI2CDevAddress(U8 dev_addr);
//8-bits address. Default address is 0xDC. (LSB is '0')

//void SetMcuMainClk(U8 clk_id);
//Set MCU clock rate. Clock interval is 10MHz. Clock range is from 10MHz to 100Mhz by 10MHz step.

void SetChannelComOffset(U8 *offset_value);
//Set self/mutual mode offset by channel.
//You need to memory the self(or mutual) mode offset setting before doing self(mutual) mode scan.
//ie. When you do self mode scan, you need to restore the self mode offset setting to corresponding
//channel. so is mutual mode.

void OpenRawTestMode(U8 switch_mode);


void SelfScanInit(U16 sensing_count, U16 pulse_len,U8 CFB);
//void SetSelfScanPulses(U16 sensing_count, U16 pulse_len);
  // Note: Call this routine to setup the charge&transfer timing.
  // Enter: The pulses_count means how many pulses in a channel's self-scan c&t times.
  // The pulse_len means how many micro-second for a single C&T pulse. The pulse is always 50% duty.

void MutualScanInit(U16 musensing_count, U16 muPulseLen,U8 CFB);
//void SetMutualScanPulse(U8 mu_sense_seq,U16 musensing_count, U16 muPulseLen);
//void SetMutualScanPulse(U16 musensing_count, U16 muPulseLen);

//void SelfScanStart(S16 *pSelfTargetAddr,S16 *pSelfBaselineAddr);  
void SelfScanStart(S16 *pSelfTargetAddr); 
//void SelfScanStart(U8 s_sense_seq,S16 *pSelfTargetAddr,S16 *pSelfBaselineAddr);
  // Note: Call this function to mantual trigger self scan function.
  // s_sense_seq: Self mode Sense seq times N = 1~8  (each time may sense more then 1 SH)
#if WP1004_B  
//void MutualScanStart(S16 *pMutuTargetAddr,S16 *pMubaselineAddr);
//void AutoMutuCalibration(S16 *pTargetAddr,S16 *pBaselineAddr); 
void MutualScanStart(S16 *pMutuTargetAddr);
#ifdef AUTOCC_MODULE
  void AutoMutuCalibration(S16 *pTargetAddr,U16 MinRaw,U8 offset);
  void AutoMutuCalibration_byNode(S16 *pTargetAddr,U16 MinRaw,U8 offset) ;
#endif
#else
void MutualScanStart(U8 *pMutuTargetAddr,U8 *pMubaselineAddr);
#ifdef AUTOCC_MODULE
  void AutoMutuCalibration(U8 *pTargetAddr,U8 *pBaselineAddr); 
  void AutoMutuCalibration_byNode(U8 *pTargetAddr); 
#endif
#endif
//void MutualScanStart(U8 mu_sense_seq,U8 *pMutuTargetAddr,U8 *pMubaselineAddr);
  // Note: Call this function to mantual trigger self scan function.
  // mu_ch_no: Mutual TX channel number N= 0~47 (N= How many TX channel)

void ADC_CYC_SETTING_GEN();

//void AutoSelfCalibration(S16 *pTargetAddr,S16 *pBaselineAddr)  ;
void AutoSelfCalibration(S16 *pTargetAddr) ;
//void Sleep_In(void);
void WatchDog_Intr(void);
void WatchDog_Initialize(U8);
void NoiseFilterSetting(U8 ,U8 , U8 ,U8);
void SenseCHControl(void);
#endif

/*for hopping:Yuho*/
extern void measureNLlevel(void);
void NoiseIndexSet(void);
void NoiseListenScanStart(void);
extern void chooisefreq1(void);
extern void chooisefreq2(void);

