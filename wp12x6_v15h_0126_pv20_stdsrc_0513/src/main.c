/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#define ON			1
#define OFF			0

extern void _SYSTEM_INIT(void);
extern void _WATCH_DOG_INIT(unsigned char);
extern void TPLoop(void);


int main(void)
{
	__dint();
	_SYSTEM_INIT();
	_WATCH_DOG_INIT(ON);
	__eint();

	TPLoop();	// while(1) loop

	return 0;
}


