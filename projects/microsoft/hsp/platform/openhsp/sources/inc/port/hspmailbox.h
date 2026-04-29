/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    hspmailbox.h

Abstract:

    This header is published by Athen HSP Team. The header file contains definition
    and declarations in use for mailbox protocol.

    See the hspmailbox.md for more detail regarding the protocol.

Author:

    <REMOVED>@microsoft.com

--*/

#pragma once

#include <stdint.h>

#pragma pack(push)
#pragma pack(1)

typedef enum _HSP_MAILBOX_SLOT
{
    HspMailbox0 = 0,
    HspMailbox1 = 1,
    HspMailbox2 = 2,
    HspMailbox3 = 3
} HSP_MAILBOX_SLOT;


#ifdef __cplusplus
extern "C"
{
#endif


typedef union _HSP_MAILBOX_MSG
{
    uint32_t AsUint32[4];
} HSP_MAILBOX_MSG, *PHSP_MAILBOX_MSG;


#ifdef __cplusplus
}
#endif

#pragma pack(pop)