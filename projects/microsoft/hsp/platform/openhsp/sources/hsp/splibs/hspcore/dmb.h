/*++

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    dmb.h

Abstract:

    This file contains DMB related functions to dynamic allocation of DMB
    segments. This API abstracts the DMB segment information into
    uint32_t - no knowledge about region 18 or any particular hardware
    implementation needed.

    Because it's allocated dynamically, there's a chance of running out of
    segments, and HSP_DMB_HANDLE_INVALID is returned on HspDmbMap function.
    Validation upon mapping is required, as HspDmbMap will return 0 as the
    address if running out of DMB Segments.

    It's important to pay attention to maintain the same number of HspDmbMap
    and HspDmbUnmap calls to avoid DMB segment leak - there are a limited
    number of segments and it may run out of segments quickly depending on
    how scattered the acquisitions are made.


Author:

    Gustavo Scotti (gscotti) 5-Apr-2021

--*/

#pragma once


//
// DMB Segment Window Sizes enumeration
//
typedef enum _HSP_DMB_SEGMENT_WINDOW_SIZE
{
    HspDmbSegmentWindowSize128KiB,
    HspDmbSegmentWindowSize256KiB,
    HspDmbSegmentWindowSize512KiB,
    HspDmbSegmentWindowSize1MiB,
    HspDmbSegmentWindowSize2MiB,
    HspDmbSegmentWindowSize4MiB,
    HspDmbSegmentWindowSize8MiB,
    HspDmbSegmentWindowSize16MiB,
    HspDmbSegmentWindowSize32MiB,
    HspDmbSegmentWindowSize64MiB,
    HspDmbSegmentWindowSize128MiB
} HSP_DMB_SEGMENT_WINDOW_SIZE, *PHSP_DMB_SEGMENT_WINDOW_SIZE;


HSP_API
HSP_STATUS
HspDmbMap(uint64_t ExternalAddress,
          HSP_DMB_SEGMENT_WINDOW_SIZE WindowSize,
          puint32_t MappedAddress);


HSP_API
HSP_STATUS
HspDmbUnmap(uint32_t MappedAddress);


HSP_API
HSP_STATUS
HspDmbSetPermission(uint32_t MappedAddress, bool ReadWrite, bool User, bool Crypto);


HSP_API
HSP_STATUS
HspDmbResetPermission(uint32_t MappedAddress);
