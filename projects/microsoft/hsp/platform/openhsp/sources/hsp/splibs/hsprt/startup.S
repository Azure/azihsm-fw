/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    startup.S

Abstract:

    The startup code for the riscv processor. This file defines the
    entrypoint for all rom and runtime

Author:

    Navin Pai (navinp)

--*/
// clang-format off

    .global     HsprtStartup
    .global     _defaultTrap

/*++

Function:

    HsprtStartup

Description:

    This function is the main entrypoint for all riscv based code. The function
    does minimal work before jumping to C code. It sets up global pointer, and
    and early trap vector.

    Riscv requires that the global pointer (gp) register must be initialized before
    accessing any variable. This needs to be done with relaxation disabled. Disabling
    the relaxation allows the linker to substitute the right address. The gp address
    comes from the linker directive file. Similarly stack pointer should also comes
    from linker directive.

--*/
    .section    .boot,"ax",@progbits                // The boot section where first instruction
                                                    // set the section as Allocatable and Executable
                                                    // will be executed by riscv cpu
    .align      2                                   // Everything aligned at 2^2 byte i.e. 4 byte
                                                    // boundary
    .option     rvc                                 // generate compressed instructions

HsprtStartup:
    .cfi_startproc
    .cfi_undefined ra                               // Inform debugger that there is nowhere
                                                    // to backtrace past HsprtStartup

    .option push                                    // save the linker options
    .option norelax                                 // disable relaxation

    la      gp, __global_pointer$                   // load address of global pointer

    .option pop                                     // restore the linker options

    la      sp, __stack_pointer$                    // load address of stack pointer


    la      t0, _defaultTrap                        // load the default trap handler

    csrw    mtvec, t0                               // set it so that we can catch anything going
                                                    // wrong during early boot

    call    HsprtMain                               // call the main function

_postMainFaultLoop:

    j       _postMainFaultLoop                      // main should never return,
                                                    // but if it does, loop forever

    .cfi_endproc

/*++

Function:

    _defaultTrap

Description:

    HsprtTrapHandler is set as weak reference. This is the default implementation
    in case the sp exe does not set up trap vector.

--*/
    .align      6                           // 2^6 = 64 byte aligned as required by CLIC

_defaultTrap:

    .cfi_startproc

1:
    csrr    t0, mcause                      // save cause of exception in t0
    csrr    t1, mepc                        // save PC of exception in t1
    csrr    t2, mtval                       // save trap value in t2


    j       1b                              // local loop label avoids relocation → usually emits smallest encoding


    .cfi_endproc


    // clang-format on
