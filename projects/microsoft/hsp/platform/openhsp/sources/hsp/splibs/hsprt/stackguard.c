/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    stackguard.c

Abstract:

    This file contains the implementation for stack guard.

Author:

    Peng Li (pengfeli)

--*/

#include "precomp.h"

//
// This is used to check the stack overflow. For ROM
// this is a secret that needs to be preserved
//
SECTION(".stackguard.cookie")
const uint32_t __stack_chk_guard = 0xBAADBEEF;


__attribute__((weak)) void __stack_chk_fail(void)
/*++

Description:

    This function is called when the stack is corrupted

--*/
{
    SYSCALL_ONE(RiscvSyscallStackError, STATUS_BUFFER_OVERFLOW);
    UNREACHABLE();
}
