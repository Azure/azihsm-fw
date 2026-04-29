// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Marvell

#ifndef GCM_ENTRY_H_
#define GCM_ENTRY_H_

#include <stdint.h>

/**
 * Data object definition for gcm entries.
 */
typedef struct GcmRequestEntry_t
{
    union
    {
        uint32_t Dw0;
        struct
        {
            uint32_t ceIndex :10;             ///< CE Index
            uint32_t rsvd1: 6;                ///< Reserved1
            uint32_t tagInvalid: 1;
            uint32_t rsvd2 : 7;               ///< Reserved2
            uint32_t pfn: 8;
        };
    };
    union 
    {
        uint32_t Dw1;
        uint32_t sqeAddr;
    };
    
}GcmRequestEntry_t;

typedef struct GcmResponseEntry_t
{
    union
    {
        uint32_t Dw0;
        struct
        {
            uint32_t ceIndex :10;             ///< CE Index
            uint32_t rsvd1  : 6;
            uint32_t status : 8;             ///< Status returned from CP
            uint32_t rsvd2  : 8;
        };
    };
}GcmResponseEntry_t;

typedef struct IVEntry_t
{
    union
    {
        uint32_t IV[3];
        struct
        {
            uint32_t DW0;
            uint32_t DW1;
            uint32_t DW2;
        };
    };
}IVEntry_t;

#endif	/* GCM_ENTRY_H_ */