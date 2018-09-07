/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _1x07_REG_H
#define _1x07_REG_H


//The address of each special-function-register is defined in "wp118x_periph.x"
//and will be include as linking process
sfrb(P2_OE               ,__P2_OE);
sfrb(DIS_IFRDFW          ,__DIS_IFRDFW);
sfrb(IF_DMA_ADDR_L       ,__IF_DMA_ADDR_L);
sfrb(IF_DMA_ADDR_H       ,__IF_DMA_ADDR_H);
sfrb(IF_DMA_READ_BYTES_L ,__IF_DMA_READ_BYTES_L);
sfrb(IF_DMA_READ_BYTES_H ,__IF_DMA_READ_BYTES_H);
sfrb(MCU_ADDR            ,__MCU_ADDR);
sfrb(MCU_INDEX           ,__MCU_INDEX);
sfrb(MCU_DATA           ,__MCU_DATA);			// T1007 modify
sfrb(MCU_READ            ,__MCU_READ);
sfrb(MCU_SPIM            ,__MCU_SPIM);
sfrb(TCON_MEM            ,__TCON_MEM);		// T1007


//Data align problem, need to define two seprate bytes
sfrb(MCU_INTEN0          ,__MCU_INTEN0);

//Bit0: Uart interrupt enable
//Bit1: Timer threshold 0(CCR0) interrupt enable
//Bit2: Timer threshold 1(CCR1) interrupt enable
//Bit3: Reserve
//Bit4: I2C key rising interrupt enable(refer to register 0xFB)
//Bit5: I2C key falling interrupt enable(refer to register 0xFB)
//Bit6: ID Tracking finish interrupt enable
//Bit7: Reaerve
sfrb(MCU_INTEN1          ,__MCU_INTEN1);

//Bit0: Frame calculate finish interrupt
//Bit1: Reserve
//Bit2: Watch dog interrupt enable(shared to other module??)
//Bit3: GPIO0 interrupt enable
//Bit4: TCON interrupt enable
//Bit5: IF interrupt enable

//0x022B??
sfrb(SYS_STATUS          ,__SYS_STATUS);
sfrb(I2C_DATA2MCU        ,__I2C_DATA2MCU);
sfrb(IF_STATUS2MCU       ,__IF_STATUS2MCU);
sfrb(SPI_DATA2MCU        ,__SPI_DATA2MCU);
sfrb(UART_EN             ,__UART_EN);
sfrb(UART_SDATA          ,__UART_SDATA);
sfrw(UART_BR             ,__UART_BR);
sfrb(UART_RDATA          ,__UART_RDATA);


//register address
#define REG_SLEEP_IN				0x20
#define REG_SLEEP_OUT				0x21
#define REG_SENSE_OFF				0x22
#define REG_SENSE_ON				0x23
#define REG_TCON_INT_CLR_0X24		0x24
#define REG_STATUS_READ				0x28
#define REG_SOFTWARE_RST			0x2A
#define REG_IFDMA_ADDR				0x2B
#define REG_HARDWARE_VER			0x2C
#define REG_CHIP_ID					0x2D	// 2 bytes

#define REG_SENSE_START				0x2F

#define REG_TCON_H_SETTING			0x30
#define REG_TCON_V_SETTING			0x31

#define REG_TCON_RELOADER_SETTING	0x34
#define REG_SENSE_CHANNEL_CTRL		0x36
#define REG_RELOAD_CTRL				0x37
#define REG_TCON_INT_CLR_0X38		0x38

#define REG_ADC_SETTING				0x3A
#define REG_ADC_ND_CTRL				0x3B
#define REG_ADC_PWD_CTRL			0x3C
#define REG_TCON_INT_ENABLE			0x3F
#define REG_REPEAT_NUM				0X4D

#define REG_TX_OFFSET_SEQ			0x53
#define REG_RAW_OFFSET_SEQ			0x54

#define REG_I2C_STRETCH				0x60

#define REG_FPDMA_0X63				0X63
#define REG_FPDMA_0X64				0X64
#define REG_PUMP_CLK_SELECT			0X65

#define REG_PUMP_VOLT_SELECT		0X67
#define REG_PUMP_SWITCH_ENABLE_I	0X68
#define REG_PUMP_SWITCH_ENABLE_II	0X69

#define REG_FCAL_ADDR_IDX			0x6A
#define REG_FCAL_SIZE				0x6B
#define REG_FCAL_MODE				0x6C
#define REG_IFDMA_MODE				0x6D
#define REG_DEGLITCH_SLP_MODE		0x6E

#define REG_OSC_CTRL				0x7C
#define REG_CLK_DIVIDER				0x7D
#define REG_TCON_DIV2				0x7E
#define REG_REGULATOR				0x7F

#define REG_SCAN_STATUS				0x81
#define REG_SLEEP_IN_2				0x82

#define REG_RESERVE_REG_A			0x98
#define REG_RESERVE_REG_B			0x99
#define REG_RESERVE_REG_C			0x9A

#define REG_FILTER_SETTING			0xB0

#define REG_RAW_COUNT_DBG			0xB3


#define REG_TCON_REG_TABLE_0		0xC0
#define REG_TCON_REG_TABLE_1		0xC1
#define REG_TCON_REG_TABLE_2		0xC2
#define REG_TCON_REG_TABLE_3		0xC3

#define REG_MODULATION_SETTING		0xC7
#define REG_MOD_TRIG_SETTING		0xC8
#define REG_MOD_STATUS				0xC9
#define REG_TCON_SRAM_TARGET		0xCA
#define REG_FPGA_RAW_ENABLE			0xCE
#define REG_FPGA_RAW_DATA			0xCF

#define REG_CC_SEL					0xE1
#define REG_CFB_SEL					0xE2
#define REG_ADC_OTHER_SETTING		0xE4
#define REG_ADC_ADJUST				0xE5

#define REG_CASCADE					0xEB
#define REG_DUM_CLK_CTRL			0xEC
#define REG_OSC_TRIM				0xED

#define REG_IO_ADJ					0xEF

#define REG_I2C_ADDR_CTRL			0xF1
#define REG_SPECIAL_PATTERN_SETTING	0xF2
#define REG_SCLK_NON_OVERLAP		0xF3
#define REG_ADC_SAMPLE_CLK_DIV		0xF4
#define REG_CC_CLK_NUM				0xF5
#define REG_SPECIAL_CONFIG			0xF6
#define REG_BIST					0xF8

#define REG_SPECIAL_FUNC_0XF9		0xF9
#define REG_SPECIAL_FUNC1_0XFA		0xFA
#define REG_I2C_KEY					0xFB
#define REG_ENABLE_DBG_PORT			0xFE
#define REG_MCU_TEST				0xFF


#define INT0_VECTOR     (0 * 2u)  //UART_INT		/* 0xFFE0 DAC/DMA */
#define INT1_VECTOR     (1 * 2u)  //Timer0 			/* 0xFFE2 Port 2 */
#define INT2_VECTOR     (2 * 2u)  //Timer1 			/* 0xFFE4 USART 1 Transmit */
#define INT3_VECTOR     (3 * 2u)  //Hover INT 		/* 0xFFE6 USART 1 Receive */
#define INT4_VECTOR     (4 * 2u)  //Hover INT		/* 0xFFE8 Port 1 */
#define INT5_VECTOR     (5 * 2u)  //GPIO0			/* 0xFFEA Timer A CC1-2, TA */
#define INT6_VECTOR     (6 * 2u)  //I2CKEY Rising	/* 0xFFEC Timer A CC0 */
#define INT7_VECTOR     (7 * 2u)  //I2CKEY Falling	/* 0xFFEE ADC */
#define INT8_VECTOR     (8 * 2u)  /* 0xFFF0 USART 0 Transmit */
#define INT9_VECTOR     (9 * 2u)  /* 0xFFF2 USART 0 Receive */
#define INT10_VECTOR    (10 * 2u) /* 0xFFF4 Watchdog Timer */
#define INT11_VECTOR    (11 * 2u) /* 0xFFF6 Comparator A */
#define INT12_VECTOR    (12 * 2u) /* 0xFFF8 Timer B CC1-6, TB */
#define INT13_VECTOR    (13 * 2u) /* 0xFFFA Timer B CC0 */

	
#endif	//_1x05_REG_H