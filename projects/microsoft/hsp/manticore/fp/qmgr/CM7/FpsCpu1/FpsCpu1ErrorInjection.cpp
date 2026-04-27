// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

//=============================================================================
//!
//! @file   FpsCpu1ErrorInjection.cpp
//! @brief  FpsCpu1 Error Injection
//!
//=============================================================================

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include "FpsCpu1ErrorInjection.h"
#ifdef SUPPORT_ERROR_INJECTION
void Cpu1InsertErr(uint8_t errorType, volatile CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd)
{
    // uint8_t errorType = pErrorInjection->errorType;
    //insert invalid opcode to produce err
    switch (errorType)
    {
        case cDescmDestAxiRdErr:
        case cDescmSrcAxiRdErr:
        case cCmdeUnexpectedCmdPhaseErr:
        case cCmdeInvalidOpcodeErr:
        case cCmdeAxiRdErr:
        case cCmpleAxiWrErr:
        case cCmdTransferLengthUnderrunErr:
        case cCmdTransferLengthOverrunErr:
        case cDoeBufferRdParityErr:
        case cCryptoeTextOutReadErr:
        case cCryptoeRedundancyMismatchErr:
        case cDbmBufferRdParityErr:
        case cDbmAxiRdErr:
        #ifdef LIONMS_B0
        case cCommandSlotErrSts1:
        #endif
        {
            pCdmaSqe->dw0 |= (INVALID_CDMA_OPCODE << CDMA_SQE_DW0_OPCODE_SHIFT);
        }
        break;
        case cDoeOverrunErr:
        case cDoeUnderrunErr:
        case cDoeAxiWrErr:
        case cCryptoeFatalErr:
        case cCryptoeKeyVaultMemRdErr:
        case cDfeOverrunErr:
        case cDfeUnderrrunErr:
        {
            pHostCmd->SrcDataPtr[0].Lo = pHostCmd->SrcDataPtr[0].Lo + 1;
        }
        break;
        default:
            break;
    }
}
void Cpu1HandleErrorInject(AesXtsCmd_t* pAesXtsCmd, volatile CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd)
{
    if (pAesXtsCmd->Signature == MSFT_SIGNATURE)
    {
        if (pAesXtsCmd->ErrorType0 != cNoErr)
        {
            Cpu1InsertErr(pAesXtsCmd->ErrorType0, pCdmaSqe, pHostCmd);
        }
        else if (pAesXtsCmd->ErrorType1 != cNoErr)
        {
            Cpu1InsertErr(pAesXtsCmd->ErrorType1, pCdmaSqe, pHostCmd);
        }
        else
        {
            // else do nothing
        }

    } // else do nothing

}
#elif defined (SUPPORT_MSGERROR_INJECTION)


uint8_t Cpu1ScanErrInjectTable(uint64_t errInjectBitmap, CP2FPMsgDataMsgErrorInjection_t* pMsgErrorInjection, \
                               uint16_t cmdId, uint8_t vfId, uint8_t ibPhyQId, uint16_t dflIdx)
{
    uint8_t errInjectIdx = INVALID_REFERENCE_WITH_BYTE;
    uint8_t errInjectCnt = 0;
    uint16_t errCmdId;
    uint8_t errVfId, errPhyQId;
    for (errInjectCnt = FindNextBit64(errInjectBitmap); (errInjectBitmap != 0); \
         errInjectBitmap &= ~(BIT_ULL(errInjectCnt)), errInjectCnt = FindNextBit64(errInjectBitmap))
    {
        CP2FPMsgDataMsgErrorInjection_t* pErrorInjection = &pMsgErrorInjection[errInjectCnt];
        if (pErrorInjection->injectSts == Cpu1WaitInject)
        {
            errCmdId = pErrorInjection->cmdId;
            errVfId = pErrorInjection->vfId;
            errPhyQId = pErrorInjection->ibPhyQId;
            if (pErrorInjection->dflIdx == dflIdx) //not first inject case
            {
                pErrorInjection->injectSts = Cpu1Inject;
                errInjectIdx = errInjectCnt;
                break;
            } // else do nothing

            if ((COMPARED_ID_WITH_2BYTE(errCmdId, cmdId)) && (COMPARED_ID_WITH_BYTE(errVfId, vfId)) && (COMPARED_ID_WITH_BYTE(errPhyQId, ibPhyQId)))
            {
                // pErrorInjection->cmdId = cmdId;
                // pErrorInjection->vfId = vfId;
                // pErrorInjection->ibPhyQId = ibPhyQId;
                pErrorInjection->dflIdx = dflIdx;
                pErrorInjection->injectSts = Cpu1Inject;
                errInjectIdx = errInjectCnt;

                break;
            } // else do nothing

        } // else do nothing
    }
    return errInjectIdx;
}
void Cpu1InsertErr(CP2FPMsgDataMsgErrorInjection_t* pErrorInjection, CdmaSqCmdDescr_t* pCdmaSqe, LionNvmeSQDescriptor_t* pHostCmd)
{
    uint8_t errorType = pErrorInjection->errorType;
    //insert invalid opcode to produce err
    switch (errorType)
    {
        case cMsgErrInjectCdmaNonFatalErr:
        {
            pCdmaSqe->dw0 |= (INVALID_CDMA_OPCODE << CDMA_SQE_DW0_OPCODE_SHIFT);
        }
        break;
        case cMsgErrInjectCdmaFatalErr:
        {
            pHostCmd->SrcDataPtr[0].Lo = pHostCmd->SrcDataPtr[0].Lo + 1;
        }
        break;
        default:
            break;
    }
}
#endif
