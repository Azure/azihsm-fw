/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_addr_map.ld

Abstract:

    This file contains common linker directives

Author:

    Navin Pai (navinp)

--*/

OUTPUT_ARCH(riscv)

MEMORY
{
    sprom (rxa)             : ORIGIN = 0x20000000, LENGTH = 64k
    spromsecrets (rwa)      : ORIGIN = 0x2000F000, LENGTH = 4k
    spdrammachine (rwa)     : ORIGIN = 0x20100000, LENGTH = 16k
    spdramuser (rwa)        : ORIGIN = 0x20104000, LENGTH = 8k
    spdramshared (rwa)      : ORIGIN = 0x20106000, LENGTH = 4k
    spdramglobal (rwa)      : ORIGIN = 0x20107000, LENGTH = 4k

    sp1iram (rxa)           : ORIGIN = 0x20080000, LENGTH = 128k
    sp2iram (rxa)           : ORIGIN = 0x200E0000, LENGTH = 128k
    sprtiram (rxa)          : ORIGIN = 0x20080000, LENGTH = 384k
}
