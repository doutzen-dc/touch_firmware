/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"
#include "T1007_reg.h"


int putchar(int c);


void drv_mcu_timer(U8 t0, vU16 rate0)
{

	Timer0_ReportRateBound = (U16) ((1000/rate0) >> 1 );	// (1000 / 100Hz)/2ms = 5 times

	//Timer 0 Setting
	if (t0)
	{
		MCU_INTEN0 |= BIT1;		// Enable INT0
		TAR = 0x00;
		CCTL0 = CCIE;
		TACTL = TASSEL_2 + MC_2 + TAIE + ID_2; // ACLK, contmode, TAIE enabled
		CCR0 = 50000;
	}
	else
	{
		MCU_INTEN0 &= ~BIT1;	// Disable INT0
	}
}


void drv_i2c_init(void)
{
	U8 rdtmp,i2ctmpb1;

	i2ctmpb1 = (SensorDefine.I2C_Device_Address & 0x03) >> 1;

    MCU_ADDR = 0xFA;
    MCU_INDEX = 0x00;
    rdtmp = MCU_READ;
	if (i2ctmpb1)
	{
		rdtmp = ( rdtmp | 0x28);  // A0 _reg enable
	}
	else
	{
		rdtmp = ( rdtmp | 0x20);  // A0 _reg enable
	}
    MCU_INDEX = 0x00;
    MCU_DATA = rdtmp;


	MCU_ADDR = 0xF1; 	//REG_I2C_ADDR_CTRL;
	MCU_DATA = SensorDefine.I2C_Device_Address;	//DEV_SLV_ADDR;

	MCU_ADDR = 0x6D;	//REG_IFDMA_MODE;
	MCU_DATA = 0x01;

	MCU_ADDR = 0xEF;	//REG_IO_ADJ;
	MCU_DATA = 0x01;	//Set DE_GLITCH_EN

	MCU_ADDR = 0xF6;	//REG_SPECIAL_CONFIG;
	MCU_INDEX = 0x00;
	MCU_DATA = (0x01 | (HwSetting.I2CEnableAll<<2));

	I2cRegKeyEnable = 0x00;
	I2cStatus = 0x00;
	I2cDataIdx = 0x00;
	I2cUnLockMaches = 0x00;
#if PROTOCOL_V20
	pI2cBuffer = I2cDataBuff;
#endif
	MCU_INTEN1 |= 0x20;	//enable i2c IF interrupt
}

void drv_mcu_uart_init(U8 ONOFF)
{
	U8 rdtmp;
	
  	buf_len = 0;
	t_in = 0;
	t_out = 0;
	rbuf_len = 0;
	r_in = 0;
	r_out = 0;


	//Osc setting
	MCU_ADDR = 0x7C;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x01;
	MCU_DATA = 0x02;//02//0x03->OSC=50MHz (0x02=FPGA07=20MHz)

	//Divide setting
	MCU_ADDR = 0x7D;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x14; //TCON_DIV
	MCU_DATA = 0x00; //MCU_DIV
	MCU_DATA = 0x02; //0x08 GLB_DIV
	MCU_DATA = 0x00; //00//0x02 SYS_DIV
	MCU_DATA = 0x07; //0x02 CC_DIV

  	//UART_BR = 0x00B3; //HwSetting.UartBR;	//set baurd rate to 115200
  	UART_BR = HwSetting.UartBR;

	if (ONOFF)
	{
		MCU_ADDR = 0xFA;
		MCU_INDEX = 0x00;
		rdtmp = MCU_READ;
		MCU_INDEX = 0x00;
		MCU_DATA = (rdtmp | 0x01);
		MCU_DATA = 0xB1;//0X01;
		UART_EN = 0x01;
	}
	else
	{
		MCU_ADDR = 0xFA;
		MCU_INDEX = 0x00;
		rdtmp = MCU_READ;
		MCU_INDEX = 0x00;
		MCU_DATA = (rdtmp & 0xFE);
		MCU_DATA = 0xB1;//0X01;
		UART_EN = 0x00;
	}
}


int putchar (int c)
{
	t_icnt = 0;
	while( (UART_EN&0x01)==0x01 )
	{ 	
		if(++t_icnt>500)
		{
			UART_EN &= 0xFE; //Tx fire=0
		}
	}
	UART_EN &= ~BIT0; //Tx fire=0
	UART_SDATA = c;
	UART_EN |= BIT0; //Tx INT Enable(b0=1) //Tx fire=1

	return c;
}


void com_service_sfr_init(void)
{
  	//clear interrupt enable
	MCU_INTEN0 = 0x00;	//set initial value
	MCU_INTEN1 = 0x00;	//set initial value
	P2_OE = 0x00;		// Port Set Output
	P1OUT = 0x21;		//enable sleep2 wakeup by wdt interrupt and set INT pin to high

	drv_mcu_timer(HwSetting.SystemTimer0,HwSetting.ReportRateBound);
}

void drv_scan_cpump_init(U8 on)
{
    U16 i;
	U8 readtmp;

	readtmp = (HwSetting.CPump_CVDDH & 0x07);

	if (HwSetting.CPump_Mode_B_18v)
	{
		readtmp |= 0x08;							//b[3] = 1;	1.8V
	}
	else
	{
		readtmp &= 0xF7;							//b[3] = 0; 1.65V
	}

	if ((readtmp&0x07) == 0x07)
	{
		if (CPTrimValue == 0x1)
		{
			// do nothing
		}
		else if (CPTrimValue == 0x11)
		{
			readtmp -= 1;
		}
	}
	else if ((readtmp&0x07) == 0x00)
	{
		if (CPTrimValue == 0x1)
		{
			readtmp += 1;
		}
		else if (CPTrimValue == 0x11)
		{
			// do nothing
		}
	}
	else if (((readtmp&0x07) < 0x07) && ((readtmp&0x07) > 0x00))
	{
		if (CPTrimValue == 0x1)
		{
			readtmp += 1;
		}
		else if (CPTrimValue == 0x11)
		{
			readtmp -= 1;
		}
	}

//	temp002 = readtmp;

	MCU_ADDR = 0x65;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0XC8;
	MCU_DATA = 0X0A;
		
	if (on)
	{
		MCU_ADDR = 0x69;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X01;
		
		//***CPUM ON***
		MCU_ADDR = 0x67;
		MCU_INDEX = 0X00;
		MCU_DATA = (0x30 | readtmp);

		MCU_ADDR = 0x68;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X01;
		for(i=0x0000; i<0x2000; i++)	// 20
		{
			__nop();
			__nop();
		}
		MCU_ADDR = 0x68;
		MCU_INDEX = 0X01;
		MCU_DATA = 0X04;
	}
	else
	{
		MCU_ADDR = 0x69;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X00;
		
		//***CPUMP OFF***
		MCU_ADDR = 0x67;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X00;

		MCU_ADDR = 0x68;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X00;

		for(i=0x0000; i<0x20; i++)
		{
			__nop();
		}		
		MCU_ADDR = 0x68;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X00;
		MCU_DATA = 0X00;
		MCU_DATA = 0X00;
	}
	MCU_ADDR = 0x68;
	MCU_INDEX = 0X02;
	MCU_DATA = 0X00;
}

void drv_scan_CT_init(void)
{
	U8 tmpdata;
	
	// RC SEL 
	MCU_ADDR = 0xE0;//REG_RC_SEL_T1007;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X01;	//b1~0: RC_SEL[1:0]

	//ADC Adjust
	MCU_ADDR = 0XE5;//REG_ADC_ADJUST;
	MCU_INDEX = 0X00;
	tmpdata = MCU_READ;
	MCU_INDEX = 0X00;
	MCU_DATA = tmpdata & 0xBF;
	
	//ADC POWER CONTROL
	MCU_ADDR = 0X3C;//REG_ADC_PWD_CTRL;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;
}

void Modulation_Matrix(U8 mod_sel)
{
	switch( mod_sel )
	{
		//0:MOD1, 1:MOD4, 2: MOD11, 3:MOD12, 4:MOD19, 5:MOD21, 6:MOD23;
		case 1://MOD1
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x00; //TxPol[0~7]
			MCU_DATA = 0x00; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 1000 (++++ -+++)
			// 1 bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x00; //RxPol[0~7] (0~3)
			MCU_DATA = 0x00; //RxPol[8~15] (4~7)
			MCU_DATA = 0x00; //RxPol[16~23] (8~11)
			MCU_DATA = 0x00; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00 00 (-+++)
			// bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
		case 4://MOD4
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
		#if REVERSE_MUTU_DELTA	
			MCU_DATA = 0x07;
		#else
			MCU_DATA = 0x08; //TxPol[0~7]
		#endif
			MCU_DATA = 0x00; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 1000 (++++ -+++)
			// 1 bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x40; //RxPol[0~7]
			MCU_DATA = 0x00; //RxPol[8~15]
			MCU_DATA = 0x00; //RxPol[16~23]
			MCU_DATA = 0x00; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00 00 (-+++)
			// 2 bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
		case 11://MOD11
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0xE9; //TxPol[0~7]
			MCU_DATA = 0x00; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 0100 0000 0000 (++++ +-+++ ++++ ++++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x82; //RxPol[0~7]
			MCU_DATA = 0xA8; //RxPol[8~15]
			MCU_DATA = 0x00; //RxPol[16~23]
			MCU_DATA = 0x00; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00 00 (-+++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
#if 0
		case 12://MOD12
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x97; //TxPol[0~7]
			MCU_DATA = 0x00; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 0100 0000 0000 (++++ +-+++ ++++ ++++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x29; //RxPol[0~7]
			MCU_DATA = 0x81; //RxPol[8~15]
			MCU_DATA = 0x02; //RxPol[16~23]
			MCU_DATA = 0x00; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00, 00 00 00 00, 00 00 00 00 (- ++ ++ ++ ++ ++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
		case 19://MOD19
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x93; //TxPol[0~7]
			MCU_DATA = 0x57; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 1000 (++++ -+++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x0A; //RxPol[0~7]
			MCU_DATA = 0x82; //RxPol[8~15]
			MCU_DATA = 0x2A; //RxPol[16~23]
			MCU_DATA = 0x22; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~47]
			//01 00 00 00 (-+++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
		case 21://MOD21
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x10;//0xEF; //TxPol[0~7]
			MCU_DATA = 0x53;//0xAC; //TxPol[8~15]
			MCU_DATA = 0x1F;//0x00; //TxPol[16~22]
			//0000 0100 0000 0000 (++++ +-+++ ++++ ++++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x20;//0x65; //RxPol[0~7]
			MCU_DATA = 0x0A;//0x5A; //RxPol[8~15]
			MCU_DATA = 0x8A;//0x9A; //RxPol[16~23]
			MCU_DATA = 0x22;//0x66; //RxPol[24~31]
			MCU_DATA = 0xAA;//0xAA; //RxPol[32~39]
			MCU_DATA = 0x02;//0x02; //RxPol[40~45]
			//01 00 00, 00 00 00 00, 00 00 00 00 (- ++ ++ ++ ++ ++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
		case 23://MOD23
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0xAF; //TxPol[0~7]
			MCU_DATA = 0x99; //TxPol[8~15]
			MCU_DATA = 0x02; //TxPol[16~22]

			//0000 0100 0000 0000 (++++ +-+++ ++++ ++++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0xAA; //RxPol[0~7]
			MCU_DATA = 0x88; //RxPol[8~15]
			MCU_DATA = 0x82; //RxPol[16~23]
			MCU_DATA = 0x82; //RxPol[24~31]
			MCU_DATA = 0x08; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00, 00 00 00 00, 00 00 00 00 (- ++ ++ ++ ++ ++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
#endif
		default://
			//TX Polarity setting************************
			MCU_ADDR = 0xC2;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x00; //TxPol[0~7]
			MCU_DATA = 0x00; //TxPol[8~15]
			MCU_DATA = 0x00; //TxPol[16~22]
			//0000 1000 (++++ -+++)
			// 1bit presents 1 operation
			//0(+); 1(-)
			
			//Demodulation RX Polarity setting************************
			//RX Polarity setting************************
			MCU_ADDR = 0xC1;
			MCU_INDEX = 0x00;
			MCU_DATA = 0x00; //RxPol[0~7] (0~3)
			MCU_DATA = 0x00; //RxPol[8~15] (4~7)
			MCU_DATA = 0x00; //RxPol[16~23] (8~11)
			MCU_DATA = 0x00; //RxPol[24~31]
			MCU_DATA = 0x00; //RxPol[32~39]
			MCU_DATA = 0x00; //RxPol[40~45]
			//01 00 00 00 (-+++)
			// 2bit presents 1 operation
			//00(+); 01(-); 10(0)
			break;
	}
}

void read_trim_value(void)
{
	U8 tmp_FPDMA;
	U8 readtmp;

// set IF block key
	MCU_ADDR = 0x32;
	MCU_INDEX= 0x02;
	MCU_DATA = 0x46;

// set FPDMA key enable
	MCU_ADDR = 0x63;
	MCU_DATA = 0x58;	


// ####### A D D E R E S S    0X0000 ######

// set address=0x400A
	MCU_ADDR = 0x64;
	MCU_DATA = 0x00;
	MCU_DATA = 0x00;					// addr: 0x4000->0x0000, for OSC40X trim, [4:0], 5-bits
	MCU_DATA = 0x40;

// check finish flag = 0
	MCU_INDEX = 0x00;
	tmp_FPDMA = MCU_READ;
	while((tmp_FPDMA & 0x80)!=0x00)		// b[7] = 0
	{
		MCU_INDEX = 0x00;
		tmp_FPDMA = MCU_READ; 
	}

	MCU_INDEX= 0x00;
	MCU_DATA = 0x02;					// read_en

// check finish flag =1
	MCU_INDEX= 0x00;
  	tmp_FPDMA= MCU_READ;

	while((tmp_FPDMA & 0x80)!=0x80)		// b[7] = 1
	{
	 MCU_INDEX= 0x00;
	 tmp_FPDMA= MCU_READ; 
	}

	MCU_ADDR = 0x64;
	MCU_INDEX = 0x05;					// read low byte [0..7]
	tmp_FPDMA= MCU_READ;

	if (tmp_FPDMA != 0xFF)
	{
		tmp_FPDMA = (tmp_FPDMA&0x1F);
		MCU_ADDR = 0xED;
		MCU_INDEX = 0x00;
		readtmp = MCU_READ;
		MCU_INDEX = 0x00;
		MCU_DATA = (readtmp | tmp_FPDMA);
	}
//	temp001 = tmp_FPDMA;

// ####### A D D E R E S S    0X000A ######
// set address=0x400A
	MCU_ADDR = 0x64;
	MCU_DATA = 0x00;
	MCU_DATA = 0x0A;		// addr: 0x4000->0x000A, for REGULATOR trim, [3:0], 4-bits
	MCU_DATA = 0x40;

// check finish flag = 0
	MCU_INDEX = 0x00;
	tmp_FPDMA = MCU_READ;
	while((tmp_FPDMA & 0x80)!=0x00)		// b[7] = 0
	{
		MCU_INDEX = 0x00;
		tmp_FPDMA = MCU_READ; 
	}

	MCU_INDEX= 0x00;
	MCU_DATA = 0x02;					// read_en

// check finish flag =1
	MCU_INDEX= 0x00;
  	tmp_FPDMA= MCU_READ;

	while((tmp_FPDMA & 0x80)!=0x80)		// b[7] = 1
	{
	 MCU_INDEX= 0x00;
	 tmp_FPDMA= MCU_READ; 
	}

	MCU_ADDR = 0x64;
	MCU_INDEX = 0x05;					// read low byte [0..7]
	tmp_FPDMA= MCU_READ;

	if (tmp_FPDMA != 0xFF)
	{
		tmp_FPDMA = (tmp_FPDMA&0x0F);
		MCU_ADDR = 0x7F;
		MCU_INDEX = 0x00;
		readtmp = MCU_READ;
		MCU_INDEX = 0x00;
		MCU_DATA = (readtmp | tmp_FPDMA);
	}

//	temp002 = tmp_FPDMA;

// ####### A D D E R E S S    0X000B ######
// set address=0x400B
	MCU_ADDR = 0x64;
	MCU_DATA = 0x00;
	MCU_DATA = 0x0B;					// addr: 0x4000->0x000A, for Charge Pump trim value, [2:0], 3-bits
	MCU_DATA = 0x40;

// check finish flag = 0
	MCU_INDEX = 0x00;
	tmp_FPDMA = MCU_READ;
	while((tmp_FPDMA & 0x80)!=0x00)		// b[7] = 0
	{
		MCU_INDEX = 0x00;
		tmp_FPDMA = MCU_READ; 
	}

	MCU_INDEX= 0x00;
	MCU_DATA = 0x02;					// read_en

// check finish flag =1
	MCU_INDEX= 0x00;
  	tmp_FPDMA= MCU_READ;

	while((tmp_FPDMA & 0x80)!=0x80)		// b[7] = 1
	{
	 MCU_INDEX= 0x00;
	 tmp_FPDMA= MCU_READ; 
	}

	MCU_ADDR = 0x64;
	MCU_INDEX = 0x05;					// read low byte [0..7]
	tmp_FPDMA = MCU_READ;

	CPTrimValue = tmp_FPDMA;		//b[2..0],0x01 = trim+1, 0x11=trim-1, 0xff = 0
//	temp001 = tmp_FPDMA;
	
//	if (tmp_FPDMA != 0xFF)
//	{
//		tmp_FPDMA = (tmp_FPDMA&0x07);
//		MCU_ADDR = 0x67;
//		MCU_INDEX = 0x00;
//		readtmp = MCU_READ;
//		MCU_INDEX = 0x00;
//		MCU_DATA = (readtmp | tmp_FPDMA);
//	}
	// --- end
// clear finish flag
	MCU_INDEX= 0x00;
	MCU_DATA = 0x00;


// release FPDMA key enable
	MCU_ADDR = 0x63;
	MCU_DATA = 0x00;

// release IF block key
	MCU_ADDR = 0x32;
	MCU_INDEX= 0x02;
	MCU_DATA = 0x00;
}

void drv_scan_tcon_init(void)
{
	U8 readtmp;

	//SENSE Channel Control Setting
	MCU_ADDR = 0X36;		// 6 bytes
	MCU_DATA = 0XFF;
	MCU_DATA = 0XFF;
	MCU_DATA = 0XFF;
	MCU_DATA = 0XFF;
	MCU_DATA = 0XFF;
	MCU_DATA = 0XFF;

	//Clock divider
	MCU_ADDR = 0X7D;
	MCU_INDEX = 0X04;
	MCU_DATA = 0X07;	// CC_DIV

	//Clock divider2
	MCU_ADDR = 0X7E;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X30;
	MCU_DATA = 0X00;
	MCU_DATA = 0X00;

	//CFGREG Access Disable and Reloader Setting
	MCU_ADDR = 0X34;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X02;		//fixed

	//Special Pattern Setting
	MCU_ADDR = 0XF2;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X02;	//b4:TX_ND, b1:NON_SEN_GND, b0:RX_FRAME_RELO, consider "loading"

	//Non-overlap setting
	MCU_ADDR = 0XF3;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X01;	//SCLK_NP_SEL[1:0]

	//Special Config
	MCU_ADDR = 0XF6;
	MCU_INDEX = 0X00;
	readtmp = MCU_READ;
	MCU_INDEX = 0X00;
	MCU_DATA = (readtmp | 0X41);	//b6:P_EN(1), b5:P_EN_ALL, b3:AUTO_Manual byIF(1), b0: STOP_EN(1)
 

	MCU_INTEN1 |= TCON_INT_ENABLE;		//Enable TCON INT
}


void drv_scan_trigger_scan(void)
{
    P1OUT &= ~0x04;
    __nop();
    P1OUT |= 0x04;
    __nop();
    P1OUT &= ~0x04;
    __nop();
}


void drv_scan_trigger_self(U8 for_short_test) // self_pulsing_all: 0 for Normal scanning. 1 for short test.
{
	U8 i,m,readtmp,selfvddh;
	U8 *p_index;

	ADCScan_Done = 0;


	//Scan Status
	MCU_ADDR = 0x81;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x01; //Disable Auto Scan

	//Special Pattern Setting
	MCU_ADDR = 0XF2;
	MCU_INDEX = 0X00;
	readtmp = MCU_READ;
	MCU_INDEX = 0X00;
	MCU_DATA = (readtmp & 0xEF);	//b4:TX_ND, 1: floating for NL, 0:GND for mutual


	//REG 0x37 Initial
	MCU_ADDR = 0X37;
	MCU_INDEX = 0X00;
	MCU_DATA = 0x1F; //0X9F;		//0x9f
	MCU_DATA = LoByte((U16)(&CFGMEM_SELF_RXEN_TABLE[0])		- 0x0400);
	MCU_DATA = HiByte((U16)(&CFGMEM_SELF_RXEN_TABLE[0])		- 0x0400);
	MCU_DATA = LoByte((U16)(&CFGMEM_MUTUAL_TXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = HiByte((U16)(&CFGMEM_MUTUAL_TXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = LoByte((U16)(&SelfAutoCC[0]) 				- 0x0400);
	MCU_DATA = HiByte((U16)(&SelfAutoCC[0]) 				- 0x0400);
	MCU_DATA = 0X06;
	MCU_DATA = 0X06;
	MCU_DATA = 0X0E;

//	if (HwSetting.CPump)	// CPump on, SELF VDDH selection
	if(1)
	{
		if (HwSetting.CPump_SelfForce33v)
		{
			selfvddh = 0xFE;
		}
		else
		{
			selfvddh = 0xFF;
		}

		//CPUMP SETTING
		MCU_ADDR = 0x69;
		MCU_INDEX = 0X00;
		readtmp = MCU_READ;
		MCU_INDEX = 0X00;
		MCU_DATA = (readtmp & selfvddh);		// 0: 3.3v, 1:Cpump;
	}

	if (Bios_ScanMode == SELF_MODE_AUTO_CC)
	{
		//MOD_TRIG_Setting
		MCU_ADDR = 0xC8;
		MCU_INDEX = 0x00;
		MCU_DATA = 0x11; //b4:SELF_EN=1, b0:S_TRIG_EN=0
		MCU_DATA = 0x21; //V_INT //b5:MANUAL_EN, b4:VSUB_MOD, b3:V_INT, b2:VSUB_INT, b0:H_INT
		MCU_DATA = 0x01;
	}
	else if (Bios_ScanMode == SELF_MODE)
	{
		//MOD_TRIG_Setting
		MCU_ADDR = 0xC8;
		MCU_INDEX = 0x00;
		MCU_DATA = 0x10; //b4:SELF_EN=1, b0:S_TRIG_EN=0
		MCU_DATA = 0x08; //V_INT //b5:MANUAL_EN, b4:VSUB_MOD, b3:V_INT, b2:VSUB_INT, b0:H_INT
		MCU_DATA = 0x01;
	}

#if !PROTOCOL_V20
	if (OpenShortTestMode)
	{
		//TCON H setting
		MCU_ADDR = 0x30;
		MCU_INDEX = 0x00;
		MCU_DATA = HwSetting.OpenShortPulseCount; //S_CT_NUM[7:0]
		MCU_DATA = 0x00; //S_CT_NUM[11:8]

		//ADC setting 
		MCU_ADDR = 0x3A;
		MCU_INDEX = 0x00;
		MCU_DATA = 0; //b1:MUTUAL_ALL(non-scanning=0:GND,1:VCM) ; b0:EN_ALL in SELF Mode(non-scanning=0:GND,1:floating )

		//Clock divider
		MCU_ADDR = 0X7D;
		MCU_INDEX = 0X00;
		MCU_DATA = ((HwSetting.OpenShortPulseLenth/4)&0x3f);	//GLB_CLK = 20Mhz/2 = 10Mhz

		// CFB SEL
		MCU_ADDR = 0XE2;//REG_CFB_SEL;
		MCU_INDEX = 0X00;
		MCU_DATA = HwSetting.OpenShortCfb;

		//CC Clock Setting
		MCU_ADDR = 0XF5;
		MCU_INDEX = 0X00;
		MCU_DATA = HwSetting.OpenShortCcClk;
		MCU_DATA = HwSetting.OpenShortCcClk;
	}
	else
#endif
	{
		//TCON H setting
		MCU_ADDR = 0x30;
		MCU_INDEX = 0x00;
		MCU_DATA = HwSetting.SelfPulseCount; //S_CT_NUM[7:0]
		MCU_DATA = 0x00; //S_CT_NUM[11:8]

		//Clock divider
		MCU_ADDR = 0X7D;
		MCU_INDEX = 0X00;
		MCU_DATA = (((HwSetting.SelfPulseLenth/4) + NOW_HoppingBestFrequencyIndex )&0x3f);	//GLB_CLK = 20Mhz/2 = 10Mhz

		//ADC setting 
		MCU_ADDR = 0x3A;
		MCU_INDEX = 0x00;
		MCU_DATA = (for_short_test ? 0x00 : 0x01); //b1:MUTUAL_ALL(non-scanning=0:GND,1:VCM) ; b0:EN_ALL in SELF Mode(non-scanning=0:GND,1:floating )

		// CFB SEL
		MCU_ADDR = 0XE2;//REG_CFB_SEL;
		MCU_INDEX = 0X00;
		MCU_DATA = HwSetting.SelfCfb; 

		//CC Clock Setting
		MCU_ADDR = 0XF5;
		MCU_INDEX = 0X00;
		MCU_DATA = HwSetting.SelfCcClk;
		MCU_DATA = HwSetting.SelfCcClk;
	}
	//TCON V setting
	MCU_ADDR = 0x31;
	MCU_INDEX = 0x00;
	MCU_DATA = SensorDefine.Self_Passes; //S_SENSE_NUM[5:0]   		//Self: repeat seq = #SeqNum

	//HOPPING
	MCU_ADDR = 0xEC;
	MCU_DATA = (NOW_HoppingBestFrequencyIndex&0x1f);

	//TCON SRAM TARGET
	MCU_ADDR = 0XCA;
	MCU_INDEX = 0X00;
	MCU_DATA = LoByte((U16)(&SelfRawDataBuffer[SelfWorking][0]) - 0x0400);
	MCU_DATA = HiByte((U16)(&SelfRawDataBuffer[SelfWorking][0]) - 0x0400);
	MCU_DATA = LoByte((U16)(&SelfRawDataBuffer[SelfWorking][0]) - 0x0400);
	MCU_DATA = HiByte((U16)(&SelfRawDataBuffer[SelfWorking][0]) - 0x0400);

	//CCPol setting************************************* //0xE1 and 0x34.b2=1
	p_index = (U8 *)(&SelfAutoCC[0]);
	for ( m = 0 ; m < (SensorDefine.Self_Passes) ; m++)
	{
		for ( i = 0 ; i < ADCNumTotal ; i++)
		{
			*p_index++ = (0x80 | SelfAutoCC[m*ADCNumTotal+i]);
		}
	}

	//SCAN Status
	MCU_ADDR = 0X81;
	MCU_INDEX = 0X00;
	MCU_DATA = 0X03;	// Disable auto scan,normal mode

	drv_scan_trigger_scan();
}

void drv_scan_trigger_mutual(void)
{
	U8 readtmp,mutuvddh;
	U16 idx;
#if REVERSE_MUTU_DELTA
	U8 i,j,k,adcnum;
	U8 *p_index;
	adcnum = ADCNumTotal;
#endif

	ADCScan_Done = 0;
	
	//Scan Status
	MCU_ADDR = 0x81;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x01; //Disable Auto Scan

	//REG 0x37 Initial
	MCU_ADDR = 0X37;
	MCU_INDEX = 0X00;
	MCU_DATA = 0x1f;//0X9F;		//0x9f  MutualAutoCC
	MCU_DATA = LoByte((U16)(&CFGMEM_MUTUAL_RXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = HiByte((U16)(&CFGMEM_MUTUAL_RXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = LoByte((U16)(&CFGMEM_MUTUAL_TXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = HiByte((U16)(&CFGMEM_MUTUAL_TXEN_TABLE[0]) 	- 0x0400);
	MCU_DATA = LoByte((U16)(&MutualAutoCC[0]) 				- 0x0400);
	MCU_DATA = HiByte((U16)(&MutualAutoCC[0]) 				- 0x0400);
	MCU_DATA = 0X06;
	MCU_DATA = 0X06;
	MCU_DATA = 0X0E;

	if (HwSetting.CPump)	// CPump on, SELF VDDH selection
	{
		if (HwSetting.CPump_SelfForce33v)
		{
			mutuvddh = 0x01;
		}
		else
		{
			mutuvddh = 0x00;
		}

		//CPUMP SETTING
		MCU_ADDR = 0x69;
		MCU_INDEX = 0X00;
		readtmp = MCU_READ;
		MCU_INDEX = 0X00;
		MCU_DATA = (readtmp | mutuvddh);		// 0:3.3v, 1:Cpump;
	}

	if (Bios_ScanMode == MUTUAL_MODE_AUTO_CC)
	{
		//MOD_TRIG_Setting
		MCU_ADDR = 0xC8;
		MCU_INDEX = 0x00;
		MCU_DATA = 0x01;	// H_int
		MCU_DATA = 0x21; 	//H_INT //b5:MANUAL_EN, b4:VSUB_MOD, b3:V_INT, b2:VSUB_INT, b0:H_INT
		MCU_DATA = 0x00;

		//Clock divider
		MCU_ADDR = 0X7D;
		MCU_INDEX = 0X00;
		MCU_DATA = ((HwSetting.MutualPulseLenth/4)&0x3F);//(HwSetting.MutualPulseLenth/2);	//GLB_CLK = 20Mhz/2 = 10Mhz	
	}
	else if (Bios_ScanMode == MUTUAL_MODE)
	{
		//MOD_TRIG_Setting
		MCU_ADDR = 0xC8;
		MCU_INDEX = 0x00;
		MCU_DATA = 0x00; 	//b4:SELF_EN=1, b0:S_TRIG_EN=0
		MCU_DATA = 0x08; 	//V_INT //b5:MANUAL_EN, b4:VSUB_MOD, b3:V_INT, b2:VSUB_INT, b0:H_INT
		MCU_DATA = 0x00;

		//Clock divider
		MCU_ADDR = 0X7D;
		MCU_INDEX = 0X00;
		MCU_DATA = (((HwSetting.MutualPulseLenth/4) + NOW_HoppingBestFrequencyIndex)&0x3F);	//GLB_CLK = 20Mhz/2 = 10Mhz	

	}
	else if (Bios_ScanMode == NL_MODE)
	{
		//MOD_TRIG_Setting
		MCU_ADDR = 0xC8;
		MCU_INDEX = 0x00;
		MCU_DATA = 0x00; 	//b4:SELF_EN=1, b0:S_TRIG_EN=0
		MCU_DATA = 0x08; 	//V_INT //b5:MANUAL_EN, b4:VSUB_MOD, b3:V_INT, b2:VSUB_INT, b0:H_INT
		MCU_DATA = 0x00;

		//Clock divider
		MCU_ADDR = 0X7D;
		MCU_INDEX = 0X00;
		MCU_DATA = (((HwSetting.MutualPulseLenth/4) + HoppingNowIndex )&0x3F);
	}

	if (Bios_ScanMode == NL_MODE)
	{
		//Modulation_Matrix(1);
		//Special Pattern Setting
		MCU_ADDR = 0XF2;
		MCU_INDEX = 0X00;
		readtmp = MCU_READ;
		MCU_INDEX = 0X00;
		MCU_DATA = (readtmp | 0x10);	//b4:TX_ND, 1: floating for NL, 0:GND for mutual
		//MCU_DATA = (readtmp & 0xEF);	//b4:TX_ND, 1: floating for NL, 0:GND for mutual

		MCU_ADDR = 0xEC;
		MCU_DATA = (HoppingNowIndex&0x1f);
		
		//TCON H setting
		MCU_ADDR = 0x30;
		MCU_INDEX = 0x02;
		MCU_DATA = HwSetting.MutualPulseCount;; //HwSetting.MutualPulseCount;//5;	//HwSetting.MutualPulseCount;//0x32;
		MCU_DATA = 0x00;

		//TCON V setting
		MCU_ADDR = 0x31;
		MCU_INDEX = 0x01;
		MCU_DATA = 1; //SensorDefine.Mutual_ModulationTxSequence;//VSubFrameNum; //M_SENSE_NUM[5:0] //Mutual: VsubFrrameNum = #TxNum/#TxModNum (=8/1=8)

		//TCON SRAM TARGET
		MCU_ADDR = 0XCA;
		MCU_INDEX = 0X00;
		MCU_DATA = LoByte((U16)(&NLRawDataBuffer[NLWorking][0])	- 0x0400);
		MCU_DATA = HiByte((U16)(&NLRawDataBuffer[NLWorking][0])	- 0x0400);
		MCU_DATA = LoByte((U16)(&NLRawDataBuffer[NLWorking][0])	- 0x0400);
		MCU_DATA = HiByte((U16)(&NLRawDataBuffer[NLWorking][0])	- 0x0400);

		//Modulation Setting
		MCU_ADDR = 0XC7;
		MCU_INDEX = 0X00;
		MCU_DATA = SensorDefine.Mutual_RxNumbers;
		MCU_DATA = 0x00;	//MODx1: 1-1
		MCU_DATA = 0x01;	//MODx1:

		// MUTUAL REG_53
		MCU_ADDR = 0X53;
		for (idx = 0 ; idx < (1) ; idx++)		// MOD * Tx
		{
			MCU_DATA = idx;
		}
	}
	else	// mutual or mutual cc mode
	{
		Modulation_Matrix(SensorDefine.Mutual_TxModulationMode);
		//Special Pattern Setting
		MCU_ADDR = 0XF2;
		MCU_INDEX = 0X00;
		readtmp = MCU_READ;
		MCU_INDEX = 0X00;
		MCU_DATA = (readtmp & 0xEF);	//b4:TX_ND, 1: floating for NL, 0:GND for mutual

		MCU_ADDR = 0xEC;
		MCU_DATA = (NOW_HoppingBestFrequencyIndex&0x1f);
			
		//TCON H setting
		MCU_ADDR = 0x30;
		MCU_INDEX = 0x02;
		MCU_DATA = HwSetting.MutualPulseCount;
		MCU_DATA = 0x00;

		//TCON V setting
		MCU_ADDR = 0x31;
		MCU_INDEX = 0x01;
		MCU_DATA = SensorDefine.Mutual_ModulationTxSequence;

		//TCON SRAM TARGET
		MCU_ADDR = 0XCA;
		MCU_INDEX = 0X00;
		MCU_DATA = LoByte((U16)(&MutualRawDataBuffer[MutualWorking][0])	- 0x0400);
		MCU_DATA = HiByte((U16)(&MutualRawDataBuffer[MutualWorking][0])	- 0x0400);
		MCU_DATA = LoByte((U16)(&MutualRawDataBuffer[MutualWorking][0])	- 0x0400);
		MCU_DATA = HiByte((U16)(&MutualRawDataBuffer[MutualWorking][0])	- 0x0400);

		//Modulation Setting
		MCU_ADDR = 0XC7;
		MCU_INDEX = 0X00;
		MCU_DATA = SensorDefine.Mutual_RxNumbers;
		MCU_DATA = (SensorDefine.Mutual_TxModulationMode - 1);
		MCU_DATA = SensorDefine.Mutual_ModulationTypeOf_REG_C7_2;


		// MUTUAL REG_53
		MCU_ADDR = 0X53;
		for (idx = 0 ; idx < (SensorDefine.Mutual_Effective_SizeOf_REG53) ; idx++)
		{
			MCU_DATA = SensorDefine.Mutual_REG53_Vertical_ID_Table[idx];
		}
	}
	//ADC setting 
	MCU_ADDR = 0x3A;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x00;

	//RepeatNum of Mutual
	MCU_ADDR = 0x4D;
	MCU_INDEX = 0x00;
	MCU_DATA = (SensorDefine.Mutual_Passes-1);

	// CFB SEL
	MCU_ADDR = 0XE2;
	MCU_INDEX = 0X00;
	MCU_DATA = HwSetting.MutualCfb; 

	//CC Clock Setting
	MCU_ADDR = 0XF5;
	MCU_INDEX = 0X00;
	MCU_DATA = HwSetting.MutualCcClk;	//cc_num
	MCU_DATA = HwSetting.MutualCcClk;	//cc_num2


	//CCPol setting************************************* //0xE1 and 0x34.b2=1
#if REVERSE_MUTU_DELTA
	p_index = (U8 *)(&MutualAutoCC[0]);
	for ( i = 0 ; i < (SensorDefine.Mutual_ModulationTxSequence) ; i++)
	{
		for ( j = 0 ; j < (SensorDefine.Mutual_TxModulationMode) ; j++)
		{
			for ( k = 0 ; k < (SensorDefine.Mutual_Passes*adcnum) ; k++)
			{
				idx = (i*SensorDefine.Mutual_TxModulationMode+j)*(SensorDefine.Mutual_Passes*adcnum)+k;
				#if REVERSE_MUTU_DELTA
					if (Bios_ScanMode == NL_MODE)
					{
						*p_index++ = (MutualAutoCC[idx] & 0x7F);		// mask polarity for NL
					}
					else
					{
						*p_index++ = (MutualAutoCC[idx] | 0x80);		// REVERSE
					}
				#else
					*p_index++ = MutualAutoCC[idx];
				#endif
			}
		}
	}
#endif

	//IDLE off, Scan On, Sleep Out
	MCU_ADDR = 0x81;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x03;

	MCU_ADDR = 0xC7;			// modulation 1/4 switching -- bug fix @ 20141030
	MCU_INDEX = 0x02;
	readtmp = MCU_READ;
	MCU_INDEX = 0x02;
	MCU_DATA = (readtmp | 0x04);
	MCU_INDEX = 0x02;
	MCU_DATA = (readtmp & 0xFB);

	drv_scan_trigger_scan();
}



void ChipSensorSetting(void)
{
	U16 idx;
	
	drv_scan_cpump_init(HwSetting.CPump);	//enable charge-pump
	drv_scan_CT_init();
	drv_scan_tcon_init();
	read_trim_value();
	Modulation_Matrix(SensorDefine.Mutual_TxModulationMode);	// only support MODx1, MODx4, MODx11

	// MUTUAL REG_54  -- Common setting
	MCU_ADDR = 0X54;
	for (idx = 0 ; idx < (SensorDefine.Mutual_Effective_SizeOf_REG54) ; idx++)
	{
		MCU_DATA = SensorDefine.Mutual_REG54_Horizontal_Offset_Table[idx];
	}
}

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
void drv_scan_read_ADCRawMode(vU16 int_count)
{
	U8 idx,adcnum;
	U8 tmpHigh,tmpLow;

	adcnum = ADCNumTotal;

	MCU_ADDR = 0XFA;
	MCU_INDEX = 0X00;
	tmpLow = MCU_READ;
	MCU_INDEX = 0X00;
	MCU_DATA = (tmpLow | BIT4);
	
	MCU_ADDR = 0XB3;
	MCU_INDEX = 0X00;
	for ( idx = 0 ; idx < adcnum ; idx++ )
	{
		tmpLow	= MCU_READ;
		tmpHigh	= MCU_READ;
		if (Bios_ScanMode == MUTUAL_MODE_AUTO_CC)
		{
			SigFrame.Msig.Mutual[idx + ((int_count-1)*adcnum)] = (tmpHigh << 8) + tmpLow;
		}
		else if (Bios_ScanMode == SELF_MODE_AUTO_CC)
		{
			SigFrame.Ssig.Self[idx + ((int_count-1)*adcnum)] = (tmpHigh << 8) + tmpLow;
		}
	}
	MCU_ADDR = 0XFA;
	MCU_INDEX = 0X00;
	tmpLow = MCU_READ;
	MCU_INDEX = 0X00;
	MCU_DATA = (tmpLow & (~BIT4));
}
#endif

void AutoSelfModeCalibration(U16 raw_min, U16 raw_max, U8 offset)
{
	U8 m,node,cc_value;
	U8 *ptrCCvalue;

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
	U8 total,count,_byte,_bit,_start,_end;
	U8 CH_flag[11]={0,0,0,0, 0,0,0,0, 0,0,0};	//6pass * 14adc /8bit = 11 bytes
	U8 total_H_int_loop;
	U8 H_count;
#endif

	ptrCCvalue = SelfAutoCC;
	node = SelfTotalADCDataNum;

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
	H_count = 0x00;
	total_H_int_loop = SensorDefine.Self_Passes;
	_start = 0;
	_end = node;
	total = _end;
	count = 0;
#endif

	if (FwSetting.ForceSelfModeCC != 0)
	{
		cc_value = FwSetting.ForceSelfModeCC;
	}
	else
	{
		cc_value = offset;
	}
	
	for ( m = 0 ; m < node ; m++)
	{
		ptrCCvalue[m] = cc_value;
	}

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
	if (FwSetting.ForceSelfModeCC == 0)
	{
		Bios_ScanMode = SELF_MODE_AUTO_CC;
		SelfAutoCC_H_INT_Times = 0x00;
		H_count = 0x00;
		
		while(total > count)
		{
			while(H_count < total_H_int_loop)
			{
				if (H_count == 0)
				{
					drv_scan_trigger_self(KnlOpMode[2] & bOPM2_SHORT_TEST);
				}
				else
				{
					ADCScan_Done = 0;
					drv_scan_trigger_scan();
				}
				while (!ADCScan_Done) {};
				drv_scan_read_ADCRawMode(SelfAutoCC_H_INT_Times);
				H_count++;	
			}
			H_count = 0x00;
			SelfAutoCC_H_INT_Times = 0x00;

			
			cc_value++;
			if (cc_value > 0x3f)
				break;
			
			for ( m = _start ; m < _end ; m++ )
			{
				_byte = m >> 3;
				_bit = m & 0x7;
				if ( (CH_flag[_byte] & BIT8_MASK[_bit]) == 0 )
				{
					if( (SigFrame.Ssig.Self[m] <= raw_max) && (SigFrame.Ssig.Self[m] >= raw_min) )
					{
						CH_flag[_byte] |= BIT8_MASK[_bit];
						count++;
					}
					else if ( SigFrame.Ssig.Self[m] < raw_min )
					{
						CH_flag[_byte] |= BIT8_MASK[_bit];
						count++;					
					}
					else
					{
						ptrCCvalue[m] = cc_value;
					}
				}
			}
		}
	}

#endif

#if 0
	printf("SelfComOffset = \r\n");
	for ( m =_start ; m < _end ; m++ )
	{
		//printf("0x%bx,",ptrComOffset[ ChMapping[ii] ]);
		printf("0x%2x,  ",ptrCCvalue[m]);
		if ((m+1)% 14 == 0)
			printf("\n\r");
	}
	printf("\r\n");
#endif
}

void AutoMutualModeCalibration_ByNode(U16 MinRaw, U8 offset)
{
	U8 *ptrCCvalue,cc_value;
	U16 m,node;

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE	
	U8 _byte,_bit,CH_flag[72];		//24*24/8 = 72 (max)
	U16 cc_min,count,total,_start,_end;
	U8 total_H_int_loop;
	U8 H_count;
#endif

#if REVERSE_MUTU_DELTA
	U16 cc_max;

	cc_max = (MUTU_PULSE_COUNT * 25);
	cc_min = (MUTU_PULSE_COUNT * 20);
#endif


	node = MutualModulationADCDataNum;

#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
	H_count = 0x00;
	// 4 * 4 * 2 = 32 times
	total_H_int_loop = SensorDefine.Mutual_TxModulationMode * SensorDefine.Mutual_ModulationTxSequence * SensorDefine.Mutual_Passes;
	count = 0x00;	
	_start = 0;
	cc_min = MinRaw;
	_end = node;
	
	for ( m = 0 ; m < 72 ; m++ )
	{
		CH_flag[m] = 0x00;
	}
#endif


	ptrCCvalue = MutualAutoCC;
	
	if (FwSetting.ForceMutualModeCC != 0)
	{
		cc_value = FwSetting.ForceMutualModeCC;
	}
	else
	{
		cc_value = offset;
	}

	for ( m = 0 ; m < node ; m++ )
	{
		ptrCCvalue[m] = cc_value;		//initialize
		//MutualAutoCC[m] = cc_value;
	}
#if !SHRINK_FLASH_SIZE__AUTOCC_CODE
	if (FwSetting.ForceMutualModeCC == 0)
	{
		total = node;
		// execute H_int trigger
		Bios_ScanMode = MUTUAL_MODE_AUTO_CC;
		MutualAutoCC_H_INT_Times = 0x00;
		H_count = 0x00;

		while (total>count)
		{
			while(H_count < total_H_int_loop)
			{
				if (H_count == 0)
				{
					drv_scan_trigger_mutual();
				}
				else
				{
					ADCScan_Done = 0;
					drv_scan_trigger_scan();	
				}
				while (!IsEndOfScan()) {};
				drv_scan_read_ADCRawMode(MutualAutoCC_H_INT_Times);
				H_count++;	
			}
			H_count = 0x00;						// after 1 frame, clear the counter
			MutualAutoCC_H_INT_Times = 0x00;	// after 1 frame, clear the counter
			
			cc_value++;
			if (cc_value > 0x3F)
			{
				break;
			}
		 
			for( m = _start ; m < _end ; m++ )
			{
				_byte = (U8)(m >> 3);
				_bit = (U8)(m & 0x7);
				if( (CH_flag[_byte] & BIT8_MASK[_bit]) == 0 )
				{
#if REVERSE_MUTU_DELTA
					if ( (SigFrame.Msig.Mutual[m] >= cc_min) && (SigFrame.Msig.Mutual[m] <= cc_max) )
					{
						CH_flag[_byte] |= BIT8_MASK[_bit];
						count++;
					}
					else if ((SigFrame.Msig.Mutual[m] < cc_min))
					{
						CH_flag[_byte] |= BIT8_MASK[_bit];
						count++;
					}
					else
					{
						ptrCCvalue[m] = cc_value;
					}
#else
					if( SigFrame.Msig.Mutual[m] >= cc_min)
					{
						CH_flag[_byte] |= BIT8_MASK[_bit];
						count++;
					}
					else
					{
						ptrCCvalue[m] = cc_value;
					}
#endif
				}
			}
		}
	}
#endif

#if 0
	printf("MutualComOffset Node = \r\n");
	for (m =_start ; m<_end ; m++)
	{
		//printf("0x%x,",ptrCCvalue[m]);
		printf("[%3d]:0x%2x,",m,MutualAutoCC[m]);
		if ((m+1)%28==0)
		printf("\r\n");
	}
#endif
}

void com_service_enter_sleep()
{
	//CP Off
	MCU_ADDR = 0x68;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x00;
	MCU_DATA = 0x00;

	//ADC setting Off
	MCU_ADDR = 0x3A;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x7C;

	//ADC PDN
	MCU_ADDR = 0x3C;
	MCU_INDEX = 0x00;
	MCU_DATA = 0xFF;
	MCU_DATA = 0x3F;

	//Sleep1
	MCU_ADDR = 0x81;
	MCU_INDEX = 0x00;
	MCU_DATA = 0x00;
}

#if !SHRINK_FLASH_SIZE__AUTO_FREQ
void drv_scan_get_noise_level(U16 *nl_raw_mem, U16 *nl_base, U8 hop_idx ,U16 *rx_level, U16 node)
{
	U16 idx;
	U8 tmpidx,tmpfreq,loopcnt;
	U16 *src_hwptr;

	S16 nlmin,nlmax;
	U16 nlsubtemp;
	U8 nowbestfreq;
	S16 iirtmp,nltmp;
	vU32 tmpbase;

	src_hwptr = (U16 *)(NLRawDataBuffer[NLWorking]);
	memcpy(nl_raw_mem,src_hwptr,(node*2));				// copy working to ready
	
	if (FwSetting.AutoFreqHopFunction == ENABLE)
	{
		for ( idx = 0 ; idx < node ; idx++ )
		{
			//get new base
			tmpbase = (*(nl_base+idx));
			tmpbase <<= 5;		// left shift 5 bit
			tmpbase -= (*(nl_base+idx));
			tmpbase += (*(nl_raw_mem+idx));
			tmpbase >>= 5;		// right shift 5 bit
			(*(nl_base+idx)) = tmpbase;
			

			nlsubtemp = (abs( (*(nl_raw_mem+idx)) - (*(nl_base+idx)) ));
			nlsubtemp = (nlsubtemp >> 1);		// down grade
			if (nlsubtemp > 0x7ff)
				nlsubtemp = 0x7ff;
			(*(rx_level+idx)) += nlsubtemp;
		}
		DoNoiseListeningCounter++;
		if (DoNoiseListeningCounter >= NL_DO_TIMES)
		{
			DoNoiseListeningCounter = 0x00;
			
			nlmax = 0;
			iirtmp = PRE_NL_FREQ_LEVEL[HoppingNowTableIndex];
			
			for ( idx = 0 ; idx < node ; idx++ )
			{
				if ( (*(rx_level+idx)) > nlmax )
				{
					nlmax = (*(rx_level+idx));
				}
				(*(rx_level+idx)) = 0x00;	// reset to 0
			}
			NL_FREQ_LEVEL[HoppingNowTableIndex] = nlmax;
			PRE_NL_FREQ_LEVEL[HoppingNowTableIndex] = ((iirtmp*5/8) + (nlmax*3/8));


			nlmin = 0x7fff;
			nowbestfreq = 0x00;
			for ( idx = 0 ; idx < MAX_NL_FREQ_NUM ; idx++ )
			{
				if ( (FwSetting.AutoFreqSelectionTable[idx] != 0xFF) && (FwSetting.AutoFreqSelectionTable[idx] <= 0x0F) )
				{
					nltmp = PRE_NL_FREQ_LEVEL[idx];
					if (nlmin > nltmp)
					{
						nlmin = nltmp;
						nowbestfreq = (U8)(idx);
					}
				}
			}
			if ( (PRE_NL_FREQ_LEVEL[NOW_BestFreqTableIndex] - PRE_NL_FREQ_LEVEL[nowbestfreq]) > FwSetting.AutoFreqSwitchingLevel )
			{
				NOW_HoppingBestFrequencyIndex = FwSetting.AutoFreqSelectionTable[nowbestfreq];
				NOW_BestFreqTableIndex = nowbestfreq;
				NLHopNoReport = 0x1;
			}
			else
			{
				// do nothing, no hopping
				NLHopNoReport = 0x00;
			}
			tmpidx = ((HoppingNowTableIndex+1)%MAX_NL_FREQ_NUM);
			tmpfreq = FwSetting.AutoFreqSelectionTable[tmpidx];

			loopcnt = 0x00;
			while ((tmpfreq > 0x0F) || ((tmpfreq == 0xFF) && (loopcnt < MAX_NL_FREQ_NUM)))
			{
				tmpidx = ((tmpidx+1)%MAX_NL_FREQ_NUM);
				tmpfreq = FwSetting.AutoFreqSelectionTable[tmpidx];
				loopcnt++;
			}
			HoppingNowTableIndex = tmpidx;
			HoppingNowIndex = tmpfreq;
		}
	}
	else	//hopping disable
	{
		HoppingNowIndex = 0x00;
		NOW_HoppingBestFrequencyIndex = 0x00;
	}
}
#endif

void drv_scan_noiselistening_baseline(U16 *nl_raw_mem, U16 *nl_base, vU8 hop_idx, U16 *rx_level, U16 node)
{
	U16 idx;
	U16 *src_hwptr;

	src_hwptr = (U16 *)(&NLRawDataBuffer[NLWorking]);
	memcpy(nl_raw_mem,src_hwptr,(node*2));				// copy working to ready

	
	for (idx = 0 ; idx < node ; idx++)
	{
		(*(nl_base+idx)) = (*(nl_raw_mem+idx));		// baseline
		(*(rx_level+idx)) = 0x00;					// NL-Rx initialize
	}
#if 0
	printf("BASE_FREQ:%2d  \n\r",HoppingNowIndex);
	for (idx = 0 ; idx < node ; idx++)
	{
		printf("[%2d]:%4d ",idx,(*(nl_base+idx)));
	}
	printf("\n\r");
#endif
}


// ####### M A I N.c Re-direct function #######

void STARTING_TRIGGER_SCAN_TESTMODE(U8 type)
{
	Bios_ScanMode = SELF_MODE;
	drv_scan_trigger_self(type);
}

void _MUTUAL_TRIGGER_SCAN(void)
{
	Bios_ScanMode = MUTUAL_MODE;
	drv_scan_trigger_mutual();
}


void _OPEN_SHORT_TEST_MODE(void)
{
#if !PROTOCOL_V20
	if(ScanStatus & bOPEN_SHORT_TEST)
	{
		OpenShortTestMode = 1;
		PowerOnInitialProcess = 0;
		
	    KnlProtocolCtrl |= bBRIDGE_I2C_DONE;    //set AP communicate flag
	    KnlPreBridgeDone = 0x01;

		_SELF_MUTUAL_AUTO_CALIBRATION();

		while(1)
		{
			if (IsEndOfAPRead())
			{
				KnlProtocolCtrl &= ~bBRIDGE_I2C_DONE;

				STARTING_TRIGGER_SCAN_TESTMODE(SELF_SINGLE);
				
				_WINPOWER_GUI_PROCESSING();

				_FEEDBACK_GUI_SELF_MUTUAL_MODE_CC();		
			}
		}
	}
#endif
}


#if GESTURE_REMOTE_CONTROL
void knl_touch_finger_gesture(POINT_OUTPUT *fgesture)
{
	S16 gX,gY;
	U8 PointCNT;

	gX = (*fgesture).Position[TX_IDX][0];
	gY = (*fgesture).Position[RX_IDX][0];
	PointCNT = (*fgesture).PointNum;
	GestureRecognized = GestureMatchProcess(PointCNT,PreOutPointNum,gX,gY);
}
#endif

#if IDLE_MODE_FUNCTION
void _POWER_MODE_SWITCH(void)
{
	U16 delay;
	U8 read_tmp;
	U16 SLP2Frame;

	read_tmp = 0x00;
		
	SLP2Frame = FwSetting.IdleModeSLP2Frame;

	if (( ((PrePointNum != 0x00) || (PreKeyPress != 0x00)) && (IDLE_MODE_STATUS == 1) ) )
	{
		FRAME_Counter = 0;
		IDLE_MODE_STATUS = 0;
	}
	else
	{
		FRAME_Counter++;
		if ( FRAME_Counter > FRAME_ReadyEnter )
		{
			IDLE_MODE_STATUS = 1;
			FRAME_Counter  = FRAME_ReadyEnter + 1;
		}
	}

	if ( IDLE_MODE_STATUS == 1 )
	{
		while(!IsEndOfScan()) {};

		Disable_SYNC_Timer;

		MCU_ADDR = 0x3C;	// power down ADC
		MCU_DATA = 0xFF;
		MCU_DATA = 0x3F;

		MCU_ADDR = 0x7D;
		MCU_INDEX = 0x01;
		MCU_DATA = 0x04;

		MCU_ADDR = 0x68;
		MCU_INDEX = 0X00;
		MCU_DATA = 0x00;
		MCU_DATA = 0x00;

		WDT64_WakeUpcount = 0;
		while (WDT64_WakeUpcount < SLP2Frame)
		{
			MCU_ADDR = REG_STATUS_READ;
			read_tmp = (MCU_READ & 0x01);   

			MCU_ADDR = 0x9a;	// mcu parking, bug! necessary
//			MCU_DATA = 0x55;

			if(read_tmp != 0x01)
				com_service_enter_sleep(); 

			Enable_WDT_64;

            //enter sleep 2
            if (WDT_INTR_HAPPENED)
            {
				for ( delay = 0 ; delay < 1000 ; delay++ )
				{
					__nop();
				}

	            MCU_ADDR = REG_SLEEP_IN_2;
    	        MCU_DATA = 0x01;
				WDT_INTR_HAPPENED = 0;
				for ( delay = 0 ; delay < 10 ; delay++ )
				{
					__nop();
				}
            }
		}
		MCU_ADDR = 0x68;
		MCU_INDEX = 0X00;
		MCU_DATA = 0X01;
		for(delay = 0x0000; delay < 0x20; delay++)
		{
			__nop();
			__nop();
		}
		MCU_ADDR = 0x68;
		MCU_INDEX = 0X01;
		MCU_DATA = 0X04;

		MCU_ADDR = 0x7D;
		MCU_INDEX = 0x01;
		MCU_DATA = 0x00;	// RECOVER; MCU_DIV = 0
		

		MCU_ADDR = 0x3C;	// power on ADC
		MCU_DATA = 0x00;
		MCU_DATA = 0x00;

		Enable_SYNC_Timer;
	}
	else
	{
		while(1)
		{
			if ( IsEndOfScan() && IsEndOfFrameTimer() )
				break;
			else
				LPM0;
		}
	}
}
#endif
// ####### M A I N.c Re-direct function #######


// #####################  I n t e r r u p t   ######################
//#pragma vector=INT0_VECTOR
interrupt (INT0_VECTOR) drv_uart_isr(void)
{
	if( (UART_EN &= 0x02)==0x02)
	{
		UART_EN &= 0xFE; //Tx INT Disable(b0=0) //Tx fire=0
	}
}

//#pragma vector=INT1_VECTOR
interrupt (INT1_VECTOR) Timer0_int1(void)
{
	__eint();

	CCR0 += 10000;		// 1000 = 2ms

	Timer0_ReportRateCount++;
	if (Timer0_ReportRateCount >= Timer0_ReportRateBound)
	{
		bFrameTimerSync = 1;
		Timer0_ReportRateCount = 0x00;
	}
}

//#pragma vector=INT2_VECTOR
interrupt (INT2_VECTOR) Timer1_int2(void)
{

}

//#pragma vector=INT3_VECTOR
interrupt (INT3_VECTOR) INT_int3(void)
{

}

//#pragma vector=INT4_VECTOR
interrupt (INT4_VECTOR) I2C_Key_Rising(void)
{

}

//#pragma vector=INT5_VECTOR
interrupt (INT5_VECTOR) I2C_Key_Failing(void)
{

}

//#pragma vector=INT6_VECTOR
interrupt (INT6_VECTOR) INT_int6(void)
{

}

//#pragma vector=INT7_VECTOR
interrupt (INT7_VECTOR) INT_int7(void)
{

}

//#pragma vector=INT8_VECTOR
interrupt (INT8_VECTOR) Frame_Calculator(void)
{

}

//#pragma vector=INT9_VECTOR
interrupt (INT9_VECTOR) INT_int9(void)
{

}

//#pragma vector=INT10_VECTOR
interrupt (INT10_VECTOR) WDT_int10(void)
{
//	P2OUT ^= 0X01;
  	Disable_WDT_Timer;
	    WDT64_WakeUpcount++;
		WDT_INTR_HAPPENED = 1;
}

//#pragma vector=INT11_VECTOR
interrupt (INT11_VECTOR) GPIO0_int11(void)
{
	
}



//#pragma vector=INT12_VECTOR
interrupt (INT12_VECTOR) drv_scan_isr(void)
{ 
    vU8 int_b0, int_b1, mcu_addr, mcu_index;

    //start the nest-interrupt function, backup the needed variable    
    int_b0 = MCU_INTEN0;
    int_b1 = MCU_INTEN1;
    mcu_index = MCU_INDEX;
	mcu_addr = MCU_ADDR;
    
    //only enable I2C nest-interrupt;
    MCU_INTEN0 = 0x00;
    MCU_INTEN1 = 0x20;
    __eint();
	
	if (Bios_ScanMode == SELF_MODE_AUTO_CC)
	{
		ADCScan_Done = 1;
		SelfAutoCC_H_INT_Times++;
	}
	else if ( Bios_ScanMode == MUTUAL_MODE_AUTO_CC)
	{
		ADCScan_Done = 1;
		MutualAutoCC_H_INT_Times++;
	}

	if (PowerOnInitialProcess == 1)
	{
#if !SHRINK_FLASH_SIZE__AUTO_FREQ
		if (Bios_ScanMode == NL_MODE)
		{
			if (HoppingNowTableIndex < MAX_NL_FREQ_NUM)
			{
				drv_scan_noiselistening_baseline((&NLRawDataBuffer[NLReady][0]), (&NL_FREQ_BASELINE[HoppingNowTableIndex][0]), HoppingNowTableIndex, (&NL_RX_NOISE_LEVEL[0]),RxNum );

				HoppingNowTableIndex++;
				if (HoppingNowTableIndex == MAX_NL_FREQ_NUM)
				{
					HoppingNowTableIndex = 0x00;
					HoppingNowIndex = 0x00;
					NOW_HoppingBestFrequencyIndex = 0x00;

				#if SELF_ASSISTANCE
					Bios_ScanMode = SELF_MODE;
					drv_scan_trigger_self(SELF_ALL);
				#else
					Bios_ScanMode = MUTUAL_MODE;
					drv_scan_trigger_mutual();
				#endif
				}
				else
				{
					Bios_ScanMode = NL_MODE;
					drv_scan_trigger_mutual();
				}
			}
		}
		else if (Bios_ScanMode == SELF_MODE)
#else
		if (Bios_ScanMode == SELF_MODE)
#endif
		{
			// initial self mode baseline
			drv_scan_get_self_raw_data((&SelfRawDataBuffer[SelfReady][0]),(&BaseFrame.Sbase.Self[0]),SelfActualDataNum);

			Bios_ScanMode = MUTUAL_MODE;
			drv_scan_trigger_mutual();
		}
		else if (Bios_ScanMode == MUTUAL_MODE)
		{
			// initial mutual mode baseline
			drv_scan_get_mutual_raw_data((&MutualRawDataBuffer[MutualReady][0]),(&BaseFrame.Mbase.Mutual[0]),MutualActualDataNum);
			ADCScan_Done = 1;
			PowerOnInitialProcess = 0x00;
		}
		else
		{
			ADCScan_Done = 1; // execption!
		}
	}
	else		// PowerOnInitialProcess = 0x00;
	{
#if !SHRINK_FLASH_SIZE__AUTO_FREQ
		if (Bios_ScanMode == NL_MODE)
		{
			drv_scan_get_noise_level((&NLRawDataBuffer[NLReady][0]), (&NL_FREQ_BASELINE[HoppingNowTableIndex][0]), HoppingNowTableIndex, (&NL_RX_NOISE_LEVEL[0]), RxNum );

		#if SELF_ASSISTANCE
			Bios_ScanMode = SELF_MODE;
			drv_scan_trigger_self(SELF_ALL);
		#else
			Bios_ScanMode = MUTUAL_MODE;
			drv_scan_trigger_mutual();
		#endif

		}
		else if (Bios_ScanMode == SELF_MODE)
#else
		if (Bios_ScanMode == SELF_MODE)
#endif
		{
			drv_scan_get_self_raw_data((&SelfRawDataBuffer[SelfReady][0]),(&BaseFrame.Sbase.Self[0]),SelfActualDataNum);
#if !PROTOCOL_V20
			if ((SelfNeedReCalibrateCC == 1) && (SelfNeddReTryDone == 0x00))
			{
				SelfModeReCalibrateCCToTargetRawData();
			}
#endif
			Bios_ScanMode = MUTUAL_MODE;
			drv_scan_trigger_mutual();
		}
		else if (Bios_ScanMode == MUTUAL_MODE)
		{
			drv_scan_get_mutual_raw_data((&MutualRawDataBuffer[MutualReady][0]),(&BaseFrame.Mbase.Mutual[0]),MutualActualDataNum);
			ADCScan_Done = 1;
		}
		else
		{
			ADCScan_Done = 1; // execption!
		}
	}

    MCU_INTEN0 = int_b0;
    MCU_INTEN1 = int_b1;
	MCU_ADDR = mcu_addr;
    MCU_INDEX = mcu_index;
}


//#pragma vector=INT3_VECTOR
#if !PROTOCOL_V20
interrupt (INT13_VECTOR) IF_int13(void)
{

	I2cStatus = IF_STATUS2MCU;

	I2cDataBuff[I2cDataIdx] = I2C_DATA2MCU;

	if(I2cDataBuff[I2cDataIdx] == ProtocolUnLockKey[I2cDataIdx])
	{
		I2cUnLockMaches++;
	}
	else
	{
		I2cUnLockMaches = 0x00;    
	}
	I2cDataIdx++;

	if(I2cStatus & I2C_STATE_STOP)
	{
		if(I2cUnLockMaches == sizeof(ProtocolUnLockKey))
		{
			if(I2cUnLockMaches == I2cDataIdx)
			{
				KnlProtocol.Customer = WINPOWER;
				KnlProtocol.ProjectID = 0x00;
			}
		}
		if(KnlProtocol.Customer == USER)
		{
			if(I2cStatus & I2C_STATE_RNW)		//i2c_status b0== 1	--> I2C Read
			{
				int rd_len;
				rd_len  = (U16)IF_DMA_READ_BYTES_L;
				rd_len |= ((U16)IF_DMA_READ_BYTES_H<<8);
				ctm_user_protocol_rcv_read(rd_len);
			}
			else
			{
				ctm_user_protocol_rcv_write(I2cDataBuff,I2cDataIdx);
				ctm_user_protocol_DMA_ptr();
			}
		}
		if(KnlProtocol.Customer == WINPOWER)
		{
			if((I2cStatus & I2C_STATE_RNW) == 0x00)
			{
				knl_protocol_WP();
			}
		}

		I2cDataIdx = 0x00;
		I2cDataBuff[0] = 0x00;
		I2cDataBuff[1] = 0x00;
		I2cDataBuff[2] = 0x00;
	}
}
#else

U8 volatile *pI2cBuffer;	// Cannot has init value, otherwise will make large BIN file.
int __rd_len;

#define USER_CHARACTER_PROCESS 1

interrupt (INT13_VECTOR) IF_int13(void)
{
	int ii;
	I2cStatus = IF_STATUS2MCU;

	U8 i2c_data	= I2C_DATA2MCU;		// Get input byte if I2C WRITE.

	if(!(I2cStatus & I2C_STATE_RNW))		//b0:1=Read, 0=Write.
	{		//--- I2C write mode
		if(!ProtocolStateIs.skip_i2c_first_ch)
		{
			if(ProtocolStateIs.redirect_i2cw || (I2cDataIdx<sizeof(I2cDataBuff)))
			{
				pI2cBuffer[I2cDataIdx++] = i2c_data;
			}
		}
		else ProtocolStateIs.skip_i2c_first_ch=FALSE;

		#if USER_CHARACTER_PROCESS
		if(ProtocolStateIs.user_protocol)
		{
			ctm_user_protocol_char_handler(i2c_data);
		}
		#endif
	}

	if((I2cStatus & I2C_STATE_STOP))	// I2C write STOP ?
	{		//--- Yes, stop.
		if(!(I2cStatus & I2C_STATE_RNW))		//b0:1=Read, 0=Write.
		{		//--- Stop of I2C write
			if((ProtocolStateIs.user_protocol) && (I2cDataIdx == sizeof(ProtocolUnLockKey)))
			{
				for(ii=0; ii<sizeof(ProtocolUnLockKey); ii++)
				{
					if(ProtocolUnLockKey[ii] != pI2cBuffer[ii]) break;
				}
				if(ii>=sizeof(ProtocolUnLockKey))	// All bytes matched ?
				{
					ProtocolStateIs.user_protocol = FALSE;
					ProtocolStateIs.redirect_i2cw = FALSE;
					pI2cBuffer = I2cDataBuff;
				}
			}

			if(ProtocolStateIs.user_protocol)
			{
				ctm_user_protocol_rcv_write((void *)pI2cBuffer,I2cDataIdx);
			}
			else
			{
				knl_protocol_WP();
			}
			I2cDataIdx = 0;	// Clear buffer.
		}
		else
		{		//--- Stop of I2C read
			__rd_len  = (U16)IF_DMA_READ_BYTES_L;
			__rd_len |= ((U16)IF_DMA_READ_BYTES_H<<8);
			if(ProtocolStateIs.user_protocol)
			{
				ctm_user_protocol_rcv_read(__rd_len);
			}
			else
			{
				I2cReadStopProcess(__rd_len);
			}
		}
	}
}
#endif
// #####################  I n t e r r u p t  ######################

