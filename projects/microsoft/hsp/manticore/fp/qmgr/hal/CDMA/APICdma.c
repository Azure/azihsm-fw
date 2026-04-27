// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "APICdma.h"

/**
 *  @brief API for setup CDMA Timer Value
 *  @param    timerValueId   CDMA Timer Value ID.
 *  @param    value   Interface select for list.
 *  @return   None.
 */
void API_CDMASetTimerValue(CDMATimerValue_t timerValueId, uint32_t value)
{
    HalCDMA_SetTimerValue(timerValueId, value);
}

/**
 *  @brief API for write Diagnostic Control register.
 *  @param   cmdSlotId  CDMA command slot ID for error status register to set Diagnostic Control register.
 *  @return   None.
 */
void API_CDMASetDiagnosticControl(uint32_t cmdSlotId)
{
    HalCDMA_SetDiagnosticControl(cmdSlotId);
}

#ifdef READ_CDMA_REG_ERR_STS_WITHOUT_CQE
/**
 *  @brief API for get command slot ID in error state.
 *  @param   None.
 *  @return   The command slot ID that in error state.
 */
uint32_t API_CDMAGetCmdSlotIdInErrorState(void)
{
    uint32_t cmdSlotId = 0xFFFF;

    if (HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0))
    {
        cmdSlotId = FindNextBit32(HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0));
    }
    else if (HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1))
    {
        cmdSlotId = FindNextBit32(HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1));

        cmdSlotId += 32;
    }
    else if (HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2))
    {
        cmdSlotId = FindNextBit32(HalCDMA_GetCmdSlotErrorStateStatus(CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2));
        cmdSlotId += 64;
    } // else do nothing


    return cmdSlotId;
}
#endif

/**
 *  @brief API for read command slot error status.
 *  @param   errStsRegId  ID of CDMA command slot error status register to set.
 *  @return   The value of CDMA command slot error status register.
 */
uint32_t API_CDMAGetCmdSlotErrorStatus(uint32_t errStsRegId)
{
    return (HalCDMA_GetCmdSlotErrorStatus(errStsRegId) & HalCDMA_GetCmdSlotErrorCheckEn(errStsRegId));
}

/**
 *  @brief API for setup command slot error checking
 *  @param    errCheckEnId  ID of CDMA command slot error check enable register to set.
 *  @param    errCheckEnMask    Value of CDMA command slot error check enable mask to set.
 *  @return    cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId, uint32_t errCheckEnMask)
{
    Error_t     errCode;
    errCode = HalCDMA_SetCmdSlotErrorCheckEn(errCheckEnRegId, errCheckEnMask);
    return errCode;
}

/**
 *  @brief API for setup DFL address and interface select for CDMA list
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetList(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr)
{
    Error_t     errCode;
    errCode = HalCDMA_SetList(listId, intfrcSel, dflAddr);
    return errCode;
}

/**
 *  @brief API for setup DFL address, configuration and interface select for CDMA list
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @param    config Configuration for
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetListWithConfig(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr, uint32_t config)
{
    Error_t     errCode;
    errCode = HalCDMA_SetListWithConfig(listId, intfrcSel, dflAddr, config);
    return errCode;
}

/**
 *  @brief API get configuration value by configuration fields
 *  @param    elementDWSize element size in dword
 *  @param    descrDWOffset dword offset of descr
 *  @param    cryptoInOffset dword offset of crypto in
 *  @param    cryptoOutOffset  dword offset of crypto out
 *  @return   Value of configuration
 */
uint32_t API_CDMAGetConfigValue(uint32_t elementDWSize, uint32_t descrDWOffset, uint32_t cryptoInOffset, uint32_t cryptoOutOffset)
{
    uint32_t cfgVal = 0;
    cfgVal = (elementDWSize << ELEMENT_DWORD_SZ_SH) |
             (descrDWOffset << DESCR_DWORD_OFFSET_SH) |
             (cryptoInOffset << CRYPTO_IN_DWORD_OFFSET_SH) |
             (cryptoOutOffset << CRYPTO_OUT_DWORD_OFFSET_SH);

    return cfgVal;
}

/**
 *  @brief API for setup DeliveryQueue register
 *  @param    sqCiShadowAddr CDMA SQ CI shadow address.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASqSetup(uint32_t sqCiShadowAddr)
{
    Error_t     errCode;
    uint32_t intfrcSel = 0;
    uint32_t sqEntryAddr = getCPU1TCMPhysicalAddress((uint32_t)M7_FPS_CPU12_CDMA_SQ_ENTRY_ADDR);
    HalCDMA_ResetSq();
    errCode = HalCDMA_SqSetup(intfrcSel, CDMA_SQ_ELMNT_SIZE, CDMA_SQ_DEPTH, sqCiShadowAddr, sqEntryAddr);

    return errCode;
}

/**
 *  @brief API for setup CompletionQueue register
 *  @param    cqPiShadowAddr CDMA CQ PI shadow address.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMACqSetup(uint32_t cqPiShadowAddr)
{
    Error_t     errCode;
    uint32_t intfrcSel = 0;
    uint32_t cqEntryAddr = getCPU2TCMPhysicalAddress((uint32_t)M7_FPS_CPU2_CDMA_CQ_ENTRY_ADDR);
    HalCDMA_ResetCq();

    errCode = HalCDMA_CqSetup(intfrcSel, CDMA_CQ_ELMNT_SIZE, CDMA_CQ_DEPTH, cqPiShadowAddr, cqEntryAddr);

    return errCode;
}

/**
 *  @brief API for change CDMA operating mode
 *  @param opMode The CDMA operation mode to update.
 *  @return  void
 */
void API_CDMAChangeOpMode(CDMAOpMode_t opMode)
{
    HalCDMA_ChangeOpMode(opMode);
}

/**
 *  @brief API for change CDMA CQ operating mode
 *  @param cqOpMode The CDMA CQ operation mode to update
 *  @return  void
 */
void API_CDMAChangeCqOpMode(CDMACqOpMode_t cqOpMode)
{
    HalCDMA_ChangeCqOpMode(cqOpMode);
}

#ifdef SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
/**
 *  @brief API for setting Max Descriptor Elements Count and enable Max Descriptor Elements Check
 *  @param maxElmntCount The value of Max Descriptor Elements Count
 *  @return  void
 */
void API_CDMAMaxDescrElmntChkCfgEn(uint32_t maxElmntCount)
{
    HalCDMA_SetDescrMaxElmntCount(maxElmntCount);
    HalCDMA_MaxDescrElmntChkEnable();
}
#endif
#if 0
/**
 *  @brief API for getting Max Descriptor Elements Count
 *
 *  @return  void
 */
uint32_t API_CDMAGetMaxDescrElmntCount(void)
{
    uint32_t val = HalCDMA_GetMaxDescrElmntCount();
    return val;
}
#endif

/**
 *  @brief API for pause CDMA engine
 *
 *  @return  void
 */
void API_CDMAPause(void)
{
    HalCDMA_CDMAPause();
}

/**
 *  @brief API for reseume CDMA engine
 *
 *  @return  void
 */
void API_CDMAResume(void)
{
    HalCDMA_CDMAResume();
}

/**
 *  @brief API for CDMA engine
 *
 *  @return  void
 */
void API_CDMAInit(bool enableCDMA)
{
    uint64_t dummyPortAddr = CDMA_DUMMY_PORT_ADDR;

    //HalCDMA_Reset();

    HalCDMA_AxiEnable();

    HalCDMA_SetDummyPortAddr(dummyPortAddr);

    HalCDMA_EnableCqeUpdate();

    HalCDMA_DataPathParityChkEnable();
    if (enableCDMA)
    {
        HalCDMA_Enable();
    }
}

/**
 *  @brief API for enable CDMA SQ
 *
 *  @return  void
 */
void API_CDMAEnableSQ(void)
{
    HalCDMA_EnableSQ();
}

/**
 *  @brief API for disable CDMA SQ
 *
 *  @return  void
 */

void API_CDMADisableSQ(void)
{
    HalCDMA_DisableSQ();
}

/**
 *  @brief API for checking CDMA BCP_ERR_HALT bit set
 *  @return  true: if halt, false otherwise
 */
uint8_t API_CDMACheckErrHalt(void)
{
    uint32_t stsVal = HalCDMA_ReadStatus();

    return stsVal & CDMA_STS_BCP_ERR_HALT;

}

/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *  @param    thresholdValue threshold value.
 *  @return  void
 */
void APICDMA_SetCorrKeyErrThreshold(uint32_t thresholdValue)
{
    HalCDMA_SetCorrKeyErrThreshold(thresholdValue);
}

/**
 *  @brief To clear (disable) bits in the CDMA global check enable register
 *
 *  @param   value Bits to clear in the global check enable register
 *
 *  @return  None
 */
void APICDMA_ClearGlobalCheckEnable(uint32_t value)
{
    HalCDMA_ClearGlobalCheckEnable(value);
}