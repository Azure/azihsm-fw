/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    printf.h

Abstract:

    Minimum formated print functionality support.  Ported and
    modified from Xbox HSP.

Author:

    Timothy Prinz (tiprinz)

--*/


#pragma once


#define DIGIT_TO_ASCII_BUFFER "0123456789ABCDEF"

typedef enum _PRINT_FORMAT_STATE
{
    STATE_PRINT_DEFAULT = 0,
    STATE_PRINT_STRING = 1,
    STATE_PRINT_DIGIT = 2
} PRINT_FORMAT_STATE;

//
// This struct stores print status of an unfinished printf function
// When the buffer is full but printing is not finished, this struct will save:
//      - Current print position in format string
//      - Current pointer of a string parameter if it's parsing %s
//      - Current digit buffer of a digit parameter if it's parsing %d/%i/%x
//
typedef struct _PrintStatus
{
    pcchar_t FmtPtr;    // Pointer of format string
    va_list Args;       // Variable arguments
    pcchar_t StrPtr;    // Pointer of unfinished string parameter if buffer is
                        // full
    char DigitBuffer[16];    // Buffer of unfinished digit parameter if buffer
                             // is full, 16, the max length of a 64-bit
                             // hexadecimal number
    char DigitBufferIdx;     // Index of head of DigitBuffer (Digits are stored
                             // in DigitBuffer in reverse order)
    PRINT_FORMAT_STATE
    PrintState;    // Current print state, upon return will be
                   // STATE_PRINT_DEFAULT unless unfinished parameter:
                   //      STATE_PRINT_STRING means an unfinished string
                   //      parameter STATE_PRINT_DIGIT means an unfinished digit
                   //      parameter
} PrintStatus, *PPrintStatus;


HSP_API
uint32_t HspCSnPrintf(pchar_t Buffer, uint32_t BufferLength, PPrintStatus Status);


HSP_API
uint32_t HspSnPrintf(pchar_t Buffer, uint32_t BufferLength, pcchar_t Fmt, ...);
