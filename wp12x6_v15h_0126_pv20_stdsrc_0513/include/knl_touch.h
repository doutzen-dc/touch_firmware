/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _KNL_TOUCH_H_
#define _KNL_TOUCH_H_

#include "com_global_type.h"




typedef struct _RX_RANGE
{
    U8 Num;
    U8 Start[MAX_RX_NUM>>0x01];
    U8 End[MAX_RX_NUM>>0x01];
}RX_RANGE;

typedef struct _RANGE_BUFFER
{
    U16 SetFlag[2];
    U16 ClosedFlag[2];
    U8 TxStart[KNL_MAX_POINT_NUM];
    U8 TxEnd[KNL_MAX_POINT_NUM];
    U8 RxStart[KNL_MAX_POINT_NUM];
    U8 RxEnd[KNL_MAX_POINT_NUM];
}RANGE_BUFFER;

typedef struct _POINT_LIST
{
    vU16 CurrSetFlag[2];
    vU16 SetFlag;							//point coordinate set flag, if not be set, coordinate will equal to 0xFFFF
    vU16 LegalFlag;							//point coordinate legal flag, if be set, point coordinate will report to host
    vU16 CurrPos[2][KNL_MAX_POINT_NUM];		//the position of this frame will be set here first
    vU16 QueuePos[2][KNL_MAX_FINGER_NUM];	//the position after dynamic tracking, 0 for RX, 1 for TX
}POINT_LIST;

typedef union _TRACKING_POINT
{
	struct
	{
		vU8 Pre;
		vU8 Curr;
		vU16 Distance;
	}P0;
	vU32 PointData;
}TRACKING_BUFFER;

typedef struct _ROTATE_QUEUE
{
    vU16 TouchInfo;
    vS16 RoQueue[KNL_MAX_FINGER_NUM];
    vS16 Position[2][KNL_MAX_FINGER_NUM];
}ROTATE_QUEUE;

typedef struct _FORCE_POINT_FILTER
{
	U16 FilteredPosition[2][KNL_MAX_FINGER_NUM];
	U16 LastFramePosition[2][KNL_MAX_FINGER_NUM];
	U16 DistanceofLastFrame[KNL_MAX_FINGER_NUM];
	U16 AlphaofLastFrame[KNL_MAX_FINGER_NUM];
	U8 PointEnterCounter[KNL_MAX_FINGER_NUM];
	U8 PointFrozen[KNL_MAX_FINGER_NUM];
}FORCE_POINT_FILTER;

#endif // _KNL_TOUCH_H_

