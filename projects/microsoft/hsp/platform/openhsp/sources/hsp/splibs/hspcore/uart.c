/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    uart.c

Abstract:

    This file contains UART related function implementations for UART I/O
    functionality.

    See uart databook:
    https://microsoft.sharepoint.com/:b:/r/teams/devices_silicon_projects/Pluton/Shared%20Documents/Vendors/Synopsys/DW_apb_uart_databook.pdf

Author:

    Timothy Prinz (tiprinz)
    Peng Li (pengfeli)

--*/

#include "precomp.h"

#define UART_1_STOP      (0)
#define UART_NONE_PARITY (0)
#define UART_WLS_8       (3)

//
// On FPGA, the UART crystal frequency is 20 MHZ. For Athena ABB, the
// frequency is 25 mhz
//
#if defined(FLAVOR_FPGA)
#define UART_XTAL_FREQ 20000000    // 20 MHz sclk
#else
#define UART_XTAL_FREQ 25000000    // 25 MHz sclk
#endif

#define UART_BAUD_RATE         115200

#define UART_FRAC_BRG_BITS     6

#define UART_PRINT_BUFFER_SIZE 64


HSP_API
void HspUartInitialize()
/*++

Description:

    Configures UART for use by initializing various control settings
    as well as initializing the Baud rate divisor.

--*/
{
    HspUartInitializeEx(UART_XTAL_FREQ, UART_BAUD_RATE);
}

HSP_API
void HspUartInitializeEx(uint32_t ClockFreq, uint32_t BaudRate)
/*++

Description:

    Configures UART for use by initializing various control settings
    as well as initializing the Baud rate divisor.

Arguments:

    ClockFreq - Frequency of the main clock driving the UART block, represented
    in Hz.
    BaudRate - Baud rate to use for UART transmissions, represented in bps.

--*/
{
    //
    // Reset values of all registers are 0, so no issues overwriting any values
    // with 0.
    //
    CREG_UART_LCR lcr = {0};
    CREG_UART_RBR_DLL dll = {0};
    CREG_UART_IER_DLH dlh = {0};
    CREG_UART_IIR_FCR fcr = {0};
    CREG_UART_DLF dlf = {0};

    //
    // Configure line control register and set DLAB high to configure baud rate.
    //
    lcr.Dlab = 1;
    lcr.Stop = UART_1_STOP;
    lcr.Pen = UART_NONE_PARITY;
    lcr.Dls = UART_WLS_8;
    HspWriteRegister32(CREG_REG(UART_LCR), lcr.u);

    //
    // Set Baud rate to 115200. We do this by calculating a Baud rate divisor,
    // setting both an integer and a fractional part in the configuration
    // registers.
    //
    // Configure integer part of Baud rate divisor according to the below
    // formula. Divisor = serial clock frequency / (16 * Baud rate) Put the
    // lower byte in the divisor latch (low) register (alias of RBR when DLAB
    // set high) and the upper byte in the divisor latch (high) register (alias
    // of IER when DLAB set high).
    //
    // See section 2.4 of data sheet
    //

    uint32_t baudXDiv = 16 * BaudRate;
    uint32_t baudInt = ClockFreq / baudXDiv;

    dll.Dll = baudInt & 0xff;
    dlh.Dlh = (baudInt >> 8) & 0xff;

    HspWriteRegister32(CREG_REG(UART_RBR_DLL), dll.u);
    HspWriteRegister32(CREG_REG(UART_IER_DLH), dlh.u);

    //
    // Configure fractional part of Baud rate divisor,
    // and set divisor latch fraction register with the value.
    //
    // See section 2.4.2 of data sheet
    // Divisor = serial clock frequency / (16 * Baud rate) = (Divisor Integer) +
    // (Divisor fraction) DLF = (divisor fraction * 2^(DLF_SIZE))
    //

    uint32_t baudRem = ((ClockFreq % baudXDiv) << (UART_FRAC_BRG_BITS + 1)) /
                       baudXDiv;
    dlf.Dlf = (baudRem >> 1) + (baudRem & 0x1);
    HspWriteRegister32(CREG_REG(UART_DLF), dlf.u);

    //
    // Set DLAB low on line control register to stop baud rate configuration,
    // make sure we don't change any other values.
    //

    lcr.Dlab = 0;
    HspWriteRegister32(CREG_REG(UART_LCR), lcr.u);

    //
    // Enable FIFO via FIFO Enable bit high on FIFO control register.
    // FCR register is alias of IIR register
    //

    fcr.u = 0;
    fcr.Fifoe = 1;
    HspWriteRegister32(CREG_REG(UART_IIR_FCR), fcr.u);

    return;
}


HSP_API
bool HspUartIsInputAvailable()
/*++

Description:

    Returns the Data Ready bit of the line status register.
    If 1, input is available.  If 0, no input available.

--*/
{
    CREG_UART_LSR lsr;
    lsr.u = HspReadRegister32(CREG_REG(UART_LSR));
    return lsr.Dr;
}


HSP_API
uint8_t HspUartInputByte()
/*++

Description:

    Returns byte from recieve buffer register,
    if no byte currently in receive buffer, waits until ready.

--*/
{
    CREG_UART_RBR_RBR rbr;

    while (!HspUartIsInputAvailable())
    {
    }

    rbr.u = HspReadRegister32(CREG_REG(UART_RBR_RBR));
    return (uint8_t)rbr.Rbr;
}


HSP_API
void HspUartOutputByte(char Character)
/*++

Description:

    NOTE: All UART output functions should build off of this,
    as this implements basic output to UART device.

    Waits for UART ready signal, then outputs
    a single character to UART output.

--*/
{
    CREG_UART_LSR lineStatus = {0};

    //
    // Wait for line status register to indicate there is space in the FIFO
    //
    do
    {
        lineStatus.u = HspReadRegister32(CREG_REG(UART_LSR));
    } while (!lineStatus.Thre);

    HspWriteRegister32(CREG_REG(UART_RBR_RBR), Character);
}


HSP_API
void HspUartOutputHex(uint32_t Word)
/*++

Description:

    Outputs a word in hexadecimal as ASCII to UART output

Arguments:

    Word - hexidecimal data to output

--*/
{
    uint32_t bits = IN_BITS(sizeof(uint32_t));

    do
    {
        bits -= 4;
        HspUartOutputByte(DIGIT_TO_ASCII_BUFFER[(Word >> bits) & 0xF]);
    } while (bits != 0);
}


HSP_API
int HspUartOutputString(pcchar_t String)
/*++

Description:

    Writes a null-terminated string to UART.

Arguments:

    String - null-terminated string to output

Returns:

    Number of bytes written
--*/
{
    int nBytesWritten = 0;
    while (*String != '\0')
    {
        HspUartOutputByte(*String++);
        nBytesWritten++;
    }
    return nBytesWritten;
}


HSP_API
void HspUartOutputBuffer(pcchar_t Buffer, uint32_t Length)
/*++

Description:

    Writes the contents of a buffer to UART,
    writing numerical values as their ASCII-equivalents.

Arguments:

    Buffer - buffer containing data to output

    Length - length in bytes of the above buffer

--*/
{
    for (uint32_t i = 0; i < Length; i++)
    {
        HspUartOutputByte(DIGIT_TO_ASCII_BUFFER[(Buffer[i] >> 4) & 0xF]);
        HspUartOutputByte(DIGIT_TO_ASCII_BUFFER[Buffer[i] & 0xF]);
    }
}


HSP_API
void HspUartOutputStringBuffer(pcchar_t Buffer, uint32_t Length)
/*++

Description:

    Writes the contents of a buffer to UART assuming the buffer is a printable
    ascii characters

Arguments:

    Buffer - buffer containing data to output

    Length - length in bytes of the above buffer

--*/
{
    for (uint32_t i = 0; i < Length; i++)
    {
        HspUartOutputByte(Buffer[i]);
    }
}


HSP_API
int HspUartVPrintf(pcchar_t Fmt, va_list Args)
/*++

Description:

    Formats null-terminated input string and any given variable arguments into
    output string, which is then outputted to UART.

Arguments:

    Fmt - input string with format specifiers to be outputted to UART

    Args - any variable arguments to be placed in output string

Returns:

    Number of bytes written
--*/
{
    int nBytesWritten = 0;
    char buffer[UART_PRINT_BUFFER_SIZE];
    PrintStatus status;

    // If nothing to print, return
    if (!Fmt)
    {
        return nBytesWritten;
    }

    status.FmtPtr = Fmt;
    status.PrintState = STATE_PRINT_DEFAULT;

    status.Args = Args;

    //
    // Continue to output to UART until null-character reached
    //
    while (*status.FmtPtr)
    {
        //
        // Write string to printing buffer.
        // If remaining string fits in buffer, Status.FmtPtr will equal nullptr
        //
        HspCSnPrintf(buffer, sizeof(buffer), &status);

        // Output the string to serial port
        nBytesWritten += HspUartOutputString(buffer);
    }

    return nBytesWritten;
}

HSP_API
int HspUartPrintf(pcchar_t Fmt, ...)
/*++

Description:

    Formats null-terminated input string and any given variable arguments into
    output string, which is then outputted to UART.

Arguments:

    Fmt - input string with format specifiers to be outputted to UART

    ... - any variable arguments to be placed in output string

Returns:

    Number of bytes written
--*/
{
    int nBytesWritten = 0;
    va_list Args;

    va_start(Args, Fmt);

    nBytesWritten = HspUartVPrintf(Fmt, Args);

    va_end(Args);
    return nBytesWritten;
}

/*++

Description:

    Weak alias of platform_printf in case the higher level platform does not
    define its own. Using a weak alias allows a higher layer to simply override
    this platform_printf.

--*/
int platform_printf(const char* fmt, ...)
    __attribute__((weak, alias("HspUartPrintf")));
