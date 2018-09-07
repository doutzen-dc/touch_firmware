/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#ifndef _WP1001HW_H_
#define _WP1001HW_H_

#include "IT51.h"

#define TOTAL_SELF_CHANNELS 31//36
#define MAX_FINGER_SUPPORT 2
#define MAX_KEY_SUPPORT 4
#define MAX_GESTURE_SUPPORT 1
#define MAX_REPORT_LEN  (MAX_FINGER_SUPPORT*4+MAX_KEY_SUPPORT*2+MAX_GESTURE_SUPPORT*2)

#define I2C_DEV_ADR 0xdc

#define SYS_CLK 30// MHz

#define T_1MS (1000*(SYS_CLK/12))
#define T_5MS (5000*(SYS_CLK/12))


#define lobyte(AA)  ((unsigned char) (AA))
#define hibyte(AA)  ((unsigned char) (((unsigned int) (AA)) >> 8))
#define loword(AA)  ((unsigned int)  (AA))   //20120116JL
#define hiword(AA)  ((unsigned int)  (((unsigned long int) (AA)) >> 16)) //20120116JL
  
//#define lobyte(v)  (*(((unsigned char *) (&v) + 1)))
//#define hibyte(v) (*((unsigned char *) (&v)))



#define ACCESS_REG() {P0_1=0;}
#define RELEASE_REG() {P0_1=1;}

//sbit INTO = P0^0; //JS20130619b XX : move to globaltype.h

//*************************************************************************************************************************//
//*  SFR Part                                                                                                              //
//*************************************************************************************************************************//
//sfr I2CDATA = 0x9A;
sfr D2CMDIF = 0x9A;
sfr I2CSTAT = 0x9B;
////sfr I2CBUF0 = 0x9C;
////sfr I2CBUF1 = 0x9D;
////sfr I2CBUF2 = 0x9E;
////sfr I2CBUF3 = 0x9F;

sfr P1_IO  = 0x94; //0 : out , 1 : In
sfr MCU_ADDR  = 0xA1;
sfr MCU_INDEX = 0xA2;
sfr MCU_DATA  = 0xA3;
sfr MCU_READ  = 0xA4;
sfr DMA_ADR_HI = 0xA5;
sfr DMA_ADR_LO = 0xA6;
sfr OPCODE    = 0xB1;
sfr OP1L      = 0xB2; //OP1:24bits
sfr OP1M      = 0xB3;
sfr OP1H      = 0xB4;
sfr OP2L      = 0xB5; //OP2:24bits
sfr OP2M      = 0xB6;
sfr OP2H      = 0xB7;
sfr DSP_STAT  = 0xB9;
sfr OP3L      = 0xBA; //OP3:24bits
sfr OP3M      = 0xBB;
sfr OP3H      = 0xBC; 
sfr OP4L      = 0xBD; //OP4:24bits
sfr OP4M      = 0xBE;
sfr OP4H      = 0xBF;

sfr DTM_EN    = 0xDA;
sfr PDTM      = 0xDB;
sfr DTM_H     = 0xDC;
sfr DTM_L     = 0xDD;
sfr UEN     = 0xE2;
sfr USDATA    = 0xE3;
sfr UBR_H     = 0xE4;
sfr UBR_L     = 0xE5;
sfr ID_FIN    = 0xE7; //IDT
  // sfr OPC (0xB1) //20120117JL
  //Working Operation
#define MACOP                         0x00 //MAC Operation
#define DIVOP                         0x01 //DIV Operation
#define LPFOP                         0x02 //LPF Operation
  //Accumulator ON or NOT
#define ACCON                         0x20 //Accumulator ON
#define ACCOFF                        0x00 //Accumulator OFF
  //Output path
#define OUTALL                        0x00 //Output bit 0~27
#define OUTDIV16                      0x40 //Output bit 4~27
  //Output operation done status
#define OPDONE                        0x80 //operation is done
  //Filter divider
#define LPFDIV2                       0x00
#define LPFDIV4                       0x04
#define LPFDIV8                       0x08
#define LPFDIV16                      0x0C
  //I2CSTAT
#define I2CSTATE_RnW                  0x01
#define I2CSTATE_DnA                  0x02
#define I2CSTATE_STOP                 0x04


//*************************************************************************************************************************//
//*  WP1001 Register Define                                                                                                //
//*************************************************************************************************************************//
#define TCON_H_PARAM                  0x30
#define TCON_V_PARAM                  0x31
#define Sense_channel_Control         0x36
#define SENSE_SEQUENCE                0x37

#define ADC_EN_SETTING                0x3A
#define ADC_ND_CTRL                   0x3B
#define ADC_PWD_CTRL                  0x3C
#define ADC0_CYC_SETTING              0x40
#define ADC1_CYC_SETTING              0x41
#define ADC2_CYC_SETTING              0x42
#define ADC3_CYC_SETTING              0x43
#define ADC4_CYC_SETTING              0x44
#define ADC5_CYC_SETTING              0x45
#define ADC6_CYC_SETTING              0x46
#define ADC7_CYC_SETTING              0x47
#define ADC8_CYC_SETTING              0x48
#define ADC9_CYC_SETTING              0x49
#define ADC10_CYC_SETTING             0x4A
#define ADC11_CYC_SETTING             0x4B
#define MUTUAL_TX_SEQ                 0x4C
#define REPEAT_SEQ                    0x4D
#define XDATA_DMA_CONTROL             0x50
#define DMA_SELF_SETTING              0x51
#define DMA_MUTUAL_SETTING            0x52
#define DMA_NORMALIZE                 0x53
#define SELF_OFFSET_SEQ               0x54
#define MUTUAL_OFFSET_SEQ             0x55
#define ID_TRACK_ADDR_IDX             0x60
#define ID_TRACK_NUM                  0x61
#define ID_TRACK_NUM_ADDBYTE          0x62
#define PUMP_CONTROL                  0x65
#define LVDD_DETECT                   0x66
#define OSC_CTRL                      0x7C
#define CLOCK_DIV                     0x7D
#define SCAN_STATUS                   0x81
#define PRE_FETCH_CONTROL             0xA9
#define FILTER_SETTING                0xB0
#define FILTER_TH_SETTING             0xB1
#define RC_SEL                        0xE0
#define CC_SEL                        0xE1
#define CFB_SEL                       0xE2
#define ADC_OTHER_SETTING             0xE4
#define ADC_ADJUST                    0xE5
#define OSC_TRIM                      0xED
#define IO_TRIM                       0xEF
#define I2C_ADDR_CTL                  0xF1
#define PREC_CTRL                     0xF2
#define SCLK_NON_OVERLAP              0xF3
#define ADC_SAMPLE_CLOCK_DIV          0xF4
#define SPEC_CONFIG_F6                0xF6
#define SPEC_FUNC_F9                  0xF9
#define SPEC_FUNC_FA                  0xFA
#define LOCK_PORT                     0xFB

#endif
