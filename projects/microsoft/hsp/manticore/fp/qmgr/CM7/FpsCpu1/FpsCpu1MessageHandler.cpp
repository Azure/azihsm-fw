// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1MessageHandler.cpp
//! @brief  FpsCpu1 Message Handler
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu1.h"
extern "C"
{
#ifdef MCR_TEST_HOOKS
#include "crashdump.h"
#endif
}

//-----------------------------------------------------------------------------
//  Static Member Variable Definitions
//-----------------------------------------------------------------------------
#define CHECK_RESOURCE_GROUP_HAS_VALID_KEY(RGID) ( _rgid2keyValid[RGID] != 0)


// CDMA key vault region requires 32-bit-only access (no STRD/STRH/STRB).
// memset/memcpy may emit STRD which faults on this MMIO region. Use this
// helper to guarantee 32-bit single-word stores. NOT inlined to avoid code
// duplication blowing the FP1 IRAM budget. Length is in 32-bit words (not
// bytes) to eliminate the runtime divide-by-sizeof(uint32_t).
static void __attribute__((noinline)) vault_write_slot(uint32_t* keySlot, const uint32_t* src, uint32_t numWords)
{
    for (uint32_t w = 0; w < numWords; w++)
    {
        writel(src[w], (uint32_t)&keySlot[w]);
    }
}

// Zeroize wrapper: calls vault_write_slot with a static all-zero source.
// Saves the duplicate function body (vault_zero_slot is now a thin wrapper);
// the 32-byte zero buffer lives in .rodata.
static const uint32_t vault_zero_src[AES_KEY_LEN_IN_WORDS] = {0};

static void __attribute__((noinline)) vault_zero_slot(uint32_t* keySlot)
{
    vault_write_slot(keySlot, vault_zero_src, AES_KEY_LEN_IN_WORDS);
}

// CDMA vault seqlock bump. Writer-side (FP1) parity counter at
// M7_FPS_CPU12_CDMA_VAULT_SEQ_LOCK. Call in pairs around any vault MMIO
// mutation: first bump = even->odd (enter), second bump = odd->even (exit).
// FP2 reader retries while value is odd or changes across a read.
// Callers: KeyUpdate() (host key create/delete), _HandleTeardown() (VF
// teardown / FLR / NSSR).
static void __attribute__((noinline)) vault_seq_bump(void)
{
    (*((volatile uint32_t*)M7_FPS_CPU12_CDMA_VAULT_SEQ_LOCK))++;
}

void fpsCpu1::FpsCpuNormalBootInitialize()
{
    Debug_Log(cLogMonitor, cLogInfo, ("FP cpu 1 normal boot!\n"));
    #ifdef LIONPERF_SUPPORT
    // Clear the log buffers for Marvell Internal Logging
    LoggingNormalBootInit();
    #endif

    for (uint8_t i = 0; i < MAX_FP_RGID_NUM; i++)
    {
        _rgid2keyValid[i] = 0;
    }
}

uint8_t fpsCpu1::ChkRecvFPMsgFiberDone()
{
    uint32_t msgCPU2toCPU1Pi = readl(pCPU2toCPU1Pi);
    uint32_t msgCPU2toCPU1Ci = CPU2toCPU1Ci;

    uint32_t msgCPU0toCPU1Pi = readl(pCPU0toCPU1Pi);
    uint32_t msgCPU0toCPU1Ci = CPU0toCPU1Ci;
    if (msgCPU2toCPU1Pi != msgCPU2toCPU1Ci) // for req from  CPU2
    {
        return false;
    } // else do nothing
    if (msgCPU0toCPU1Pi != msgCPU0toCPU1Ci) //for req from CPU0
    {
        return false;
    } // else do nothing

    return true;
}
void fpsCpu1:: GetFwUpdateInfo(FWupdateBackupInfo* pFWupdateInfo, RecoverDataBlk dataType)
{
    FwUpdateDataHeader* pDataHeader = (FwUpdateDataHeader*)PSRAM_BACKUP_DATA_HEADER_ADDR;
    if (pDataHeader->signature != FW_UPDATE_SIGNATURE) // this will be from old structure to new
    {
        pFWupdateInfo->sts = cNoSignature;
        return;
    } // else do nothing
    FwUdSts FwSts = cNewVer;
    // chk data blk cnt
    uint32_t blkcnt = (uint32_t)dataType + 1;
    uint32_t blkIdx = (uint32_t)cUCDData;
    uint32_t HeaderStartAddr = (uint32_t)(PSRAM_BACKUP_DATA_HEADER_ADDR);
    uint32_t dataOffset = (uint32_t)(PSRAM_DATA_BACKUP_HEADER_SIZE);
    uint32_t dataLen = 0;
    uint32_t dataLenSize = (uint32_t)PSRAM_DATA_BACKUP_LENGTH_SIZE;
    // blk 0
    for (blkIdx = (uint32_t)cUCDData; blkIdx < blkcnt; blkIdx++)
    {

        switch (blkIdx)
        {
            case cUCDData: // UCD data
            {
                uint32_t dataLenAddr = (uint32_t)(HeaderStartAddr + dataOffset);
                dataLen = readl(dataLenAddr);
                dataOffset += PSRAM_DATA_BACKUP_LENGTH_SIZE;
            }
            break;
            case cLoggingData: // logging data
            {
                uint32_t dataLenAddr = (uint32_t)(HeaderStartAddr + dataOffset);
                dataLen = readl(dataLenAddr);

                if (dataType == cLoggingData)
                {
                    uint32_t newDataLen = (uint32_t)PSRAM_LOGGING_BACKUP_SIZE;
                    if (dataLen > newDataLen)
                    {
                        FwSts = cOldVer;
                    }
                    pFWupdateInfo->addr = (uint32_t)(dataLenAddr + dataLenSize);
                    pFWupdateInfo->length = dataLen;
                    pFWupdateInfo->sts = FwSts;
                }
                dataOffset += PSRAM_LOGGING_BACKUP_LENGTH_SIZE;
            }
            break;
            default:
                break;

        }
        dataOffset += (dataLen);

    }
}

// Clear out the key information when a key is deleted
void fpsCpu1::CleanKeyInfo(uint16_t keyIndex)
{
    _key2OwnerVfid[keyIndex] = RGID_NO_OWNER_VF;
    _pKey2SessionID[keyIndex] = 0;
    _pKey2AppID[keyIndex] = 0;
    _pKeyIsEphemeral[keyIndex] = 0;
}

CP2FPMsgSts fpsCpu1::FpsCpuHandleStatusChange(Fastpath_Status_t changeStatus, uint8_t change, uint8_t* pDone)
{
    if (change)
    {
        //DebugLogLvDbgInfoInline(cLogCPU1Common, cLogInfo, ("FP CPU 1 Change Requested. change[0x%x]\n", change), "32");
        switch (changeStatus)
        {
            case FP_STS_NORMAL_BOOT:
            {
                if(gResetType == cPor)
                {
                    FpsCpuNormalBootInitialize();
                }
            }
            break;
            case FP_STS_FP_START:
            {
                SetupIdleCmd();
                _fpsCpu1QueueManagerFiber.Activate();
            }
            break;
            default:
                *pDone = false;
                break;
        }
    } // else do nothing
    if (*pDone)
    {
        writel(changeStatus, pCpuStatus);
        DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("FP CPU 1 Status Change Completed. Status [0x%x]\n", readl(pCpuStatus)), "32");
    }
    else
    {
        return msgInvalidField;
    }
    return msgSuccess;
}

Error_t fpsCpu1::KeyUpdate(CP2FPMsgDataKeyUpdate_t* pKeyUpdate)
{
    Error_t errCode = cEcNoError;
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    uint8_t VFId = MAP_FUNCTION_ID(pKeyUpdate->vfId);
    uint16_t sessionId = pKeyUpdate->sessionId;
    uint8_t appId = pKeyUpdate->appId;
    uint8_t flag = pKeyUpdate-> flag;
    uint16_t keyIndex = (pKeyUpdate->resourceGroupId * (KEYUPDATE_KEY_SUB_IDX_MAX + 1)) + pKeyUpdate->keySubIndex;
    uint8_t rgIndex, keySubIndex;
    bool doDeleteMatching = false;
    bool deleteAllForApp  = false;
    AesKeyVault_t* keyVaultArr = (AesKeyVault_t*)(&(rCdma->aesKeyVaultAddr));

    if(keyIndex >= KEY_INDEX_MAX)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, keyIndex:0x%X \n", keyIndex ), "32");
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("qb handler: invalid key, vfId:0x%X\n", VFId), "32");
        return cEcError;
    }
    // Verify Key Subindex is within valid range
    else if (pKeyUpdate->keySubIndex > KEYUPDATE_KEY_SUB_IDX_MAX)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("key sub idx out of range, vfId:0x%X keySubIdx:0x%X\n", ((pKeyUpdate->keySubIndex & 0xFF) << 0x10UL) | VFId ), "16,16");
        return cEcError;
    }
    // Verify if the action is valid
    else if (pKeyUpdate->action >= cActionKeyInvalidAction)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("invalid action, vfId:0x%X action:0x%X\n", (pKeyUpdate->action << 0x10UL) | VFId ), "16,16");
        return cEcError;
    }
 
    // CDMA IO Key Update [NOTE: CDMAIO is IO between CP and FP only. Host is not involved in this type of IO]
    if (CDMAIO_VF_ID == VFId)
    {
        // Verify the resourceGroupId is valid
        if (pKeyUpdate->resourceGroupId != KEYUPDATE_CDMAIO_RGID)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("CDMAIO rgid out of range, vfId:0x%X rgid:0x%X\n", (pKeyUpdate->resourceGroupId << 0x10UL) | VFId ), "16,16");
            return cEcError;
        }
    }
    else
    {
        if (pKeyUpdate->resourceGroupId > KEYUPDATE_RGID_MAX)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("rgid out of range, vfId:0x%X rgid:0x%X\n", (pKeyUpdate->resourceGroupId << 0x10UL) | VFId), "16,16");
            return cEcError;
        }
        else if (VFId > KEYUPDATE_VF_ID_MAX)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("vfid out of range, vfId:0x%X\n", VFId), "32");
            return cEcError;
        }
    }

    if (RGID_NO_OWNER_VF == _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] && cActionKeyEnable == pKeyUpdate->action)
    {
        _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] = VFId;
    }
    else if (VFId != _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] && cActionKeyDisable == pKeyUpdate->action)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Invalid vfId:0x%X _rgid2OwnerVfid[pKeyUpdate->resourceGroupId]:0x%X\n", (_rgid2OwnerVfid[pKeyUpdate->resourceGroupId] << 0x10UL) | VFId ), "16,16");
        return cEcError;
    } // else do nothing

    // Seqlock: enter writer critical section (counter: even -> odd).
    vault_seq_bump();
    switch(pKeyUpdate->action)
    {
        // Delete Key at a given Key Index if SessionId and Application ID match
        case cActionKeyDisable:
            if(VFId == _key2OwnerVfid[keyIndex] && sessionId == _pKey2SessionID[keyIndex] && appId == _pKey2AppID[keyIndex])
            {
                _rgid2keyValid[pKeyUpdate->resourceGroupId] &= ~(BIT(pKeyUpdate->keySubIndex));
                vault_zero_slot(keyVaultArr[keyIndex].key);
                CleanKeyInfo(keyIndex);
                if (!CHECK_RESOURCE_GROUP_HAS_VALID_KEY(pKeyUpdate->resourceGroupId))
                {
                    _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] = RGID_NO_OWNER_VF;
                }
            }
            else
            {
                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Invalid Key:VFID 0x%X _key2OwnerVfid[keyIndex]\n", (_key2OwnerVfid[keyIndex] << 0x10UL) | VFId), "16,16");
                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Invalid Key:SessionID 0x%X _pKey2SessionID[keyIndex]\n", (_pKey2SessionID[keyIndex] << 0x10UL) | sessionId), "16,16");
                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Invalid Key:AppID 0x%X _pKey2AppID[keyIndex]\n", (_pKey2AppID[keyIndex] << 0x10UL) | appId), "16,16");

                errCode = cEcError;
            }
            break;
        case cActionEphemeralKeyForSessionDelete:
            doDeleteMatching = true;
            break;
        case cActionAllKeysDeleteForApp:
            doDeleteMatching = true;
            deleteAllForApp  = true;
            break;
        case cActionKeyEnable:
            vault_write_slot(keyVaultArr[keyIndex].key, pKeyUpdate->keyData, AES_KEY_LEN_IN_WORDS);
            memset(pKeyUpdate->keyData, 0, sizeof(pKeyUpdate->keyData));

            _rgid2keyValid[pKeyUpdate->resourceGroupId] |= BIT(pKeyUpdate->keySubIndex);
            _key2OwnerVfid[keyIndex] = VFId;
            _pKey2SessionID[keyIndex] = sessionId;
            _pKey2AppID[keyIndex] = appId;
            _pKeyIsEphemeral[keyIndex] = flag;
            break;
        default:
            errCode = cEcError;
            break;
    }

    if (doDeleteMatching)
    {
        for (keyIndex = 0; keyIndex < KEY_INDEX_MAX; keyIndex++)
        {
            if (VFId != _key2OwnerVfid[keyIndex] || appId != _pKey2AppID[keyIndex])
            {
                continue;
            }
            if (!deleteAllForApp &&
                (sessionId != _pKey2SessionID[keyIndex] || 1 != _pKeyIsEphemeral[keyIndex]))
            {
                continue;
            }
            rgIndex     = keyIndex / (KEYUPDATE_KEY_SUB_IDX_MAX + 1);
            keySubIndex = keyIndex % (KEYUPDATE_KEY_SUB_IDX_MAX + 1);
            _rgid2keyValid[rgIndex] &= ~(BIT(keySubIndex));
            vault_zero_slot(keyVaultArr[keyIndex].key);
            CleanKeyInfo(keyIndex);
        }
        for (rgIndex = 0; rgIndex < KEYUPDATE_RGID_MAX; rgIndex++)
        {
            if (!CHECK_RESOURCE_GROUP_HAS_VALID_KEY(rgIndex))
            {
                _rgid2OwnerVfid[rgIndex] = RGID_NO_OWNER_VF;
            }
        }
    }
    // Seqlock: leave writer critical section (counter: odd -> even).
    vault_seq_bump();
    #else
    uint8_t VFId = MAP_FUNCTION_ID(pKeyUpdate->vfId);

    uint16_t keyIndex;

    if (CDMAIO_VF_ID == VFId)
    {
        if (pKeyUpdate->keySubIndex > KEYUPDATE_KEY_SUB_IDX_MAX)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("key sub idx out of range, vfId:0x%X keySubIdx:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->keySubIndex & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
            return cEcError;
        }
        else if (pKeyUpdate->resourceGroupId != KEYUPDATE_CDMAIO_RGID)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("rgid out of range, vfId:0x%X rgid:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->resourceGroupId & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
            return cEcError;
        }
        else if (pKeyUpdate->action > cActionKeyEnable)
        {
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("invalid action, vfId:0x%X action:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->action & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
            return cEcError;
        }
        else
        {
            // else do nothing
        }

        if (cActionKeyEnable == pKeyUpdate->action)  //Enable key
        {
            keyIndex = (pKeyUpdate->resourceGroupId * 7) + pKeyUpdate->keySubIndex;
            _key2OwnerVfid[keyIndex] = VFId;
        }
        else if (cActionKeyDisable == pKeyUpdate->action)
        {
            keyIndex = (pKeyUpdate->resourceGroupId * 7) + pKeyUpdate->keySubIndex;
            _key2OwnerVfid[keyIndex] = RGID_NO_OWNER_VF;
        } // else do nothing
        else
        {
            // else do nothing
        }
        return cEcNoError;
    }

    if (pKeyUpdate->keySubIndex > KEYUPDATE_KEY_SUB_IDX_MAX)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("key sub idx out of range, vfId:0x%X keySubIdx:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->keySubIndex & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
        return cEcError;
    }
    else if (pKeyUpdate->resourceGroupId > KEYUPDATE_RGID_MAX)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("rgid out of range, vfId:0x%X rgid:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->resourceGroupId & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
        return cEcError;
    }
    else if (VFId > KEYUPDATE_VF_ID_MAX)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("vfid out of range, vfId:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | (VFId & 0xFF))), "24,8");
        return cEcError;
    }
    else if (pKeyUpdate->action > cActionKeyEnable)
    {
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("invalid action, vfId:0x%X action:0x%X msgOp:0x%X\n", (((msgOpKeyUpdate & 0xFF) << 0x18UL) | ((pKeyUpdate->action & 0xFF) << 0x8UL) | (VFId & 0xFF))), "8,16,8");
        return cEcError;
    }
    else
    {
        // else do nothing
    }

    if (RGID_NO_OWNER_VF == _rgid2OwnerVfid[pKeyUpdate->resourceGroupId])
    {
        _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] = VFId;
    }
    else if (VFId != _rgid2OwnerVfid[pKeyUpdate->resourceGroupId]) // && RGID_NO_OWNER_VF != _rgid2OwnerVfid[pKeyUpdate->resourceGroupId]
    {
        return cEcError;
    } // else do nothing
    else
    {
        // else do nothing
    }

    if (cActionKeyEnable == pKeyUpdate->action)  //Enable key
    {
        _rgid2keyValid[pKeyUpdate->resourceGroupId] |= BIT(pKeyUpdate->keySubIndex);
        keyIndex = (pKeyUpdate->resourceGroupId * 7) + pKeyUpdate->keySubIndex;
        _key2OwnerVfid[keyIndex] = VFId;
    }
    else if (cActionKeyDisable == pKeyUpdate->action)
    {
        _rgid2keyValid[pKeyUpdate->resourceGroupId] &= ~(BIT(pKeyUpdate->keySubIndex));
        keyIndex = (pKeyUpdate->resourceGroupId * 7) + pKeyUpdate->keySubIndex;
        _key2OwnerVfid[keyIndex] = RGID_NO_OWNER_VF;
        if (0 == _rgid2keyValid[pKeyUpdate->resourceGroupId])
        {
            _rgid2OwnerVfid[pKeyUpdate->resourceGroupId] = RGID_NO_OWNER_VF;
        }
    } // else do nothing
    else
    {
        // else do nothing
    }
    #endif
    return errCode;
}

Error_t fpsCpu1::FpModeChange(CP2FPMsgDataVfModeChange_t* pCtx)
{
    if (pCtx->VFMode > 1)
    {
        return cEcError;
    } // else do nothing
    Fastpath_OP_Mode_t newMode = static_cast<Fastpath_OP_Mode_t>(pCtx->VFMode);
    if (newMode == *_fpMode)
    {
        return cEcNoError;
    } // else do nothing
    *_fpMode = newMode;
    //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("FpModeChange newMode [0x%X]", newMode), "32");
    return cEcNoError;
}
#ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
Error_t fpsCpu1::FpsCpuUpdateTimestampAddr(CP2FPMsgDataUpdateTimestampAddr_t* pCtx)
{
    // init system tick
    gTimerCounterBase = readl(REG_GLOBAL_SYNC_COUNTER_LO) & SYSTICK_MASK;
    writel(0x0, REG_SYSTICK_CONTROL_STATUS);
    writel(SYSTICK_TIMER_VALUE - 1, REG_SYSTICK_RELOAD_VALUE);
    writel(0x0, REG_SYSTICK_CURRENT_VALUE);      //any write to current val clears it.
    writel(0x7, REG_SYSTICK_CONTROL_STATUS);      //enable systick with core clock and enable interrupts
    return cEcNoError;
}
#endif

void fpsCpu1::CpCdmaIO(CP2FPMsgDataOpCpCdmaIo_t* pCtx)
{
    M7_MEM_COPY(pCPCDMAdata, pCtx, sizeof(CP2FPMsgDataOpCpCdmaIo_t));//copy message
    hasCPCDMACmd = 1;
    _fpsCpu1CPCDMAIOFiber.Resume();
}

CP2FPMsgSts fpsCpu1::ManageCPMsg(uint8_t msgIdx, uint8_t msgSrc)
{
    CP2FPMsgContext_t* pMsgTemp;
    switch (msgSrc)
    {
        case CP0:
            pMsgTemp = (CP2FPMsgContext_t*)(((uint32_t)pCP0toFPMsgQ) + msgIdx * PSRAM_CP2FP_MSG_ELMNT_SIZE);
            break;
        case CP1:
            pMsgTemp = (CP2FPMsgContext_t*)(((uint32_t)pCP1toFPMsgQ) + msgIdx * PSRAM_CP2FP_MSG_ELMNT_SIZE);
            break;
        default:
            return msgNotSupport;
    }

    uint8_t msgOp = pMsgTemp->msgOp;
    uint8_t* pData = pMsgTemp->data;
    CP2FPMsgSts msgSts = msgSuccess;
    switch (msgOp)
    {
        case msgOpFpStsChange:
        {
            Fastpath_Status_t changeStatus;
            uint8_t change = 1;
            uint8_t done = 1;
            changeStatus = (Fastpath_Status_t)(*pData);
            msgSts = FpsCpuHandleStatusChange(changeStatus, change, &done);
            break;
        }

        #ifdef MCR_TEST_HOOKS
        case msgOpInjectErrorReq:
        {
            CP2FPMsgDataInjectErrorReq_t* ptmpData = (CP2FPMsgDataInjectErrorReq_t*)(pData);
            if(ptmpData->errorType > InjErrFpIoLvl1Abrt && ptmpData->errorType <= InjErrHang){
                TriggerCrash(ptmpData->errorType);
            }
            break;
        }
        #endif

        case msgOpErrQSet:
        {
            uint32_t subOp = *(uint32_t*)pData;
            switch (subOp)
            {
                case msgSubOpAdminAbort:
                {
                    HandleAdminAbort((CP2FPMsgAdminAbort_t*)(pData));
                    break;
                }
                default:
                {
                    msgSts = msgInvalidField;
                    break;
                }
            }
            break;
        }

        case msgOpFpModeChange:
        {
            Error_t errCode;
            errCode = FpModeChange((CP2FPMsgDataVfModeChange_t*)(pData));
            if (cEcNoError != errCode)
            {
                return msgInvalidField;
            } // else do nothing
            break;
        }

        #ifndef SUPPORT_UPDATE_TIMESTAMP_IPC
        case msgOpUpdateTimestampAddr:
        {
            Error_t errCode;
            errCode = FpsCpuUpdateTimestampAddr((CP2FPMsgDataUpdateTimestampAddr_t*)(pData));
            if (errCode != cEcNoError)
            {
                return msgFail;
            }
            break;
        }
        #endif

        case msgOpCpCdmaIo:
        {
            CpCdmaIO((CP2FPMsgDataOpCpCdmaIo_t*)(pData));
            break;
        }

        case msgOpKeyUpdate:
        {
            Error_t errCode;
            CP2FPMsgDataKeyUpdate_t* pKeyUpdate = (CP2FPMsgDataKeyUpdate_t*)(pData);
            errCode = KeyUpdate(pKeyUpdate);
            if (cEcNoError != errCode)
            {
                return msgInvalidField;
            } // else do nothing
            break;
        }

        #ifdef LIONPERF_SUPPORT
        case msgOpLogEnDisUpdate:
        {
            CP2FPMsgDataLogEnDisUpdate_t* ptmpData = (CP2FPMsgDataLogEnDisUpdate_t*)(pData);
            if (ptmpData->action == 1)
            {
                LoggingUpdateGdmaInfo(ptmpData->gdmaQSizeFpsCpu1, ptmpData->piInfoFpsCpu1, ptmpData->pingPongIndexFpsCpu1);
            } // else do nothing
            LoggingUpdateLogExtByLogExtShared();
            break;
        }

        case msgOpSetLogLevel:
        {
            LoggingUpdateLogExtByLogExtShared();
            break;
        }
        #endif

        case msgOpUcdQuery:
        {
            break;
        }

        case msgOpVfSlotSQ2CQMapUpdate:
        {
            Error_t errCode;
            errCode = ManageQueueCfg(pMsgTemp);
            if (cEcNoError != errCode)
            {
                return msgInvalidField;
            } // else do nothing
            break;
        }

        case msgOpVfUpdate:
        {
            Error_t errCode;
            CP2FPMsgDataVfUpdate_t* pCtx = (CP2FPMsgDataVfUpdate_t*)(pData);
            errCode = ManageVFUpdate(pCtx);
            if (cEcNoError != errCode)
            {
                return msgInvalidField;
            } // else do nothing
            break;
        }
        default:
        {
            msgSts = msgNotSupport;
            break;
        }
    }
    return msgSts;
}

Error_t fpsCpu1::ManageVFUpdate(CP2FPMsgDataVfUpdate_t* pData)
{
    if (pData->Action != cActionTearDown)
    {
        return cEcError;
    } // else do nothing

    uint8_t VFId = MAP_FUNCTION_ID(pData->VFId);
    _HandleTeardown(VFId);
    return cEcNoError;
}

Error_t fpsCpu1::ManageQueueCfg(CP2FPMsgContext_t* pMsg)
{
    CP2FPMsgDataVfSlotSq2CqMapUpdate_t* pCtx = (CP2FPMsgDataVfSlotSq2CqMapUpdate_t*)(&pMsg->data[0]);
    #ifdef QOS_LATENCY_ERROR_HANDLING
    uint8_t vfId = MAP_FUNCTION_ID(pCtx->VFId);
    VFNodeInfo_t* pVFNodeInfo = &this->_pVfInfoBase[vfId];
    #ifdef QOS_LATENCY_GLOBAL_UNIQUE
    CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &this->_pQosPenalty[0];
    #else
    CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &this->_pQosPenalty[vfId];
    #endif
    #endif

    if ((pCtx->Action != cActionCreate) && (pCtx->Action != cActionRemove))
    {
        return cEcError;
    } // else do nothing

    if(vfId > MAX_VF_NUM)
    {
        DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("ManageQueueCfg: vfId out of range, vfId:0x%X\n", vfId), "32");
        return cEcError;
    }


    #ifdef QOS_LATENCY_TEST
    //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("[MSG Q_CFG] vfId:0x%X, Action:0x%X\n", (((uint32_t)pCtx->Action & 0xFFFFUL << 0x10UL) | (uint32_t)vfId)), "16,16");
    #endif

    if (pMsg->sts == msgNotifyCpu1)
    {
        #ifdef QOS_LATENCY_ERROR_HANDLING
        if (*_pQosVFBitmap[(vfId >> 5)] & BIT((vfId & 0x1f)))
        {
            pVFNodeInfo->credit = ((pCtx->Action == cActionCreate) ? (pVFNodeInfo->credit + CHUNK_SIZE) : (pVFNodeInfo->credit - CHUNK_SIZE));
            _pVfCredit[vfId] = ((((uint32_t)(pVFNodeInfo->credit) * ((uint32_t)(pQosPenaltyInfo->Cfg.qosPenaltyCreditRatio))) / 100) & (~0xFFF));
            *_pTotalCredit =  ((pCtx->Action == cActionCreate) ? (*_pTotalCredit + _pVfCredit[vfId]) : (*_pTotalCredit - _pVfCredit[vfId]));
        }
        else
        #endif
        {
            pVFNodeInfo->credit = ((pCtx->Action == cActionCreate) ? (pVFNodeInfo->credit + CHUNK_SIZE) : (pVFNodeInfo->credit - CHUNK_SIZE));
            _pVfCredit[vfId] = pVFNodeInfo->credit;
            *_pTotalCredit =  ((pCtx->Action == cActionCreate) ? (*_pTotalCredit + CHUNK_SIZE) : (*_pTotalCredit - CHUNK_SIZE));
        }

        #ifdef QOS_LATENCY_TEST
        //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("[MSG Q_CFG] credit:0x%X, orgCredit:0x%X\n", _pVfCredit[vfId], pVFNodeInfo->credit), "32", "32");
        #endif
    }
    else
    {
        if (pCtx->Action == cActionRemove)
        {
            _SkipAndAbortWithSQid(pCtx->SqPId, cCEStsDelQ);
        } // else do nothing
    }

    DMB();

    return cEcNoError;
}

#ifdef QOS_LATENCY_ERROR_HANDLING
uint8_t fpsCpu1::ChkAvailableFPInterMsgRes(M7CoreId_t cpu)
{
    uint32_t* pMsgCi;   //uint32_t* pMsgPi;
    uint32_t msgPi, msgCi;

    switch (cpu)
    {
        case cM7Core2:
        {
            //pMsgPi = (uint32_t*)pCPU1toCPU2Pi;
            pMsgCi = (uint32_t*)pCPU1toCPU2Ci;
            msgPi = CPU1toCPU2Pi;
        }
        break;
        default:
        {
            return 0;
        }
    }
    msgCi = readl(pMsgCi);

    if (M7_QUEUE_FULL(msgPi, msgCi, PSRAM_INTL_CPUX2CPUY_MSG_MASK))    //chk if fp msg Q full
    {
        return 0;   //msgNoEmptyEntry
    } // else do nothing

    return 1;
}
#endif

CP2FPMsgSts fpsCpu1::SendFPMsg(M7CoreId_t cpu, uint8_t fpMsgOp, uint8_t resp, CP2FPMsgSts fpSts, uint8_t cmdSpecific0, uint8_t cmdSpecific1)
{
    FPInterMsgHeader* pFPMsgHeader;
    volatile uint32_t* pMsgPi, * pMsgCi;
    uint32_t msgPi, msgCi;
    CP2FPMsgSts sts = msgSuccess;
    FPInterMsgHeader fpInterMsgTmpHeader;
    fpInterMsgTmpHeader.fpMsgOp = fpMsgOp;
    fpInterMsgTmpHeader.resp = resp;
    fpInterMsgTmpHeader.sts = fpSts;
    fpInterMsgTmpHeader.cmdSpecific[0] = cmdSpecific0;
    fpInterMsgTmpHeader.cmdSpecific[1] = cmdSpecific1;
    switch (cpu)
    {
        case cM7Core0:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU1toCPU0MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU1toCPU0Pi;
            pMsgCi = (volatile uint32_t*)pCPU1toCPU0Ci;
            CPU1toCPU0Pi = readl(pMsgPi);
            msgPi = CPU1toCPU0Pi;
        }
        break;
        case cM7Core2:
        {
            pFPMsgHeader = (FPInterMsgHeader*)pCPU1toCPU2MsgQ;
            pMsgPi = (volatile uint32_t*)pCPU1toCPU2Pi;
            pMsgCi = (volatile uint32_t*)pCPU1toCPU2Ci;
            CPU1toCPU2Pi = readl(pMsgPi);
            msgPi = CPU1toCPU2Pi;
        }
        break;
        default:
        {
            return msgNotSupport;
        }
    }
    msgCi = readl(pMsgCi);

    if (M7_QUEUE_FULL(msgPi, msgCi, PSRAM_INTL_CPUX2CPUY_MSG_MASK))    //chk fp msg Q full wait q space
    {
        return msgNoEmptyEntry;
    } // else do nothing

    M7_MEM_COPY(&pFPMsgHeader[msgPi], &fpInterMsgTmpHeader, sizeof(FPInterMsgHeader));
    //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("SendFPMsg: OP[0x%X], msgPi[0x%X] \n", pFPMsgHeader[msgPi].fpMsgOp | (msgPi << 0x10UL)), "16,16");

    msgPi = M7_QUEUE_INC(msgPi, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
    writel(msgPi, pMsgPi);

    sts = fpSts;
    switch (cpu)
    {
        case cM7Core0:
        {
            CPU1toCPU0Pi = msgPi;
        }
        break;
        case cM7Core2:
        {
            CPU1toCPU2Pi = msgPi;
        }
        break;
        default:
        {
            return msgNotSupport;
        }
        break;
    }
    #ifdef IPC_SUPPORT
    switch (cpu)
    {
        case cM7Core0:
        {
            IpcDescTrigger(CPU1toCPU0_DESC, msgPi);
        }
        break;
        case cM7Core2:
        {
            IpcDescTrigger(CPU1toCPU2_DESC, msgPi);
        }
        break;
        default:
        {
        }
        break;
    }

    #endif //IPC_SUPPORT
    return sts;
}

CP2FPMsgSts fpsCpu1::RecvFPMsg(FPInterMsgHeader* pFPMsgHeader, M7CoreId_t msgCpu)
{
    FpInterMsgOp fpMsgOp;
    CP2FPMsgSts fpSts = msgSuccess;
    CP2FPMsgSts recvSts = msgSuccess;
    fpMsgOp = (FpInterMsgOp)pFPMsgHeader->fpMsgOp;

    switch (fpMsgOp)
    {
        case cp2FpMsg:
        {
            fpSts = ManageCPMsg(pFPMsgHeader->msgIdx, pFPMsgHeader->msgSrc);
            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);
            //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("RecvFPMsg: msgidx [0x%X], fpSts[0x%X] \n", pFPMsgHeader->msgIdx | (fpSts << 0x10UL)), "16,16");
            _ProcessTeardownBitMap();
            break;
        }

        case fatalErrorMsg:
        {
            cdmaSqPi = 0;
            for (uint16_t i = 0; i < (UCD_FP_IO_Q_SLOT_NUM + FPS_QUEUE_BLOCK_65); i++)
            {
                _pQBlockInfoBase[i].remainLen = 0;
            }
            for (uint16_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
            {
                _CPU1SubmitAbortInfo[i] = ABORT_NOT_SUBMIT;
            }

            uint16_t tmpCPU2toCPU1Pi = readl(pCPU2toCPU1Pi);
            uint16_t tmpCPU2toCPU1Ci = CPU2toCPU1Ci;
            while (tmpCPU2toCPU1Pi !=  tmpCPU2toCPU1Ci)
            {
                FPInterMsgHeader* pTmpMsgHeader = &pCPU2toCPU1MsgQ[tmpCPU2toCPU1Ci];
                if (skipAbortMsg == pTmpMsgHeader->fpMsgOp)
                {
                    pTmpMsgHeader->fpMsgOp = doNothingMsg;
                }
                tmpCPU2toCPU1Ci = M7_QUEUE_INC(tmpCPU2toCPU1Ci, PSRAM_INTL_CPUX2CPUY_MSG_MASK);
            }

            _fpsCpu1QueueManagerFiber.WaitCurrentRound();
            _fpsCpu1CPCDMAIOFiber.WaitCurrentRound();
            cdmaSlotAbortQueueCi = 0;
            writel(0, pCdmaSlotAbortQueueCi);
            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);
        }
        break;

        case cdmaResetMsg:
        {
            #ifdef CDMA_CMD_COUNT
            cdmaCmdSlotQueuePi = 0;
            writel(cdmaCmdSlotQueuePi, pCdmaCmdSlotQueuePi);
            #endif

            writel(0, _cdmaSq.pHwPi);
            #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
            if (readl(REG_FPS_INDIRECT_REG_WR_DISABLE) & FP2HWE_Q_PI_04_WR_BIT)
            {
                writel(0, cdmaSqPiHwAddr);
            }
            #endif
            writel(0, _cdmaSq.pHwCi);

            uint32_t* pCmdArrayHwCi = NULL;
            for (uint16_t i = 0; i < UCD_FP_IO_Q_NUM; i++)
            {
                pCmdArrayHwCi = (uint32_t*)(REG_FPS_SLOT_ARRAY_CI_BASE + (i << CMD_ARRAY_SHIFT));
                cmdArrayCi[i] = readl(pCmdArrayHwCi);
            }

            SetupIdleCmd();
            _fpsCpu1QueueManagerFiber.Resume();

            CmdEntryStatus_t currentCPIOStatus = (CmdEntryStatus_t)(readl(PSRAM_FP_CPIO_CDMA_STATUS_ADDR));
            if (hasCPCDMACmd || currentCPIOStatus == cCEStsFatalError)
            {
                hasCPCDMACmd = 1;
                if (currentCPIOStatus == cCEStsFatalError)
                {
                    CPCDMAStatus = cCEStsFatalError;
                }

                _fpsCpu1CPCDMAIOFiber.Resume();

            }

            writel(0, _cdmaFatalErrorFlag);
        }
        break;

        #ifdef SUPPORT_MSGERROR_INJECTION
        case errInjectMsg:
        {
            if (pFPMsgHeader->enable)
            {
                errInjectFlag = 1;
            }
            else
            {
                errInjectFlag = 0;
            }
        }
        break;
        #endif

        #ifdef QOS_LATENCY_ERROR_HANDLING
        case qosPenaltyMsg:
        {
            uint8_t isAvalMsgRes = this->ChkAvailableFPInterMsgRes(cM7Core2);
            if (isAvalMsgRes)
            {
                // check if it needs to update QoS penalty and QoS VF bitmap
                uint16_t ceIdx = (pFPMsgHeader->skipAbort_ceIndex);
                uint16_t caIdx = (ceIdx >> CA_SIZE_SHIFT);
                uint16_t phyIbqId = pCa2IbPhysicalId[caIdx];
                uint8_t qbIndex = SQ_PID_2_QBIDX(phyIbqId);
                QueueBlockInfo_t* pQBlockInfo = &this->_pQBlockInfoBase[qbIndex];
                uint8_t vfId = pQBlockInfo->vfId;

                if(vfId > MAX_VF_NUM)
                {
                    recvSts = msgVfOutOfRange;
                }
                else
                {
                    VFNodeInfo_t* pVFNodeInfo = &this->_pVfInfoBase[vfId];
                    uint8_t vfGroupIndex = vfId >> 5;
                    if (((*_pQosVFBitmap[vfGroupIndex] & BIT((vfId & 0x1f))) == 0) && (pVFNodeInfo->credit > CHUNK_SIZE))
                    {
                        #ifdef QOS_LATENCY_GLOBAL_UNIQUE
                        CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &this->_pQosPenalty[0];
                        #else
                        CP2FPMsgDataQoSPenalty_t* pQosPenaltyInfo = &this->_pQosPenalty[vfId];
                        #endif
                        pVFNodeInfo->qosPenaltyPeriod = pQosPenaltyInfo->Cfg.qosPenaltyPeriod;

                        // 4K alignment or round down
                        _pVfCredit[vfId] = ((((uint32_t)(pVFNodeInfo->credit) * ((uint32_t)(pQosPenaltyInfo->Cfg.qosPenaltyCreditRatio))) / 100) & (~0xFFF));
                        *_pTotalCredit = *_pTotalCredit - pVFNodeInfo->credit + _pVfCredit[vfId];
                        *_pQosVFBitmap[vfGroupIndex] |= BIT((vfId & 0x1f));

                        //DebugLogLvDbgInfo(cLogCPU1Common, cLogDebug, ("[CDMA ERR] vfId:0x%X, ceIndex:0x%X, qCnt:0x%X, period:0x%X, ratio:0x%X\n",                                   \
                                                                (((pVFNodeInfo->credit << 0xCUL) & 0xFF000000UL) | ((uint32_t)(ceIdx) << 0x8UL) | ((uint32_t)vfId & 0xFFUL)), \
                                                                (((uint32_t)(pQosPenaltyInfo->Cfg.qosPenaltyCreditRatio & 0xFFUL) << 0x8UL) | ((uint32_t)pVFNodeInfo->qosPenaltyPeriod & 0xFFUL))), "8,16,8", "8,24");
                    }
                }

                DMB();

                // send response back to cpu2
                recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);
            }
            else
            {
                recvSts = msgNoEmptyEntry;
            }
        }
        break;
        #endif

        case ConfigIOMsg:
        {
            if ((ConfigIOStatus_t)pFPMsgHeader->configIo == cPauseIO)
            {
                _fpsCpu1QueueManagerFiber.WaitCurrentRound();
                _fpsCpu1CPCDMAIOFiber.WaitCurrentRound();
                for (uint16_t i = 0; i < (UCD_FP_IO_Q_SLOT_NUM + FPS_QUEUE_BLOCK_65); i++)
                {
                    if (_pQBlockInfoBase[i].remainLen)
                    {
                        uint32_t ceIndex = _pQBlockInfoBase[i].remainCeIdx;
                        _pQBlockInfoBase[i].remainLen = 0;
                        _pCmdArrayBase[ceIndex].Status = cCEStsCorrKeyErrHandling;
                        API_CDMASendAbort(ceIndex,   &cdmaSqPi,  0,  0,  &_cdmaSq, _cdmaFatalErrorFlag);
                    }
                }
            }
            else
            {
                _fpsCpu1QueueManagerFiber.Resume();
                _fpsCpu1CPCDMAIOFiber.Resume();
            }
            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);
            break;
        }

        case resetHandlingMsg:
        {
            uint32_t resetCause = IpcIntGetDescValue(ResetCP2FP);
            uint64_t FLRRequestBitMap = readq(pFLRRequestBitMapLocal);
            uint8_t vfId;

            for (vfId = FindNextBit64(FLRRequestBitMap); FLRRequestBitMap; FLRRequestBitMap &= ~(BIT_ULL(vfId)), vfId = FindNextBit64(FLRRequestBitMap))
            {
                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Handle function reset ID:0x%x \n", (vfId)), "32");
                vfId = MAP_FUNCTION_ID(vfId);
                _HandleTeardown(vfId);
            }
            if (resetCause != VFLR && readl(_pVF65EnBitmap))
            {
                //DebugLogLvDbgInfo(cLogCPU1Common, cLogInfo, ("Handle function reset ID:0x%x \n", (PF_ID)), "32");
                _HandleTeardown(PF_ID);
            }

            _ProcessTeardownBitMap();

            recvSts = SendFPMsg(msgCpu, fpMsgOp, 1, fpSts, pFPMsgHeader->cmdSpecific[0], pFPMsgHeader->cmdSpecific[1]);

            break;
        }

        case doNothingMsg:
        {
            break;
        }

        default:
            recvSts = msgNotSupport;
            break;
    }
    return recvSts;
}

void fpsCpu1::SetupIdleCmd()
{
    CdmaSq_t* pCdmaSq = &_cdmaSq;
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    volatile CdmaSqCmdDescr_t* pCdmaSqe;

    if (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
    {
        cdmaSqCi = readl(pCdmaSq->pHwCi);
        while (QUEUE_FULL(cdmaSqPi, cdmaSqCi, FPS_CDMA_QUEUE_DEPTH_MASK))
        {
            if (readl(_cdmaFatalErrorFlag))
            {
                return;
            }
            cdmaSqCi = readl(pCdmaSq->pHwCi);
        }
    }

    pCdmaSqe = pCdmaSqBase + cdmaSqPi;
    pCdmaSqe->dw0 = CDMA_IDLE_CMD_CPU_ID | (NEW_CMD << CDMA_SQE_DW0_CMD_STATE_SHIFT) | \
                    (CDMA_OPCODE_IDLE << CDMA_SQE_DW0_OPCODE_SHIFT) |                  \
                    (CDMA_LIST_1 << CDMA_SQE_DW0_CDMA_LIST_NUM_SHIFT);
    pCdmaSqe->dw1 = CHUNK_SIZE  << CDMA_SQE_DW1_CHUNK_BCNT_SHIFT;
    pCdmaSqe->dw2 = 0;
    pCdmaSqe->dw3 = 0;

    DMB();

    cdmaSqPi = QUEUE_INC(cdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);
    writel(cdmaSqPi, pCdmaSq->pHwPi);
    #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
    writel(cdmaSqPi, cdmaSqPiHwAddr);
    #endif
}

void fpsCpu1::_HandleTeardown(uint8_t vfId)
{
    if(vfId > MAX_VF_NUM)
    {
        DebugLogLvDbgInfo(cLogCPU1Common, cLogError, ("_HandleTeardown: vfId out of range, vfId:0x%X\n", vfId), "32");
    }

    VFNodeInfo_t* pVfInfo = &_pVfInfoBase[vfId];
    _teardownQueueBlockBitMap |= pVfInfo->queueBlkBitMap;
    _teardownQueueBlock65BitMap = pVfInfo->queueBlk65BitMap;

    // Reset key and rgid owner table in teardown.
    AesKeyVault_t* keyVaultArr = (AesKeyVault_t*)(&(rCdma->aesKeyVaultAddr));
    // Seqlock: enter writer critical section before zeroing vault slots.
    vault_seq_bump();
    for (uint8_t i = 0; i < MAX_FP_RGID_NUM; i++)
    {
        if (_rgid2OwnerVfid[i] == vfId)
        {
            for (uint8_t k = 0; k < (KEYUPDATE_KEY_SUB_IDX_MAX + 1); k++)
            {
                if (_rgid2keyValid[i] & BIT(k))
                {
                    uint16_t idx = i * (KEYUPDATE_KEY_SUB_IDX_MAX + 1) + k;
                    vault_zero_slot(keyVaultArr[idx].key);
                }
            }
            _rgid2keyValid[i] = 0;
            _rgid2OwnerVfid[i] = RGID_NO_OWNER_VF;
        }
    }
    // Seqlock: leave writer critical section; FP2 readers may now proceed.
    vault_seq_bump();

    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    for(uint32_t keyIndex = 0; keyIndex < KEY_INDEX_MAX; keyIndex++)
    {
        if(vfId == _key2OwnerVfid[keyIndex])
        {
            CleanKeyInfo(keyIndex);
        }
    }
    #endif

    #ifdef QOS_LATENCY_ERROR_HANDLING
    uint8_t vfGroupIndex = vfId >> 5;
    switch (vfGroupIndex)
    {
        case VF0_VF31:
        case VF32_VF63:
        {
            *this->_pQosVFBitmap[vfGroupIndex] &= (~(BIT(vfId & 0x1f)));
            break;
        }
        case VF64:
        {
            *this->_pQosVFBitmap[vfGroupIndex] = 0;
            break;
        }
        default:
            break;
    }
    #endif
}
