/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spglobal.h

Abstract:

    This file contains the declaration for globals stored in shared section
    of DRAM, such as postcode logs.

Author:

    Peng Li (pengfeli)

--*/

#ifdef __clang__

#pragma once

#include "inc/port/hsp_global.h"


//
// Use void to denote the global memory type is unknown from the linker script
//
extern void _global_memory;

static volatile SP_GLOBAL_TABLE* const gSpGlobal =
    (SP_GLOBAL_TABLE*)(&_global_memory);

#endif
