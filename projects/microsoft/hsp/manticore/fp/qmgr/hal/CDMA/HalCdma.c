// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "HalCdma.h"

/**
 *  @brief   To setup CDMA Timer Value
 *  @param    timerValueId   CDMA Timer Value ID.
 *  @param    value   Interface select for list.
 *  @return  None.
 */
void HalCDMA_SetTimerValue(CDMATimerValue_t timerValueId, uint32_t value)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    switch (timerValueId)
    {
        case cQoSLatencyTimerValue:
        {
            WRITEL(value, &pCdmaRegs->timerValue);
        }
        break;
        case cDestDataXferTimerValue:
        {
            WRITEL(value, &pCdmaRegs->timerValue2);
        }
        break;
        default:
            break;
    }
}

/**
 *  @brief  To write the value of CDMA interrupt enable register.
 *  @param   interruptId Interrupt to enable
 *  @param   value Value to write
 *  @return  None.
 */
void HalCDMA_SetInterruptEnable(uint32_t interruptId, uint32_t value)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    switch (interruptId)
    {
        case 0:
        {
            WRITEL(value, &pCdmaRegs->intrEnable0CdmaIrqEn0);
        }
        break;
        case 1:
        {
            WRITEL(value, &pCdmaRegs->intrEnable1CdmaIrqEn1);
        }
        break;
        default:
            break;
    }
}

/**
 *  @brief  To write the value of CDMA command slot id to Diagnostic Control registers.
 *  @param  cmdSlotId  CDMA command slot ID.
 *  @return  None.
 */
void HalCDMA_SetDiagnosticControl(uint32_t cmdSlotId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(cmdSlotId, &pCdmaRegs->diagnosticControlDiagnosticCmdSlotId);
}

/**
 *  @brief  To read the value of CDMA Diagnostic Command Slot Status 0.
 *
 *  @return  None.
 */
uint32_t HalCDMA_GetDiagnosticCommandSlotStatus(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->diagnosticCommandSlotStatus0);
}

/**
 *  @brief  To read the value of CDMA interrupt cause.
 *
 *  @return  None.
 */
uint32_t HalCDMA_GetInterruptCause(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->intrCause);
}

#ifdef READ_CDMA_REG_ERR_STS_WITHOUT_CQE
/**
 *  @brief  To read the value of CDMA command slot error state status 0/1/2 registers.
 *  @param  errStateStsRegId  ID of CDMA command slot error state status register to set.
 *  @return  The value of CDMA command slot error state status register.
 */
uint32_t HalCDMA_GetCmdSlotErrorStateStatus(uint32_t errStateStsRegId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t ret_val = 0;
    switch (errStateStsRegId)
    {
        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0:
            ret_val = READL(&pCdmaRegs->commandSlotErrorStateStatus0CmdSlotErrorStateStatus0);
            break;

        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1:
            ret_val = READL(&pCdmaRegs->commandSlotErrorStateStatus1CmdSlotErrorStateStatus1);
            break;

        case CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2:
            ret_val = (READL(&pCdmaRegs->commandSlotErrorStateStatus2.all) & 0xF);
            break;

        default:
            break;
    }
    return ret_val;
}
#endif

/**
 *  @brief  To read the value of CDMA command slot error status 0/1 registers.
 *  @param  errStsRegId     ID of CDMA command slot error status register to set.
 *  @return  The value of CDMA command slot error status register.
 */
uint32_t HalCDMA_GetCmdSlotErrorStatus(uint32_t errStsRegId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    switch (errStsRegId)
    {
        case CDMA_CMD_SLOT_ERROR_STATUS_REG_ID_0:
            return READL(&pCdmaRegs->commandSlotErrorStatus0.all);
            break;

        case CDMA_CMD_SLOT_ERROR_STATUS_REG_ID_1:
            return READL(&pCdmaRegs->commandSlotErrorStatus1.all);
            break;

        default:
            break;
    }
    return 0x0;
}

/**
 *  @brief To read CDMA command slot error check enable 0/1 registers.
 *  @param   errCheckEnRegId    ID of CDMA command slot error check enable register to set.
 *  @return   The value of CDMA command slot error check enable 0/1 register.
 */
uint32_t HalCDMA_GetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    switch (errCheckEnRegId)
    {
        case CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_0:
            return READL(&pCdmaRegs->commandSlotErrorCheckEnable0CmdSlotErrorCheckEnable0);
            break;

        case CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_1:
            return READL(&pCdmaRegs->commandSlotErrorCheckEnable1CmdSlotErrorCheckEnable1);
            break;

        default:
            break;
    }
    return 0x0;
}

/**
 *  @brief To program CDMA command slot error check enable 0/1 registers.
 *  @param   errCheckEnRegId   ID of CDMA command slot error check enable register to set.
 *  @param   errCheckEnMask    Value of CDMA command slot error check enable mask to set.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId, uint32_t errCheckEnMask)
{
    Error_t     errCode = cEcNoError;
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    switch (errCheckEnRegId)
    {
        case CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_0:
            WRITEL(errCheckEnMask, &pCdmaRegs->commandSlotErrorCheckEnable0CmdSlotErrorCheckEnable0);
            break;

        case CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_1:
            WRITEL(errCheckEnMask, &pCdmaRegs->commandSlotErrorCheckEnable1CmdSlotErrorCheckEnable1);
            break;

        default:
            errCode = cEcError;
            break;
    }
    return errCode;
}

/**
 *  @brief To program CDMA list configuration registers and setup list address (DFL address)
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetList(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr)
{
    Error_t     errCode = cEcNoError;
    uint32_t cfgVal = 0;
    cfgVal = (ELEMENT_SIZE << ELEMENT_DWORD_SZ_SH) |
             (DESCR_OFFSET << DESCR_DWORD_OFFSET_SH) |
             (CRYPTO_IN_OFFSET << CRYPTO_IN_DWORD_OFFSET_SH) |
             (CRYPTO_OUT_OFFSET << CRYPTO_OUT_DWORD_OFFSET_SH) |
             (intfrcSel << INTRFC_SEL_SH);

    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;

    switch (listId)
    {
        case 0:
            WRITEL(cfgVal, &pCdmaRegs->list0Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list0BaseAddressLowList0BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list0BaseAddressHighList0BaseAddrH);
            break;

        case 1:
            WRITEL(cfgVal, &pCdmaRegs->list1Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list1BaseAddressLowList1BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list1BaseAddressHighList1BaseAddrH);
            break;

        case 2:
            WRITEL(cfgVal, &pCdmaRegs->list2Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list2BaseAddressLowList2BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list2BaseAddressHighList2BaseAddrH);
            break;

        case 3:
            WRITEL(cfgVal, &pCdmaRegs->list3Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list3BaseAddressLowList3BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list3BaseAddressHighList3BaseAddrH);
            break;

        default:
            errCode = cEcError;
            break;
    }

    return errCode;
}

/**
 *  @brief To program CDMA list configuration registers and setup list address (DFL address)
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetListWithConfig(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr, uint32_t config)
{
    Error_t     errCode = cEcNoError;
    uint32_t cfgVal = 0;
    cfgVal = config | (intfrcSel << INTRFC_SEL_SH);
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;

    switch (listId)
    {
        case 0:
            WRITEL(cfgVal, &pCdmaRegs->list0Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list0BaseAddressLowList0BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list0BaseAddressHighList0BaseAddrH);
            break;

        case 1:
            WRITEL(cfgVal, &pCdmaRegs->list1Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list1BaseAddressLowList1BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list1BaseAddressHighList1BaseAddrH);
            break;

        case 2:
            WRITEL(cfgVal, &pCdmaRegs->list2Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list2BaseAddressLowList2BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list2BaseAddressHighList2BaseAddrH);
            break;

        case 3:
            WRITEL(cfgVal, &pCdmaRegs->list3Cfg.all);
            WRITEL((dflAddr & 0xffffffff), &pCdmaRegs->list3BaseAddressLowList3BaseAddrL);
            WRITEL((dflAddr >> 32), &pCdmaRegs->list3BaseAddressHighList3BaseAddrH);
            break;

        default:
            errCode = cEcError;
            break;
    }

    return errCode;
}

/**
 *  @brief To program DeliveryQueue register
 *  @param    intfrcSel    Interface select for CDMA SQ.
 *  @param    sqElmntSize  Element size in CDMA SQ.
 *  @param    sqDepth      The depth of CDMA SQ.
 *  @param    sqCiAddr     The address of CDMA SQ CI shadow.
 *  @param    sqEntryAddr  The base address of CDMA SQ.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SqSetup(uint32_t intfrcSel, uint32_t sqElmntSize, uint32_t sqDepth, uint32_t sqCiAddr, uint32_t sqEntryAddr)
{

    Error_t     errCode = cEcNoError;
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = 0;
    // DeliveryQueue0
    val |= intfrcSel << CDMA_SQ_IFSEL_SHIFT;
    switch (sqElmntSize)
    {
        case 16: // ElementSz
        {
            val |= 0x0UL << CDMA_SQ_ELEMENT_SZ_SHIFT;
        }
        break;
        case 32:
        {
            val |= 0x1UL << CDMA_SQ_ELEMENT_SZ_SHIFT;
        }
        break;
        case 64:
        {
            val |= 0x2UL << CDMA_SQ_ELEMENT_SZ_SHIFT;
        }
        break;
        default:
        {
            return cEcError;
        }
        break;
    }
    switch (sqDepth)
    {
        case 8:  // NnumElements
        {
            val |= 0x0UL << CDMA_SQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 16:
        {
            val |= 0x1UL << CDMA_SQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 32:
        {
            val |= 0x2UL << CDMA_SQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 64:
        {
            val |= 0x3UL << CDMA_SQ_ELEMENT_NUM_SHIFT;
        }
        break;
        default:
        {
            return cEcError;
        }
        break;
    }
    WRITEL((uint32_t)sqCiAddr, &pCdmaRegs->deliveryQueue0CiShadowAddressLowDlvryQ0CnsmrIndxShdwAddrL);
    val |= CDMA_SQ_SHADOW_EN;  // CnsmrIndxShdwEn
    WRITEL(val, &pCdmaRegs->deliveryQueue0Cfg.all);

    WRITEL(sqEntryAddr, &pCdmaRegs->deliveryQueue0BaseAddressLowDlvryQ0BaseAddrL);

    val = READL(&pCdmaRegs->deliveryQueue0Cfg.all);
    val |= CDMA_SQ_EN;  // En
    WRITEL(val, &pCdmaRegs->deliveryQueue0Cfg.all);

    return errCode;
}

/**
 *  @brief To program CompletionQueue register
 *  @param    intfrcSel    Interface select for CDMA SQ.
 *  @param    cqElmntSize  Element size in CDMA CQ.
 *  @param    cqDepth      The depth of CDMA CQ.
 *  @param    cqPiAddr     The address of CDMA CQ PI shadow.
 *  @param    cqEntryAddr  The base address of CDMA CQ.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_CqSetup(uint32_t intfrcSel, uint32_t cqElmntSize, uint32_t cqDepth, uint32_t cqPiAddr, uint32_t cqEntryAddr)
{
    Error_t     errCode = cEcNoError;
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = 0;
    val |= intfrcSel << CDMA_CQ_IFSEL_SHIFT;
    switch (cqElmntSize)
    {
        case 8: // ElementSz
        {
            val |= 0x0UL << CDMA_CQ_ELEMENT_SZ_SHIFT;
        }
        break;
        case 16:
        {
            val |= 0x1UL << CDMA_CQ_ELEMENT_SZ_SHIFT;
        }
        break;
        case 32:
        {
            val |= 0x2UL << CDMA_CQ_ELEMENT_SZ_SHIFT;
        }
        break;
        case 64:
        {
            val |= 0x3UL << CDMA_CQ_ELEMENT_SZ_SHIFT;
        }
        break;
        default:
        {
            return cEcError;
        }
        break;
    }
    switch (cqDepth)
    {
        case 8:     // NnumElements
        {
            val |= 0x0UL << CDMA_CQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 16:
        {
            val |= 0x1UL << CDMA_CQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 32:
        {
            val |= 0x2UL << CDMA_CQ_ELEMENT_NUM_SHIFT;
        }
        break;
        case 64:
        {
            val |= 0x3UL << CDMA_CQ_ELEMENT_NUM_SHIFT;  // use default 64
        }
        break;
        default:
        {
            return cEcError;
        }
        break;
    }
    // enable pi shadow
    WRITEL((uint32_t)cqPiAddr, &pCdmaRegs->completionQueue0PiShadowAddressLowCmpltnQ0PrdcrIndxShdwAddrL);
    val |= CDMA_CQ_SHADOW_EN;  // PrdcrIndxShdwEn
    WRITEL(val, &pCdmaRegs->completionQueue0Cfg.all);

    //SET_ADDR_64((uint64_t)M7_FDP_CDMA_CQ_ENTRY_ADDR, &pCdmaRegs->cdmaRegCompletionQueue0.BaseAddress);
    WRITEL((uint32_t)cqEntryAddr, &pCdmaRegs->completionQueue0BaseAddressLowCmpltnQ0BaseAddrL);

    val = READL(&pCdmaRegs->completionQueue0Cfg.all);
    val |= CDMA_CQ_EN;  // En
    WRITEL(val, &pCdmaRegs->completionQueue0Cfg.all);

    return errCode;
}

/**
 *  @brief To program register to set Pi and Ci of CDMA CQ to 0
 *
 *  @return  void
 */
void HalCDMA_ResetSq(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(0, &pCdmaRegs->deliveryQueue0ProducerIndex.all);
}

/**
 *  @brief To program register to reset bit in CDMA controll register
 *
 *  @return  void
 */
void HalCDMA_ResetCq(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(0, &pCdmaRegs->completionQueue0ConsumerIndex.all);
}

/**
 *  @brief To set CDMA dummy port address
 *  @param   baseAddr Address of CDMA dummy port.
 *  @return  void
 */
void HalCDMA_SetDummyPortAddr(uint64_t baseAddr)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL((uint32_t)(baseAddr & 0xffffffff),  &pCdmaRegs->dummySlavePortBaseAddressLowDummySlavePortBaseAddrL);
    WRITEL((uint32_t)(baseAddr >> 32),  &pCdmaRegs->dummySlavePortBaseAddressHighDummySlavePortBaseAddrH);
}

/**
 *  @brief To set CDMA interrupt coalescing configuration register
 *
 *  @return  void
 */
void HalCDMA_SetIntCoal(void)
{
    IntrCoalescingCfg1_t intrCoalescingCfg1;
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    intrCoalescingCfg1.all = READL(&pCdmaRegs->intrCoalescingCfg1.all);
    intrCoalescingCfg1.b.EN_INT_COAL = ENABLE;
    WRITEL(intrCoalescingCfg1.all, &pCdmaRegs->intrCoalescingCfg1.all);
}


/**
 *  @brief To program reset bit in CDMA controll register
 *
 *  @return  void
 */
void HalCDMA_Reset(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    //All registers must be configured after a soft CDMA reset
    //TBD: delay to wait HW ready
    uint32_t val = READL(&pCdmaRegs->control.all);
    val |= CDMA_RST;
    WRITEL(val, &pCdmaRegs->control.all);

}

/**
 *  @brief To enable CDMA engine
 *
 *  @return  void
 */
void HalCDMA_Enable(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    //This bit must be set to enable CDMA operation
    uint32_t val = READL(&pCdmaRegs->control.all);
    val |= CDMA_EN;
    WRITEL(val, &pCdmaRegs->control.all);
}

/**
 *  @brief To enable CDMA AXI read/write
 *
 *  @return  void
 */
void HalCDMA_AxiEnable(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = READL(&pCdmaRegs->control.all);
    if (!(val & AXI_RD_EN) && !(val & AXI_WR_EN))
    {
        val |= (AXI_RD_EN | AXI_WR_EN);
        WRITEL(val, &pCdmaRegs->control.all);
    }
}

#ifdef SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
/**
 *  @brief To enable Max Descriptor Elements Check
 *
 *  @return  void
 */
void HalCDMA_MaxDescrElmntChkEnable(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = READL(&pCdmaRegs->cfg.all);
    val |= CDMA_CFG_MAX_DESCR_ELMNT_CHK_EN;
    WRITEL(val, &pCdmaRegs->cfg.all);
}

/**
 *  @brief To set Descriptor Max Elements Count
 *
 *  @return  void
 */
void HalCDMA_SetDescrMaxElmntCount(uint32_t maxElmntCount)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(maxElmntCount, &pCdmaRegs->descMaxElementCountMaxDescrElmntCountPerChunk);
}
#endif
#if 0
/**
 *  @brief API for getting Max Descriptor Elements Count
 *
 *  @return  void
 */
uint32_t HalCDMA_GetMaxDescrElmntCount(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->descMaxElementCountMaxDescrElmntCountPerChunk);
}
#endif

/**
 *  @brief To enable Data Path Parity Check
 *
 *  @return  void
 */
void HalCDMA_DataPathParityChkEnable(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = READL(&pCdmaRegs->dataPathParityControl.all);
    val |= (DP_PERR_EN | DP_FERR_EN);
    WRITEL(val, &pCdmaRegs->dataPathParityControl.all);
}

/**
 *  @brief To read the value of CDMA Data Path Parity status register
 *
 *  @return  The value of CDMA Data Path Parity status register
 */
uint32_t HalCDMA_GetDataPathParityStatusReg(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->dataPathParityStatus.all);
}

/**
 *  @brief To read the value of CDMA controll register
 *
 *  @return  The value of controll register
 */
uint32_t HalCDMA_GetControllReg(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->control.all);
}

/**
 *  @brief To program CDMA controll register
 *  @param val Value to write to controll register.
 *  @return  void
 */
void HalCDMA_SetControllReg(uint32_t val)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(val, &pCdmaRegs->control.all);
}

/**
 *  @brief To program CDMA configuration register
 *  @param val Value to write to configuration register.
 *  @return void
 */
void HalCDMA_SetConfigurationReg(uint32_t val)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(val, &pCdmaRegs->cfg.all);
}

/**
 *  @brief To read the value of CDMA configuration register
 *
 *  @return The value of configuration register
 */
uint32_t HalCDMA_GetConfigurationReg(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->cfg.all);
}

/**
 *  @brief Program register to change CDMA operating mode
 *  @param opMode The CDMA operation mode to update.
 *  @return  void
 */
void HalCDMA_ChangeOpMode(CDMAOpMode_t opMode)
{
    uint32_t val = HalCDMA_GetConfigurationReg();
    switch (opMode)
    {
        case cCDMANormalMode:
            if (val & CMDA_CFG_OP_MODE_DEGRADE_EN)
            {
                val &= ~CMDA_CFG_OP_MODE_DEGRADE_EN;
                HalCDMA_SetConfigurationReg(val);
            }
            break;
        case cCDMADegradeMode:
            if (!(val & CMDA_CFG_OP_MODE_DEGRADE_EN))
            {
                val |= CMDA_CFG_OP_MODE_DEGRADE_EN;
                HalCDMA_SetConfigurationReg(val);
            }
            break;
        default:
            break;
    }
}

/**
 *  @brief Program register to change CDMA CQ operating mode
 *  @param cqOpMode The CDMA CQ operation mode to update
 *  @return  void
 */
void HalCDMA_ChangeCqOpMode(CDMACqOpMode_t cqOpMode)
{
    uint32_t val = HalCDMA_GetConfigurationReg();
    switch (cqOpMode)
    {
        case cCDMACqLastPost:
            if (val & CMDA_CFG_CP_EVERY_POST_EN)
            {
                val &= ~CMDA_CFG_CP_EVERY_POST_EN;
                HalCDMA_SetConfigurationReg(val);
            }
            break;
        case cCDMACqEveryPost:
            if (!(val & CMDA_CFG_CP_EVERY_POST_EN))
            {
                val |= CMDA_CFG_CP_EVERY_POST_EN;
                HalCDMA_SetConfigurationReg(val);
            }
            break;
        default:
            break;
    }
}

/**
 *  @brief Program CDMA register pause CDMA engine
 *
 *  @return  void
 */
void HalCDMA_CDMAPause(void)
{
    uint32_t val = HalCDMA_GetControllReg();
    if (val & CDMA_EN)
    {
        val &= ~CDMA_EN;
        HalCDMA_SetControllReg(val);
    }
}

/**
 *  @brief Program CDMA register resume CDMA engine
 *
 *  @return  void
 */
void HalCDMA_CDMAResume(void)
{
    uint32_t val = HalCDMA_GetControllReg();
    if (!(val & CDMA_EN))
    {
        val |= CDMA_EN;
        HalCDMA_SetControllReg(val);
    }
}

/**
 *  @brief To set "Host CQE Crypto Section Update Enable" and "Host CQE NVMe Section Update Enable"
 *
 *  @return  void
 */
void HalCDMA_EnableCqeUpdate(void)
{
    uint32_t val = HalCDMA_GetControllReg();
    if (!(val & CDMA_CTRL_CQ_CRYPTO_UPDT_EN))
    {
        val |= CDMA_CTRL_CQ_CRYPTO_UPDT_EN;
    }
    if (!(val & CDMA_CTRL_CQ_NVME_UPDT_EN))
    {
        val |= CDMA_CTRL_CQ_NVME_UPDT_EN;
    }
    HalCDMA_SetControllReg(val);
}

/**
 *  @brief To disable CDMA SQ
 *
 *  @return  void
 */
void HalCDMA_DisableSQ(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val;
    val = READL(&pCdmaRegs->deliveryQueue0Cfg.all);
    val &= ~(CDMA_SQ_EN);
    WRITEL(val, &pCdmaRegs->deliveryQueue0Cfg.all);
}


/**
 *  @brief To enable CDMA SQ
 *
 *  @return  void
 */
void HalCDMA_EnableSQ(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val;
    val = READL(&pCdmaRegs->deliveryQueue0Cfg.all);
    val |= CDMA_SQ_EN;
    WRITEL(val, &pCdmaRegs->deliveryQueue0Cfg.all);
}

/**
 *  @brief To disable CDMA CQ
 *
 *  @return  void
 */
void HalCDMA_DisableCQ(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val;
    val = READL(&pCdmaRegs->completionQueue0Cfg.all);
    val &= ~(CDMA_CQ_EN);
    WRITEL(val, &pCdmaRegs->completionQueue0Cfg.all);
}

/**
 *  @brief To enable CDMA CQ
 *
 *  @return  void
 */
void HalCDMA_EnableCQ(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val;
    val = READL(&pCdmaRegs->completionQueue0Cfg.all);
    val |= CDMA_CQ_EN;
    WRITEL(val, &pCdmaRegs->completionQueue0Cfg.all);
}


/**
 *  @brief To read CDMA list configuration
 *  @param listId The id of list
 *  @return  void
 */
uint32_t HalCDMA_ReadListConfiguration(uint32_t listId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t val = 0;
    switch (listId)
    {
        case 0:
            val = READL(&pCdmaRegs->list0Cfg.all);
            break;

        case 1:
            val = READL(&pCdmaRegs->list1Cfg.all);
            break;

        case 2:
            val = READL(&pCdmaRegs->list2Cfg.all);
            break;

        case 3:
            val = READL(&pCdmaRegs->list3Cfg.all);
            break;
        default:
            break;
    }
    return val;
}

/**
 *  @brief To read CDMA list base address
 *  @param listId The id of list
 *  @return  void
 */
uint64_t HalCDMA_ReadListAddress(uint32_t listId)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint64_t addr = 0;
    uint32_t addrHigh = 0, addrLow = 0;
    switch (listId)
    {
        case 0:
            addrLow = READL(&pCdmaRegs->list0BaseAddressLowList0BaseAddrL);
            addrHigh = READL(&pCdmaRegs->list0BaseAddressHighList0BaseAddrH);
            addr = (((uint64_t)(addrHigh)) << 32) | (addrLow);
            break;

        case 1:
            addrLow = READL(&pCdmaRegs->list1BaseAddressLowList1BaseAddrL);
            addrHigh = READL(&pCdmaRegs->list1BaseAddressHighList1BaseAddrH);
            addr = (((uint64_t)(addrHigh)) << 32) | (addrLow);
            break;

        case 2:
            addrLow = READL(&pCdmaRegs->list2BaseAddressLowList2BaseAddrL);
            addrHigh = READL(&pCdmaRegs->list2BaseAddressHighList2BaseAddrH);
            addr = (((uint64_t)(addrHigh)) << 32) | (addrLow);
            break;

        case 3:
            addrLow = READL(&pCdmaRegs->list3BaseAddressLowList3BaseAddrL);
            addrHigh = READL(&pCdmaRegs->list3BaseAddressHighList3BaseAddrH);
            addr = (((uint64_t)(addrHigh)) << 32) | (addrLow);
            break;
        default:
            break;
    }
    return addr;

}

/**
 *  @brief To read CDMA status register
 *  @return  void
 */
uint32_t HalCDMA_ReadStatus(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    return READL(&pCdmaRegs->status);
}

/**
 *  @brief To enable CDMA halt when fatal error happen
 *  @return  void
 */
void HalCDMA_SetFatalErrorHaltEnable(void)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t haltEnableVal = READL(&pCdmaRegs->haltEnableBcpErrorHaltEn);
    haltEnableVal |= CDMA_HALT_ENABLE_FATAL_ERROR;
    WRITEL(haltEnableVal,  &pCdmaRegs->haltEnableBcpErrorHaltEn);
}


/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *  @param    thresholdValue threshold value.
 *  @return  void
 */
void HalCDMA_SetCorrKeyErrThreshold(uint32_t thresholdValue)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t globalErrorCheckEnableValue = READL(&pCdmaRegs->globalErrorCheckEnable);
    globalErrorCheckEnableValue |= (thresholdValue & KV_MEM_CORR_ECC_ERR_THRESHOLD_COUNT_MASK);
    WRITEL(globalErrorCheckEnableValue, &pCdmaRegs->globalErrorCheckEnable);
}

/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *
 *  @param   keyErrorSelect To select uncorrtable or corrtable key error count to read
 *
 *  @return  Current Correctable error count value
 */
uint32_t HalCDMA_GetCorrKeyErrCount(CDMAKeyErrorType_t keyErrorSelect)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t errorCount = 0;
    if (keyErrorSelect == cCDMAUncorrKeyError)
    {
        errorCount = READL(&pCdmaRegs->keyVaultMemoryUncorrectableErrorCount);
    }
    else if (keyErrorSelect == cCDMACorrKeyError)
    {
        errorCount = READL(&pCdmaRegs->keyVaultMemoryCorrectableErrorCount);
    }

    return errorCount;
}

/**
 *  @brief To set CDMA global check enable register
 *
 *  @param   value value to add to global check enable register
 *
 *  @return  Current Correctable error count value
 */
void HalCDMA_SetGlobalCheckEnable(uint32_t value)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t curGlobalCheckEnValue = READL(&pCdmaRegs->globalErrorCheckEnable);
    curGlobalCheckEnValue |= value;
    WRITEL(curGlobalCheckEnValue, &pCdmaRegs->globalErrorCheckEnable);
}

/**
 *  @brief To clear (disable) bits in the CDMA global check enable register
 *
 *  @param   value Bits to clear in the global check enable register
 *
 *  @return  None
 */
void HalCDMA_ClearGlobalCheckEnable(uint32_t value)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    uint32_t curGlobalCheckEnValue = READL(&pCdmaRegs->globalErrorCheckEnable);
    curGlobalCheckEnValue &= ~value; // Clear the bits specified by value
    WRITEL(curGlobalCheckEnValue, &pCdmaRegs->globalErrorCheckEnable);
}

/**
 *  @brief   Write value to interrupt cause register to clear interrupt cause
 *
 *  @param   value value to write to interrupt cause register
 *
 *  @return  Current Correctable error count value
 */
void HalCDMA_WriteOneClearInterruptCause(uint32_t value)
{
    Cdma_t* pCdmaRegs = (Cdma_t*)CDMA_REG_ADDR;
    WRITEL(value, &pCdmaRegs->intrCause);

}
