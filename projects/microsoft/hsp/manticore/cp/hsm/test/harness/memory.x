/*
  Copyright (c) Microsoft Corporation. All rights reserved.
*/

MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00080000
	RAM	  (rwx): ORIGIN = 0x20000000, LENGTH = 0x00040000
}

