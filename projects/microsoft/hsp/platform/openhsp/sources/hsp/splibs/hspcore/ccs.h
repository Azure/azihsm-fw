/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    ccs.h

Abstract:

    Define low level common api used by both shack1 and shack2

Author:

    Peng Li (pengfeli)

--*/

#pragma once

//
// Forward declaration
// Individual platform has their own enum definition
//
enum HSP_CRYPTO_REGISTER;


#define HSP_CMD_CCS_CMD_CODE_ID_RESET 0x6u


HSP_API
HSP_STATUS
HspShackExecuteCcs();


HSP_API
HSP_STATUS
HspShackCcsPrepareCcmd(uint32_t Command,
                       HSP_CRYPTO_REGISTER TargetKey,
                       HSP_CRYPTO_REGISTER SourceKey,
                       HSP_CRYPTO_REGISTER Input,
                       HSP_CRYPTO_REGISTER Output,
                       uint32_t Attributes);