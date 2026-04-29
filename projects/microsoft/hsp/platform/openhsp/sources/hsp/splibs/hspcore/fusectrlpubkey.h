/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    fusectrlpubkey.h

Abstract:

    This file contains fuse control for pubkeys

Author:

    Peng Li (pengfeli)

--*/


#pragma once


HSP_API
static INLINE void HspGetPubX(PHSP_FUSE_PUB Pub)
{
    Pub->AsUint32[0] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_0));
    Pub->AsUint32[1] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_1));
    Pub->AsUint32[2] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_2));
    Pub->AsUint32[3] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_3));
    Pub->AsUint32[4] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_4));
    Pub->AsUint32[5] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_5));
    Pub->AsUint32[6] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_6));
    Pub->AsUint32[7] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_X_7));
}


HSP_API
static INLINE void HspGetPubY(PHSP_FUSE_PUB Pub)
{
    Pub->AsUint32[0] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_0));
    Pub->AsUint32[1] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_1));
    Pub->AsUint32[2] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_2));
    Pub->AsUint32[3] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_3));
    Pub->AsUint32[4] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_4));
    Pub->AsUint32[5] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_5));
    Pub->AsUint32[6] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_6));
    Pub->AsUint32[7] = HspReadRegister32(GFC_REG(BOOT_PUB_KEY_Y_7));
}