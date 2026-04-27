// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once
#include "HalCdma.h"
#include "ErrorCodes.h"
#include "BitmapOp.h"
#include "CDMA.h"

/**
 *  @brief API for setup CDMA Timer Value
 *  @param    timerValueId   CDMA Timer Value ID.
 *  @param    value   Interface select for list.
 *  @return   None.
 */
void API_CDMASetTimerValue(CDMATimerValue_t timerValueId, uint32_t value);

/**
 *  @brief API for write Diagnostic Control register.
 *  @param   cmdSlotId  CDMA command slot ID for error status register to set Diagnostic Control register.
 *  @return   None.
 */
void API_CDMASetDiagnosticControl(uint32_t cmdSlotId);

#ifdef READ_CDMA_REG_ERR_STS_WITHOUT_CQE
/**
 *  @brief API for get command slot ID in error state.
 *  @param   None.
 *  @return   The command slot ID that in error state.
 */
uint32_t API_CDMAGetCmdSlotIdInErrorState(void);
#endif

/**
 *  @brief API for read command slot error status.
 *  @param   errStsRegId  ID of CDMA command slot error status register to set.
 *  @return   The value of CDMA command slot error status register.
 */
uint32_t API_CDMAGetCmdSlotErrorStatus(uint32_t errStsRegId);

/**
 *  @brief API for setup command slot error checking
 *  @param    errCheckEnId  ID of CDMA command slot error check enable register to set.
 *  @param    errCheckEnMask    Value of CDMA command slot error check enable mask to set.
 *  @return    cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetCmdSlotErrorCheckEn(uint32_t errCheckEnRegId, uint32_t errCheckEnMask);

/**
 *  @brief API for setup DFL address and interface select for CDMA list
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetList(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr);

/**
 *  @brief API for setup DFL address, configuration and interface select for CDMA list
 *  @param    listId ID of CDMA list to set.
 *  @param    intfrcSel Interface select for list.
 *  @param    dflAddr Base address of the list.
 *  @param    config Configuration for
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASetListWithConfig(uint32_t listId, uint32_t intfrcSel, uint64_t dflAddr, uint32_t config);

/**
 *  @brief API get configuration value by configuration fields
 *  @param    elementDWSize element size in dword
 *  @param    descrDWOffset dword offset of descr
 *  @param    cryptoInOffset dword offset of crypto in
 *  @param    cryptoOutOffset  dword offset of crypto out
 *  @return   Value of configuration
 */
uint32_t API_CDMAGetConfigValue(uint32_t elementDWSize, uint32_t descrDWOffset, uint32_t cryptoInOffset, uint32_t cryptoOutOffset);

/**
 *  @brief API for setup DeliveryQueue register
 *  @param    sqCiShadowAddr CDMA SQ CI shadow address.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMASqSetup(uint32_t sqCiShadowAddr);

/**
 *  @brief API for setup CompletionQueue register
 *  @param    cqPiShadowAddr CDMA CQ PI shadow address.
 *  @return   cEcError: if fail / cEcNoError: if success
 */
Error_t API_CDMACqSetup(uint32_t cqPiShadowAddr);

/**
 *  @brief API for change CDMA operating mode
 *  @param opMode The CDMA operation mode to update.
 *  @return  void
 */
void API_CDMAChangeOpMode(CDMAOpMode_t opMode);

/**
 *  @brief API for change CDMA CQ operating mode
 *  @param cqOpMode The CDMA CQ operation mode to update
 *  @return  void
 */
void API_CDMAChangeCqOpMode(CDMACqOpMode_t cqOpMode);

#ifdef SUPPORT_CFG_CDMA_REG_MAX_ELEMT_CNT
/**
 *  @brief API for setting Max Descriptor Elements Count and enable Max Descriptor Elements Check
 *  @param maxElmntCount The value of Max Descriptor Elements Count
 *  @return  void
 */
void API_CDMAMaxDescrElmntChkCfgEn(uint32_t maxElmntCount);
#endif
#if 0
/**
 *  @brief API for getting Max Descriptor Elements Count
 *
 *  @return  void
 */
uint32_t API_CDMAGetMaxDescrElmntCount(void);
#endif

/**
 *  @brief API for pause CDMA engine
 *
 *  @return  void
 */
void API_CDMAPause(void);

/**
 *  @brief API for reseume CDMA engine
 *
 *  @return  void
 */
void API_CDMAResume(void);

/**
 *  @brief API for CDMA engine
 *
 *  @param enableCDMA Whether eanble CDMA engine after init
 *
 *  @return  void
 */
void API_CDMAInit(bool enableCDMA);

/**
 *  @brief API for enable CDMA SQ
 *
 *  @return  void
 */
void API_CDMAEnableSQ(void);
/**
 *  @brief API for disable CDMA SQ
 *  @return  void
 */
void API_CDMADisableSQ(void);

/**
 *  @brief API for checking CDMA BCP_ERR_HALT bit set
 *  @return  true: if halt, false otherwise
 */
uint8_t API_CDMACheckErrHalt(void);


/**
 *  @brief To set CDMA key vault memory correctable ECC error threshold count
 *  @param    thresholdValue threshold value.
 *  @return  void
 */
void APICDMA_SetCorrKeyErrThreshold(uint32_t thresholdValue);

/**
 *  @brief To clear (disable) bits in the CDMA global check enable register
 *
 *  @param   value Bits to clear in the global check enable register
 *
 *  @return  None
 */
void APICDMA_ClearGlobalCheckEnable(uint32_t value);