/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    uart.h

Abstract:

    This file contains UART related function declarations and constant
    definitions to enable UART I/O functionality.

Author:

    Timothy Prinz (tiprinz)

--*/

#pragma once

#include <stdarg.h>

#include "splibs/inc/sptypes.h"

HSP_API
void HspUartInitialize();

HSP_API
void HspUartInitializeEx(uint32_t ClockFreq, uint32_t BaudRate);


HSP_API
bool HspUartIsInputAvailable();


HSP_API
uint8_t HspUartInputByte();


HSP_API
void HspUartOutputByte(char Character);


HSP_API
void HspUartOutputHex(uint32_t Word);


HSP_API
int HspUartOutputString(pcchar_t String);


HSP_API
void HspUartOutputBuffer(pcchar_t Buffer, uint32_t Length);


HSP_API
void HspUartOutputStringBuffer(pcchar_t Buffer, uint32_t Length);


HSP_API
int HspUartVPrintf(pcchar_t Fmt, va_list Args);


HSP_API
int HspUartPrintf(pcchar_t Fmt, ...);
