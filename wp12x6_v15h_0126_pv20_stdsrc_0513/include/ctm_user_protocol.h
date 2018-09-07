/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _CTM_USER_PROTOCOL_H
#define _CTM_USER_PROTOCOL_H

void ctm_user_protocol_set_report(void);
void ctm_user_protocol_rcv_write(unsigned char *, int);
void ctm_user_protocol_char_handler(int c);
void ctm_user_protocol_rcv_read(int rd_len);


#endif //_CTM_USER_PROTOCOL_H
