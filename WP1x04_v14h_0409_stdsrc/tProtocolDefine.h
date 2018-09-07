//---------------------------------------------------------------------------

#ifndef tProtocolDefineH
#define tProtocolDefineH
//---------------------------------------------------------------------------

typedef volatile struct
{
  U8 PROT_VER;
  U8 ID;
  U8 HW_VER;
  U8 FW_VER;
  U32 SERIAL_NO; // Big endian
  U16 V_ID; // Big endian
  U16 P_ID;
  U16 RES_X;
  U16 RES_Y;
  U8 XL_SIZE;
  U8 YR_SIZE;
  U8 SUPPORT_FINGERS;
  U8 KEYS;
  U8 MAX_RPT_LEN;
  U8 CAP_1;
  U8 CAP_2;
} tProtocolHeader;

typedef struct
{
    U16 x_pos;
    U16 y_pos;
    U16 z_force;
    U8  id;
    U8  is_down;
} tFingerInfo;

typedef struct
{
    U8  id;
    U8  is_down;
} tKeyInfo;

typedef volatile struct
{
  U8 l_id4_xh3;
  U8 xl8;
  U8 zf5_yh3;
  U8 yl8;
} tFingerReport;

typedef volatile struct
{
  U8 l_00_yh5;
  U8 yl8;
  U8 id4_xh4;
  U8 xl8;
  U8 zf8;
} tFingerReportExt;


typedef volatile struct
{
  U8 ud1;
  U8 id;
} tKeyReport;
#define KEY_UP 0x20
#define KEY_DOWN 0x30

typedef U8 tGestureMsg;




//--- USB/I2C common commands
#define MSG_GET_HEADER      0x01
#define MSG_GET_STATUS      0x02
#define MSG_GET_REPORTS       0x03
#define MSG_SET_MODE        0x04
#define MSG_CLEAR_REPORT      0x05
#define MSG_GOTO_STATE      0x06
#define MSG_RESET_TP        0x07
#define MSG_SET_SFR         0x08
#define MSG_SET_BLANK         0x09
//#define MSG_CTRL_BITS       0x0A    // andy's 
#define MSG_GET_FNSL        0x0A
#define MSG_SET_TIME_TO_IDLE    0x0B
//0x0C  --- reserved for Owen's driver, <MSG_READ_SFR>
#define MSG_SELF_SIG_ALL_TRIG   0x0D    //0x0D
#define MSG_READ_CCvalue      0x0E    //
#define MSG_WRITE_CCvalue     0x0F    //org 0x1C,     
#define I2C_READ_RAW_SELF       0x10
#define I2C_READ_RAW_MUTUAL     0x11
#define I2C_READ_RAW_KEY      0x12
#define MSG_UNFORCE_REPORTING   0x13
#define MSG_FORCE_REPORTING     0x14
//0x15
#define MSG_ENTER_SCAN_MODE   0x16  //0x16, 20131106 by poly
#define MSG_APK_DEBUG_MODE    0x17//poly @ 20130530 // for indicate APK enter DEBUG mode
#define MSG_SET_ENG_Mode      0x18 // tim modfy at 2012/09/10
#define MSG_USER_DEFINE1      0x19 //20130522JSb
#define MSG_USER_DEFINE2      0x1A //20130522JSb
#define MSG_SET_CHARGER               0x1B //20130522JSa
//0x1C
#define MSG_GET_MuCCvalue     0x1D // tim modfy at 2012/10/30
#define MSG_GET_PROTOCOL            0x1E // tim modfy at 2012/10/15
#define MSG_SET_PROTOCOL            0x1F// tim modfy at 2012/10/15

#define MSG_Sleep_In      0x20
//--- I2C only commands
#define I2C_READ_RAW_MUTUAL2  0x15

//--- USB only commands
#define USB_SET_I2C_DEVA    0x70
#define USB_SET_SPI_DEVA    0x71
#define USB_TEST_LINK     0x72
#define USB_FORCE_RAW_SELF  0x30
#define USB_FORCE_RAW_MUTUAL  0x31
#define USB_FORCE_RAW_KEY     0x32
#define USB_UNFORCE_RAW_ALL   0x38
#define USB_I2C_WRITE       0x54
#define USB_I2C_READ      0x55
#define USB_SPI_WRITE       0x56
#define USB_SPI_READ      0x57
#define USB_TP_PACKET_ID          0x58
#define USB_TEST_PACKET     0x52
#define USB_DEBUG_STRING          0x53
#define USB_FLASH_OPEN      0x60
#define USB_FLASH_OPEN_ACK  0x60
#define USB_FLASH_WRITE   0x61
#define USB_FLASH_WRITE_ACK 0x61
#define USB_FLASH_READ      0x62
#define USB_FLASH_READ_ACK  0x62
#define USB_FLASH_CLOSE   0x63

//--- Auto echo to USB messages
#define USB_ECHO_RAW_SELF     0x10
#define USB_ECHO_RAW_MUTUAL 0x11
#define USB_ECHO_RAW_KEY    0x12
//--- Inter AP messages
#define APMSG_SELF_RAW      (USB_ECHO_RAW_SELF+0x30)    // 0x40
#define APMSG_MUTUAL_RAW      (USB_ECHO_RAW_MUTUAL+0x30)  // 0x41
#define APMSG_KEY_RAW       (USB_ECHO_RAW_KEY+0x30)   // 0x42
#define APMSG_UPDATE_FLASH    0x45
#define APMSG_UPDATE_FLASH_ACK  0x45
#define APMSG_UPDATE_STATUS   0x46
#define APMSG_FLASH_READ      0x47
#define APMSG_FLASH_READ_ACK    0x47
#define APMSG_FLASH_PROGRESS    0x48

#define MASK_RAW_SELF         0x01
#define MASK_RAW_MUTUAL       0x02
#define MASK_RAW_KEY        0x04

//---MODE_BITS masks
#define MASK_FORCE_REPORT   0x08
#define MASK_AUTO_IDLE    0x04
#define MASK_ENABLE_STYLUS  0x01

//---ERROR defined for Block data transfering.
#define BLOCK_TX_SUCCEED   0x00
#define BLOCK_ID_INCORRECT 0x01
#define BLOCK_CHKSUM_ERROR 0x02
#define BLOCK_OPEN_ERROR   0x03

#if 0
//---Define for CtrlByte commands.
#define SET_BITS  0x00
#define CLEAR_BITS  0x40
#define TOGGLE_BITS 0x80
#define WRITE_BITS  0xC0
#endif

//---REPORT MACRO DEFINE
#define IsFingerReport(xx)  (xx & 0x80)
#define IsKeyReport(xx)     ((xx&0xef)==0x20)
#define IsKeyUp(xx)         (!(xx&0x10))
#define IsKeyDown(xx)       (xx&0x10)
#define IsNullReport(xx)    (!xx)
#define IsDebugMsg(xx)    ((xx & 0xf0)==0x10)


#endif



