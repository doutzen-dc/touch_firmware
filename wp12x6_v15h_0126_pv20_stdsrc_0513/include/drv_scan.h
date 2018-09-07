/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _DRV_SCAN_H
#define _DRV_SCAN_H

#define CFGMEM_SELF___CCPOL_SIZE		84		// 6 * 14 >> 4 * 14
#define CFGMEM_MUTUAL_CCPOL_SIZE		616 	// 616		// MOD4 * 5TxSeq  * 2RxSeq * 14ADC = 560; Tx20,Rx28
#define	MAX_NL_NODE_NUM					30		// 4Tx * 30Rx
#define MAX_NL_FREQ_NUM					10		// max freq number
#define	NL_DO_TIMES						4		//

#define MAX_SELF_AP_CH_NUM				48		// (18 + 30)
#define MAX_MUTUAL_AP_NODE_NUM			560		// (28 * 20)

typedef struct _SELF_RAW
{
	U16 Self[CFGMEM_SELF___CCPOL_SIZE];
}SELF_RAW;

typedef struct _MUTUAL_RAW
{
	U16 Mutual[CFGMEM_MUTUAL_CCPOL_SIZE];
}MUTUAL_RAW;

typedef struct _RAW_FRAME
{  
//	MUTUAL_RAW Mraw[2];
//	SELF_RAW Sraw;
}RAW_FRAME;


typedef struct _SELF_SIG
{
	S16 Self[CFGMEM_SELF___CCPOL_SIZE];
}SELF_SIG;

typedef struct _MUTUAL_SIG
{
	S16 Mutual[CFGMEM_MUTUAL_CCPOL_SIZE];
}MUTUAL_SIG;

typedef struct _SIG_FRAME
{
	SELF_SIG Ssig;
	MUTUAL_SIG Msig;
}SIG_FRAME;

typedef struct _BASE_FRAME
{
	SELF_RAW Sbase;
	MUTUAL_RAW Mbase;
}BASE_FRAME;


#endif	//_DRV_SCAN_H
