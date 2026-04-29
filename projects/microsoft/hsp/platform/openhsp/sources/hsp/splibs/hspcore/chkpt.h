/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    chkpt.h

Abstract:

    This file contains check point related function declarations and constant
definitions

Author:

    Peng Li (pengfeli)

--*/

#pragma once


void HspChkptWriteRegister(pvuint32_t Address, uint32_t Value);


void HspChkptSetConfig(const HSP_CHKPT_CONFIG* Config);


bool HspChkptCheckReady();


static INLINE void HspChkptTerminateChain()
{
    HspChkptWriteRegister(CREG_REG(CHKPT_CHKPT_MSG), 0xDEADBEEF);
}


static INLINE void HspChkptWriteMsg(uint32_t Msg)
{
    HspChkptWriteRegister(CREG_REG(CHKPT_CHKPT_MSG), Msg);
}