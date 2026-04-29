/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hsp_global.h

Abstract:

    This file contains the declaration for globals stored in shared section
    of DRAM, such as postcode logs.

Author:

    Peng Li (pengfeli)

--*/

#pragma once

#include "inc/port/hsp_header.h"

#define     MAX_POST_CODE_SIZE      64
#define     MAX_ERROR_LOG           11


typedef struct _SP_GLOBAL_TABLE
{
    struct _SP_BOOT_LOG
    {
        SP_MSG_256  Sp1CodeHash;

        uint32_t    PostCodeList[MAX_POST_CODE_SIZE];
        uint32_t    PostCodeListSize;

        uint32_t    FatalErrorLog[MAX_ERROR_LOG];

    } BootLog;

} SP_GLOBAL_TABLE;