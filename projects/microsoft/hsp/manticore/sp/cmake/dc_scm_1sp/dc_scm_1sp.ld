/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    linkerscript.lds

Abstract:

    This file contains linker directives for all versions of 1sp

--*/

OUTPUT_ARCH(riscv)

ENTRY(HsprtStartup)

__stack_size = 8k;

MEMORY
{
    sp_rom(rxa)               : ORIGIN = 0x20000000, LENGTH = 64k
    sp_iram(rwa)              : ORIGIN = 0x20080000, LENGTH = 384k
    sp_dram(rwxa)             : ORIGIN = 0x20100000, LENGTH = 180k
    sp_dram_shared(rwa)       : ORIGIN = 0x2012D000, LENGTH = 12k
    sp_sharedram(rwa)         : ORIGIN = 0x8F020000, LENGTH = 8k
}

SECTIONS
{
    /*
     *  This section contains the boot code and runtime code
     */
    .boot :
    {
        _exe_start = .;

        *(.boot*)           /* include .boot section from every file        */
        *(.textsv*)         /* include machine mode code from every file    */
        *(.text*)           /* include text section from every file         */

        . = ALIGN(4096);
        _exe_end = .;

    } > sp_dram             /* put it all in 1SP DRAM                       */

    /*
     * All the read only and constant data. This data does not need to be
     * relocated before running the program.
     */
    .readonly :
    {
        _ro_start = .;

        *(.const*)          /* include constants from every file            */
        *(.rdata*)          /* include read only data from every file       */
        *(.rodata*)         /* include read only data from every file       */
        *(.srodata*)        /* static readonly data from functions          */
        *buffer_util*(.sdata*)  /* memset_ptr */
        *(.keyPUB*)         /* include public key data                      */
        *(.stackguard*)     /* include the stack guard cookie.              */

        . = ALIGN(4096);
        _code_end = .;      /* address where all of initialized data goes   */

    } > sp_dram             /* put it all in 1SP DRAM                       */


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

        . = ALIGN(16);
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
    .sharedmemory : AT ( _code_end + SIZEOF (.customData))
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

    } > sp_dram


    /*
     *
     * Handle anything marked for post-init RO by SPRT.  Just treat it as bss is 1SP.
     *
     */
    .roafterinit (NOLOAD) :
    {
        *(.sprtro*)

        _end_bss = .;
    } > sp_dram


    /*
     *
     * The heap will use the rest of SP DRAM, except for the reserved stack space.
     *
     */
    . = ALIGN(8);
    .heap (NOLOAD) :
    {
        __HeapBase = .;
        PROVIDE(__heap_start = __HeapBase);
        PROVIDE(end = __heap_start);
        PROVIDE(_end = end);
        PROVIDE(__end = end);
        . += (LENGTH(sp_dram) - __stack_size) - (. - ORIGIN(sp_dram));
        __HeapLimit = .;
        __heap_limit = __HeapLimit; /* Add for _sbrk */
        PROVIDE(_heap_end = __HeapLimit);
        PROVIDE(__heap_end = __HeapLimit);
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

        . = ALIGN(4096);
        _memory_end = .;

    } > sp_dram


    /*
     *
     * Artifacts from 1SP execution available for SPRT use.
     *
     */
    .sp1_shared (NOLOAD):
    {
        *(.sp1_shared_data*)
    } > sp_dram_shared


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
