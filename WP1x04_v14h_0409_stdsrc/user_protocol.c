#include "globaltype.h"
#include "user_protocol.h"
#include "private_data.h"

#include "global_vars.h"

sfr DMA_ADR_HI = 0xA5;
sfr DMA_ADR_LO = 0xA6;

extern unsigned char xdata *rcv_ptr;  // A pointer to xdata ram.
extern bit notLoad_rcv_ptr ;
#define  MAX_FINGERS 5





//------ Don't Change above section ----------------------------------
#if 0
void userKeyCheck()
{
    //User programming
}
#endif /*Terry 2015-06-04*/
void user_var_init(void)
{
#if 0
    //User Variable init
#endif /*Terry 2015-06-04*/
    
}

void report_SendKey_user(U8 key_mask) // B0= KEY 1, B1 = KEY 2, ...,. Bn=1 means key pressing, Bn=0 means key was not touching.
{
#if 0    
    U8 Key_mask;
    U8 i;
    U8 Previous_KeyStatus = 0;
    U8 KeyEvent = 0;
    if(KeyPress)    //The current frame button pressing
    {
        key_mask = 0x01;
        for(i = 0; i < KEY_NUMBER;i++)
        {
            if(KeyPress&key_mask)
            {
                //Encode Key Down Event
                //KeyEvent = 1;
                break;

            }
            Key_mask <<= 1;
        }
    }
    else            //the current frame not pressing
    {
        key_mask = 0x01;
        for(i = 0; i < KEY_NUMBER;i++)
        {
            if(Previous_KeyStatus&key_mask)//the previous frame button pressed 
            {
                //Encode Key Up Event
                //KeyEvent = 1;
                break;

            }
            Key_mask <<= 1;
        }        
            
    }
    Previous_KeyStatus = KeyPress;
	if(KeyEvent)
	{
        KeyEvent  = 0;
        RequestINTOLow();
    }
#endif /*Terry 2015-06-04*/
}


void report_SendPoint_user(void)  // YPOS = TxNumber *  64, XPOS=RxNumber * 64;
{
#if 0
    U16 XPOS,YPOS;
    U8 bit_mask;
    U8 i;
    U8 Previous_Finger = 0,TotalFingers;
    bit FingerEvent = 0;
    if(FingerIDList_Output.fingers_number > MAX_FINGERS)
        FingerIDList_Output.fingers_number = MAX_FINGERS;
    if(FingerIDList_Output.fingers_number > 0)
    {
        for(x = 0;x < MAX_FINGERS;x++)
        {
            if(FingerIDList_Output.Touch_Info[x])
            {
                //Encode Report point data format
                FingerEvent = 1;
            }
        }
        Previous_Finger = FingerIDList_Output.fingers_number;
    }
    else
    {
        if(Previous_Finger)
        {
            Previous_Finger = 0

            //Encode Report Point data formar
            FingerEvent = 1
        }
    }
    if(FingerEvent)
    {
        FingerEvent = 0;
        RequestINTOLow();
    }
    
#endif
} 

void i2c_rcv_ISR1_user(U8 *i2c_buf, U8 rcv_len)           
{
#if 0
    U16 command;
    command = i2c_buf[0]<<8+i2c_buf[1];//Combination command
    switch(command)
    {
        case 0xXXXX:
            SetI2CReadTo(array_name);
            break;
        case GetPointCommand:
            SetI2CReadTo(report_Buffer);
            INTO = 1;
            Break;
        case SuspendCommad:
            ResetAllBaseline();
            break;
            
        case ResumeCommand:
            SetINTOHigh();
            break;
            
        default:
            break;
    }
#endif/*Terry 2015-06-04*/
    
}

void i2c_rcv_ISR1_read_user(void)
{
    
#if 0
    This function wil be call after i2c read stop
    Always use to SetINTOHigh();
#endif/*Terry 2015-06-04*/
    
}           





//------ Don't Change below section----------------------------------

extern void SystemInit(void);
extern void TpInit(void);
extern void TpLoop();
extern void WatchDog_Initialize(U8);
extern void ResetAllBaseline(void);// Terry add 2015-06-04
void main()
{

    INTO= 1; //JS20130620c  Move to main.c for FAE who can set it
    user_var_init();
    SystemInit();//After call it, the I2C is ready for call.  
    TpInit();//--- Execution time: 0.5sec~1.5 sec depends on sensor size.
    WatchDog_Initialize(1);//Andy: Moved from Protocol_init(). Enable watchdog for 67M MCU clock ticks expired.
    while(1)
    {
        TpLoop();
    }
}
