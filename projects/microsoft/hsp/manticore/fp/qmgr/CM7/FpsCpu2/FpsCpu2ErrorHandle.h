// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu2ErrorHandle.h
//! @brief  FpsCpu2 Error Handle function
//!
//=============================================================================

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu2.h"

//-----------------------------------------------------------------------------
//  Interface Function Declarations
//-----------------------------------------------------------------------------

typedef enum returnAction
{
    cNone,
    cContinue,
    cReturn,
    cTeardownDeleteErrHandle,
    cBreak
}returnAction;

typedef enum adminAbortMsgCmplSts
{
    cObSuccess,
    cCdmaCmpl,
    cCdmaErr,
    cZerotransfer
}adminAbortMsgCmplSts;

//Command Slot Error Status 0

#define CMDE_AXI_RD_ERR               BIT(0)  //non fatal
#define CMDE_STRUCTURE_ERR            BIT(1)  //non fatal No retry
#define CMDE_INVALID_OPCODE_ERR       BIT(2)  //non fatal No retry
#define CMDE_UNEXPECTED_CMD_PHASE_ERR BIT(3)  //non fatal
#define FUNC_IN_ERR_STATE             BIT(9)  //non fatal No retry
#define DESCM_SRC_DESCR_SGL_SEG_ERR   BIT(10) //non fatal No retry
#define DESCM_DEST_DESCR_SGL_SEG_ERR  BIT(11) //non fatal No retry

#ifdef LIONMS_B0
#define QOS_LATENCY_TO_ERR            BIT(12) //non fatal No retry
#define DEST_DATA_XFR_TO_ERR          BIT(13) //non fatal No retry
#endif

#define DESCM_SRC_AXI_RD_ERR          BIT(14) //non fatal

#define DESCM_SRC_DESCR_STRUCTURE_ERR         BIT(15) //fatal No retry
#define DESCM_SRC_SGL_CROSS_4K_ERR            BIT(16) //fatal No retry
#define DESCM_SRC_SGL_ILLEGAL_DSCRPTR_ERR     BIT(17) //fatal No retry
#define DESCM_SRC_SGL_UNDEFINED_DSCRPTR_TYPE  BIT(18) //fatal No retry
#define DESCM_SRC_SGL_TBL_LENGTH_ERR          BIT(19) //fatal No retry
#define DESCM_SRC_SGL_LENGTH_ALIGNMENT_ERR    BIT(20) //fatal No retry
#define DESCM_SRC_PRP_OFST_ERR                BIT(21) //fatal No retry
#define DESCM_SRC_PRP_ALIGN_ERR               BIT(22) //fatal No retry

#define DESCM_DEST_AXI_RD_ERR                 BIT(23) //non fatal

#define DESCM_DEST_DESCR_STRUCTURE_ERR        BIT(24) //fatal No retry
#define DESCM_DEST_SGL_CROSS_4K_ERR           BIT(25) //fatal No retry
#define DESCM_DEST_SGL_ILLEGAL_DSCRPTR_ERR    BIT(26) //fatal No retry
#define DESCM_DEST_SGL_UNDEFINED_DSCRPTR_TYPE BIT(27) //fatal No retry
#define DESCM_DEST_SGL_TBL_LENGTH_ERR         BIT(28) //fatal No retry
#define DESCM_DEST_SGL_LENGTH_ALIGNMENT_ERR   BIT(29) //fatal No retry
#define DESCM_DEST_PRP_OFST_ERR               BIT(30) //fatal No retry
#define DESCM_DEST_PRP_ALIGN_ERR              BIT(31) //fatal No retry

#define NON_DEFINED_ERROR_MASK_REG_0 (BIT(4) | BIT(5) | BIT(6) | BIT(7) | BIT(8))

#define FATAL_ERROR_MASK_REG_0  (DESCM_SRC_DESCR_STRUCTURE_ERR | DESCM_SRC_SGL_CROSS_4K_ERR | DESCM_SRC_SGL_ILLEGAL_DSCRPTR_ERR | \
    DESCM_SRC_SGL_UNDEFINED_DSCRPTR_TYPE | DESCM_SRC_SGL_TBL_LENGTH_ERR | DESCM_SRC_SGL_LENGTH_ALIGNMENT_ERR | DESCM_SRC_PRP_OFST_ERR | \
    DESCM_SRC_PRP_ALIGN_ERR | DESCM_DEST_DESCR_STRUCTURE_ERR | DESCM_DEST_SGL_CROSS_4K_ERR | DESCM_DEST_SGL_ILLEGAL_DSCRPTR_ERR | \
    DESCM_DEST_SGL_UNDEFINED_DSCRPTR_TYPE | DESCM_DEST_SGL_TBL_LENGTH_ERR | DESCM_DEST_SGL_LENGTH_ALIGNMENT_ERR | DESCM_DEST_PRP_OFST_ERR |\
    DESCM_DEST_PRP_ALIGN_ERR)

#define NON_FATAL_RETRY_MASK_REG_0 (CMDE_AXI_RD_ERR | CMDE_UNEXPECTED_CMD_PHASE_ERR | DESCM_SRC_AXI_RD_ERR | DESCM_DEST_AXI_RD_ERR)

//Command Slot Error Status 1

#define DFE_MAX_ELMNT_COUNT_ERR           BIT(0) //non fatal
#define DFE_UNDERRUN_ERR                  BIT(1) //fatal
#define DFE_OVERRUN_ERR                   BIT(2) //fatal

#define DBM_AXI_RD_ERR                    BIT(5) //non fatal
#define DBM_BUFFER_RD_PARITY_ERR          BIT(6) //non fatal

#define CRYPTOE_KEY_VAULT_MEM_RD_ERR      BIT(9) //fatal
#define CRYPTOE_REDUNDANCY_MISMATCH_ERR   BIT(10) //non fatal No retry
#define CRYPTOE_TEXT_OUT_READ_ERR         BIT(11) //non fatal
#define CRYPTOE_FATAL_ERR                 BIT(12) //fatal
#define CRYPTOE_TAG_MISMATCH_ERR          BIT(13) //non fatal No retry

#define DOE_AXI_WR_ERR                    BIT(16) //non fatal
#define DOE_BUFFER_RD_PARITY_ERR          BIT(17) //non fatal
#define DOE_UNDERRUN_ERR                  BIT(18) //fatal
#define DOE_OVERRUN_ERR                   BIT(19) //fatal
#define DOE_MAX_ELMNT_COUNT_ERR           BIT(20) //non fatal

#define CMD_TRANSFER_LENGTH_OVERRUN_ERR   BIT(28) //non fatal No retry
#define CMD_TRANSFER_LENGTH_UNDERRUN_ERR  BIT(29) //non fatal No retry
#define CMPLE_AXI_WR_ERR                  BIT(30) //non fatal

#define CDMA_CMD_SLOT_ERR_STS_REG_POORLY_SGL_ERR_MASK (DOE_MAX_ELMNT_COUNT_ERR | DFE_MAX_ELMNT_COUNT_ERR)

#define NON_DEFINED_ERROR_MASK_REG_1   (BIT(3) | BIT(4) | BIT(7) | BIT(8) | BIT(14) | BIT(15) | BIT(21) | \
    BIT(22) | BIT(23) | BIT(24) | BIT(25) | BIT(26) | BIT(27) | BIT(31))

#define FATAL_ERROR_MASK_REG_1 (DFE_UNDERRUN_ERR | DFE_OVERRUN_ERR | CRYPTOE_KEY_VAULT_MEM_RD_ERR | \
    CRYPTOE_FATAL_ERR | DOE_UNDERRUN_ERR | DOE_OVERRUN_ERR)

#define FATAL_ERROR_RETRY_MASK_REG_1 (CRYPTOE_KEY_VAULT_MEM_RD_ERR)

#define NON_FATAL_RETRY_MASK_REG_1 (DBM_AXI_RD_ERR | DBM_BUFFER_RD_PARITY_ERR | CRYPTOE_TEXT_OUT_READ_ERR | \
    DOE_AXI_WR_ERR | DOE_BUFFER_RD_PARITY_ERR | CMPLE_AXI_WR_ERR)


 #define GET_SGL_RETRYTIMES_BY_CHUNKSIZE(ChunkSize) \
    (((ChunkSize) > 512) ? 0 : (((ChunkSize) > 128) ? 1 : 2))

LionFPCQEStatusCode FpsCpu2FillHostStatusCode(uint8_t ceStatus);
LionFPCQEErrorCode FpsCpu2FillHostGcmErrorCode(AesGcmExtRespErr respSts);

//-----------------------------------------------------------------------------
//  Inline Member Function Definitions
//-----------------------------------------------------------------------------