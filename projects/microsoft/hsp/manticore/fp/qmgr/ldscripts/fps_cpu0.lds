// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "../CM7/ldscripts/M7MemMap.h"

MEMORY
{
    IRAM  (rwx) : ORIGIN = FPS_CPU0_IRAM_START,        LENGTH = FPS_CPU0_IRAM_SIZE
    DFTCM (rwx) : ORIGIN = FPS_CPU0_DTCM_DATA_START,   LENGTH = FPS_CPU0_DTCM_DATA_SIZE

    STACK (rwx) : ORIGIN = FPS_CPU0_STACK_BOT,         LENGTH = FPS_CPU0_STACK_SIZE
    C2SV  (rwx) : ORIGIN = FPS_CPU0_STACK_PROTECT,     LENGTH = C2SV_SIZE

}


SECTIONS
{
#ifdef FLASH_BOOT
    .text : AT (FPS_CPU0_FLASH_START)
#else
    .text :
#endif
       {
         _text_start = .;
         *(.interrupt_vector*)
         *(.boot_loader*)
         *(.text*)
         . = ALIGN(16);
         _text_end = .;
       } > IRAM

#ifdef FLASH_BOOT
    .data : AT (FPS_CPU0_FLASH_START+FPS_CPU0_IRAM_SIZE)
#else
    .data :
#endif
       {
         _data_start = .;
         *(.data*)
         *(.rodata*)
         . = ALIGN(16);
         _data_end = .;
       } > DFTCM

    .bss :
       {
         _bss_start = .;
         *(.bss*)
         *(COMMON)
         . = ALIGN(16);
         _bss_end = .;
       } > DFTCM

    .stack_dummy :
       {
         . = FPS_CPU0_STACK_SIZE;
       } > STACK
     _stack_top = ORIGIN(STACK) + LENGTH(STACK);

    .c2sv_dummy :
       {
         . = C2SV_SIZE;
       } > C2SV
}

ASSERT(SIZEOF(.text) <= FPS_CPU0_IRAM_SIZE, "Total code size exceeds memory limit!");
ASSERT((SIZEOF(.data) + SIZEOF(.bss)) <= FPS_CPU0_DTCM_DATA_SIZE, "Total data size exceeds memory limit!");
