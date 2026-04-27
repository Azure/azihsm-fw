// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PLATFORM_IO_PLUTON_H_
#define PLATFORM_IO_PLUTON_H_

/* If a platform wants to use the old implementation of platform IO,
 * their platform_io.h can include this file. */

/* UART */
#include <stdbool.h>
#include "splibs/hspcore/uart.h"
#include "splibs/inc/sptypes.h"


/* HSP debug output. */
#define	platform_printf		HspUartPrintf
#define	NEWLINE				"\n"


#endif	/* PLATFORM_IO_PLUTON_H_ */
