/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    sptestinterface.h

Abstract:

    This file contains the shared declaration for test interface between
    the various test roms/test binaries and the test cases

Platform:

    Risc-V
    Windows

Author:

    Navin Pai (navinp)

--*/

#pragma once
#pragma pack(push)
#pragma pack(1)

#define MAX_HSPT_DATA                   0x1000    // 4K

//
// The names corresponding to the types. There are only three types of devices
// either simulated, fpga, or real.
//
// The simulated devices can use any mechanism to communicate with com port
// i.e it could be named-pipes, sockets, memcpy, etc. This is identified by
// the generation string which method will be used. For eg, Bemu uses
// named-pipes for uart and Pemu uses sockets for uart.
//
// The fpga and real devices can use only com port to communicate. So the
// port number must be specified for them. These devices do not distinguish
// between whether it is running on fpga or real chip as com-ports are
// available, none the less they are distinct because the commands run on fpga
// vs. device can be different.
//
// This simplifies the model where we do not have to add more strings here
// to identify the platform that test is running on to connect to platform.
// There may still be cases that requires which generation/version the test is
// running on but that is only required for individual tests rather than the
// connection framework.
//
#define HSPT_DEVICE_TYPE_SIMULATION_STR "Simulation"
#define HSPT_DEVICE_TYPE_FPGA_STR       "Fpga"
#define HSPT_DEVICE_TYPE_DEVICE_STR     "Device"


typedef enum _HSPT_TEST_DEVICE_STATE
#ifdef PLATFORM_WIN
    : uint32_t
#endif
{
    HSPT_DEVICE_STATE_BLANK,
    HSPT_DEVICE_STATE_TEST,
    HSPT_DEVICE_STATE_SECURE,
    HSPT_DEVICE_STATE_PROD,
    HSPT_DEVICE_STATE_RETEST

} HSPT_TEST_DEVICE_STATE;


typedef enum _HSPT_TEST_CMDS
#ifdef PLATFORM_WIN
    : uint32_t
#endif
{
    HSPT_TESTCMD_ECHO,
    HSPT_TESTCMD_EXEC,
    HSPT_TESTCMD_LOG_DATA,
    HSPT_TESTCMD_ERROR,

    HSPT_TESTCMD_LAST    // This should always be last entry in this enum
} HSPT_TEST_CMD;


typedef enum _HSPT_TEST_RESULT
#ifdef PLATFORM_WIN
    : uint32_t
#endif
{
    HSPT_STATUS_FAILED,
    HSPT_STATUS_RESERVED,
    HSPT_STATUS_SKIPPED,
    HSPT_STATUS_BLOCKED,
    HSPT_STATUS_WARNED,
    HSPT_STATUS_SUCCESS,

} HSPT_TEST_RESULT, *PHSPT_TEST_RESULT;


//
// This structure is passed for each test to be executed
// Be sure to set the Size to 0 if no data is to be sent
//
typedef struct _HSPT_PACKET_HEADER
{
    HSPT_TEST_CMD TestCmd;      // The test command to carry out
    HSPT_TEST_RESULT Status;    // Status of the test executed
    uint32_t Size;              // Number of bytes in data.

} HSPT_PACKET_HEADER, *PHSPT_PACKET_HEADER;

static_assert(sizeof(HSPT_PACKET_HEADER) == 12,
              "Size of HSPT_PACKET_HEADER is incorrect.");

//
// The packet between driver and test
//
typedef union _HSPT_PACKET_DATA
{
    struct
    {
        int UnittestId;
        uint8_t UnittestData[MAX_HSPT_DATA - sizeof(int)];
    } Unittest;

    uint8_t Buffer[MAX_HSPT_DATA];
} HSPT_PACKET_DATA, *PHSPT_PACKET_DATA;

typedef struct _HSPT_PACKET
{
    HSPT_PACKET_HEADER Header;
    HSPT_PACKET_DATA Data;
} HSPT_PACKET, *PHSPT_PACKET;

static_assert(sizeof(HSPT_PACKET) == (MAX_HSPT_DATA + 12),
              "HSPT_PACKET size is incorrect.");

//
// All test function should conform to the following signature
//
//  The function should return true/false for pass/fail
//  The function can accept a buffer for arguments.
//
typedef HSPT_TEST_RESULT (*HsptFunction_t)(uint8_t* InBuffer,
                                           uint32_t InBufferSize);

typedef struct _HSPT_DISPATCH_TABLE
{
    int TestId;
    HsptFunction_t TestFunction;
} HSPT_DISPATCH_TABLE, *PHSPT_DISPATCH_TABLE;


//
// The macros below help generate the test enumeration
// and other header information
//

#define HSPT_ENUM_NAME(_Name)  Spt_##_Name
#define HSPT_ENUM_ENTRY(_Name) HSPT_ENUM_NAME(_Name),
#define HSPT_FUNCTION_DECL(_Name) \
    HSPT_TEST_RESULT _Name(uint8_t* InBuffer, uint32_t InBufferSize);
#define HSPT_DISPATCH_TABLE_ENTRY(_Name) {Spt_##_Name, _Name},

//
// The macros below help generate the test dispatch table
//

#define GENERATE_HSPT_ENUM(_EnumName) \
    typedef enum _##_EnumName {SP_TEST_LIST(HSPT_ENUM_ENTRY)} _EnumName;


#define GENERATE_HSPT_DECLARATION() SP_TEST_LIST(HSPT_FUNCTION_DECL)

#pragma pack(pop)
