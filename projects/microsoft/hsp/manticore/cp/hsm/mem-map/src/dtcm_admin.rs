// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_mem_map_derive::mem_map;

/// DTCM memory map for CP Admin and HSM
///
/// Note: Keep this file in parity with cp1_dtcm::Cp1DtcmMemMap,
/// Any modifications made to this file should be reflected to cp1_dtcm that is shared
/// between CP Admin and CP HSM.
#[mem_map(address = 0x2003_FB00, length = 0x500)]
pub struct AdminDtcmMemMap {
    /// Core run status
    #[field(mutable = true, volatile = true)]
    core_run_status: u32,

    /// Crashdump address
    #[field(cardinality = 0x400, mutable = true)]
    crashdump_base: u8,

    /// Reserved
    #[field(cardinality = 0xFC, mutable = true)]
    _rsvd: u8,
}

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_base_address() {
        assert_eq!(AdminDtcmMemMap::BASE_ADDRESS, 0x2003_FB00);
        assert_eq!(AdminDtcmMemMap::LENGTH, 0x500);
    }

    #[test]
    fn test_total_length() {
        assert_eq!(
            AdminDtcmMemMap::LENGTH,
            AdminDtcmMemMap::CRASHDUMP_BASE_SIZE
                + AdminDtcmMemMap::CORE_RUN_STATUS_SIZE
                + AdminDtcmMemMap::_RSVD_SIZE
        );
    }

    #[test]
    fn test_core_run_status() {
        assert_eq!(AdminDtcmMemMap::CORE_RUN_STATUS_SIZE, 4);
        assert_eq!(AdminDtcmMemMap::CORE_RUN_STATUS_OFFSET, 0x2003_FB00);
        assert_eq!(AdminDtcmMemMap::CORE_RUN_STATUS_OFFSET % 4, 0);
        sa::const_assert!(
            AdminDtcmMemMap::CORE_RUN_STATUS_OFFSET + AdminDtcmMemMap::CORE_RUN_STATUS_SIZE
                <= AdminDtcmMemMap::BASE_ADDRESS + AdminDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_crashdump_base() {
        assert_eq!(AdminDtcmMemMap::CRASHDUMP_BASE_SIZE, 0x0400);
        assert_eq!(AdminDtcmMemMap::CRASHDUMP_BASE_OFFSET, 0x2003_FB04);
        assert_eq!(AdminDtcmMemMap::CRASHDUMP_BASE_OFFSET % 4, 0);
        sa::const_assert!(
            AdminDtcmMemMap::CRASHDUMP_BASE_OFFSET + AdminDtcmMemMap::CRASHDUMP_BASE_SIZE
                <= AdminDtcmMemMap::BASE_ADDRESS + AdminDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_rsvd() {
        assert_eq!(AdminDtcmMemMap::_RSVD_SIZE, 0xFC);
        assert_eq!(AdminDtcmMemMap::_RSVD_OFFSET, 0x2003_FF04);

        assert_eq!(AdminDtcmMemMap::_RSVD_OFFSET % 4, 0);
        sa::const_assert!(
            AdminDtcmMemMap::_RSVD_OFFSET + AdminDtcmMemMap::_RSVD_SIZE
                <= AdminDtcmMemMap::BASE_ADDRESS + AdminDtcmMemMap::LENGTH
        );
    }
}
