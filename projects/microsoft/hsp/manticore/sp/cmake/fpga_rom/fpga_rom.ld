/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    linkerscript.lds

Abstract:

    This file contains linker directives for all versions of 0sp

Author:

    Navin Pai (navinp)

--*/


OUTPUT_ARCH(riscv)

ENTRY(HsprtStartup)

__stack_size = 8k;

MEMORY
{
    sp_rom(rxa)               : ORIGIN = 0x20000000, LENGTH = 64k
    sp_iram(rxa)              : ORIGIN = 0x20080000, LENGTH = 384k
    sp_dram(rwa)              : ORIGIN = 0x20100000, LENGTH = 192k
    sp_sharedram(rwa)         : ORIGIN = 0x8F020000, LENGTH = 8k
}

SECTIONS
{
    /*
     *
     *  This section contains the boot code and runtime code of
     *  ROM along with all the read only and constant data. This data
     *  does not need to be relocated before running the program
     *
     */
    .boot :
    {

        *(.boot*)           /* include .boot section from every file        */
        *(.textsv*)         /* include machine mode code from every file    */
        *(.text*)           /* include text section from every file         */
        *(.const*)          /* include constants from every file            */
        *(.rdata*)          /* include read only data from every file       */
        *(.rodata*)         /* include read only data from every file       */
        *(.srodata*)        /* static readonly data from functions          */
        *(.keyPUB*)         /* include public key data                      */

        _code_end = .;      /* address where all of initialized data goes   */

    } > sp_rom              /* put it all in the rom */


    /*
     *  This section contains the ROM secrets that will be access locked
     *  regarless of the security state before ROM jumps into 1SP. All ROM
     *  secrets should be put into this section.
     *
     */
    .bootsecrets :
    {
        *(.romsecrets*)     /* Include all of the sections marked with secrts */
        *(.stackguard*)     /* Include the stack guard cookie */

    } > sp_rom        /* put all of the secrets in rom secret area */


    /*
     *
     *  All of the initialized read/write data goes into the following
     *  location. Before the program starts executing, it is the job of the
     *  setup to make sure that the initialized data is properly located in
     *  the ram section.
     *  Use .customData section name instead of .data because the linker
     *  script treat the default .data differently
     *
     */
     . = ALIGN(4);
     _data_lma = LOADADDR(.customData);
    .customData : AT (_code_end)
    {
        _start_data = .;

        *(.data*)           /* initialized data                             */
        *(.sdata*)          /* static const data                            */
        *(.tdata*)

        . = ALIGN(8);
        PROVIDE(__global_pointer$ = .);

        . = ALIGN(4);
        _end_data = .;

    } > sp_dram


    /*
     *
     * Memory space to share between user mode and machine mode
     * The LMA address will be at the end of .customData's LMA
     * The VMA address will be in SP DRAM.
     *
     * This needs to be here to satisfy HSP init code, but is not used.
     *
     */
    . = ALIGN(4);
    _sharedmemory_lma = LOADADDR(.sharedmemory);
    .sharedmemory :  AT ( _code_end + SIZEOF (.customData))
    {
        _start_sharedmemory = .;

        *(.sharedmemory*)

        . = ALIGN(4);
        _end_sharedmemory = .;

    } > sp_dram


    /*
     *
     *  All of the zero initialized data goes into SP DRAM.
     *
     */
    . = ALIGN(4);
    .bss :
    {
        _start_bss = .;

        *(.tbss*)
        *(.bsssv*)          /* zero initialized data from sv code       */
        *(.sbss*)           /* zero initialized static bss data         */
        *(.bss*)            /* zero initialized data from um code       */

        _end_bss = .;

    } > sp_dram


    /*
     *
     * The stack space goes at the end of SP DRAM.
     *
     */
    . = ALIGN(16);
    .machinestack (NOLOAD) :
    {
        . += __stack_size;
        PROVIDE(__stack_pointer$ = .);
    } > sp_dram


    /*
     *
     * Discard the c++ exception handling information. It is not useful
     * with our current runtime environment.
     *
     */
    /DISCARD/ : {
        *(.eh_frame*)
    }
}
