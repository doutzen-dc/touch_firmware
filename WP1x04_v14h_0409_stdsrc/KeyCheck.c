#include <stdio.h>              // prinf()
#include <math.h>               // abs()
#include <intrins.h>
#include "global_vars.h"

#define ONE_KEY_TOUCH_ONLY      1 //JS20130805

#if ONE_KEY_TOUCH_ONLY
extern bit enableOneButtonTouchOnly;
#endif //ONE_KEY_TOUCH_ONLY

extern U8  data BIT8_MASK[] ;
//extern U8 data BIT8_MASK_0[] ;

#define POSITIVE_DIFF 10    // 5% of delta
#define NEGATIVE_DIFF 10      // 5% of delta
#define POS_SLOW_STEP  5    // 2.5%+ of delta
#define NEG_SLOW_STEP  5    // 2.5%+ of delta

extern code tKeyPara flKeyPara;

//extern tAlgorithmParametersSet AlgorithmParam ;

U8 KeyPressCounter[4]={0,0,0,0};
S16 Delta_key_LPF[4]; //JS20130619a : Modify for FAE real mutual key case by Gavin

extern S16 *pSelfdelta;

U8 Key_state;

#if 0 
/* 
  Edge type detection
*/
bit bPowerOnMutual_Key;
S16 mutukeyBaseline[4]; // tim add mutual key buf
void CalMutuKeyBaseline(S16 *keyraw, U8 key_num)
{
    U8 data xxx;
  S16 data key_ceiling;
    S16 data delta;
  S16 data kraw, mbase; //, key_base_th;
  U8 bit_mask=0x01;
    if (bPowerOnMutual_Key)
    {
        bPowerOnMutual_Key=0;
        for (xxx=0; xxx<key_num ; xxx++)
        {
          //KeyPressCounter[xxx]=0;
            Delta_key_LPF[xxx]=0;   // Initial delta values filter to be 0.
            mutukeyBaseline[freqIndex][xxx]=keyraw[xxx]; //-Delta_key_LPF[xxx];
        }
    }
    else
    {
    key_ceiling= flKeyPara.KeyCeiling;
    //key_base_th = flKeyPara.KeyBaseTH1;
        for (xxx=0; xxx<key_num; xxx++)
        {
      kraw = keyraw[xxx]; //4X data.
      mbase = mutukeyBaseline[xxx];
      
            delta=kraw - mbase;  //4X
      if((Key_state & bit_mask))  //This key is pressing now?
      {// Expecting a stable falling edge
        if(delta > POSITIVE_DIFF)
        { // Fast tracking to higher delta to unlock dead keys.
          mbase += POS_SLOW_STEP; //POS_INC_STEP;
        } else if(delta < -NEGATIVE_DIFF)
        { // A falling edge happened.
          mbase -= NEG_SLOW_STEP;
        }
      }
      else
      { //Expecting a stable rising edge
        if(delta > POSITIVE_DIFF)
        { // A rising edge happened.
          mbase += POS_SLOW_STEP;
        } else if(delta < -NEGATIVE_DIFF)
        { // Tracking to lower delta to unlock power-on-touching keys.
          mbase -= NEG_SLOW_STEP;
        }
      }
            if ( flKeyPara.KeyGain[xxx]>1) delta*=flKeyPara.KeyGain[xxx];

            if (delta > key_ceiling)
            {
                delta=key_ceiling;
            }
      mutukeyBaseline[xxx]=mbase;
            //Delta_key_LPF[xxx]=Delta_key_LPF[xxx] + (delta-Delta_key_LPF[xxx])/2;
            Delta_key_LPF[xxx]=delta;
      bit_mask <<= 1;
        }
    }
}


S16 mutuKey_Raw[4];

void keycheck_init(void)
{
    bPowerOnMutual_Key=1;
  Key_state = 0;
}



U8 ComboKeyCheck(S16 *keyraw, U8 key_num)
{            
    U8 data xxx, kpc, kdebounce;
  U8 data bit_mask = 0x01;  // Start shift left from bit0.
  S16 data buf;

/* 
#if ONE_KEY_TOUCH_ONLY
  S16 data maxDelta=0;
  U8 data maxDeltaIndex=9;
#endif //ONE_KEY_TOUCH_ONLY
*/
  CalMutuKeyBaseline(keyraw, key_num);
  kdebounce = flKeyPara.keydebounce;
    for (xxx=0; xxx<key_num ; xxx++)
    {
        buf=Delta_key_LPF[xxx];
    kpc = KeyPressCounter[xxx];
    if(Key_state & bit_mask)  // The key is already pressing
    {
      if(buf < -(flKeyPara.KeyThreshold))
      //if(buf < flKeyPara.KeyThreshold)
      {
        ++kpc;
        //if(kpc >= kdebounce)
        if(kpc >= 1)  // One and only one.
        {
          Key_state &= ~bit_mask; // Recognize key released.
          kpc=0;  // Clear for release checking
          mutukeyBaseline[xxx]=keyraw[xxx];
        }
      }
      else if(buf>0)
      {
        kpc=0;  //Reset debounce counter.
        mutukeyBaseline[xxx]=keyraw[xxx];
      }
    }
    else  // This key is waiting for touch
    {
      if(buf > flKeyPara.KeyThreshold)
      {
        ++kpc;
        if(kpc >= kdebounce)
        //if(kpc >= 2)
        {
          Key_state |= bit_mask;  // Recognize key pressed.
          kpc=0;  // Clear for release checking
          mutukeyBaseline[xxx]=keyraw[xxx];
        }
      }
      else
      {
        kpc=0;  //Reset debounce counter.
        if(buf<0) mutukeyBaseline[xxx]=keyraw[xxx];
      }
    }
    KeyPressCounter[xxx] = kpc; // Write back the record.
    bit_mask <<= 1;
    //------------------------
    }
  return(Key_state);
}
#else //-----------------------------------------------------------------------------------------
/* 
  Level type detection
*/
S16 mutukeyBaseline[3][4];
S16 KeyBaseline[4];

U8 KBaseValid=0;
U8 prev_freqIndex = 0xff;

extern U8 freqIndex;


void CalMutuKeyBaseline(S16 *keyraw, U8 key_num)
{
    U8 data xxx;
  S16 data key_ceiling;
    S16 data delta;
  S16 data kraw, mbase; //, key_base_th;
  U8 bit_mask=0x01;

  if(!(KBaseValid & BIT8_MASK[freqIndex]))
  { // Baseline was not filled.
        for (xxx=0; xxx<key_num ; xxx++)
        {
        mutukeyBaseline[freqIndex][xxx]=keyraw[xxx];
        }
    KBaseValid |= BIT8_MASK[freqIndex];
  }
    //--- Check if jumping freq ?
  if(prev_freqIndex != freqIndex)
  {
        for (xxx=0; xxx<key_num ; xxx++)
        {
        delta = KeyBaseline[xxx];
        KeyBaseline[xxx]=mutukeyBaseline[freqIndex][xxx];
        if(prev_freqIndex!=0xff) mutukeyBaseline[prev_freqIndex][xxx] = delta;  // Save last baseline.
        }
    prev_freqIndex = freqIndex;
  }

  key_ceiling= flKeyPara.KeyCeiling;
  //key_base_th = flKeyPara.KeyBaseTH1;
    for (xxx=0; xxx<key_num; xxx++)
    {
    kraw = keyraw[xxx]; //4X data.
    mbase = KeyBaseline[xxx];    
    delta=kraw - mbase;  //4X
    if ( flKeyPara.KeyGain[xxx]>1) delta*=flKeyPara.KeyGain[xxx];
    if(!(Key_state & bit_mask)) //This key is blanking now?
    { // Yes, update baseline.
      if(delta > POSITIVE_DIFF)
      { // A rising edge happened.
        mbase += POS_SLOW_STEP;
      } else if(delta < -NEGATIVE_DIFF)
      { // Tracking to lower delta to unlock power-on-touching keys.
        mbase -= NEG_SLOW_STEP;
      }
      KeyBaseline[xxx]=mbase;
    }

     if (delta > key_ceiling)
    {
        delta=key_ceiling;
    }
        //Delta_key_LPF[xxx]=Delta_key_LPF[xxx] + (delta-Delta_key_LPF[xxx])/2;
     Delta_key_LPF[xxx]=delta;
     bit_mask <<= 1;
    }
}


S16 mutuKey_Raw[4];

void keycheck_init(void)
{
  Key_state = 0;
}



U8 ComboKeyCheck(S16 *keyraw, U8 key_num)
{            
  U8 data xxx, kpc, kdebounce;
  U8 data bit_mask = 0x01;  // Start shift left from bit0.
  S16 data buf;
  CalMutuKeyBaseline(keyraw, key_num);
  kdebounce = flKeyPara.keydebounce;
    for (xxx=0; xxx<key_num ; xxx++)
    {
        buf=Delta_key_LPF[xxx];
        kpc = KeyPressCounter[xxx];
        if(Key_state & bit_mask)  // The key is already pressing
        {
            if(buf < (flKeyPara.KeyThreshold>>1))  //if(buf < flKeyPara.KeyThreshold)
            {
                ++kpc;
                if(kpc >= kdebounce)  // One and only one.
                {
                    Key_state &= ~bit_mask; // Recognize key released.
                    kpc=0;  // Clear for release checking
                }
            }
        }
    else  // This key is waiting for touch
    {
      if(buf > (flKeyPara.KeyThreshold))
      {
        ++kpc;
        if(kpc >= kdebounce)
        {
          Key_state |= bit_mask;  // Recognize key pressed.
          kpc=0;  // Clear for release checking
        }
      }
    }
    KeyPressCounter[xxx] = kpc; // Write back the record.
    bit_mask <<= 1;
    //------------------------
    }
  return(Key_state);
}
#endif

