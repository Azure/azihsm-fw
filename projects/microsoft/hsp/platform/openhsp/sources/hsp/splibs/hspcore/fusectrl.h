/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    fusectrl.h

Abstract:

    This file contains fuse control related function declarations
    and constant definitions

Author:

    Peng Li (pengfeli)

--*/

#pragma once


#define IS_SECURE_STATE(x)                         \
    (LIKELY(((x) == HspSecurityStateProduction) || \
            ((x) == HspSecurityStateSecure)))

typedef enum _HspSecurityState
{
    HspSecurityStateUnknown = 0x0,
    HspSecurityStateBlank = 0x1,
    HspSecurityStateTest = 0x2,
    HspSecurityStateProduction = 0x4,
    HspSecurityStateSecure = 0x8,
    HspSecurityStateRetest = 0x10,

    HspSecurityStateMask = 0x1F
} HspSecurityState;


typedef enum _HspGfcCmd
{
    HspGfcCmdProgramData = 0x01,
    HspGfcCmdReadData = 0x02,
    HspGfcCmdBlankCheck = 0x03,
    HspGfcCmdChangeSecurityStateTest = 0x10,
    HspGfcCmdChangeSecurityStateProduction = 0x20,
    HspGfcCmdChangeSecurityStateSecure = 0x30,
    HspGfcCmdChangeSecurityStateRetest = 0x40,
} HspGfcCmd;


HSP_API
static INLINE void HspGetSocId(PHSP_FUSE_SOC_ID SocId)
{
    SocId->AsUint32[0] = HspReadRegister32(GFC_REG(SOCID_0));
    SocId->AsUint32[1] = HspReadRegister32(GFC_REG(SOCID_1));
    SocId->AsUint32[2] = HspReadRegister32(GFC_REG(SOCID_2));
    SocId->AsUint32[3] = HspReadRegister32(GFC_REG(SOCID_3));
}


HSP_API
void HspFuseInit();


HSP_API
static INLINE uint32_t HspFuseGetStatus()
{
    return HspReadRegister32(GFC_REG(COMMAND_STATUS));
}


HSP_API
static INLINE HspSecurityState HspGetSecurityState()
{
    GFC_ONE_HOT_SS hotSS = {0};

    hotSS.u = HspReadRegister32(GFC_REG(ONE_HOT_SS));

    return hotSS.One_Hot_Ss;
}


HSP_API
static INLINE bool HspIsSecureState()
{
    HspSecurityState state = HspGetSecurityState();
    return IS_SECURE_STATE(state);
}


HSP_API
HSP_STATUS
HspFuseGotoSecurityState(HspSecurityState SecState);


HSP_API
HSP_STATUS
HspFuseWrite(uint32_t AddressOffset, uint32_t Data);


HSP_API
HSP_STATUS
HspFuseRead(uint32_t AddressOffset, puint32_t Data);


HSP_API
HSP_STATUS
HspGetSoftwareFuse(uint32_t SoftwareIndex, PSP_MSG_384 SoftwareFuse);
