#include <stdio.h>
#include "bios.h"

extern void SystemInit(void);
extern void TpInit(void);
extern void TpLoop();
extern void WatchDog_Initialize(U8);

#if !TP_CUSTOMER_PROTOCOL
void main()
{

  //INTO= 0; //JS20130620c  Move to main.c for FAE who can set it
//  user_var_init();
  //INTO = 0; // for min 2 need INTO LOW fast
  SystemInit(); //After call it, the I2C is ready for call.   
  TpInit(); //--- Execution time: 0.5sec~1.5 sec depends on sensor size.
  WatchDog_Initialize(1);   //Andy: Moved from Protocol_init(). Enable watchdog for 67M MCU clock ticks expired.
  while(1)
  { 
    TpLoop();
  }
}
#endif
