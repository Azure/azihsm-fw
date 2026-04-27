// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

#include "APICdmaErrorHandle.h"

/**
 *  @brief API for CDMA to set skip bit given cpu command id
 *  @param cpuCid The cpu command id to skip
 *  @param pCdmaSq The pointer to CDMA SQ
 *  @param pCdmaSqPi The pointer to local CDMA SQ PI
 *  @return  true if find cpu cid in cdma SQ, false otherwise
 */
bool API_CDMASkipWithCpuCID(uint16_t cpuCid, const uint32_t* pCdmaSqPi, const CdmaSq_t* pCdmaSq)
{
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    CdmaSqCmdDescr_t* pCdmaSqe;
    uint32_t tmpCi = readl(pCdmaSq->pHwCi);
    bool foundCe = false;
    while (!QUEUE_EMPTY(*pCdmaSqPi,  tmpCi))
    {
        pCdmaSqe = pCdmaSqBase + tmpCi;
        if (pCdmaSqe->Dw0.CmdId == cpuCid)
        {
            foundCe = true;
            pCdmaSqe->Dw0.CmdOpcode = CDMA_OPCODE_SKIP;
        }
        tmpCi = QUEUE_INC(tmpCi, FPS_CDMA_QUEUE_DEPTH_MASK);
    }

    return foundCe;
}

/**
 *  @brief API for CDMA to set skip bit given queue id
 *  @param sqid The sqid to skip
 *  @param pCdmaSq The pointer to CDMA SQ
 *  @param pCdmaSqPi The pointer to local CDMA SQ PI
 *  @param cidInSQEBitMap The bit map to record which commands are found
 *  @void
 */
void API_CDMASkipWithSQID(uint16_t sqid, const uint32_t* pCdmaSqPi, const CdmaSq_t* pCdmaSq,   uint32_t* cidInSQEBitMap)
{
    /*
       If last chunk in SQE:
        Unset last command bit
        And set skip operation
        And send abort

     */
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    CdmaSqCmdDescr_t* pCdmaSqe;
    uint32_t tmpCi = readl(pCdmaSq->pHwCi);
    //bool foundCe = false;
    while (!QUEUE_EMPTY(*pCdmaSqPi,  tmpCi))
    {
        pCdmaSqe = pCdmaSqBase + tmpCi;
        if (pCdmaSqe->Dw1.UcdIqId == sqid)
        {
            if (pCdmaSqe->Dw0.CmdState & LAST_CMD)
            {
                pCdmaSqe->Dw0.CmdState &= ~(LAST_CMD);
            }
            *cidInSQEBitMap |= BIT((pCdmaSqe->Dw0.CmdId - (sqid << CA_SIZE_SHIFT)));
            pCdmaSqe->Dw0.CmdOpcode = CDMA_OPCODE_SKIP;
        }
        tmpCi = QUEUE_INC(tmpCi, FPS_CDMA_QUEUE_DEPTH_MASK);
    }
}


bool API_CDMASendAbort(uint16_t cpuCid,  uint32_t* pCdmaSqPi,  uint8_t ucdQid,  uint16_t dflIndex, const CdmaSq_t* pCdmaSq, const uint32_t* pFatalFlag)
{
    while (readl(pCdmaSq->pHwStatus) & FP2HWE_WQ_04_CDMA_SQ_FULL_BIT)
    {
        // busy waiting
        if (readl(pFatalFlag))
        {
            return false;
        }// else do nothing
    }
    uint8_t cmdState = CONTINUE_CMD | LAST_CMD;
    CdmaSqCmdDescr_t* pCdmaSqBase = pCdmaSq->pCdmaSqBase;
    CdmaSqCmdDescr_t* pCdmaSqe;
    pCdmaSqe = pCdmaSqBase + *pCdmaSqPi;
    // pCdmaSqe->dw0 = (uint32_t)cpuCid | ((uint32_t)cmdState << CDMA_SQE_DW0_CMD_STATE_SHIFT) | ((uint32_t)CDMA_OPCODE_ABORT << CDMA_SQE_DW0_OPCODE_SHIFT) | ((uint32_t)dflIndex << CDMA_SQE_DW0_DFL_BUF_SHIFT);
    // pCdmaSqe->dw1 = ((uint32_t)ucdQid << CDMA_SQE_DW1_IBQ_SHIFT)  | (0x1UL << CDMA_SQE_DW1_CONTINUE_SHIFT); //TBD: value for dw1
    pCdmaSqe->dw0 = (uint32_t)cpuCid | ((uint32_t)cmdState << CDMA_SQE_DW0_CMD_STATE_SHIFT) | ((uint32_t)CDMA_OPCODE_ABORT << CDMA_SQE_DW0_OPCODE_SHIFT);
    pCdmaSqe->dw1 = 0;
    pCdmaSqe->dw2 = 0;
    pCdmaSqe->dw3 = 0;
    *pCdmaSqPi =  QUEUE_INC(*pCdmaSqPi, FPS_CDMA_QUEUE_DEPTH_MASK);
    // API_CDMAEnableSQ();
    writel(*pCdmaSqPi, pCdmaSq->pHwPi);
    #ifdef DISABLE_CDMA_SQ_PI_INDIRECT_REG_WRITE
    writel(*pCdmaSqPi, pCdmaSq->PiHwAddr);
    #endif
    if (cpuCid != CP_CDMA_IO_CMD_ID && cpuCid != CDMA_IDLE_CMD_CPU_ID)
    {
        return true;
    }
    else
    {
        return false;
    }

}
