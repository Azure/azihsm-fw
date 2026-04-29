// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

#pragma once

#include "assert.h"
#include "CDMA.h"
#include "CDMA.h"
#include "APICdma.h"
#include "MemIo.h"

/**
 *  @brief API for CDMA to set skip bit given cpu command id
 *  @param cpuCid The cpu command id to skip
 *  @param pCdmaSq The pointer to CDMA SQ
 *  @param pCdmaSqPi The pointer to local CDMA SQ PI
 *  @return  true if find cpu cid in cdma SQ, false otherwise
 */
bool API_CDMASkipWithCpuCID(uint16_t cpuCid, const uint32_t* pCdmaSqPi, const CdmaSq_t* pCdmaSq);

/**
 *  @brief API for CDMA to set skip bit given queue id
 *  @param sqid The sqid to skip
 *  @param pCdmaSq The pointer to CDMA SQ
 *  @param pCdmaSqPi The pointer to local CDMA SQ PI
 *  @param cidInSQEBitMap The bit map to record which commands are found
 *  @return  void
 */
void API_CDMASkipWithSQID(uint16_t sqid, const uint32_t* pCdmaSqPi, const  CdmaSq_t* pCdmaSq,   uint32_t* cidInSQEBitMap);

/**
 *  @brief API for CDMA send abort given command id
 *  @param cpuCid The cpu command to skip
 *  @param pCdmaSq The pointer to CDMA SQ
 *  @param pCdmaSqPi The pointer to local CDMA SQ PI
 *  @param ucdQid The ucd qid of abort command
 *  @param pFatalFlag Pointer to CDMA fatal error flag
 *  @return  true if need to update command array ci
 */
bool API_CDMASendAbort(uint16_t cpuCid,  uint32_t* pCdmaSqPi,  uint8_t ucdQid,  uint16_t dflIndex, const CdmaSq_t* pCdmaSq, const uint32_t* pFatalFlag);
