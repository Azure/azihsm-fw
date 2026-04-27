/* 
  Copyright (c) Microsoft Corporation. All rights reserved.
*/
 
MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00080000
	RAM	  (rwx): ORIGIN = 0x20000000, LENGTH = 0x00030000
}

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
