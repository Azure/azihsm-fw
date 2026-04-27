// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#include "RegCdma.h"
#include "M7MemMap.h"
#include "ErrorCodes.h"
#include "FpCommon.h"
#ifdef SUPPORT_FPS_REGISTER
#include "RegFps.h"
#endif

///< CdmaCfg_t
#define CMDA_CFG_OP_MODE_DEGRADE_EN (BIT(16))
#define CMDA_CFG_CP_EVERY_POST_EN (BIT(17))
#define CDMA_CFG_MAX_DESCR_ELMNT_CHK_EN (BIT(18))

//cdma_reg_CONTROL
#define CDMA_RST (BIT(0))
#define CDMA_EN (BIT(1))
#define AXI_RD_EN (BIT(2))
#define AXI_WR_EN (BIT(3))
#define CDMA_CTRL_CQ_CRYPTO_UPDT_EN (BIT(4))
#define CDMA_CTRL_CQ_NVME_UPDT_EN  (BIT(5))

//cdma list configuration shift
#define ELEMENT_DWORD_SZ_SH 0
#define DESCR_DWORD_OFFSET_SH 6
#define CRYPTO_IN_DWORD_OFFSET_SH 12
#define CRYPTO_OUT_DWORD_OFFSET_SH 18
#define INTRFC_SEL_SH 24
#define CDMA_LIST_NUM 4

//CDMA list field value definition
#define ELEMENT_SIZE ((uint32_t)DFL_BUFF_ELMNT_SIZE >> 2)
#define DESCR_OFFSET 0x6UL
#define CRYPTO_IN_OFFSET 0x13UL//19
#define CRYPTO_OUT_OFFSET 0x1UL

///< CdmaDeliveryQueue0Cfg_t
#define CDMA_SQ_IFSEL_SHIFT 16
#define CDMA_SQ_ELEMENT_SZ_SHIFT 8
#define CDMA_SQ_ELEMENT_NUM_SHIFT 0
#define CDMA_SQ_SHADOW_EN (BIT(30))
#define CDMA_SQ_EN (BIT(31))

///< CdmaCompletionQueue0Cfg_t
#define CDMA_CQ_IFSEL_SHIFT 16
#define CDMA_CQ_ELEMENT_SZ_SHIFT 8
#define CDMA_CQ_ELEMENT_NUM_SHIFT 0
#define CDMA_CQ_SHADOW_EN (BIT(30))
#define CDMA_CQ_EN (BIT(31))

//DataPathParityControl_t
#define DP_PERR_EN (BIT(1))
#define DP_FERR_EN (BIT(2))

#ifdef READ_CDMA_REG_ERR_STS_WITHOUT_CQE
// CdmaCmdSlotErrorStateStatus
#define CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_0 0
#define CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_1 1
#define CDMA_CMD_SLOT_ERROR_STATE_STATUS_REG_ID_2 2
#endif

// CdmaCmdSlotErrorStatus
#define CDMA_CMD_SLOT_ERROR_STATUS_REG_ID_0 0
#define CDMA_CMD_SLOT_ERROR_STATUS_REG_ID_1 1

// CdmaCmdSlotErrorCheckEnable
#define CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_0 0
#define CDMA_CMD_SLOT_ERROR_CHECK_EN_REG_ID_1 1

// globalErrorCheckEnable
#define KV_MEM_CORR_ECC_ERR_THRESHOLD_COUNT_MASK 0xFFFF
#define KV_MEM_ECC_ERR_CHECK_EN (BIT(16))
#define AXI_WRITE_BRESP_CHECK_EN (BIT(20))
#define CMDE_FREE_SLOT_EMPTY_TIMEOUT_ERR_CHECK_EN (BIT(21))
#define AXI_READ_BUS_PARITY_ERROR_CHECK_EN (BIT(24))
#define AXI_READ_RRESP_CHECK_EN (BIT(25))

// Correctable error threshold
#define CORRECTABLE_KEY_ERROR_THRESHOLD  2


#define WRITEL(data, addr) (*((volatile uint32_t*)(addr)) = (uint32_t)(data))
#define WRITEQ(data, addr) (*((volatile uint64_t*)(addr)) = (uint64_t)(data))
#define READL(addr) (*((volatile uint32_t*)(addr)))

typedef enum CDMACqOpMode_t
{
    cCDMACqLastPost,
    cCDMACqEveryPost
}CDMACqOpMode_t;

typedef enum CDMAOpMode_t
{
    cCDMANormalMode,
    cCDMADegradeMode
}CDMAOpMode_t;

typedef enum CDMAKeyErrorType_t
{
    cCDMAUncorrKeyError,
    cCDMACorrKeyError
}CDMAKeyErrorType_t;

typedef enum CDMATimerValue_t
{
    cQoSLatencyTimerValue,
    cDestDataXferTimerValue
} CDMATimerValue_t;


/**
 *  @brief   To setup CDMA Timer Value
 *  @param    timerValueId   CDMA Timer Value ID.
 *  @param    value   Interface select for list.
 *  @return  None.
 */
void HalCDMA_SetTimerValue(CDMATimerValue_t timerValueId, uint32_t value);

/**
 *  @brief  To write the value of CDMA interrupt enable register.
 *  @param   interruptId Interrupt to enable
 *  @param   value Value to write
 *  @return  None.
 */
void HalCDMA_SetInterruptEnable(uint32_t interruptId, uint32_t value);

/**
 *  @brief  To write the value of CDMA command slot id to Diagnostic Control registers.
 *  @param  cmdSlotId  CDMA command slot ID.
 *  @return  None.
 */
void HalCDMA_SetDiagnosticControl(uint32_t cmdSlotId);
/**
 *  @brief  To read the value of CDMA Diagnostic Command Slot Status 0.
 *
 *  @return  None.
 */
uint32_t HalCDMA_GetDiagnosticCommandSlotStatus(void);

/**
 *  @brief  To read the value of CDMA interrupt cause.
 *
 *  @return  None.
 */
uint32_t HalCDMA_GetInterruptCause(void);


#ifdef READ_CDMA_REG_ERR_STS_WITHOUT_CQE
/**
 *  @brief  To read the value of CDMA command slot error state status 0/1/2 registers.
 *  @param  errStateStsRegId  ID of CDMA command slot error state status register to set.
 *  @return  The value of CDMA command slot error state status register.
 */
uint32_t HalCDMA_GetCmdSlotErrorStateStatus(uint32_t errStateStsRegId);
#endif

/**
 *  @brief  To read the value of CDMA command slot error status 0/1 registers.
 *  @param  errStsRegId     ID of CDMA command slot error status register to set.
 *  @return  The value of CDMA command slot error status register.
 */
uint32_t HalCDMA_GetCmdSlotErrorStatus(uint32_t errStsRegId);

/**
 *  @brief To read CDMA command slot error check enable 0/1 registers.
 *  @param   errCheckEnRegId    ID of CDMA command slot error check enable register to set.
 *  @return   The value of CDMA command slot error check enable 0/1 register.
 */
uint32_t HalCDMA_GetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId);

/**
 *  @brief To program CDMA command slot error check enable 0/1 registers.
 *  @param   errCheckEnRegId    ID of CDMA command slot error check enable register to set.
 *  @param   errCheckEnMask     Value of CDMA command slot error check enable mask to set.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId, uint32_t errCheckEnMask);

/**
 *  @brief To program CDMA list configuration registers and setup list address (DFL address)
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetList(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr);

/**
 *  @brief To program CDMA list configuration registers and setup list address (DFL address)
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SetListWithConfig(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr, uint32_t config);

/**
 *  @brief To program DeliveryQueue register
 *  @param    intfrcSel    Interface select for CDMA SQ.
 *  @param    sqElmntSize  Element size in CDMA SQ.
 *  @param    sqDepth      The depth of CDMA SQ.
 *  @param    sqCiAddr     The address of CDMA SQ CI shadow.
 *  @param    sqEntryAddr  The base address of CDMA SQ.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_SqSetup(uint32_t intfrcSel, uint32_t sqElmntSize, uint32_t sqDepth, uint32_t sqCiAddr, uint32_t sqEntryAddr);

/**
 *  @brief To program CompletionQueue register
 *  @param    intfrcSel    Interface select for CDMA SQ.
 *  @param    cqElmntSize  Element size in CDMA CQ.
 *  @param    cqDepth      The depth of CDMA CQ.
 *  @param    cqPiAddr     The address of CDMA CQ PI shadow.
 *  @param    cqEntryAddr  The base address of CDMA CQ.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t HalCDMA_CqSetup(uint32_t intfrcSel, uint32_t cqElmntSize, uint32_t cqDepth, uint32_t cqPiAddr, uint32_t cqEntryAddr);

/**
 *  @brief To program register to set Pi and Ci of CDMA SQ to 0
 *
 *  @return  void
 */
void HalCDMA_ResetSq(void);

/**
 *  @brief To program register to set Pi and Ci of CDMA CQ to 0
 *
 *  @return  void
 */
void HalCDMA_ResetCq(void);

/**
 *  @brief To set CDMA dummy port address
 *  @param   baseAddr Address of CDMA dummy port.
 *  @return  void
 */
void HalCDMA_SetDummyPortAddr(uint64_t baseAddr);

/**
 *  @brief To set CDMA interrupt coalescing configuration register
 *
 *  @return  void
 */
void HalCDMA_SetIntCoal(void);

/**
 *  @brief To program reset bit in CDMA controll register
 *
 *  @return  void
 */
void HalCDMA_Reset(void);

/**
 *  @brief To enable CDMA engine
 *
 *  @return  void
 */
void HalCDMA_Enable(void);

/**
 *  @brief To enable CDMA AXI read/write
 *
 *  @return  void
 */
void HalCDMA_AxiEnable(void);

#ifdef SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
/**
 *  @brief To enable Max Descriptor Elements Check
 *
 *  @return  void
 */
void HalCDMA_MaxDescrElmntChkEnable(void);

/**
 *  @brief To set Descriptor Max Elements Count
 *
 *  @return  void
 */
void HalCDMA_SetDescrMaxElmntCount(uint32_t maxElmntCount);
#endif
#if 0
/**
 *  @brief API for getting Max Descriptor Elements Count
 *
 *  @return  void
 */
uint32_t HalCDMA_GetMaxDescrElmntCount(void);
#endif

/**
 *  @brief To enable Data Path Parity Check
 *
 *  @return  void
 */
void HalCDMA_DataPathParityChkEnable(void);

/**
 *  @brief To read the value of CDMA Data Path Parity status register
 *
 *  @return  The value of CDMA Data Path Parity status register
 */
uint32_t HalCDMA_GetDataPathParityStatusReg(void);

/**
 *  @brief To read the value of CDMA controll register
 *
 *  @return  The value of controll register
 */
uint32_t HalCDMA_GetControllReg(void);

/**
 *  @brief To program CDMA controll register
 *  @param val Value to write to controll register.
 *  @return  void
 */
void HalCDMA_SetControllReg(uint32_t val);


/**
 *  @brief To read the value of CDMA configuration register
 *
 *  @return The value of configuration register
 */
uint32_t HalCDMA_GetConfigurationReg(void);

/**
 *  @brief To program CDMA configuration register
 *  @param val Value to write to configuration register.
 *  @return void
 */
void HalCDMA_SetConfigurationReg(uint32_t val);


/**
 *  @brief Program register to change CDMA operating mode
 *  @param opMode The CDMA operation mode to update.
 *  @return  void
 */
void HalCDMA_ChangeOpMode(CDMAOpMode_t opMode);

/**
 *  @brief Program register to change CDMA CQ operating mode
 *  @param cqOpMode The CDMA CQ operation mode to update
 *  @return  void
 */
void HalCDMA_ChangeCqOpMode(CDMACqOpMode_t cqOpMode);

/**
 *  @brief Program CDMA register pause CDMA engine
 *
 *  @return  void
 */
void HalCDMA_CDMAPause(void);

/**
 *  @brief Program CDMA register resume CDMA engine
 *
 *  @return  void
 */
void HalCDMA_CDMAResume(void);

/**
 *  @brief To set "Host CQE Crypto Section Update Enable" and "Host CQE NVMe Section Update Enable"
 *
 *  @return  void
 */
void HalCDMA_EnableCqeUpdate(void);

/**
 *  @brief To disable CDMA SQ
 *
 *  @return  void
 */
void HalCDMA_DisableSQ(void);

/**
 *  @brief To enable CDMA SQ
 *
 *  @return  void
 */
void HalCDMA_EnableSQ(void);

/**
 *  @brief To disable CDMA CQ
 *
 *  @return  void
 */
void HalCDMA_DisableCQ(void);

/**
 *  @brief To enable CDMA CQ
 *
 *  @return  void
 */
void HalCDMA_EnableCQ(void);

/**
 *  @brief To read CDMA list configuration
 *  @param listId The id of list
 *  @return  void
 */
uint32_t HalCDMA_ReadListConfiguration(uint32_t listId);

/**
 *  @brief To read CDMA list base address
 *  @param listId The id of list
 *  @return  void
 */
uint64_t HalCDMA_ReadListAddress(uint32_t listId);

/**
 *  @brief To read CDMA status register
 *  @return  void
 */
uint32_t HalCDMA_ReadStatus(void);


/**
 *  @brief To enable CDMA halt when fatal error happen
 *  @return  void
 */
void HalCDMA_SetFatalErrorHaltEnable(void);

/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *  @param    thresholdValue threshold value.
 *  @return  void
 */
void HalCDMA_SetCorrKeyErrThreshold(uint32_t thresholdValue);


/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *
 *  @param   keyErrorSelect To select uncorrtable or corrtable key error count to read
 *
 *  @return  Current Correctable error count value
 */
uint32_t HalCDMA_GetCorrKeyErrCount(CDMAKeyErrorType_t keyErrorSelect);


/**
 *  @brief To set CDMA global check enable register
 *
 *  @param   value value to add to global check enable register
 *
 *  @return  Current Correctable error count value
 */
void HalCDMA_SetGlobalCheckEnable(uint32_t value);

/**
 *  @brief To clear (disable) bits in the CDMA global check enable register
 *
 *  @param   value Bits to clear in the global check enable register
 *
 *  @return  None
 */
void HalCDMA_ClearGlobalCheckEnable(uint32_t value);


/**
 *  @brief   Write value to interrupt cause register to clear interrupt cause
 *
 *  @param   value value to write to interrupt cause register
 *
 *  @return  Current Correctable error count value
 */
void HalCDMA_WriteOneClearInterruptCause(uint32_t value);
