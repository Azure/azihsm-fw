/* 
  Copyright (c) Microsoft Corporation. All rights reserved.
*/

STACK_SIZE = 0xA000;  /* 40K */
 
MEMORY
{
	RAM	  (rwx): ORIGIN = 0x20000000, LENGTH = 0x00020000
	FLASH (rx) : ORIGIN = 0x20020000, LENGTH = 0x0001FB00
}

/* Stack contract for runtime and stack guard without reserving RAM in linker sections. */
__stack_top__ = ORIGIN(RAM) + LENGTH(RAM);
_stack_start = __stack_top__;
__stack_limit__ = __stack_top__ - STACK_SIZE;
