/*
*	Copyright 2014 by Winpower Optronics Inc. All rights reserved.
*	$Id$
*/

#ifndef _DRV_UART_H
#define _DRV_UART_H

#define UART_TX_BUFF_LENGTH		0x40
#define UART_TX_BUFF_MASK		0x3F

#define UART_TX_IDLE			0x80
#define UART_TX_WAIT_ISR		0x40

#define UART_TX_ENABLE			0x01
#define UART_TX_ISR_FLAG		0x02
#define UART_RX_ENABLE			0x10
#define UART_RX_ISR_FLAG		0x20


#define TBUF_SIZE	32

unsigned char t_in;
unsigned char t_out;
unsigned char buf_len;
unsigned char tbuf [TBUF_SIZE];
unsigned int t_icnt;

unsigned char r_in;
unsigned char r_out;
unsigned char rbuf_len;
unsigned char rbuf [TBUF_SIZE];



#endif