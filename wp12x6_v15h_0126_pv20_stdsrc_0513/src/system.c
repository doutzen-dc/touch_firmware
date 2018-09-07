/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#include "com_global.h"
#include "system.h"

void _SYSTEM_INIT(void)
{
	com_service_sfr_init();
	com_service_variable_init();
	drv_i2c_init();
	drv_mcu_uart_init(ON);
	ChipSensorSetting();
}

#if PROTOCOL_V20
U8 ExecuteOneTime;
#endif
void TPLoop(void)
{
	U8 sflag,mflag,point_num;

	sflag = 0x00;
	mflag = 0x00;
	point_num = 0x00;

#if PROTOCOL_V20
	ExecuteOneTime = 0x00;
	_SELF_MUTUAL_AUTO_CALIBRATION();

	while(1)
	{

		if(IsEndOfScan() && IsEndOfFrameTimer())
		{
			ExecuteHostRequest();
			STARTING_TRIGGER_SCAN_NORMAL();

			if(ExecuteOneTime)
			{
				_FRAME_FLAG_INITIALIZE();

				sflag = _SELF___GET_TOUCH_STATUS();//if this frame has peak. the sflag will be set 1
				mflag = _MUTUAL_GET_TOUCH_STATUS();//this function has pre_delta and devcon ,if this frame has peak less than -mutual_finger_th*3.the mflag will be
												   //set 0x04,others will bit ser 0x01

				_SELF_MUTUAL_CHECK_TOUCH_STATUS(mflag,sflag);//chect self & mutual touch status. if Mflag equal 0x04 ,will be influence mutual baseline 

				_SELF___UPDATE_BASELINE(sflag);//if self flag equal 0,then will be reset baseline .
				_MUTUAL_UPDATE_BASELINE(mflag);//if mutual flag equal 0,then will be reset baseline.

				_SELF___GET_PEAK_INFO(sflag);  //calc peaks number,if it is equal to 0x11(Tx<<4 | Rx),then will be set doselfreport 1 
				_TOUCH_KEY_CHECK();


				point_num = _COORD_XY_ADDRESSING(mflag,sflag);//Calc X pos & y Pos
				
				_COORD_XY_LINEARITY_SMOOTHING();//Filter
				_COORD_XY_REPORT_TO_HOST();

				_WINPOWER_GUI_PROCESSING();

				_TRIGGER_INTR();

				_SAVE_FRAME_INFO(point_num);

				_OPEN_SHORT_TEST_MODE();


			#if IDLE_MODE_FUNCTION
				_POWER_MODE_SWITCH();
			#endif

				do
				{
					ExecuteHostRequest();
				}while ( CommandVirtualRegister.debug_mode  && !ProtocolStateIs.end_of_fetch) ;

			}
			ExecuteOneTime = 1;
		}
		ExecuteHostRequest();
	}
#else
	_SELF_MUTUAL_AUTO_CALIBRATION();
	STARTING_TRIGGER_SCAN_NORMAL();

	while(1)
	{
		if ( IsEndOfScan() && IsEndOfFrameTimer() && IsEndOfAPRead() )
		{
			_FRAME_FLAG_INITIALIZE();

			sflag = _SELF___GET_TOUCH_STATUS();
			mflag = _MUTUAL_GET_TOUCH_STATUS();

			_SELF_MUTUAL_CHECK_TOUCH_STATUS(mflag,sflag);

			_SELF___UPDATE_BASELINE(sflag);
			_MUTUAL_UPDATE_BASELINE(mflag);
			
			_SELF___GET_PEAK_INFO(sflag);
			_TOUCH_KEY_CHECK();

			STARTING_TRIGGER_SCAN_NORMAL();
			
			point_num = _COORD_XY_ADDRESSING(mflag,sflag);
			_COORD_XY_LINEARITY_SMOOTHING();
			_COORD_XY_REPORT_TO_HOST();

			_WINPOWER_GUI_PROCESSING();

			_TRIGGER_INTR();

			_SAVE_FRAME_INFO(point_num);

			_OPEN_SHORT_TEST_MODE();

			#if IDLE_MODE_FUNCTION
			_POWER_MODE_SWITCH();
			#endif
		}
	}
#endif
}
