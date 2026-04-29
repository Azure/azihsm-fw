/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsprtmain.c

Abstract:

    This is the main entry point that is called by the startup
    assembly code. This does some house keeping like setting up
    interrupts, etc

Author:

    Navin Pai (navinp)

--*/

#include "precomp.h"

//
// The following variables comes from the linker scripts
//
extern uint32_t _data_lma;            // Start addr of data in ROM
extern uint32_t _start_data;          // Start addr of data in RAM
extern uint32_t _end_data;            // End addr of data in RAM
extern uint32_t _start_bss;           // Start addr of uninitialized data in RAM
extern uint32_t _end_bss;             // End addr of uninitialized data in RAM
extern uint32_t _sharedmemory_lma;    // Start addr of shared memory data in ROM
extern uint32_t _start_sharedmemory;    // Start addr of shared memory data in
                                        // RAM
extern uint32_t _end_sharedmemory;      // End addr of shared memory data in RAM


#ifdef __clang__
//
// disable stack overflow guard since it will read from the stack
// which is not allowed
//

__attribute__((naked)) __attribute__((no_stack_protector)) void PreMain()
/*++

Description:

    Default Premain method if other doesn't implement their own.
    This just returns back to HsprtMain()

--*/
{
    __asm__("jalr s1");
}
#endif


NORETURN
void HsprtMain()
/*++

Description:

    This is the main function that is called by the startup.
    Stack pointer and global pointers are already setup

--*/
{
    register volatile puint32_t src, dst;

#ifdef __clang__
    //
    // PreMain() is defined a naked function so it doesn't set up
    // Prolog. So we need to save the return address for PreMain()
    //
    __asm__ volatile("lla s1, PostPreMain");

    PreMain();

    // PreMain() will return here
    __asm__ volatile("PostPreMain:");
#else
    // Do not support this with GCC builds since older versions of GCC (e.g.
    // gcc10) do not support the 'no_stack_protector' attribute.  This pre-main
    // hooks is generally not used, so not supporting it doesn't really break
    // any functionality.
#endif

    // Copy the initialized data to its destination
    src = &_data_lma;
    dst = &_start_data;
    while (dst < &_end_data)
    {
        *dst++ = *src++;
    }

    // Zero the uninitialized data
    dst = &_start_bss;
    while (dst < &_end_bss)
    {
        *dst++ = 0;
    }

    // Copy the initialized shared memory data to its destination
    src = &_sharedmemory_lma;
    dst = &_start_sharedmemory;
    while (dst < &_end_sharedmemory)
    {
        *dst++ = *src++;
    }

    // Now that the data is initialized, run main
    main();
    UNREACHABLE();
}
