/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    print.c

Abstract:

    Minimum formated print functionality support.  Ported and modified from Xbox
HSP.

Author:

    Timothy Prinz (tiprinz)

--*/

#include "precomp.h"


HSP_API
uint32_t HspCSnPrintf(pchar_t Buffer, uint32_t BufferLength, PPrintStatus Status)
/*

Description:

    Continuable version of Snprintf.  Prints out a formatted string with
    arguments to an output buffer. All string inputs, as well as any state
    variables, are kept in the Status structure. If the output buffer's length
    BufferLength is reached before the string is fully written, Status will
    contain the state needed to restart printing from where the buffer filled
    up, including location in variable parameters.

Arguments:

    Buffer - output buffer to write string to

    BufferLength - length of above buffer

    Status - State variable containing location in format string to start
        printing from, any variable arguments, pointer of location in
        unfinished parameters if buffer was full, as well as the current print
        state - to determine if printing stopped in the middle of outputting
        a variable argument.

Return:

    This function returns the number of characters written.

--*/
{
    // Params for string parsing
    pcchar_t strPtr;

    // Params for digit parsing
    uint32_t digitVal;
    uint64_t longDigitVal;
    bool isNegative;
    bool isUnsignedType = false;
    bool done;
    uint8_t digitBase = 10;
    uint8_t idx;

    pchar_t bufferPtr = Buffer;
    pcchar_t bufferEnd = Buffer + BufferLength - 1;
    pcchar_t fmtPtr = Status->FmtPtr;

    while ((*fmtPtr) && bufferPtr < bufferEnd)
    {
        switch (Status->PrintState)
        {
            case STATE_PRINT_STRING:
                strPtr = Status->StrPtr;

                if (!strPtr)
                {
                    strPtr = "\0";
                }

                // Print string argument to buffer
                while (*strPtr && bufferPtr < bufferEnd)
                {
                    *(bufferPtr++) = *(strPtr++);
                }

                // Switch state if string has been successfully printed, else
                // save status
                if (!*strPtr)
                {
                    Status->PrintState = STATE_PRINT_DEFAULT;
                    fmtPtr++;
                }
                else
                {
                    Status->StrPtr = strPtr;
                }

                break;

            case STATE_PRINT_DIGIT:
                idx = Status->DigitBufferIdx;

                // Flush digit buffer to buffer
                while (idx != 0 && bufferPtr < bufferEnd)
                {
                    *(bufferPtr++) = Status->DigitBuffer[--idx];
                }

                // Switch state if digit has been successfully printed, else
                // save status
                if (idx == 0)
                {
                    Status->PrintState = STATE_PRINT_DEFAULT;
                    fmtPtr++;
                }
                else
                {
                    Status->DigitBufferIdx = idx;
                }

                break;

            default:
                switch (*fmtPtr)
                {
                    case '%':
                        fmtPtr++;

                        do
                        {
                            // Assume this is only specifier
                            done = true;
                            switch (*fmtPtr)
                            {
                                case 's':
                                    // Switch state
                                    Status->StrPtr = va_arg(Status->Args, char*);
                                    Status->PrintState = STATE_PRINT_STRING;
                                    break;

                                case 'l':
                                    if (fmtPtr[1] == 'l' &&
                                        (fmtPtr[2] == 'x' || fmtPtr[2] == 'X'))
                                    {
                                        //
                                        // Only support 64-bit integer output to
                                        // be in hexadecimal as otherwise 32-bit
                                        // RiscV requires additional library
                                        // support for division/modulus of long
                                        // int.
                                        //
                                        fmtPtr += 2;
                                        longDigitVal = va_arg(Status->Args,
                                                              uint64_t);
                                        idx = 0;
                                        while (longDigitVal != 0)
                                        {
                                            Status->DigitBuffer[idx++] =
                                                DIGIT_TO_ASCII_BUFFER[longDigitVal &
                                                                      0xF];
                                            longDigitVal = longDigitVal >> 4;
                                        }
                                        if (idx == 0)
                                        {
                                            Status->DigitBuffer[idx++] = '0';
                                        }

                                        // Switch state to print hex
                                        Status->DigitBufferIdx = idx;
                                        Status->PrintState = STATE_PRINT_DIGIT;
                                    }
                                    else if (fmtPtr[1] == 'x' ||
                                             fmtPtr[1] == 'X' ||
                                             fmtPtr[1] == 'u' ||
                                             fmtPtr[1] == 'd' || fmtPtr[1] == 'i')
                                    {
                                        // if there is a long integer specifier,
                                        // treat as normal as we are printing
                                        // all 32-bit values anyways
                                        fmtPtr += 1;
                                        done = false;
                                    }
                                    else
                                    {
                                        *(bufferPtr++) = *(fmtPtr++);
                                    }

                                    break;

                                case 'x':
                                case 'X':
                                    // fallthrough to unsigned int handling, as
                                    // both are unsigned types
                                    digitBase = 16;
                                    FALL_THROUGH;
                                case 'u':
                                    // both unsigned int and hexadecimal value
                                    // fallthrough to printing to temp buffer
                                    // handled in next case
                                    isUnsignedType = true;
                                    FALL_THROUGH;
                                case 'd':
                                    FALL_THROUGH;
                                case 'i':
                                    digitVal = va_arg(Status->Args, uint32_t);

                                    // Print digit to temp digit buffer
                                    isNegative = ((int32_t)digitVal < 0);

                                    if (isNegative && !isUnsignedType)
                                    {
                                        digitVal = -(int32_t)digitVal;
                                    }

                                    idx = 0;
                                    while (digitVal != 0)
                                    {
                                        Status->DigitBuffer[idx++] =
                                            DIGIT_TO_ASCII_BUFFER[digitVal % digitBase];
                                        digitVal /= digitBase;
                                    }

                                    if (idx == 0)
                                    {
                                        Status->DigitBuffer[idx++] = '0';
                                    }

                                    if (isNegative && !isUnsignedType)
                                    {
                                        Status->DigitBuffer[idx++] = '-';
                                    }

                                    // reset any values to default
                                    isUnsignedType = false;
                                    digitBase = 10;

                                    // Switch state
                                    Status->DigitBufferIdx = idx;
                                    Status->PrintState = STATE_PRINT_DIGIT;
                                    break;
                                case '\0':
                                    //
                                    // we have hit end of format string, break
                                    // before we advance past it
                                    //
                                    break;
                                default:
                                    *(bufferPtr++) = *(fmtPtr++);
                            }
                        } while (!done);

                        break;

                    default:
                        *(bufferPtr++) = *(fmtPtr++);
                }
        }
    }

    //
    // Update print status, and ensure last character of buffer is null char
    //
    Status->FmtPtr = fmtPtr;
    *bufferPtr = 0;

    return (uint32_t)(bufferPtr - Buffer);
}


HSP_API
uint32_t HspSnPrintf(pchar_t Buffer, uint32_t BufferLength, pcchar_t Fmt, ...)
/*

Description:

    Hsp implementation of Snprintf.  Will not save state of printing upon
    completion. Writes a string Fmt, placing arguments (...) into the format
    specifiers as appropriate, to an output buffer.  The function will write
    until the string is fully written, or BufferLength is reached.

Arguments:

    Buffer - output buffer to write string to

    BufferLength - length of above buffer

    Fmt - string with format specifiers to be converted to output string in
        buffer

    ... - any variables to be used where format specifier are placed

Return:

    This function returns the number of characters written.

--*/
{
    va_list args;
    uint32_t result;
    PrintStatus status;

    va_start(args, Fmt);

    status.FmtPtr = Fmt;
    status.Args = args;
    status.PrintState = STATE_PRINT_DEFAULT;

    result = HspCSnPrintf(Buffer, BufferLength, &status);

    va_end(args);

    return result;
}
