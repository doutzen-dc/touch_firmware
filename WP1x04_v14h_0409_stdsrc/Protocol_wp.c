#include <string.h>
#include <stdio.h>
#include "bios.h"
#include "protocol_wp.h"
#include "tp_lib.h"
#include "user_protocol.h"

#if  !TP_CUSTOMER_PROTOCOL //0
#define AMS5505K_Protocol
#define PD_Horizontal
#endif
//--------------- Report related functions ----------------------
//#define RPT_BUF_SIZE 58
tProtocolHeader xdata tp_header;
//U8 xdata report_Buffer[RPT_BUF_SIZE];
volatile U8 xdata *ptr_report;
U8 report_BufPtr=0;
#ifdef AMS5505K_Protocol
volatile U8 fg_MTK_8byte = 0;
volatile U8 fg_MTK_Header = 0;
#endif
//idata U8 xdata *ptr;
U8 xdata *ptr;

//#define SHRINK_XRAM

#if 0 // DebugForAPKTest
#define  APK_DEBUG_SIZE 16  //words
S16 xdata report_DebugInfo_1[APK_DEBUG_SIZE];   // for Debug Info
S16 xdata report_DebugInfo_2[APK_DEBUG_SIZE];   // for Debug Info
//for APK debug
extern bit APK_ReadBackOK_1;
extern bit APK_ReadBackOK_2;
extern bit APK_Enter_DEBUG_Mode;  // 1: enter debug mode, 0:exit
#endif

#if !TP_CUSTOMER_PROTOCOL
#define APK_Debug_NSL_size    1//64   //1: in order to gain more xram size
S16 xdata report_DebugInfo_NSL[APK_Debug_NSL_size];
U8 fgGETFNSL = 1;
U8 GETCCmode=0;
U8 GETCCPacketLen = 0;
U16 IndexPointTo = 0;
U8 PacketSize = 0;
U16 LONGPacket = 0;
U8 APActiveCC = 0;
#endif

#define report_IsExist() (report_BufPtr >= 1)
#define report_ClearReportBuffer() {report_BufPtr = 0;}

#define INTO_Out 1

#ifdef JUST_for_MobilePhone_DEBUG
//U16 mutuPeak,self_peakX,self_peakY;
bit debugEnable =0;
//extern S16 *pSelfdelta;
S16 debug_data[30];
U8 debug_cn=0;
U16 debuger=0;
#endif
//#if SwitchLowFreqMode
extern U8 AP_ActiveFlag;
//#endif
U8 DriverAlive = 0;
U8 cmdSELF_SING_ALL = 1;    // 1:EN-ALL; 0:Single
extern const U8 I2C_BufferSize;
U16 Timer_Counter = 0;
extern tProtocol Protocol;
extern tProtocol preProtocol;
extern volatile U8  MutualComOffset[];
extern volatile U8  SelfComOffset[];
extern code U8 ChMapping[];
#if !TP_CUSTOMER_PROTOCOL
extern volatile U8 TestModeSelfCC[];
#endif
extern U8 last_finger;
extern bit TestModeFlag;
extern S16 * ptrLastQuaDelta; // for store testMode mutual CC by nodes data

// --- extern flash Init parameter ---//
extern code U8  Flash_data_en ;
extern code tProtocol  flProtocol ;
extern code tSystemPara flSystemPara ;
extern code tPannel  flPannel ;
extern code tRawcountPara  flRawPara ;
extern code tKeyPara  flKeyPara;
extern code tMutuPofPara flMutuPofPara;
extern code tLargeAreaPara flLargeAreaPara;
extern code tJitterIIRPara flJitterIIRPara;
extern code tEnvironmentTHDPara flEnvironmentTHDPara;
extern code tDEBUGSettingPara fDEBUGSettingPara;
extern code tFREQAnalysis fFREQAnalysis;


void report_Reset(void)
{
    report_BufPtr=0;
    if (Protocol.Customer==0)
        ptr = &report_Buffer[1];
}
void report_TxOut(void)
{
#ifdef AMS5505K_Protocol
  fg_MTK_8byte = 0;
  ptr_report = report_Buffer;
#endif  
  if(Protocol.Customer==0)
     report_Buffer[0] = report_BufPtr+1;
#if INTO_Out
    INTO=0;
#endif
    report_Reset();
} // Alert host to get the messages.
#ifdef GROUND_NOISE
U8 sort_number = 0;
data U8 first_sorting = 0;
data U16 sorting_buffer_x[8],sorting_buffer_y[8];
U16 sorting_temp;
#endif
extern bit Intr_trig;

#if DebugForAPKTest
void report_DebugInfo_UserDefine1(U8 position, S16 message)   //command 0x19
{
    if (APK_Enter_DEBUG_Mode == 1)
    {
        report_DebugInfo_1[position]=(S16)(message);
        Intr_trig = 1;
    }
    else
    {
        Intr_trig = 0;
    }
}

void report_DebugInfo_UserDefine2(U8 position, S16 message)   //command 0x1A
{
    if (APK_Enter_DEBUG_Mode == 1)
    {
        report_DebugInfo_2[position]=(S16)(message);
        Intr_trig = 1;
    }
    else
    {
        Intr_trig = 0;
    }
}

void report_DebugInfo_UserDefine_Clear()
{
    U8 x;
    if (APK_ReadBackOK_1 ==1)    // initialize for user define buffer
    {
        for (x=0; x<APK_DEBUG_SIZE ; x++)
            report_DebugInfo_1[x] =0x1111;
        APK_ReadBackOK_1 = 0;     // clear for next frame
    }
    else if (APK_ReadBackOK_2 ==1)
    {
        for (x=0; x<APK_DEBUG_SIZE ; x++)
            report_DebugInfo_2[x] =0x2222;

        APK_ReadBackOK_2 = 0;     // clear for next frame
    }
}
#endif

void report_SendKey(U8 key_mask)
{
    *ptr++ = 0x20;  // type-code for key
    *ptr++ = key_mask;
    report_BufPtr += 2;
}
void report_SendPoint() //tFingerIDList FingerIDList
{   // This routine will over-ride the current buffer !!
    U8 data x, z_force;
    U8 jj;
#ifdef JUST_for_MobilePhone_DEBUG
    U16 u16cn =0;
#endif
#ifdef AMS5505K_Protocol
    S16 AMStmp_Y[MAX_FINGERS]=0;
    S16 AMStmp_X[MAX_FINGERS]=0;
#endif
    z_force=0xc0;
    x=0;
    jj=FingerIDList_Output.fingers_number;
    while (jj)  //if(FingerIDList_Output.fingers_number>0)
    {
        if (FingerIDList_Output.Touch_Info[x])
        {
        #ifdef AMS5505K_Protocol
            // for Jack AP
            #ifdef JackAPShow
            AMStmp_X[x] = FingerIDList_Output.Pos[1][x];
            AMStmp_Y[x] = FingerIDList_Output.Pos[0][x];
            
            AMStmp_X[x] = (((AMStmp_X[x] * 10) /7)-1); 
            AMStmp_Y[x] = ((((AMStmp_Y[x]+3) * 7) /5));

            // x-offset
            if (AMStmp_X[x] < 0)
              AMStmp_X[x] = 0;
            if (AMStmp_X[x] >= 1920)
              AMStmp_X[x]  = 1919;      //X max-1

            //y-offset
            if (AMStmp_Y[x] <= 4)
              AMStmp_Y[x] = 0;
            else if (AMStmp_Y[x]  >= 1080)
              AMStmp_Y[x]  = 1079;      //Y max-1

            FingerIDList_Output.Pos[1][x] = AMStmp_X[x];
            FingerIDList_Output.Pos[0][x] = AMStmp_Y[x];
            #endif

          #ifdef PD_Horizontal
            AMStmp_X[x] = FingerIDList_Output.Pos[1][x];
            AMStmp_Y[x] = FingerIDList_Output.Pos[0][x];
            
            AMStmp_X[x] = (((AMStmp_X[x] * 10) /7)-1); 
            AMStmp_Y[x] = ((((AMStmp_Y[x]+3) * 7) /5));

            // x-offset
            if (AMStmp_X[x] < 0)
              AMStmp_X[x] = 0;
            if (AMStmp_X[x] >= 1920)
              AMStmp_X[x]  = 1919;      //X max-1

            //y-offset
            if (AMStmp_Y[x] <= 4)
              AMStmp_Y[x] = 0;
            else if (AMStmp_Y[x]  >= 1080)
              AMStmp_Y[x]  = 1079;      //Y max-1

            FingerIDList_Output.Pos[1][x] = (1079 - AMStmp_Y[x]);
            FingerIDList_Output.Pos[0][x] = AMStmp_X[x];
          #else
            AMStmp_X[x] = FingerIDList_Output.Pos[1][x];
            AMStmp_Y[x] = FingerIDList_Output.Pos[0][x];
            
            AMStmp_X[x] = ((((AMStmp_X[x]+3) * 7) /5));
            AMStmp_Y[x] = (((AMStmp_Y[x] * 10) /7)-1);

            // x-offset
            if (AMStmp_X[x] <= 4)
              AMStmp_X[x] = 0;
            else if (AMStmp_X[x]  >= 1080)
              AMStmp_X[x]  = 1079;      //X max-1


            //y-offset
            if (AMStmp_Y[x] < 0)
              AMStmp_Y[x] = 0;
            if (AMStmp_Y[x] >= 1920)
              AMStmp_Y[x]  = 1919;      //Y max-1

            FingerIDList_Output.Pos[1][x] = AMStmp_X[x];
            FingerIDList_Output.Pos[0][x] = AMStmp_Y[x];
          #endif

        #endif
            *ptr++ = ((x& 0x0f) << 3) | (FingerIDList_Output.Pos[1][x] >>8) | 0x80;
            *ptr++ = (U8) (FingerIDList_Output.Pos[1][x] & 0xff);
            *ptr++ = (z_force & 0xe0) | (FingerIDList_Output.Pos[0][x]>>8);
            *ptr++ = (U8) (FingerIDList_Output.Pos[0][x] & 0xff);
            //printf("ID %02bu  Y=%04d X=%04d \r\n",x,FingerIDList_Output.Pos[0][x],FingerIDList_Output.Pos[1][x]);
            jj -- ;
        }
        x++;
        //report_BufPtr += (sizeof(tFingerReport) * FingerIDList_Output.fingers_number);  // Include the length 1 byte.
    }
  #ifdef JUST_for_MobilePhone_DEBUG
    if(debuger<500)
    {
      debuger ++;
    }   
  
    if((debugEnable==0)&&(FingerIDList_Output.fingers_number==3)) // enable debug mode
    {
      if(debuger==500)
      {
        debugEnable=1;
        debug_cn = 0;     
      }
    }
    else if((debugEnable==1)&&(FingerIDList_Output.fingers_number==4)) // disable debug mode
    {
      debuger = 0;
      debugEnable=0;
      debug_cn = 0;     
    }
    if(debugEnable)
    {
      #if 1 //--- check mutual delta    
      if(FingerIDList_Output.fingers_number <4)
      {
        *ptr++ = ((2& 0x0f) << 3) | (10 >>8) | 0x80;
        *ptr++ = (U8) (10 & 0xff);
        *ptr++ = (z_force & 0xe0) | ((Debug_SelfDeltaMax)>>8);
        *ptr++ = (U8) ((Debug_SelfDeltaMax) & 0xff);      

        *ptr++ = ((3& 0x0f) << 3) | (20 >>8) | 0x80;
        *ptr++ = (U8) (20 & 0xff);
        *ptr++ = (z_force & 0xe0) | ((Debug_MutuDeltaMax)>>8);
        *ptr++ = (U8) ((Debug_MutuDeltaMax) & 0xff);          
        FingerIDList_Output.fingers_number = 4;       
      }
      KeyPress = 0 ;
      for(u16cn = 0;u16cn<10000;u16cn++)
      {
        for(jj =0;jj<40;jj++)     
        {
          //u16cn = u16cn;
          TA = 0xAA;TA = 0x55;  RWT = 1;
        }
      }
      #endif
    }   
  #endif  
    report_BufPtr += (sizeof(tFingerReport) * FingerIDList_Output.fingers_number);  // Include the length 1 byte.
}

//--------------- I2C related functions ----------------------

U8 data i2c_RcvLen;
U8 data i2c_status;
void i2c_rcv_ISR5(void) interrupt 10 using 1    // This is INT5: i2c write 0x20~0x25 interrupt for byte received
{
    EIF |= 0x08;  // Clear INT5 flag for next interrupt.
}

#define I2CSTATE_RnW 0x01
#define I2CSTATE_DnA 0x02
#define I2CSTATE_STOP 0x04

extern bit nbridge_i2c_EN;
extern bit bridge_i2c_done;
extern bit bridge_i2c_done_pre;
unsigned char xdata *rcv_ptr; // A pointer to xdata ram.
bit notLoad_rcv_ptr =0;

extern U8 xdata i2c_RcvBuf[];

#if !TP_CUSTOMER_PROTOCOL
void i2c_rcv_ISR1_Winpower(void)
{
    U8 x;

    switch (i2c_RcvBuf[0])  // Register/Command/Message.
    {
    case MSG_GET_REPORTS:
      if (DriverAlive == 1)
        {
          ptr_report = ( report_Buffer + (i2c_RcvBuf[1]<<3));   
          rcv_ptr = ptr_report;
        }
        else
          rcv_ptr = report_Buffer;
#if INTO_Out
        INTO=1; //De-active INTO.
#endif
        break;
    case MSG_SET_MODE:
        preOpMode[0] =i2c_RcvBuf[1] ;
        preOpMode[1] =i2c_RcvBuf[2];
    #ifdef AMS5505K_Protocol
        // for PD 5 "
        fg_MTK_Header = 0;
    #endif
        break;
    case MSG_GET_HEADER:
        rcv_ptr = (char *)&tp_header;
        rcv_ptr += (i2c_RcvBuf[1]<<3);
        // jan ok
        if (i2c_RcvBuf[1] > 0)
          DriverAlive = 1;
        else
          DriverAlive = 0;   
        break;
    case MSG_GET_STATUS:
        //rcv_ptr = (char *)&TpStatus;
        rcv_ptr = OpMode;
        nbridge_i2c_EN = 0;
        break;
        //20130522JSa : Start
#if !TP_CUSTOMER_PROTOCOL
    case MSG_SET_CHARGER:
        charger_on_status = i2c_RcvBuf[1];
        break;
#endif
        //20130522JSa : End
    case I2C_READ_RAW_SELF:
  rcv_ptr = (U8 xdata *)ptrReportSelfData + (i2c_RcvBuf[1]<<3); 
        break;
    case I2C_READ_RAW_MUTUAL:
  rcv_ptr = (U8 xdata *)ptrReportMutualData + (i2c_RcvBuf[1]<<3);
        break;
    case I2C_READ_RAW_MUTUAL2:
  rcv_ptr = (U8 xdata *)ptrReportMutualData_1 + (i2c_RcvBuf[1]<<3); 
        break;
  
    case I2C_READ_RAW_KEY:
        break;
    case MSG_CLEAR_REPORT:
        //report_ClearReportBuffer();
        break;
    case MSG_SET_SFR:
        //SetSFR(i2c_RcvBuf[1], &i2c_RcvBuf[2], i2c_RcvLen-2);
        P0_1 = 0;                   // Take Register control from I2C.
        MCU_ADDR = i2c_RcvBuf[1];     // Unlock access right.
        for (x=2; x< (i2c_RcvLen); x++) MCU_DATA = i2c_RcvBuf[x];
        P0_1 = 1;                   // Take Register control from I2C.
        break;
#if !TP_CUSTOMER_PROTOCOL
    case MSG_SET_ENG_Mode:
        bridge_i2c_done = 1;
        AP_ActiveFlag = 1;
        break;
#endif
    case MSG_SELF_SIG_ALL_TRIG:
    cmdSELF_SING_ALL = i2c_RcvBuf[1];   // 1: EN-ALL; 0: Single
  break;
    case MSG_Sleep_In:
        //Sleep_In();
        if (fgGotoSleep==0) Sleep();  //AD20130724: snooze mode.
        else if (fgGotoSleep==2) WakeUp();
        break;
    case MSG_SET_PROTOCOL:
        //preProtocol.Customer = i2c_RcvBuf[0];
        //preProtocol.Project_ID = i2c_RcvBuf[1];
        break;
    case MSG_GET_PROTOCOL:
        rcv_ptr = (U8 *)&Protocol;
        break;
    case MSG_GET_MuCCvalue:   // old type, wait delete
        if (TestModeFlag)
            rcv_ptr = (U8 *)ptrLastQuaDelta; // for store testMode mutual CC by nodes data
        else
            rcv_ptr = (U8 *)MutualComOffset; // // for store mutual CC by RX
        break;
    
#if !TP_CUSTOMER_PROTOCOL
  case MSG_ENTER_SCAN_MODE:     //0x16
    GETCCmode =i2c_RcvBuf[1];     
    if(GETCCmode == 0x80 || GETCCmode == 0x82 || GETCCmode == 0x84)
    APActiveCC = 1;    
    else
    { 
      bridge_i2c_done = 0;  
      bridge_i2c_done_pre = 0;        
    }
    AP_ActiveFlag = 1;
    break;         
    
  case MSG_READ_CCvalue:
    if( GETCCmode == 0x80 ) //mutual    
    {
      rcv_ptr = (U8 *)MutualComOffset; // for store testMode mutual CC by line
    }
    else if ( GETCCmode == 0x82 ) //self
    {
      rcv_ptr = (U8 *)TestModeSelfCC;
    }
    else if ( GETCCmode == 0x84 ) //mutual by node    
    {
      rcv_ptr = (U8 *)MutualComOffset;  // by node
    }
    AP_ActiveFlag = 0;        
    break;   

  case MSG_WRITE_CCvalue:    
    bridge_i2c_done = 0;        
    bridge_i2c_done_pre = 0;      
        
    GETCCPacketLen =i2c_RcvBuf[1];    // for over 32 byte buffer length
    IndexPointTo = LONGPacket*(I2C_BufferSize-2);
    if (GETCCPacketLen < (I2C_BufferSize-2))   
    {
      PacketSize = GETCCPacketLen;
      LONGPacket = 0;
    }
    else
    {
      PacketSize = (I2C_BufferSize-2);  
      LONGPacket++;   // Initialize
    }
    if (GETCCPacketLen > 0)            
    {
      if (GETCCmode == 0x81)    //mutual        
      {
        for (x=0;x<PacketSize;x++)    
        {
          (U8)MutualComOffset[IndexPointTo+x]=i2c_RcvBuf[x+2];    
        }    
      }
      else if (GETCCmode == 0x83)       
      {
        for (x=0;x<PacketSize;x++)                    
        {
          TestModeSelfCC[IndexPointTo+x] = SelfComOffset[ChMapping[IndexPointTo+x]]=i2c_RcvBuf[x+2];
        }    
      }
      if (GETCCmode == 0x85)  // write mutual cc by node*/    
      {
        for (x=0;x<PacketSize;x++)                
        {       
          (U8) MutualComOffset[IndexPointTo+x]= i2c_RcvBuf[x+2];    
        }       
      }
    }
    if(LONGPacket == 0)  
    {
      bridge_i2c_done  = 1;        
      bridge_i2c_done_pre =1;
      AP_ActiveFlag = 0;
    } 
    break;              
  case MSG_GET_FNSL:    // 0x0A          
    if (DriverAlive == 0)             
    {
      fgGETFNSL = 1;
      AP_ActiveFlag = 1;
      rcv_ptr = (U8 *)report_DebugInfo_NSL;
    }
    else        
    {
      if (i2c_RcvBuf[1] == ((APK_Debug_NSL_size>>2)-1)) fgGETFNSL = 1;
      rcv_ptr = (U8 *)report_DebugInfo_NSL+(i2c_RcvBuf[1]<<3);
    } 
  break;   
#endif

#if DebugForAPKTest
    case MSG_APK_DEBUG_MODE:    // 0x17
        INTO=1; //De-active INTO.
        APK_Enter_DEBUG_Mode = i2c_RcvBuf[1];
        break;
        //20130522JSb : Start
    case MSG_USER_DEFINE1:
        //user set rcv_ptr by himself
        rcv_ptr = (U8 *)report_DebugInfo_1;   //[poly,20130527] fill user define format to buffer
        APK_ReadBackOK_1 = 1;
        break;
    case MSG_USER_DEFINE2:
        rcv_ptr = (U8 *)report_DebugInfo_2;   //[poly,20130527] fill user define format to buffer
        APK_ReadBackOK_2 = 1;
        break;
        //20130522JSb : End
#endif
#if 0
    case MSG_CTRL_BITS:   //From 0902R.
        x=i2c_RcvBuf[1] & 0x3F; // Get offset
        switch (i2c_RcvBuf[1] & 0xC0) // Get command
        {
        case SET_BITS:
            for (i=0; i<x; i++) CtrlByte[i] |= i2c_RcvBuf[2+i];
            break;
        case CLEAR_BITS:
            for (i=0; i<x; i++) CtrlByte[i] &= ~(i2c_RcvBuf[2+i]);
            break;
        case TOGGLE_BITS:
            for (i=0; i<x; i++) CtrlByte[i] ^= i2c_RcvBuf[2+i];
            break;
        default:  //case WRITE_BITS:
            for (i=0; i<x; i++) CtrlByte[i] = i2c_RcvBuf[2+i];
            break;
        }
        rcv_ptr = &CtrlByte[x];
        break;
#endif
    default:
        //rcv_ptr = report_Buffer;
        notLoad_rcv_ptr =1;
        break;
    }

}


#else
void i2c_rcv_ISR1_Winpower(void)
{
#if !TP_CUSTOMER_PROTOCOL
    U8 x;//,i;
#endif
    switch (i2c_RcvBuf[0])  // Register/Command/Message.
    {
    case MSG_GET_REPORTS:
    #ifdef AMS5505K_Protocol
        fg_MTK_8byte++;
        if (fg_MTK_8byte >= 3 )
        {
          ptr_report = report_Buffer + ((fg_MTK_8byte -2) <<3);   // 1,3,5, ......
          rcv_ptr = ptr_report;
        }
        else
    #endif
        {
          rcv_ptr = report_Buffer;
        }
#if INTO_Out
        INTO=1; //De-active INTO.
#endif
        break;
    case MSG_SET_MODE:
        preOpMode[0] =i2c_RcvBuf[1] ;
        preOpMode[1] =i2c_RcvBuf[2];
    #ifdef AMS5505K_Protocol
        // for PD 5 "
        fg_MTK_Header = 0;
    #endif
        break;
    case MSG_GET_HEADER:
        rcv_ptr = (char *)&tp_header;
    #ifdef AMS5505K_Protocol
        if (fg_MTK_Header >= 3)
          fg_MTK_Header = 0;
        rcv_ptr += (fg_MTK_Header <<3);
        fg_MTK_Header++;
    #endif
        break;
    case MSG_GET_STATUS:
        //rcv_ptr = (char *)&TpStatus;
        rcv_ptr = OpMode;
        nbridge_i2c_EN = 0;
        break;
        //20130522JSa : Start
#if !TP_CUSTOMER_PROTOCOL
    case MSG_SET_CHARGER:
       charger_on_status = i2c_RcvBuf[1];
        break;
#endif
        //20130522JSa : End
    case I2C_READ_RAW_SELF:
        rcv_ptr = (U8 xdata *)ptrReportSelfData;
        break;
    case I2C_READ_RAW_MUTUAL:
        rcv_ptr = (U8 xdata *)ptrReportMutualData;
        break;
    case I2C_READ_RAW_MUTUAL2:
        rcv_ptr = (U8 xdata *)(ptrReportMutualData_1);
        break;
    case I2C_READ_RAW_KEY:
        break;
    case MSG_CLEAR_REPORT:
        //report_ClearReportBuffer();
        break;
#if !TP_CUSTOMER_PROTOCOL
    case MSG_SET_SFR:
        P0_1 = 0;                   // Take Register control from I2C.
        MCU_ADDR = i2c_RcvBuf[1];     // Unlock access right.
        for (x=2; x< (i2c_RcvLen); x++) MCU_DATA = i2c_RcvBuf[x];
        P0_1 = 1;                   // Take Register control from I2C.
        break;
#endif
    case MSG_SET_ENG_Mode:
        bridge_i2c_done = 1;
#if SwitchLowFreqMode
        AP_ActiveFlag = 1;
#endif
        break;
    case MSG_SELF_SIG_ALL_TRIG:
  cmdSELF_SING_ALL = i2c_RcvBuf[1];   // 1: EN-ALL; 0: Single
  break;
    case MSG_Sleep_In:
        //Sleep_In();
        if (fgGotoSleep==0) Sleep();  //AD20130724: snooze mode.
        else if (fgGotoSleep==2) WakeUp();
        break;
    case MSG_SET_PROTOCOL:

        break;
    case MSG_GET_PROTOCOL:
        rcv_ptr = (U8 *)&Protocol;
        break;
#if !TP_CUSTOMER_PROTOCOL
    case MSG_GET_MuCCvalue:
        if (TestModeFlag)
            rcv_ptr = (U8 *)ptrLastQuaDelta; // for store testMode mutual CC by nodes data
        else
            rcv_ptr = (U8 *)MutualComOffset; // // for store mutual CC by RX
        break;
#endif
#if DebugForAPKTest
    case MSG_APK_DEBUG_MODE:    // 0x17
        INTO=1; //De-active INTO.
        APK_Enter_DEBUG_Mode = i2c_RcvBuf[1];
        break;
        //20130522JSb : Start
    case MSG_USER_DEFINE1:
        //user set rcv_ptr by himself
        rcv_ptr = (U8 *)report_DebugInfo_1;   //[poly,20130527] fill user define format to buffer
        APK_ReadBackOK_1 = 1;
        break;
    case MSG_USER_DEFINE2:
        rcv_ptr = (U8 *)report_DebugInfo_2;   //[poly,20130527] fill user define format to buffer
        APK_ReadBackOK_2 = 1;
        break;
        //20130522JSb : End
#endif
#if 0
    case MSG_CTRL_BITS:   //From 0902R.
        x=i2c_RcvBuf[1] & 0x3F; // Get offset
        switch (i2c_RcvBuf[1] & 0xC0) // Get command
        {
        case SET_BITS:
            for (i=0; i<x; i++) CtrlByte[i] |= i2c_RcvBuf[2+i];
            break;
        case CLEAR_BITS:
            for (i=0; i<x; i++) CtrlByte[i] &= ~(i2c_RcvBuf[2+i]);
            break;
        case TOGGLE_BITS:
            for (i=0; i<x; i++) CtrlByte[i] ^= i2c_RcvBuf[2+i];
            break;
        default:  //case WRITE_BITS:
            for (i=0; i<x; i++) CtrlByte[i] = i2c_RcvBuf[2+i];
            break;
        }
        rcv_ptr = &CtrlByte[x];
        break;
#endif    
    default:
        //rcv_ptr = report_Buffer;
        notLoad_rcv_ptr =1;
        break;
    }

}
#endif

//#define SUO_CASE  //JS20130330a
//#ifndef TP_CUSTOMER_PROTOCOL    // Protocol written by customer //JS20130616a XX: It is for Library-ize
#ifdef SUO_CASE //JS20130330a
void i2c_rcv_ISR1_SUO(void)
{
    U8 x;
#if 1
    switch (i2c_RcvBuf[0])
    {
    case 0x0f:
        if ((i2c_RcvBuf[1] < 0x5B) && (i2c_RcvBuf[1] > 0x3f))
        {
            rcv_ptr = &report_Buffer[i2c_RcvBuf[1] - 0x40];
            if (i2c_RcvBuf[1] == 0x40)
                INTO=1;
        }
        else if ( i2c_RcvBuf[1] == 0xF2 && i2c_RcvBuf[2] == 0xC0)
        {
            Sleep_In();
        }
        else
        {
            notLoad_rcv_ptr =1;
        }
        break;
    case 0x03:
        if (i2c_RcvBuf[1] == 0x94)
        {
            if (Timer_Counter > 20)
            {
                rcv_ptr = SUO_Idle_Echo;
            }
            else
            {
                if (echo_change == 0)
                {
                    rcv_ptr = SUO_Echo_1;
                    echo_change = 1;
                }
                else if (echo_change == 1)
                {
                    rcv_ptr = SUO_Echo_2;
                    echo_change = 0;
                }
            }
        }
        INTO=1;
        break;

    case MSG_SET_MODE:
        preOpMode[0] =i2c_RcvBuf[1] ;
        preOpMode[1] =i2c_RcvBuf[2];
        break;
    case MSG_GET_HEADER:
        rcv_ptr = (char *)&tp_header;
        break;
    case MSG_GET_STATUS:
        rcv_ptr = OpMode;
        nbridge_i2c_EN = 0;
        break;
    case I2C_READ_RAW_SELF:
        rcv_ptr = (U8 *)ptrReportSelfData;
        break;
    case I2C_READ_RAW_MUTUAL:
        rcv_ptr = (U8 *)ptrReportMutualData;
        break;
    case I2C_READ_RAW_MUTUAL2:
        rcv_ptr = (U8 *)(ptrReportMutualData_1);
        break;
    case MSG_SET_SFR:
        //SetSFR(i2c_RcvBuf[1], &i2c_RcvBuf[2], i2c_RcvLen-2);
        P0_1 = 0;                   // Take Register control from I2C.
        MCU_ADDR = i2c_RcvBuf[1];     // Unlock access right.
        for (x=2; x< (i2c_RcvLen); x++) MCU_DATA = i2c_RcvBuf[x];
        P0_1 = 1;                   // Take Register control from I2C.
        break;
    case MSG_SET_ENG_Mode:
        bridge_i2c_done = 1;
        break;
    case MSG_SET_PROTOCOL:
        preProtocol.Customer = i2c_RcvBuf[0];
        preProtocol.Project_ID = i2c_RcvBuf[1];
        break;
    case MSG_GET_PROTOCOL:
        rcv_ptr = (U8 *)&Protocol; //
        break;
    case MSG_GET_MuCCvalue:
        if (TestModeFlag)
            rcv_ptr = (U8 *)ptrLastQuaDelta; // for store testMode mutual CC by nodes data
        else
            rcv_ptr = (U8 *)MutualComOffset; // // for store mutual CC by RX
        break;
    default:
        //rcv_ptr = report_Buffer;
        notLoad_rcv_ptr =1;
        break;
    }
#else
    if (i2c_RcvBuf[0] == 0x0f)
    {
        if (i2c_RcvBuf[1] < 0x5B)
        {
            //DMA_ADR_LO = lobyte(&report_Buffer[i2c_RcvBuf[1] - 0x40]);
            //DMA_ADR_HI =  hibyte(&report_Buffer[i2c_RcvBuf[1] - 0x40]);
            rcv_ptr = &report_Buffer[i2c_RcvBuf[1] - 0x40];
            if (i2c_RcvBuf[1] == 0x40)
                INTO=1;
        }
        else if ( i2c_RcvBuf[1] == 0xF2 && i2c_RcvBuf[2] == 0xC0)
        {
            Sleep_In();
        }

    }
    else if (i2c_RcvBuf[0] == 0x03)
    {
        if (i2c_RcvBuf[1] == 0x94)
        {
            if (Timer_Counter > 20)
            {
                rcv_ptr = SUO_Idle_Echo;
            }
            else
            {
                if (echo_change == 0)
                {
                    rcv_ptr = SUO_Echo_1;
                    echo_change = 1;
                }
                else if (echo_change == 1)
                {
                    rcv_ptr = SUO_Echo_2;
                    echo_change = 0;
                }
            }
        }

    }
    //else if(i2c_RcvBuf[0] == 0x80)
    //{
    //    DMA_ADR_LO = lobyte(&report_Buffer[0]);  // 0x80
    //    DMA_ADR_HI =  hibyte(&report_Buffer[0]);
    //rcv_ptr = &report_Buffer[0];
    //}
#endif

}
#endif //SUO_CASE //JS20130330a
//#endif //#ifndef TP_CUSTOMER_PROTOCOL

#define LockPort_OPEN(){P0_1 = 0; }
#define LockPort_CLOSE(){P0_1 = 1; }



U8 data ProtocolUnlockKey[]={MSG_GOTO_STATE, 0xff, 0xa5, 0xb1, 0xc6, 0x54};
U8 UnlockKeyMatches;
U8 i2c_data;
U8 data _mcu_div;

//extern const U8 I2C_BufferSize;

void i2c_rcv_ISR1(void) interrupt 2 using 1
{
    i2c_status= I2CSTAT;  // Get the state immediately.
    //i2c_RcvBuf[i2c_RcvLen++] = D2CMDIF;
    i2c_data = D2CMDIF;

    if (i2c_data == ProtocolUnlockKey[i2c_RcvLen]) 
        UnlockKeyMatches++;
    i2c_RcvBuf[i2c_RcvLen++] = i2c_data;
    if(i2c_RcvLen>=I2C_BufferSize) 
        i2c_RcvLen= I2C_BufferSize-1;  //AD0924: Limit the I2C received not overflow. 
    if (i2c_status & I2CSTATE_STOP)   //
    {
        if ((i2c_status & 0x01)==0)   // I2C Write
        {
            if (UnlockKeyMatches==sizeof(ProtocolUnlockKey))
            {
                if (UnlockKeyMatches==i2c_RcvLen)
                {
                    preProtocol.Customer = Winpower;  // Return to winpower protocol
                    preProtocol.Project_ID = 0;
                }
            }
            else
            {
                if (preProtocol.Customer == Winpower)
                {
                    i2c_rcv_ISR1_Winpower();
                }
    #if TP_CUSTOMER_PROTOCOL
                else
                {
                    i2c_rcv_ISR1_user(i2c_RcvBuf, i2c_RcvLen);
                }
    #endif
                if (!notLoad_rcv_ptr)
                {
                    DMA_ADR_LO =  lobyte(rcv_ptr);
                    DMA_ADR_HI =  hibyte(rcv_ptr);
                }
                else 
                    notLoad_rcv_ptr = 0;
            }
        }
  #if TP_CUSTOMER_PROTOCOL
        else
        { //i2c_status b0== 1 --> I2C Read
            i2c_rcv_ISR1_read_user();
        }
  #endif
        i2c_RcvLen =0 ; // After process the command, clear pointer.
        UnlockKeyMatches =0;
    }
}
//--------------- I2C related functions END----------------------
void ProtocolInit(void)
{
    U8 ratio;

    AA_RxLen = flPannel.X_RX;
    AA_TxLen = flPannel.Y_TX;

    if (flKeyPara.type==MUTU_DOWNKEY)
    {
        AA_TxLen -= 1;
    }
    else if (flKeyPara.type==MUTU_RIGHTKEY)
    {
        AA_RxLen -= 1;
    }

    Protocol.Customer = flProtocol.CustomerDefault; // Customer_default
    Protocol.Project_ID = flProtocol.Project_ID; //Project_ID_default
    if (Protocol.Customer ==Winpower)
        { ratio = 64; }
    else
        { ratio = flPannel.scale; }
        
    preProtocol.Customer = Protocol.Customer;
    preProtocol.Project_ID = Protocol.Project_ID ;

    tp_header.HW_VER = flPannel.HW_VER ;
    tp_header.FW_VER = flPannel.FW_VER ;
    tp_header.SERIAL_NO = flPannel.SERIAL_NO;
    tp_header.SUPPORT_FINGERS= flPannel.SUPPORT_FINGERS;

    preProtocol.Customer = Protocol.Customer;
    preProtocol.Project_ID = Protocol.Project_ID ;

    tp_header.PROT_VER = 0x01;
    tp_header.ID = 0x02;
    tp_header.V_ID = flPannel.V_ID; // Big endian
    tp_header.P_ID = flPannel.P_ID;

    //tp_header.XL_SIZE= RxLen;
    //tp_header.YR_SIZE= TxLen;
    tp_header.XL_SIZE= flPannel.X_RX;     //AD20130730 New protocol
    tp_header.YR_SIZE= flPannel.Y_TX;     //AD20130730 New protocol
#ifdef AMS5505K_Protocol
  #ifdef PD_Horizontal
      tp_header.RES_X= 1920; //AA_RxLen*ratio;
      tp_header.RES_Y= 1080; //AA_TxLen*ratio;
  #else
      tp_header.RES_X= 1080; //AA_RxLen*ratio;
      tp_header.RES_Y= 1920; //AA_TxLen*ratio;
  #endif
#else
      tp_header.RES_X= AA_RxLen*ratio;
      tp_header.RES_Y= AA_TxLen*ratio;
#endif
    tp_header.KEYS = flKeyPara.num;

    tp_header.MAX_RPT_LEN=60;
    tp_header.CAP_1=flKeyPara.type & 0x07;  //AD20130730 New protocol to embedd key_type
    tp_header.CAP_2=0xC4;
    //WatchDog_Initialize(1); //Andy moved away to main().

    i2c_RcvLen =0;
    UnlockKeyMatches =0;
    I2cInit();
}

U8 I2C_Send_Enable = 0;
#ifdef SUO_LIB
U8 Get_AllReport_Info(tSUO_Report_Send * SUO_Report)
{

    U8 x;

    for (x=0;x<MAX_FINGERS;x++)
    {
        SUO_Report->Pos[1][x] = 0xfff;
        SUO_Report->Pos[0][x] = 0xfff;
        SUO_Report->touch_id[x] = 0;
    }

    if (FingerIDList_Output.fingers_number > 5)
        FingerIDList_Output.fingers_number= 5;

    SUO_Report->finger_number = FingerIDList_Output.fingers_number;
    SUO_Report->touch_key = KeyPress;

    if (FingerIDList_Output.fingers_number>0)
    {
        for (x=0;x<MAX_FINGERS;x++)
        {
            if (FingerIDList_Output.Touch_Info[x])
            {
                SUO_Report->Pos[1][x] = ((FingerIDList_Output.Pos[1][x]>>1 ) *flPannel.scale)>>5;
                SUO_Report->Pos[0][x] = ((FingerIDList_Output.Pos[0][x]>>1 ) *flPannel.scale)>>5;
                SUO_Report->touch_id[x] = x+1;

            }
        }
    }

    if (SUO_Report->finger_number>0)
        return 1;
    else
        return 0;
}


U8 Get_Fingers(void)
{
    if (FingerIDList_Output.fingers_number > 5)
        return 5;
    else
        return FingerIDList_Output.fingers_number;;
}


U8 Get_TouchKey(void)
{
    //Keys = KeyPress;

    //if(Keys > 0)
    return KeyPress;
    //else
    //return 0;

}



void Get_XY_Pos(short *X_Pos,short *Y_Pos)
{

    U8 x;

    for (x=0;x<MAX_FINGERS;x++)
    {
        X_Pos[x] = 0xfff;
        Y_Pos[x] = 0xfff;
    }

    if (FingerIDList_Output.fingers_number>0)
    {
        for (x=0;x<MAX_FINGERS;x++)
        {
            if (FingerIDList_Output.Touch_Info[x])
            {
                X_Pos[x] = ((FingerIDList_Output.Pos[1][x]>>1 ) *flPannel.scale)>>5;
                Y_Pos[x] = ((FingerIDList_Output.Pos[0][x]>>1 ) *flPannel.scale)>>5;
            }
        }
    }

    return ;

}
void Get_XY_ID(U8 *ID)
{
    U8 x;
    for (x=0;x<MAX_FINGERS;x++)
    {
        ID[x] = 0xff;
    }
    if (FingerIDList_Output.fingers_number>0)
    {
        for (x=0;x<MAX_FINGERS;x++)
        {
            if (FingerIDList_Output.Touch_Info[x])
            {
                ID[x] = x;
            }
        }
    }
    return ;
}
void Report_Send_Enable(void)
{
    I2C_Send_Enable = 1;
}


#endif




