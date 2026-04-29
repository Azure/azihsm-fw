/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spchkptdefs.h

Abstract:

    This file contains check point related function structures declarations
    and constant definitions

Author:

    Navin Pai (navinp)
    Peng Li (pengfeli)

--*/

#pragma once

#ifdef PLATFORM_WIN
#pragma warning(disable : 4201)
#endif


typedef union _HSP_CHKPT_STATUS
{
    struct
    {
        uint32_t AebEnable      : 1;     // 0
        uint32_t IllegalWrite   : 1;     // 1
        uint32_t TimerExpired   : 1;     // 2
        uint32_t BufferOverflow : 1;     // 3
        uint32_t BufferFull     : 1;     // 4
        uint32_t BufferEmpty    : 1;     // 5
        uint32_t HashBusy       : 1;     // 6
        uint32_t Reserved       : 25;    // [7:31]
    };

    struct
    {
        uint32_t Status    : 7;     // [0:6]
        uint32_t Reserved0 : 25;    // [7:31]
    };

    uint32_t AsUint32;

} HSP_CHKPT_STATUS, *PHSP_CHKPT_STATUS;


// clang-format off

typedef union _HSP_CHKPT_FENCE
{
    struct
    {
        uint32_t NotUsedRpRamNx         : 1;        // 0    Do not allow instruction fetch from RP
        uint32_t SpRamNx                : 1;        // 1    Do not allow instruction fetch from SP
        uint32_t WriteLockMpu           : 1;        // 2    Ignore writes to any MPU config register
        uint32_t DisableCrypto          : 1;        // 3    Disable all crypto engine including CCS
        uint32_t NotUsedKeepRpInHalt    : 1;        // 4    Disable RP to run
        uint32_t KeepSpInHalt           : 1;        // 5    Disallow SP to run
        uint32_t NotUsedKeepCpuInReset  : 1;        // 6    Disallow main CPU to come out of halt
        uint32_t AebLock                : 1;        // 7    Do not allow write to AEB
        uint32_t NoExpiration           : 1;        // 8    Hash never expires
        uint32_t PrivStopAllowed        : 1;        // 9    Allow priviledge mode to stop the hash logic
        uint32_t UserStopAllowed        : 1;        // 10   Allow user mode to stop the hash logic
        uint32_t Reserved               : 21;       // [11:31]
    };
    
    uint32_t AsUint32;

} HSP_CHKPT_FENCE, *PHSP_CHKPT_FENCE;

// clang-format on

typedef struct _HSP_CHKPT_CONFIG
{
    uint32_t Digest0;
    uint32_t Digest1;
    uint32_t Digest2;
    uint32_t Digest3;
    uint32_t Timer0;
    uint32_t Timer1;
    HSP_CHKPT_FENCE Fence;
    uint32_t Ctrl;

} HSP_CHKPT_CONFIG, *PHSP_CHKPT_CONFIG;


//
// Magic IV: 8cb61f5b'f67e90f9'4f6eeed4'f430ab59 (byte-order)
//
#define HSP_CHKPT_MAGIC_IV                                                \
    {                                                                     \
        0x8c, 0xb6, 0x1f, 0x5b, 0xf6, 0x7e, 0x90, 0xf9, 0x4f, 0x6e, 0xee, \
            0xd4, 0xf4, 0x30, 0xab, 0x59                                  \
    }
