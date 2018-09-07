
#include "bios.h"
#include "GLOBAL_VARS.H"
#include "WP1004HW.h"
#include <stdio.h>
#include <absacc.h>    // for CBYTE, CWORD
#define ShowAUTOCC_Self 0
#define ShowAUTOCC_Mutu 0
#define AUTOCC_OFFSET 0x01
#if WP1004_B
#define AUTOCC_Min 1000
#else
#define AUTOCC_Min 0x20
#endif
#define lobyte(AA)  ((unsigned char) (AA))
#define hibyte(AA)  ((unsigned char) (((unsigned int) (AA)) >> 8))
bit ADCScan_Done;
bit timer0_fgEvent;


/*for hopping:Yuho*/
extern code U8 MutuTxSeq_0x4C[];
extern volatile S16 xdata NoiseTargetAddr[] ;//_at_ Start_MuTargetAddr;
extern U8 NLIndex; 
extern U8 UsedCTFreq[];
extern bit startnoiselisten;

//#define PRINTFUNCTION



U8 TxChNumber, RxChNumber;
U16 timer0_CntrValue;
U16 McuMainClock=20;//Actually it is OSC clock.

U8 CpuClock=20;//Actually it is real 8051 clock.
U8 MuINTCount;
U8 bios_ScanMode;
U8 TxMuSequ;
U8 TxMuSequ_1;
U8 self_sequ;
U8 mutu_sequ;
U8 SelfPulseTime;
U8 MutuPulseTime;
U8 NLPulseTime;

//U8 SystemClock;
U8 SystemClockParameter;
U8 CFB_mutu;
U8 CFB_self;
U8 gKey_mode;
U8 gkey_num;
U16* gmukeyArray;
extern bit TestModeFlag;
extern bit SelfADCEnALLBit;
extern S16 * ptrLastQuaDelta; // for store testMode mutual CC by nodes data
#define self_mode 0
#define mutual_mode 1
#define NL_mode 2

void _nop_(void);
void OnAdcInterrupt(U8);
void ReadBack_Info_OSC(void);
#define pre_TCON_DIV_clk 10//set as 12MHz

// SH35  SH36 SH37 SH38  SH39 SH40 SH41 SH42  SH43 SH11 SH10 SH9  SH8 SH7 SH6 SH5 SH4
//U8 code rx_offset[17] = {34,35,36,37,38,39,40,41,42,10,9,8,7,6,5,4,3};
#if 0
U8 ChMapping[31] =
{
    27,26,25,24,17,16,15,14,13,12, 				// Y CH : 10
    34,35,36,37,38,39,40,41,42,10,9,8,7,6,5,4,3,	// X Ch : 17
    1,2,43,44									// Key : 4
};
#else
extern U8 code ChMapping[];
#endif
// 7 bits
U8  SelfComOffset[48]={   1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 1
                      };
#if !TP_CUSTOMER_PROTOCOL
U8 TestModeSelfCC[48]={0};
#endif

#ifdef MutuCCByNode
volatile U8  MutualComOffset[336] ; // for RX
#else
volatile U8  MutualComOffset[32] ; // for RX
#endif

// ############################
#ifdef PRINTFUNCTION
// Hardware UART
#define TBUF_SIZE	32
static xdata U8 t_in = 0;
static xdata U8 t_out = 0;
static xdata U8 tbuf [TBUF_SIZE];
xdata U8 buf_len = 0;
#endif
// ############################
#define LockPort_OPEN(){P0_1 = 0; }
#define LockPort_CLOSE(){P0_1 = 1; }



#define SetFlashWaitForMcuClock(ck) {MCU_ADDR=PRE_FETCH_CONTROL; MCU_DATA=(((ck)+63)/32);}


void ADCInit(U8 Tx,U8 Rx)
{
    U8 rdtmp;
    TxChNumber = Tx  ;
    RxChNumber = Rx;

    gKey_mode = 0;
    gkey_num = 0;

    EA = 0;// Disable All Interrupt

    P0_2 = 1;
    P0_3 = 1;
    IE0 = 0;
    IT0= 1;// ADC Falling Interrupt
    EX0= 1;// Enable ADC Interrupt
    EIE = EIE & 0xDF;// Disable Watchdog

    LockPort_OPEN();
    ADC_CYC_SETTING_GEN();// auto generate slef/mutual mode channel setting code  // [poly]
    SenseCHControl();
    TxMuSequ = TxChNumber*mutu_sequ -(mutu_sequ-1)-1;
    TxMuSequ_1 = (TxChNumber-1)*(mutu_sequ) -1;

    MCU_ADDR = IO_TRIM ;// 0xef;
    MCU_DATA = 0x0F;

    MCU_ADDR = ADC_ND_CTRL ;// 0x3b;
    MCU_DATA = 0x00;
    MCU_DATA = 0x00;

    MCU_ADDR = ADC_PWD_CTRL; // 0x3c;
    MCU_DATA = 0x00;
    MCU_DATA = 0x00;

    MCU_ADDR = SCAN_STATUS;// 0x81
    MCU_DATA = 0x03;// scan on

    MCU_ADDR =  PREC_CTRL;// 0xF2 Pre-Charge CTRL
    MCU_DATA  = 1;       // Pre-Charge Position (Reference to H_BP)
    MCU_DATA  = 0x81;// Bit[7]=P_EN, Bit[3:0]=P_WIDTH

    MCU_ADDR =  SCLK_NON_OVERLAP;// 0xF3 SCLK Non Overlapping Setting
    MCU_DATA  =  0;// 1;	// 0822					// 0= 1xOSC CLK, 1= 2xOSC_CLK, 2=3xOSC_CLK

    MCU_ADDR = ADC_SAMPLE_CLOCK_DIV;// 0xF4, ADC Sample Clock Divider
    MCU_DATA = 0x01;

    MCU_ADDR = 0x65;// Disable Charge Pumper
    MCU_DATA = 0x00;

    MCU_ADDR = 0x66;// Disable LVDD Detect
    MCU_DATA = 0x80;

    MCU_ADDR = SPEC_CONFIG_F6;// 0xF6
    //MCU_DATA = 0x03;					// MUTUAL_EN = 1, STOP_EN = 1; auto:01, manual:03
    MCU_DATA = 0x07;// For China Version, MUTUAL_EN = 1, STOP_EN = 1; auto:01, manual:03


    //#############  RAWDMA Setting ######################
    MCU_ADDR = XDATA_DMA_CONTROL; // 0x50, xdata DMA control
    MCU_DATA = 0x01;  //  RAW_PROC=0,RAWDMA EN=1,

    MCU_ADDR = SPEC_FUNC_FA; // 0xFA, Diable RAW test mode & Enable HW UART
    rdtmp = MCU_READ;// To ensure A0_PL_EN = 1
    rdtmp = rdtmp | 0x40; // rdtmp | 0x40
    MCU_DATA = rdtmp;

    LockPort_CLOSE();
//    NoiseFilterSetting(0,0,0,0);
}


void I2cInit(void)
{
    P1_IO = 0;//set P1.0 , P1.1 output
    //INTO= 1;							// De-active INTO.  //JS20130620c  Move to main.c for FAE who can set it
    IT1 = 1;// Set INT1 as edge trig.
    EX1 = 1;// Enable INT1 = I2C recv byte interrupt.
}

void SetI2CDevAddress(U8 dev_adr)
// 7 bits Aligned to MSB. Default 0xDC.
{
    U8 rdtmp;
    LockPort_OPEN();
    MCU_ADDR = SPEC_FUNC_FA;
    MCU_INDEX = 0;
    rdtmp = MCU_READ;
    rdtmp = (rdtmp | 0x60) & 0xF7;  // A0 _reg enable
    rdtmp = (rdtmp ) | ((dev_adr & 0x02)<<2);  // A0 _reg enable
    MCU_INDEX = 0;
    MCU_DATA = rdtmp;

    MCU_ADDR = I2C_ADDR_CTL;   // 0xF1
    MCU_DATA = dev_adr;    // 8bits slave address

    LockPort_CLOSE();
}

#ifdef ACTIVE_POWER_SAVING

void SetMcuMainClk(U8 osc_clk, U8 cpu_clk_factor)	// OCS_CLK; CPU_clock=OCS_CLK /	cpu_clk_factor.
{
    S16 tmpclk;

    U8 cpu_clk;
    //SystemClock = clk_id;
    //LockPort_OPEN();
    if (osc_clk >= 100)// over 100M
    {
        tmpclk = 0x0f;// set as 100M
        osc_clk = 100;
        SystemClockParameter = 8;
    }
    else
    {// Round clk to times of 10 to meet hardware limit.

        osc_clk = osc_clk/10;
        tmpclk = (osc_clk-2);// get the close value; if 23M -> 20M setting
        if (tmpclk <= 0) tmpclk= 0;

        osc_clk =osc_clk * 10;
        SystemClockParameter = tmpclk;
    }

    cpu_clk = osc_clk / cpu_clk_factor;

    LockPort_OPEN();
    if (cpu_clk > CpuClock)	//New clock faster, so change FLASH waitstate first.

        SetFlashWaitForMcuClock(cpu_clk);
    MCU_ADDR = OSC_CTRL;// 0x7C
    MCU_DATA = 0x01;// OSCEN = 1: enable 12M OSC
    MCU_DATA = tmpclk;

    MCU_ADDR = CLOCK_DIV;// 0x7D
    MCU_INDEX = 0x1;
    MCU_DATA = cpu_clk_factor;// MCU_DIV ratio=1x. (0=1)
    MCU_DATA = osc_clk/pre_TCON_DIV_clk; // 4;		// GLB_DIV; here = 10MHz
    if (cpu_clk < CpuClock)	//New clock faster, so change FLASH waitstate first.
        SetFlashWaitForMcuClock(cpu_clk);
    McuMainClock = osc_clk;
    CpuClock = cpu_clk;	//Default MCU_DIV=1.
    ReadBack_Info_OSC();
    LockPort_CLOSE();

}

#else
void SetMcuMainClk(U8 clk_id)
{
    S16 tmpclk;
    //SystemClock = clk_id;
    //LockPort_OPEN();
    if (clk_id >= 100)// over 100M
    {
        tmpclk = 0x0f;// set as 100M
        clk_id = 100;
        SystemClockParameter = 8;
    }
    else
    {	// Round clk to times of 10 to meet hardware limit.

        clk_id = clk_id/10;
        tmpclk = (clk_id-2);// get the close value; if 23M -> 20M setting
        if (tmpclk <= 0) tmpclk= 0;

        clk_id =clk_id * 10;
        SystemClockParameter = tmpclk;
    }

    LockPort_OPEN();
    if (clk_id > CpuClock)	//New clock faster, so change FLASH waitstate first.

        SetFlashWaitForMcuClock(clk_id);

    MCU_ADDR = OSC_CTRL;// 0x7C
    MCU_DATA = 0x01;// OSCEN = 1: enable 12M OSC
    MCU_DATA = tmpclk;

    MCU_ADDR = CLOCK_DIV;// 0x7D
    MCU_INDEX = 0x1;
    MCU_DATA = 0x0;// MCU_DIV ratio=1x. (0=1)
    MCU_DATA = clk_id/pre_TCON_DIV_clk; // 4;		// GLB_DIV; here = 10MHz

    if (clk_id < CpuClock)	//New clock faster, so change FLASH waitstate first.

        SetFlashWaitForMcuClock(clk_id);

    McuMainClock = clk_id;
    CpuClock = clk_id;	//Default MCU_DIV=1.

    ReadBack_Info_OSC();
    LockPort_CLOSE();

}

#endif

void SetFeedBackRegSetting(U8 cfeedback)
{
    U8 tmpfback;

    LockPort_OPEN();
    MCU_ADDR = CFB_SEL;// 0xE2
    for (tmpfback = 0; tmpfback<48; tmpfback++)
        MCU_DATA = cfeedback;
    LockPort_CLOSE();
}
void SelfScanInit(U16 sensing_count, U16 pulse_len,U8 CFB)
//void SetSelfScanPulses(U8 s_sense_seq, U16 sensing_count, U16 pulse_len)
// Note: Call this routine to setup the charge&transfer timing.
// Enter: The pulses_count means how many pulses in a channel's self-scan c&t times.
//		The pulse_len means how many micro-second for a single C&T pulse. The pulse is always 50% duty.
//		The setting applied to both ADCs.
{
	U16 tmpSHTotal/*,SGLB_DIV_Var*/;
    U8 SHTotal_High,SHTotal_Low,Sensing_High,Sensing_Low,Self_V_Total;
    U8 H_BP;

    CFB_self = CFB;
    H_BP = 3;
    Self_V_Total = self_sequ+2;
    tmpSHTotal = sensing_count + H_BP +1;//H_TOTAL > H_BP + sensing number
    SelfPulseTime = ((pre_TCON_DIV_clk*pulse_len)/40);//(8 * pulse_len)/(40);
    SelfPulseTime = (SelfPulseTime & 0x3F);//[5:0]: 6bits
	//SGLB_DIV_Var = (McuMainClock/pre_TCON_DIV_clk);


	SHTotal_Low = (tmpSHTotal%256);

    if ((tmpSHTotal>>8) > 0)
    {
        SHTotal_High = tmpSHTotal>>8;
    }
    else
    {
        SHTotal_High = 0x00;
    }


	Sensing_Low = (sensing_count%256);
    if ((sensing_count>>8) > 0)
    {
        Sensing_High = sensing_count>>8;
    }
    else
    {
        Sensing_High = 0x00;
    }
    LockPort_OPEN();
    MCU_ADDR = TCON_H_PARAM;// 0x30 - Self mode part
    MCU_DATA = SHTotal_Low; //160				// self - H Total(Low)
    MCU_DATA = SHTotal_High; //0;				// self - H Total(High)
    MCU_DATA = H_BP;// H_BP = 2
    MCU_DATA = Sensing_Low;//150;				// self - Sensing(Low)
    MCU_DATA = Sensing_High;//0;				// Self - Sensing(High

    MCU_ADDR = TCON_V_PARAM;// 0x31
    MCU_INDEX = 0x00;
    MCU_DATA = Self_V_Total ;//5;				// Self_V_Total > S_SEQ_NUM
    MCU_INDEX = 0x02;
    MCU_DATA = self_sequ ;// 3;			// Self Mode Sense seq times N = 1~8  (each time may sense more then 1 SH)

    LockPort_CLOSE();

}

void MutualScanInit(U16 musensing_count, U16 muPulseLen,U8 CFB)
//void SetMutualScanPulse(U8 mu_sense_seq,U16 musensing_count, U16 muPulseLen)
{
	U16 tmpMuTotal/*,MGLB_DIV_Var*/;
    U8 HMuTotal_High,HMuTotal_Low,MuSensing_High,MuSensing_Low;
    U8 Mu_V_Total, reg_mu_sense_seq;
    U8 H_BP;

    CFB_mutu = CFB;
    if (TestModeFlag)	{	H_BP = 5;	}
    else
    {
#ifdef MutuCCByNode
        H_BP = 5;
#else
        H_BP = 2;
#endif
    }

    tmpMuTotal = (musensing_count + H_BP + 1);
    MutuPulseTime = ((pre_TCON_DIV_clk*muPulseLen)/40);
    MutuPulseTime = (MutuPulseTime & 0x3F);
    //MGLB_DIV_Var = (McuMainClock/pre_TCON_DIV_clk);	  //AD20130725 killed.

    Mu_V_Total = TxChNumber + 2;// in e28 case: tx=10, total = 10+2 = 12;
    reg_mu_sense_seq = mutu_sequ-1;



	HMuTotal_Low = (tmpMuTotal%256);

    if ((tmpMuTotal>>8) > 0)
    {
        HMuTotal_High = tmpMuTotal>>8;
    }
    else
    {
        HMuTotal_High = 0x00;
    }

	MuSensing_Low = (musensing_count%256);

    if ((musensing_count>>8) > 0)
    {
        MuSensing_High = musensing_count>>8;
    }
    else
    {
        MuSensing_High = 0x00;
    }
    LockPort_OPEN();
    MCU_ADDR = TCON_H_PARAM;// 0x30 - Mutual mode part
    MCU_INDEX = 0x02;
    MCU_DATA = H_BP;// H_BP = 2
    MCU_INDEX = 0x05;
    MCU_DATA = HMuTotal_Low;
    MCU_DATA = HMuTotal_High;
    MCU_DATA = MuSensing_Low;
    MCU_DATA = MuSensing_High;

    MCU_ADDR = TCON_V_PARAM;// 0x31
    MCU_INDEX = 0x01;
    MCU_DATA = Mu_V_Total;// Mu_V_TOTAL > TX CH NUM
    MCU_INDEX = 0x03;
    MCU_DATA = TxChNumber;// Mutual TX channel number N= 0~47 (N= How many TX channel)

    MCU_ADDR = 0x4D; // 0x4D
    MCU_DATA = reg_mu_sense_seq;// Repeat time for each channel in mutual node, repeat_seq[1:0] = N+1;
    LockPort_CLOSE();
}

void SetChannelComOffset(U8 *offset_value)
{
    U8 offset_no;
    U8 x;
    offset_no = 48;

    LockPort_OPEN();

    MCU_ADDR = CC_SEL;     // 0xE1: self & mutual mode share same register 0xE1

    for (x=0; x<offset_no; x++) MCU_DATA = offset_value[x];  // total 48 bytes


    LockPort_CLOSE();
}
void SelfScanStart(S16 *pSelfTargetAddr)
{
    U8 tmp;

    ADCScan_Done = 0;
    bios_ScanMode = self_mode;
    OpenRawTestMode(0);

    SetChannelComOffset(SelfComOffset);
    SetFeedBackRegSetting(CFB_self);

    LockPort_OPEN();
    MCU_ADDR = CLOCK_DIV;// 0x7D
    MCU_DATA = SelfPulseTime;//8;			// TCON_DIV

    MCU_ADDR =  DMA_SELF_SETTING; // 0x51				    	//Self Baseline & Target Addr setting
    MCU_INDEX = 2;
    MCU_DATA  = lobyte(pSelfTargetAddr); //
    MCU_DATA  = (hibyte(pSelfTargetAddr)&0x3F);//

    MCU_ADDR = RC_SEL; // 0xE0;					// RC_SEL => Mutual: 0, Self: 1
    for (tmp=0; tmp < 12; tmp++)
    {
        MCU_DATA = 0x55;
    }

    if ( ((SelfADCEnALLBit&0x1) && (paraSELF_SING_ALL &0x1) && (cmdSELF_SING_ALL &0x1)) == 1 )
    {
        MCU_ADDR = ADC_EN_SETTING;// 0x3A
        MCU_DATA = 0x01;// [1]: EN-ALL
    }
    else
    {
        MCU_ADDR = ADC_EN_SETTING;// 0x3A
        MCU_DATA = 0x00;// b[0]=0, Single
    }

    MCU_ADDR = XDATA_DMA_CONTROL; 	// 0x50, xdata DMA control
    MCU_DATA = 0x01;  //  RAW_PROC=0,RAWDMA EN=1,
    LockPort_CLOSE();

    // Self Scan Start
    P0_3 = 1;
    _nop_();
    P0_2 = 1;
    _nop_();
    P0_2 = 0;
    _nop_();

}

void OpenRawTestMode(U8 switch_mode)
{
    // switch_mode : 0 auto mode
    // switch_mode : 1 debug mode
    U8 tmpvar;
    LockPort_OPEN();
    MCU_ADDR = SPEC_FUNC_FA;//0xFA;
    MCU_INDEX = 0x00;
    tmpvar = MCU_READ;
    MCU_INDEX = 0x00;
    if (switch_mode==1)
        MCU_DATA = (tmpvar |0x10);// RAW test mode	// debug mode
    else
        MCU_DATA = (tmpvar & 0xEF);
    LockPort_CLOSE();
}

extern U8 MutualForceMCC;
void MutualCC_by_Tx(U8 TxIndex) // 0.
{
    U8 tmp_i,_end;
    U16 addr_cc ;
    U8* pMutualComOffset;
    LockPort_OPEN();

    addr_cc = TxIndex*RxChNumber;
    _end = TxChNumber+RxChNumber;
    if (TestModeFlag)
        pMutualComOffset	 = (U8*)ptrLastQuaDelta;
    else
        pMutualComOffset	 = MutualComOffset;
    MCU_ADDR = 0xE1;

    for (tmp_i = TxChNumber; tmp_i < _end; tmp_i++)
    {
        //rx_offset
        MCU_INDEX = ChMapping[tmp_i];
		/*	// 13-0915A killed
        if (MutualForceMCC)

        {	// Force CC to be the same value


            MCU_DATA = MutualForceMCC;		// Andy for China version.

        }

        else
		*/
        {	// Use AutoCC results.

            MCU_DATA = pMutualComOffset[addr_cc ];
            addr_cc ++;

        }
        //MCU_DATA = pMutualComOffset[addr_cc ];
        //addr_cc ++;
    }

    LockPort_CLOSE();
}

#if WP1004_B
//void MutualScanStart(S16 *pMutuTargetAddr,S16 *pMubaselineAddr)
void MutualScanStart(S16 *pMutuTargetAddr)
#else
void MutualScanStart(U8 *pMutuTargetAddr,U8 *pMubaselineAddr)
#endif
{
    U8 tmp;

    ADCScan_Done = 0;
    MuINTCount = 0;
    bios_ScanMode = mutual_mode;
    if (TestModeFlag)	{OpenRawTestMode(1);}
    else
    {
#ifdef MutuCCByNode
        OpenRawTestMode(1);
#else
        OpenRawTestMode(0);
#endif
    }

    MutualCC_by_Tx(0);
    SetFeedBackRegSetting(CFB_mutu);

    LockPort_OPEN();
    //@
    MCU_ADDR = CLOCK_DIV;// 0x7D
    MCU_DATA = MutuPulseTime;// TCON_DIV

    MCU_ADDR =  DMA_MUTUAL_SETTING;//Mutu Baseline & Target Addr setting
    MCU_INDEX = 2;
    MCU_DATA  = lobyte(pMutuTargetAddr);//
    MCU_DATA  = (hibyte(pMutuTargetAddr)&0x3F);	//
    MCU_DATA  = RxChNumber;

    MCU_ADDR = ADC_EN_SETTING;// 0x3A
    MCU_DATA = 0x00;// b[0]=0, non-sensing channels set to LOW while EN_ALL = 0;

    MCU_ADDR = XDATA_DMA_CONTROL; 	// 0x50, xdata DMA control
    MCU_DATA = 0x01;  //  RAW_PROC=1,RAWDMA EN=1,

    MCU_ADDR = RC_SEL; // 0xE0;					// RC_SEL => Mutual: 0, Self: 1
    for (tmp=0; tmp < 12; tmp++)
    {
        MCU_DATA = 0x00;
    }

	/*for hopping:Yuho*/	
	MCU_ADDR = 0x4C;
	MCU_DATA = ChMapping[0];//MutuTxSeq_0x4C[0];
	MCU_DATA = ChMapping[1];//MutuTxSeq_0x4C[1];
	MCU_ADDR = TCON_V_PARAM;// 0x31
	MCU_INDEX = 0x01;
	MCU_DATA = TxChNumber+2;// Mu_V_TOTAL > TX CH NUM
	MCU_INDEX = 0x03;
	MCU_DATA = TxChNumber;// Mutual TX channel number N= 0~47 (N= How many TX channel)
	/*for hopping:Yuho*/	
	
	


    // ------- 0x81 for dma reset --------
    MCU_ADDR = SCAN_STATUS; // 0x81, SCAN_STATUS
    MCU_DATA = 0x01;
    MCU_ADDR = SCAN_STATUS;
    MCU_DATA = 0x03;

    LockPort_CLOSE();

    // Mutual Scan Start
    P0_2=1;
    _nop_();
    P0_3=1;
    _nop_();
    P0_3=0;
    _nop_();
}

U8 Tcounter;
bit STflag;
void SystemTimerInit(U8 intr_mode, U16 period)	// period: 1 unit = 1us.
{
    U16 Tperiod;

    TR0=0;// Disable counting.

    if (period >= 10000)
    {
        STflag = 1;
        Tcounter = 10;
        Tperiod = period/10;
    }
    else
    {
        STflag = 0;
        Tperiod = period;
        Tcounter = 1;
    }

    CKCON &= 0xF7;//Counter base, b3: 1:=MCU_CLK/4, 0:=MCU_CLK/12
    //CKCON |= 0x08;					//
    TMOD = (TMOD & 0xf0) | 0x01; // Set Timer0, Enabled by TR0, clocking and Mode 1.
    if (intr_mode) ET0=1; // Enable interrupt mode.
    else ET0 = 0; // Disable interrupt, use polling method
    TF0=0;// Clear status.

    //timer0_CntrValue= ((long)Tperiod * (U16) McuMainClock ) / 12; //McuMainClock  // timer clock = mcu_clk / 12;
    timer0_CntrValue= ((long)Tperiod * (U16) CpuClock ) / 12; //McuMainClock  // timer clock = mcu_clk / 12;
    timer0_CntrValue = ~timer0_CntrValue;	 // Invert value because 8051 Timer is up-counting.
    TH0= hibyte(timer0_CntrValue);
    TL0= lobyte(timer0_CntrValue);
}


void SystemTimerReload(void)
{
    TH0= hibyte(timer0_CntrValue);
    TL0= lobyte(timer0_CntrValue);
    Tcounter--;
}

void NoiseIndexSet(void)
{
	NLIndex = ((++NLIndex)%9);
	NLPulseTime = UsedCTFreq[NLIndex/3]+(NLIndex%3)-1;

}

void NoiseListenScanStart(void)
{
    U8 tmp;

	ADCScan_Done = 0;
	bios_ScanMode = NL_mode;	

	OpenRawTestMode(0);

	LockPort_OPEN();


	//@
	MCU_ADDR = CLOCK_DIV;// 0x7D
	MCU_DATA = NLPulseTime;	// TCON_DIV

	MCU_ADDR = CC_SEL;     // 0xE1: self & mutual mode share same register 0xE1
	for (tmp=0; tmp<48; tmp++)
	  	//MCU_DATA = MutualForceMCC ; //8;  // total 48 bytes
	  	MCU_DATA = 8;  // fixrd by poly for noiselistening fake channels

	
	
	MCU_ADDR =  DMA_MUTUAL_SETTING; //Mutu Baseline & Target Addr setting
	MCU_INDEX = 2;
	MCU_DATA  = lobyte(NoiseTargetAddr);//
	MCU_DATA  = (hibyte(NoiseTargetAddr)&0x3F);	//


	/*for hopping:Yuho*/	
	MCU_ADDR = 0x4C;
	MCU_DATA = 63;
	MCU_DATA = 63;
	MCU_ADDR = TCON_V_PARAM;// 0x31
	MCU_INDEX = 0x01;
	MCU_DATA = 3;// Mu_V_TOTAL > TX CH NUM
	MCU_INDEX = 0x03;
	MCU_DATA = 2;// Mutual TX channel number N= 0~47 (N= How many TX channel)
	/*for hopping:Yuho*/

	// ------- 0x81 for dma reset --------
	MCU_ADDR = SCAN_STATUS; 	// 0x81, SCAN_STATUS
	MCU_DATA = 0x01;  
	MCU_ADDR = SCAN_STATUS;
	MCU_DATA = 0x03;  
	
	LockPort_CLOSE();

	// Mutual Scan Start	
	P0_2=1;
	_nop_();
	P0_3=1;
	_nop_();
	P0_3=0;
	_nop_();	
}


//============ BEGIN: Timer1 for User ========================================================

U16 timer1_CntrValue;
U8 Tcounter1;
bit Timer1OnTime, STflag1;

void SystemTimer1Reload(void)
{
	TH1= hibyte(timer1_CntrValue);
	TL1= lobyte(timer1_CntrValue);
	Tcounter1--;
}

void Timer1_ISR (void) interrupt 3
{
	TF1=0;
	if (Tcounter1==0)
	{
     Timer1OnTime = 1;
       if (STflag1 == 1)
           Tcounter1 = 10;
		else 
			Tcounter1 = 1;
	}
	else
	{
		SystemTimer1Reload();
		SystemTimer1Start();
	}
}

void SystemTimer1Init(U16 period)		// period: 1 unit = 1us.
{
	U16 Tperiod;

	TR1=0;					// Disable counting.

	if (period >= 10000)
	{
		STflag1 = 1;
		Tcounter1 = 10;
		Tperiod = period/10;
	}
	else
	{
		STflag1 = 0;
		Tperiod = period;
		Tcounter1 = 1;
	}

	CKCON &= 0xEF;				//Counter base, b4: 1:=MCU_CLK/4, 0:=MCU_CLK/12
	TMOD = (TMOD & 0x0f) | 0x10;		// Set Timer1, Enabled by TR1, clocking and Mode 1.
	ET1=1; 					// Enable interrupt mode.
	TF1=0;					// Clear status.

	timer1_CntrValue= ((long)Tperiod * (U16) CpuClock ) / 12;  // timer clock = mcu_clk / 12;	
	timer1_CntrValue = ~timer1_CntrValue;	 // Invert value because 8051 Timer is up-counting.
	TH1= hibyte(timer1_CntrValue);
	TL1= lobyte(timer1_CntrValue);
}
//============ END: Timer1 for User ========================================================
void adc_rcv_intr(void) interrupt 0 using 1
{
	U8 tmpvar;	
	IE0 = 0;					// Clear INT0 flag
 	//EX0= 0; 	

#ifdef MutuCCByNode
	if(bios_ScanMode==self_mode|| bios_ScanMode==NL_mode)
#else
	if( !((bios_ScanMode==mutual_mode) && (TestModeFlag)) )
#endif	 	 	
	{
		if(bios_ScanMode==self_mode || bios_ScanMode==NL_mode)
		{
			ADCScan_Done=1;				
		}else if(bios_ScanMode==mutual_mode){
			if(startnoiselisten==1)
			{
				NoiseIndexSet();
				NoiseListenScanStart();			
			}else
				ADCScan_Done=1;				
		}
		
	}
	else  // mutual_mode & TestMode
	{	
		if(MuINTCount<TxMuSequ) //   
		{
			if((MuINTCount %mutu_sequ)==(mutu_sequ -1))
			{			
				MutualCC_by_Tx((MuINTCount/mutu_sequ)+1);								
			}
			if(MuINTCount==TxMuSequ_1) // 
			{
				LockPort_OPEN();
				MCU_ADDR = SPEC_FUNC_FA;
				tmpvar = MCU_READ;
				MCU_INDEX = 0x00;				
				MCU_DATA = (tmpvar & 0xEF);
				LockPort_CLOSE();			
			}
			MuINTCount++;
		} 
		else
		{		
			if(startnoiselisten==1)
			{
				NoiseIndexSet();
				NoiseListenScanStart();			
			}else
			{
				ADCScan_Done=1;				
			}	
		}			
	} 	  		
}

void WatchDog_Intr(void) interrupt 0x0c using 2
{
    // wdt key
    TA=0xaa;
    TA=0x55;
    WDIF = 0;		// clear wdt flag
}

// ####################################################
// WatchDog Timer Setting
// WD[1:0] = 00: 131072 clock =  2.048 ms
// WD[1:0] = 01: 1048576 clock = 16.384 ms
// WD[1:0] = 10: 8388608 clock =  131.072 ms
// WD[1:0] = 11: 67108846 clock = 1048.576 ms
// ####################################################



void WatchDog_Initialize(U8 En_bit)

{

    if (En_bit)

    {

        EWDI = 1;		// enable wdt interrupt



        TA = 0xAA;		// wdt key

        TA = 0x55;

        WDCON = 0x03;



        //CKCON = CKCON & 0X3f;	// WD[1:0] = 00

        CKCON = CKCON | 0XC0;	// WD[1:0] = 10

    }

    else

    {

        EWDI = 0;

        TA = 0xAA;		// wdt key

        TA = 0x55;

        WDCON = 0x01;

    }

}


#if 0
void MCU_Stop_mode(void)
{
    PCON = 0x02;		// MCU stop
    _nop_();
    _nop_();
}

void DTM_Init(U8 setDTM_H, U8 setDTM_L)
{
    DTM_H = setDTM_H;
    DTM_L  = setDTM_L;
    PDTM = 0x05;
}

void DTM_Start(void)
{
    DTM_EN = 1;			//enable
}

void DTM_Timer_Int(void) interrupt 6 using 3
{
    DTM_EN = 0;
    EIF = 0x10;
}
#endif

//void switch_MCUDIV_FreqMode(U8 paraMCU_DIV, U8 PowerDnOn)

extern U16 Tperiod;
void switch_MCUDIV_FreqMode(U8 paraMCU_DIV)
{
    U8 paraMCUDIV, new_mcu_clk;



    paraMCUDIV = paraMCU_DIV & 0x0f;


    new_mcu_clk = McuMainClock/paraMCU_DIV;



    LockPort_OPEN();


    if (new_mcu_clk > CpuClock)	//New clock faster, so change FLASH waitstate first.

        SetFlashWaitForMcuClock(new_mcu_clk);



    MCU_ADDR = CLOCK_DIV;		// 0x7D
    MCU_INDEX = 0x1;
    MCU_DATA = paraMCUDIV;



    if (new_mcu_clk < CpuClock)	//New clock slower, so change FLASH waitstate here.

        SetFlashWaitForMcuClock(new_mcu_clk);



    CpuClock = new_mcu_clk;	//Update the record.





#if 0
    if (PowerDnOn == 1)				// power down = 1
    {
        MCU_ADDR =ADC_EN_SETTING;		//0x3A
        MCU_DATA = 0x70;				// power down analog block
        MCU_ADDR = SCAN_STATUS;
        MCU_DATA = 0x01;				// disable auto-scan logic
    }
    else
    {
        MCU_ADDR =ADC_EN_SETTING;		//0x3A
        MCU_DATA = 0x01;				// power on analog block
        MCU_ADDR = SCAN_STATUS;
        MCU_DATA = 0x03;				// Enable auto-scan logic
    }
#endif
    LockPort_CLOSE();
}

// ####################################################
// HARDWARE UART Baud Rate Setting
// ####################################################
#ifdef	PRINTFUNCTION
void HWUART_INIT(void)
{
    unsigned char rdata;

    buf_len = 0;
    t_in = 0;
    t_out = 0;

    P0_1 = 0;

//	MCU_ADDR = WAIT_CLK_CNT;
//	MCU_DATA = 0x04;

    CKCON =0x01;

    // set system clk and mcu clk frequence
//	MCU_ADDR = 0x7C;
//	MCU_DATA = 0x01;
//	MCU_DATA = 0x01;
//
//	MCU_ADDR = 0x7D;
//	MCU_DATA = 0x08;
//	MCU_DATA = 0x00;
//	MCU_DATA = 0x04;

    // set UART enable
    MCU_ADDR = SPEC_FUNC_FA;// 0xFA;
    MCU_INDEX = 0x00;
    rdata = MCU_READ;
    MCU_INDEX = 0x00;
    MCU_DATA = rdata| 0x01; // UART_EN
    P0_1 = 1;

    // set UART
    // Baudrate115200
    UEN   = 0x10; 	// bit4 : 1->HW UART  0->8051 UART

    if (SystemClockParameter == 1)	// 3x MHz
    {
        //UBR_H = 0x01;	//  33.29 M
        //UBR_L = 0x21;	//21
        UBR_H = 0x01;
        UBR_L = 0x04;
    }
    else if (SystemClockParameter == 4)	// 6xMHz
    {
        //UBR_H = 0x02;	//    yc 61M
        //UBR_L = 0x11;	//    yc
        UBR_H = 0x02;
        UBR_L = 0x09;

    }
    else if (SystemClockParameter == 0)			// 2x MHz,
    {											// HW no trim
        UBR_H = 0x00;	//    22M
        UBR_L = 0xBF;	//
    }
    // set interrupt enable
    EA = 1;
    EX4 = 1;

}
#endif

//#define CBYTE ((unsigned char volatile code *) 0)
void ReadBack_Info_OSC()
{
// information blobk read back data
// = {ff,ff,ff,ff, ed,00,0c,00, ff,ff,ff,ff, ff,ff,ff,ff, ed,00,10,00, ff,ff,ff,ff, ff,ff,ff,ff, ff,ff,ff,ff, ff,ff,ff,ff, ed,01,0d,00}
    U8 IB_OSCtmp,IB_500Ktmp;

    IB_OSCtmp = SystemClockParameter *4+2;
    IB_500Ktmp =  9*4 +2;

    //if ((CBYTE[0x8000+IB_OSCtmp] && CBYTE[0x8000+IB_500Ktmp]) != 0xff)		// if 0xff, no trim.

    if ((CBYTE[0x8000+IB_OSCtmp]!=0xff) && (CBYTE[0x8000+IB_500Ktmp] != 0xff) )		// if 0xff, no trim.
    {
        MCU_ADDR = OSC_TRIM;					// 0xED
        MCU_DATA = CBYTE[0x8000+IB_OSCtmp];	// OSC parameter
        MCU_DATA = CBYTE[0x8000+IB_500Ktmp];	// 500K parameter
    }
}

#ifdef	PRINTFUNCTION
void HWUART_INT(void) interrupt 0x09// using 3
{
    UEN   = 0x10; //disable interrupt

    if (buf_len != 0)
    {
        USDATA= tbuf[t_out];
        //P2 = USDATA;
        t_out++;
        t_out = (t_out&(TBUF_SIZE-1));
        buf_len--;
        UEN = 0x11;
    }
}
// YC

unsigned char uart_putchar (unsigned char c)
{
    EA = 0;

    if (buf_len == TBUF_SIZE)
    {
        EA = 1;
        return (-1);
    }

    //calculate the buf length

    tbuf [t_in] = c;
    t_in++;
    t_in = (t_in&(TBUF_SIZE-1));
    buf_len++;
    //P1 = buf_len;

    if (buf_len == 1 && UEN == 0x10)
    {
        //P1_IO = 0;
        //P1 = tbuf[t_out];

        USDATA = tbuf[t_out];
        t_out++;
        t_out = (t_out&(TBUF_SIZE-1));
        buf_len--;
        UEN= 0x11;
    }

    EA = 1;

    return (0);
}
char putchar (char c)
{
    volatile unsigned int i;
    //P2 = c;
    while (uart_putchar (c) != 0)
    {
        for (i=0; i<1000; i++)
        {
            i= i	;
        }
    }

    return (c);
}
#endif

U8  data BIT8_MASK[8]  ={	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
U8 data BIT8_MASK_0[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
void key_set(U8 mode,U8 key_num,U16 *mukeyArray)
{
    // mode : 0x81=> self ,  0x82 => mutual

    gKey_mode = mode;
    gkey_num = key_num;
    gmukeyArray = mukeyArray;

}
void AutoSelfCalibration(S16 *pTargetAddr)
//void AutoSelfCalibration(S16 *pTargetAddr,S16 *pBaselineAddr)
{
    // mode = 0 => self
    // mode = 1 => mutual
    U8 data ii ,_byte,_bit,_start,_end;//JStest
    U8 data Total,count,cc_value;//JStest
    U8 data CH_flag[5] ={0,0,0,0,0} ; //JStest
    U8 *ptrComOffset;
    U16 data cc_min,cc_max,node; //JStest

    ptrComOffset = SelfComOffset;
    _start = 0;
    _end = TxChNumber+RxChNumber;
    if (gKey_mode==0x81)
    {
        _end +=gkey_num;
    }
    Total = _end;
 //   cc_min = 2000;
 //   cc_max = 2400;
   cc_min = 700;	//1000
   cc_max =1000;	//1200

    node = 48;

    count = 0;
    cc_value = 1;
    for (ii =0;ii<node;ii++)	{ptrComOffset[ii] = cc_value;} // clear CC
    //---drop 2 frames
    //SetChannelComOffset(ptrComOffset);
    SelfScanStart(pTargetAddr);				// start self scan, feed sense seq = 3;
    while (!ADCScan_Done) {};

    SelfScanStart(pTargetAddr);				// start self scan, feed sense seq = 3;
    while (!ADCScan_Done) {};



    //--- self mode ---
    while (Total>count)
    {
        //SetChannelComOffset(ptrComOffset);
        SelfScanStart(pTargetAddr);				// start self scan, feed sense seq = 3;
        while (!ADCScan_Done) {};

        cc_value ++;
        if (cc_value > 127)	break;

        for (ii =_start;ii<_end;ii++)
        {
            _byte =  ii>>3;
            _bit =  ii &0x7;
            if ( ( CH_flag[ _byte] & BIT8_MASK[_bit] ) ==0 )
            {
                if ((pTargetAddr[ii] <= cc_max) && (pTargetAddr[ii] >= cc_min))
                {
                    CH_flag[ _byte] |= BIT8_MASK[_bit];
                    count ++;
                }
                else if ((pTargetAddr[ii] < cc_min))
                {
                    CH_flag[ _byte] |= BIT8_MASK[_bit];
                    count ++;
                }
                else
                {
                    ptrComOffset[ ChMapping[ii] ] = cc_value;
                }
            }

        }
    }

#if ShowAUTOCC_Self
    printf("SelfComOffset = \r\n");
    for (ii =_start;ii<_end;ii++)
    {
        printf("0x%bx,",ptrComOffset[ ChMapping[ii] ]);
    }
    printf("\r\n");
#endif
    //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

}
#ifdef AUTOCC_MODULE
#if WP1004_B
//void AutoMutuCalibration(S16 *pTargetAddr,S16 *pBaselineAddr)
void AutoMutuCalibration(S16 *pTargetAddr,U16 MinRaw,U8 offset)
#else
void AutoMutuCalibration(U8 *pTargetAddr,U8 *pBaselineAddr)
#endif
{
    U8 data _byte,_bit; //JStest
    U8 data _byteA,_bitA; //JStest
    U8 data cc_value; //JStest
    U8 CH_flag[64] ;
    U8 *ptrComOffset;
    U16 data cc_min,node,ii,_start,_end; //cc_max ,count ,Total //JStest
    U8 data nRxCH_Done[3] = {0,0,0}; //JStest
    U8 data DoRxCH[3]; //JStest
    U8 tmp;
    for (ii = 0;ii<64;ii++)
    {
        CH_flag[ii] = 0;
    }
    node = TxChNumber*RxChNumber;
    //count = 0;
    if (gKey_mode==0x82)
    {
        // --- set no use node  flag to 1 --- //
        for (ii = 0;ii<RxChNumber;ii++)
        {
            cc_min = (TxChNumber-1)*RxChNumber + ii;
            _byte =  cc_min>>3;
            _bit =  cc_min &0x7;
            CH_flag[ _byte] |= BIT8_MASK[_bit];
        }
        for (ii = 0;ii<gkey_num;ii++)
        {
            _byte =  gmukeyArray[ii]>>3;
            _bit =  gmukeyArray[ii] &0x7;
            CH_flag[ _byte] &= ~BIT8_MASK[_bit];
        }
        //Total = (TxChNumber-1)*RxChNumber + gkey_num;
    }
    else if (gKey_mode==0x83)
    {
        // --- set no use node  flag to 1 --- //
        for (ii = 0;ii<TxChNumber;ii++)
        {
            cc_min = (ii+1)*RxChNumber -1;
            _byte =  cc_min>>3;
            _bit =  cc_min &0x7;
            CH_flag[ _byte] |= BIT8_MASK[_bit];
        }
        for (ii = 0;ii<gkey_num;ii++)
        {
            _byte =  gmukeyArray[ii]>>3;
            _bit =  gmukeyArray[ii] &0x7;
            CH_flag[ _byte] &= ~BIT8_MASK[_bit];
        }
        //Total = TxChNumber*(RxChNumber-1) + gkey_num;

    }
    else
    {
        //Total = node;
    }
    ptrComOffset = MutualComOffset;
    _start = 0;
    _end = node;

    cc_min = MinRaw;
    cc_value = offset;
    MutualScanStart(pTargetAddr);
    while (!IsEndOfScan()) {};
    MutualScanStart(pTargetAddr);
    while (!IsEndOfScan()) { };
    for (ii =0;ii<RxChNumber;ii++)	{ptrComOffset[ii] = cc_value;} // clear CC
    for (ii =0;ii<RxChNumber;ii++)
    {
        _byte =  (U8)(ii>>3);
        _bit =  (U8)(ii &0x7);
        nRxCH_Done[_byte]  |= BIT8_MASK[_bit]; // all nRxCH_Done set 1
    }
    while (1) // Total>count
    {

        MutualScanStart(pTargetAddr);
        while (!IsEndOfScan());
        cc_value ++;
        if (cc_value > 0x7F)
        {
            //printf("MU CC break");
            break;
        }
        for (ii =0;ii<3;ii++) {DoRxCH[ii] = 0;}
        for (ii =_start;ii<_end;ii++)		// end = node
        {
            _byte =  (U8)(ii>>3);
            _bit =  (U8)(ii &0x7);

            tmp = (U8)(ii%RxChNumber);
            _byteA =  (U8)(tmp>>3);		// _byteA = row in 2-bytes,
            _bitA = (U8)(tmp &0x7);		// node in 8-bits
            if ( ( CH_flag[ _byte] & BIT8_MASK[_bit] ) ==0 )
            {
                if ( (( DoRxCH[ _byteA] & BIT8_MASK[_bitA] ) ==0) && ( nRxCH_Done[ _byteA] & BIT8_MASK[_bitA]  ) )
                {
                    if ( pTargetAddr[ii] < cc_min)
                    {
                        ptrComOffset[ tmp ] = cc_value;
                        DoRxCH[ _byteA] |= BIT8_MASK[_bitA];
                    }
                }
            }
        }
        for (ii =0;ii<RxChNumber;ii++)
        {
            _byteA =  (U8)(ii>>3);
            _bitA =  (U8)(ii &0x7);
            if (( DoRxCH[ _byteA] & BIT8_MASK[_bitA] ) ==0)
            {
                nRxCH_Done[_byteA]  &= BIT8_MASK_0[_bitA];
            }
        }

        ii= nRxCH_Done[0]+ nRxCH_Done[1]+nRxCH_Done[2] ;
        if (ii==0)
        {
            break;
        }
    }

#if ShowAUTOCC_Mutu
    printf("MutualComOffset = \r\n");
    for (ii =0;ii<RxChNumber;ii++)
    {
        //MutualComOffset[ii] = MutualComOffset[ii] -1;
        printf("0x%bx,",ptrComOffset[ii]);
    }
    printf("\r\n");
#endif
}
#if WP1004_B
void AutoMutuCalibration_byNode(S16 *pTargetAddr,U16 MinRaw,U8 offset)
#else
void AutoMutuCalibration_byNode(U8 *pTargetAddr,U8 *pBaselineAddr)
#endif
{
    U8 data _byte,_bit;	//JStest
    U8 data cc_value; //JStest
    U8 CH_flag[64] ;
    U8 *ptrComOffset;
    U16 data cc_min,node,ii,_start,_end,Total,count; //cc_max //JStest
    //U8 tmp;
    U8 data tmpA[5] = {0,0,0,0,0}; //JStest
    for (ii = 0;ii<64;ii++)
    {
        CH_flag[ii] = 0;
    }
	node = TxChNumber*RxChNumber;
    count = 0;
    if (gKey_mode==0x82)
    {
        // --- set no use node  flag to 1 --- //
        for (ii = 0;ii<RxChNumber;ii++)
        {
            cc_min = (TxChNumber-1)*RxChNumber + ii;
            _byte =  cc_min>>3;
            _bit =  cc_min &0x7;
            CH_flag[ _byte] |= BIT8_MASK[_bit];
        }
        for (ii = 0;ii<gkey_num;ii++)
        {
            _byte =  gmukeyArray[ii]>>3;
            _bit =  gmukeyArray[ii] &0x7;
            CH_flag[ _byte] &= ~BIT8_MASK[_bit];
        }
        Total = (TxChNumber-1)*RxChNumber + gkey_num;
    }
    else if (gKey_mode==0x83)
    {
        // --- set no use node  flag to 1 --- //
        for (ii = 0;ii<TxChNumber;ii++)
        {
            cc_min = (ii+1)*RxChNumber -1;
            _byte =  cc_min>>3;
            _bit =  cc_min &0x7;
            CH_flag[ _byte] |= BIT8_MASK[_bit];
        }
        for (ii = 0;ii<gkey_num;ii++)
        {
            _byte =  gmukeyArray[ii]>>3;
            _bit =  gmukeyArray[ii] &0x7;
            CH_flag[ _byte] &= ~BIT8_MASK[_bit];
        }
        Total = TxChNumber*(RxChNumber-1) + gkey_num;

    }
    else
    {
        Total = node;
    }
#ifdef MutuCCByNode
    ptrComOffset = (U8*)MutualComOffset; // MutualComOffset
#else
    ptrComOffset = (U8*)ptrLastQuaDelta; // MutualComOffset
#endif
    _start = 0;
    _end = node;

    cc_min = MinRaw;
    cc_value = offset;
    MutualScanStart(pTargetAddr);
    while (!IsEndOfScan()) {};
    MutualScanStart(pTargetAddr);
    while (!IsEndOfScan()) { };
    for (ii =0;ii<node;ii++)	{ptrComOffset[ii] = cc_value;} // clear CC

    while (Total>count)
    {

        MutualScanStart(pTargetAddr);
        while (!IsEndOfScan());
        cc_value ++;

        if (cc_value > 0x7F)
        {
            //printf("MU CC break");
            break;
        }

        for (ii =_start;ii<_end;ii++)
        {
            _byte =  (U8)(ii>>3);
            _bit =  (U8)(ii &0x7);
            if ( ( CH_flag[ _byte] & BIT8_MASK[_bit] ) ==0 )
            {
                if ( pTargetAddr[ii] >= cc_min)	//(pTargetAddr[ii] <= cc_max) &&
                {
                    CH_flag[ _byte] |= BIT8_MASK[_bit];
                    count ++;
                }
                else
                {
                    ptrComOffset[ ii ] = cc_value;
                }
            }
        }
    }

#if ShowAUTOCC_Mutu
    printf("MutualComOffset Node = \r\n");
    for (ii =_start;ii<_end;ii++)
    {
        printf("0x%bx,",ptrComOffset[ii]);
        if ((ii%RxChNumber)==(RxChNumber-1))
            printf("\r\n");
    }
#endif
}
#endif	// AUTOCC_MODULE



#if 0
void Sleep_In(void)
{
    LockPort_OPEN();
    MCU_ADDR = SPEC_CONFIG_F6;
    MCU_DATA = 0x00;

    MCU_ADDR = ADC_EN_SETTING;
    MCU_DATA = 0x70;

    MCU_ADDR = LVDD_DETECT;
    MCU_DATA = 0x80;

    MCU_ADDR = PUMP_CONTROL;
    MCU_DATA = 0x00;

    MCU_ADDR = SCAN_STATUS;
    MCU_DATA = 0x00;
    LockPort_CLOSE();
    return ;
}

#endif
#if 0
void NoiseFilterSetting(U8 Enable,U8 NTH_H, U8 NTH_L,U8 NTH_INIT)
{
    LockPort_OPEN();
    if (Enable)
    {
        MCU_ADDR = FILTER_SETTING;   // 0xB0
        MCU_DATA = 0x11;     // by TH setting

        MCU_ADDR = FILTER_TH_SETTING;  // 0xB1
        MCU_DATA = (NTH_H & 0x7F);
        MCU_DATA = (NTH_L & 0x7F);
        MCU_DATA = (NTH_INIT & 0x7F);  // 7 bits
    }
    else
    {
        MCU_ADDR = FILTER_SETTING;   // 0xB0
        MCU_DATA = 0x00;     // by TH setting
    }
    LockPort_CLOSE();
}
#endif
void SenseCHControl()
{
    U8 aa,_byte,_bit;
    U8 SenseChByte[6] = {0x00,0x00,0x00,0x00,0x00,0x00};

    for (aa=0; aa<(TxChNumber+RxChNumber); aa++)
    {
        _byte = ChMapping[aa]>>3;
        _bit = ChMapping[aa]&0x7;
        SenseChByte[_byte] = SenseChByte[_byte] | BIT8_MASK[_bit];
    }

    MCU_ADDR = Sense_channel_Control;  //0x36
    for (aa=0; aa<6; aa++)
        MCU_DATA = SenseChByte[aa];

}



#ifdef flashpara
extern code U8 flSelf_sequ;
extern code U8 flMutu_sequ;
extern code U8 ChMapping[];
extern code U8 AdcCycSet[12][6];
extern code U8 SelfAdcOffset_0x54[];
extern code U8 MutuAdcOffset_0x55[];
extern code U8 MutuTxSeq_0x4C[];


void ADC_CYC_SETTING_GEN()
{
    U8 aa,bb;
    self_sequ = flSelf_sequ;		// no use
    mutu_sequ = flMutu_sequ;
    // ---ADC CYC Setting 0x40~0x4B---//
    MCU_ADDR =  0x40;
    for (aa = 0;aa<12;aa++)
    {
        for (bb = 0;bb<6;bb++)
        {
            MCU_DATA = AdcCycSet[aa][bb];
        }
        MCU_ADDR ++;
    }

    // --- Self Mode ADC Offset Value 0x54---//
    MCU_ADDR =  0x54;
    for (aa = 0;aa<48;aa++)
    {
        MCU_DATA = SelfAdcOffset_0x54[aa];
    }


    // ---Mutual Mode ADC Offset Value 0x55---//
    MCU_ADDR =  0x55;
    for (aa = 0;aa<48;aa++)
    {
        MCU_DATA = MutuAdcOffset_0x55[aa];
    }

    // ---Mutual Mode TX Sequence 0x4C---//
    MCU_ADDR =  0x4C;
    for (aa = 0;aa<32;aa++)
    {
        MCU_DATA = MutuTxSeq_0x4C[aa];
    }
}
#endif



//=================  UART END ===================================================
