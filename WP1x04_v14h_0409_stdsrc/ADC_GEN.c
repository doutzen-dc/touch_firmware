#include "Globaltype.h"

#ifndef flashpara

//typedef unsigned char U8;
extern U8 self_sequ;
extern U8 mutu_sequ;
sfr MCU_ADDR = 0xA1;
sfr MCU_DATA = 0xA3;


U8 code ChMapping[48] = {
  29,28,27,26,25,24,17,16,15,14,13,12,  //TX Channel No =12
  11,10,9,8,7,6,5,4,3,2,1,37,38,39,40,41,42,43,44,45,46,47, //RX Channel No =22
  0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void ADC_CYC_SETTING_GEN()
{
  self_sequ = 3;
  mutu_sequ = 2;

  //ADC0 CYC Setting
  MCU_ADDR = 0x40;
  //---Self Mode---//
  MCU_DATA = 0x76;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x07;
  MCU_DATA = 0x00;

  //ADC1 CYC Setting
  MCU_ADDR = 0x41;
  //---Self Mode---//
  MCU_DATA = 0x67;
  MCU_DATA = 0x04;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x00;

  //ADC2 CYC Setting
  MCU_ADDR = 0x42;
  //---Self Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x06;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x47;
  MCU_DATA = 0x00;

  //ADC3 CYC Setting
  MCU_ADDR = 0x43;
  //---Self Mode---//
  MCU_DATA = 0x46;
  MCU_DATA = 0x07;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x00;

  //ADC4 CYC Setting
  MCU_ADDR = 0x44;
  //---Self Mode---//
  MCU_DATA = 0x67;
  MCU_DATA = 0x04;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
    MCU_DATA = 0x47;
  MCU_DATA = 0x00;

  //ADC5 CYC Setting
  MCU_ADDR = 0x45;
  //---Self Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x06;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x00;

  //ADC6 CYC Setting
  MCU_ADDR = 0x46;
  //---Self Mode---//
  MCU_DATA = 0x45;
  MCU_DATA = 0x07;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x47;
  MCU_DATA = 0x00;

  //ADC7 CYC Setting
  MCU_ADDR = 0x47;
  //---Self Mode---//
  MCU_DATA = 0x57;
  MCU_DATA = 0x04;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x00;

  //ADC8 CYC Setting
  MCU_ADDR = 0x48;
  //---Self Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x05;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x47;
  MCU_DATA = 0x00;

  //ADC9 CYC Setting
  MCU_ADDR = 0x49;
  //---Self Mode---//
  MCU_DATA = 0x45;
  MCU_DATA = 0x07;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x74;
  MCU_DATA = 0x00;

  //ADC10 CYC Setting
  MCU_ADDR = 0x4A;
  //---Self Mode---//
  MCU_DATA = 0x57;
  MCU_DATA = 0x04;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x47;
  MCU_DATA = 0x00;

  //ADC11 CYC Setting
  MCU_ADDR = 0x4B;
  //---Self Mode---//
  MCU_DATA = 0x04;
  MCU_DATA = 0x05;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  //---Mutual Mode---//
  MCU_DATA = 0x04;
  MCU_DATA = 0x00;

  //Self Mode ADC Offset Value
  MCU_ADDR = 0x54;
  MCU_DATA = 0x05;
  MCU_DATA = 0x20;
  MCU_DATA = 0x15;
  MCU_DATA = 0x02;
  MCU_DATA = 0x1D;
  MCU_DATA = 0x12;
  MCU_DATA = 0x06;
  MCU_DATA = 0x1A;
  MCU_DATA = 0x0F;
  MCU_DATA = 0x09;
  MCU_DATA = 0x17;
  MCU_DATA = 0x0C;
  MCU_DATA = 0x21;
  MCU_DATA = 0x04;
  MCU_DATA = 0x1F;
  MCU_DATA = 0x14;
  MCU_DATA = 0x01;
  MCU_DATA = 0x1C;
  MCU_DATA = 0x11;
  MCU_DATA = 0x07;
  MCU_DATA = 0x19;
  MCU_DATA = 0x0E;
  MCU_DATA = 0x0A;
  MCU_DATA = 0x16;
  MCU_DATA = 0x03;
  MCU_DATA = 0x1E;
  MCU_DATA = 0x13;
  MCU_DATA = 0x00;
  MCU_DATA = 0x1B;
  MCU_DATA = 0x10;
  MCU_DATA = 0x08;
  MCU_DATA = 0x18;
  MCU_DATA = 0x0D;
  MCU_DATA = 0x0B;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;


  //Mutual Mode ADC Offset Value
  MCU_ADDR = 0x55;
  MCU_DATA = 0x15;
  MCU_DATA = 0x0A;
  MCU_DATA = 0x13;
  MCU_DATA = 0x08;
  MCU_DATA = 0x11;
  MCU_DATA = 0x06;
  MCU_DATA = 0x0F;
  MCU_DATA = 0x04;
  MCU_DATA = 0x0D;
  MCU_DATA = 0x02;
  MCU_DATA = 0x0B;
  MCU_DATA = 0x00;
  MCU_DATA = 0x14;
  MCU_DATA = 0x09;
  MCU_DATA = 0x12;
  MCU_DATA = 0x07;
  MCU_DATA = 0x10;
  MCU_DATA = 0x05;
  MCU_DATA = 0x0E;
  MCU_DATA = 0x03;
  MCU_DATA = 0x0C;
  MCU_DATA = 0x01;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;


  //Mutual Mode TX Sequence
  MCU_ADDR = 0x4C;
  MCU_DATA = 0x1D;
  MCU_DATA = 0x1C;
  MCU_DATA = 0x1B;
  MCU_DATA = 0x1A;
  MCU_DATA = 0x19;
  MCU_DATA = 0x18;
  MCU_DATA = 0x11;
  MCU_DATA = 0x10;
  MCU_DATA = 0x0F;
  MCU_DATA = 0x0E;
  MCU_DATA = 0x0D;
  MCU_DATA = 0x0C;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
  MCU_DATA = 0x00;
}
#endif