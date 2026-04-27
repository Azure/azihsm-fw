/*
 * uart.c
 *
 *  Created on: Dec 6, 2020
 *      Author: kgupta
 */

// ============================================================================
//
//     Copyright (c) Marvell Corporation  2000   -   All rights reserved
//
//  This computer program contains confidential and proprietary information,
//  and  may NOT  be reproduced or transmitted, in whole or in part,  in any
//  form,  or by any means electronic, mechanical, photo-optical, or  other-
//  wise, and  may NOT  be translated  into  another  language  without  the
//  express written permission from Marvell Corporation.
//
// ============================================================================
// =      C O M P A N Y      P R O P R I E T A R Y      M A T E R I A L       =
// ============================================================================
/*
-------------------------------------------------------------------------------
*  $Revision:
*
*  Description:	 Generic serial access routines
*
Initial revision.
*
*---------------------------------------------------------------------------*/

#include "marvell/RegUartMacros.h"
#include "common/unused.h"

volatile Uart_t *Uart_Ptr = (Uart_t*)(UART_BASE_ADDRESS);


/*----------------------------------------------------------------------------
  NAME: init_serial

  Description: Initialization
 ---------------------------------------------------------------------------*/
void init_serial (int uart_clk, int br)
{
	uint32_t data;

	UNUSED (uart_clk);
	UNUSED (br);

	//
	// read RBR register to clear errors
	//
	data = Uart_Ptr->receiverBuffer.all;
	UNUSED (data);

	//
	// clear RX Toggle bit (bit 9), write a "1" to clear the bit
	//
	Uart_Ptr->status.b.RX_TOGGLE_UART = 1;

	//
	// reset RXFIFO_RESET bit (bit 14), write a "1" then write a "0" to reset
	//
	Uart_Ptr->control.b.RXFIFO_RST_UART = 1;
	for (data = 0; data < 1000; data++);
	Uart_Ptr->control.b.RXFIFO_RST_UART = 0;

	//
	// reset TXFIFO_RESET bit (bit 15), write a "1" then write a "0" to reset
	//
	Uart_Ptr->control.b.TXFIFO_RST_UART = 1;
	for (data = 0; data < 1000; data++);
	Uart_Ptr->control.b.TXFIFO_RST_UART = 0;

	//
	// Select oscillator clock and divisor
	//
	Set_Uart_prgrmmblOversamplingStack1 (0x22220040);
	Set_Uart_prgrmmblOversamplingStack2 (0x00002122);
}

/*----------------------------------------------------------------------------
  NAME:	putDebugCharReady

  Description:
 ---------------------------------------------------------------------------*/
int putDebugCharReady ()
{
	return (Uart_Ptr->status.all & UART_STATUS_TX_READY_UART_MASK);
}

/*----------------------------------------------------------------------------
  NAME:	putDebugChar

  Description:
 ---------------------------------------------------------------------------*/
void put_char (char ch)
{
	while (!putDebugCharReady());
	/* Write data into TDR and clear TDRE */
	//THR = ch;
	Uart_Ptr->transmitterHolding.b.UART_TRANS_HLD_UART = ch;
}

/*----------------------------------------------------------------------
  NAME:	putstring

  Description: Writes a string to the serial port
 ---------------------------------------------------------------------------*/
void putstring (char *str)
{
	char ch;

	while ((ch = *str) != '\0') {
		put_char (ch);
		if(ch == '\n') {
			put_char ('\r');
		}
		str++;
	}
}
