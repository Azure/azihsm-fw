// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

//=============================================================================
//
//! @file
//! @brief API for logging producer module
//!
//=============================================================================

#include "../system/Debug/logging/logging_producer.h"
#include "../system/Debug/logging/manticore_logging_record.h"
#include "../system/Boot/M7Partition.h"
#include "M7MemMap.h"

void API_LoggingProducerOneTimeInit()
{
    struct logging_producer * log_handle;
    // set core id
    #if defined (CPU0)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU0_LOG_HANDLE)));
    logging_producer_init(log_handle, (void *)getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOG_DUMP_START + ((PSRAM_LOG_BUFFER_SIZE * PSRAM_LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core0))), PSRAM_CERBERUS_LOGGING_BUFFER_ARRAY_SIZE, MSFT_LOGGING_COMPONENT_MANTICORE_FP0);
    #elif defined (CPU1)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU1_LOG_HANDLE)));
    logging_producer_init(log_handle, (void *)getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOG_DUMP_START + ((PSRAM_LOG_BUFFER_SIZE * PSRAM_LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core1))), PSRAM_CERBERUS_LOGGING_BUFFER_ARRAY_SIZE, MSFT_LOGGING_COMPONENT_MANTICORE_FP1);
    #elif defined (CPU2)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU2_LOG_HANDLE)));
    logging_producer_init(log_handle, (void *)getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOG_DUMP_START + ((PSRAM_LOG_BUFFER_SIZE * PSRAM_LOG_BUFFER_NUMBER_PER_FP_CPU) * cM7Core2))), PSRAM_CERBERUS_LOGGING_BUFFER_ARRAY_SIZE, MSFT_LOGGING_COMPONENT_MANTICORE_FP2);
    #endif
}

void API_AddDebugLog(uint8_t tokenIdx, uint8_t severity, uint32_t arg1, uint32_t arg2)
{
    struct logging_producer * log_handle;
    #if defined (CPU0)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU0_LOG_HANDLE)));
    #elif defined (CPU1)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU1_LOG_HANDLE)));
    #elif defined (CPU2)
    log_handle = (struct logging_producer *)(getpSRAMPhysicalAddress((uint32_t)(PSRAM_LOGGING_CPU2_LOG_HANDLE)));
    #endif
    logging_producer_send(log_handle, severity, tokenIdx, arg1, arg2);
}
