/* 
  Copyright (c) Microsoft Corporation. All rights reserved.
*/

STACK_SIZE = 0x10000;  /* 64KB */

MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00080000
	RAM	  (rwx): ORIGIN = 0x20000000, LENGTH = 0x00030000
}

/* Stack contract for runtime and stack guard without reserving RAM in linker sections. */
__stack_top__ = ORIGIN(RAM) + LENGTH(RAM);
_stack_start = __stack_top__;
__stack_limit__ = __stack_top__ - STACK_SIZE;

/* Override the .data section to only use RAM */
SECTIONS
{
    .data : ALIGN(4)
    {
        . = ALIGN(4);
        __sdata = .;
        *(.data .data.*);
        . = ALIGN(4);
        __edata = .;
    } > RAM  /* Only in RAM, no AT>FLASH */
    
    /* This makes sidata point to sdata, so no copy occurs */
    __sidata = __sdata;
}
