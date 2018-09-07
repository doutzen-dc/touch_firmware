/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _OPR_GUSTURE_H
#define _OPR_GUSTURE_H

typedef unsigned char     u8;
typedef signed char       s8;
typedef unsigned int      u16;
typedef signed int        s16;
typedef unsigned long     u32;
typedef signed long       s32;
typedef float             fp32;


extern s8 GestureMatchProcess(u8 ucCurrentPointNum,u8 ucLastPointNum,s16 curx,s16 cury);
extern void GestureVariableInitialFunction(void);
#endif

