// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_mem_map_derive::mem_map;
use mcr_self_test::NegKind;
use mcr_self_test::SelfTest;
use mcr_self_test::{SelfTestReqPacket, SelfTestRespPacket};
use mcr_types::GetBulkKeyReqEntry;
use mcr_types::GetBulkKeyRespEntry;

/// DTCM memory map for CP Admin and HSM
///
/// Note: Keep this file in parity with cp1_dtcm::Cp1DtcmMemMap,
/// Any modifications made to this file should be reflected to cp1_dtcm that is shared
/// between CP Admin and CP HSM.
#[mem_map(address = 0x2003_0000, length = 0x10000)]
pub struct HsmDtcmMemMap {
    /// Performance log base
    #[field(cardinality = 0xF400, mutable = true)]
    perf_log_base: u8,

    /// Crashdump address
    #[field(cardinality = 0x400, mutable = true)]
    crashdump_base: u8,

    /// Core run status
    #[field(mutable = true, volatile = true)]
    core_run_status: u32,

    /// SoftAes response consumer index
    #[field(mutable = true, volatile = true)]
    soft_aes_resp_ci: u32,

    /// SoftAes response producer index
    #[field(mutable = true, volatile = true)]
    soft_aes_resp_pi: u32,

    /// Self test request consumer index
    #[field(mutable = true, volatile = true)]
    self_test_req_ci: u32,

    /// Self test request producer index
    #[field(mutable = true, volatile = true)]
    self_test_req_pi: u32,

    /// Self test request queue
    #[field(cardinality = 2, mutable = true)]
    self_test_req_queue: SelfTestReqPacket,

    /// Self test response consumer index
    #[field(mutable = true, volatile = true)]
    self_test_resp_ci: u32,

    /// Self test response producer index
    #[field(mutable = true, volatile = true)]
    self_test_resp_pi: u32,

    /// Self test request response queue
    #[field(cardinality = 2, mutable = true)]
    self_test_resp_queue: SelfTestRespPacket,

    /// Negative Self Test Identifier
    #[field(cardinality = 0x1, mutable = true)]
    negative_self_test_id: Option<SelfTest>,

    /// FIPS approval status
    #[field(cardinality = 65, mutable = true)]
    fips_approved: bool,

    /// negative health-test kind (0=None, 1=RngRct, 2=RngApt)
    #[field(cardinality = 0x1, mutable = true)]
    negative_kind: NegKind,

    /// Corrected ECC Error Interrupt count
    #[field(cardinality = 1, mutable = true)]
    corr_ecc_err_intr_count: u32,

    /// Get Bulk Key request consumer index
    #[field(mutable = true, volatile = true, offset = 0xF884)]
    get_bulk_key_req_ci: u32,

    /// Get Bulk Key request producer index
    #[field(mutable = true, volatile = true)]
    get_bulk_key_req_pi: u32,

    /// Get Bulk Key request queue
    #[field(cardinality = 2, mutable = true)]
    get_bulk_key_req_queue: GetBulkKeyReqEntry,

    /// Get Bulk Key response consumer index
    #[field(mutable = true, volatile = true)]
    get_bulk_key_resp_ci: u32,

    /// Get Bulk Key response producer index
    #[field(mutable = true, volatile = true)]
    get_bulk_key_resp_pi: u32,

    /// Get Bulk Key response queue
    #[field(cardinality = 2, mutable = true)]
    get_bulk_key_resp_queue: GetBulkKeyRespEntry,

    // Reserved
    #[field(cardinality = 0x71C)]
    _rsvd: u8,
}

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_base_address() {
        assert_eq!(HsmDtcmMemMap::BASE_ADDRESS, 0x2003_0000);
        assert_eq!(HsmDtcmMemMap::LENGTH, 0x10000);
    }

    #[test]
    fn test_total_length() {
        assert_eq!(
            HsmDtcmMemMap::LENGTH,
            HsmDtcmMemMap::CRASHDUMP_BASE_SIZE
                + HsmDtcmMemMap::_RSVD_SIZE
                + HsmDtcmMemMap::PERF_LOG_BASE_SIZE
                + HsmDtcmMemMap::SOFT_AES_RESP_CI_SIZE
                + HsmDtcmMemMap::SOFT_AES_RESP_PI_SIZE
                + HsmDtcmMemMap::CORE_RUN_STATUS_SIZE
                + HsmDtcmMemMap::SELF_TEST_REQ_CI_SIZE
                + HsmDtcmMemMap::SELF_TEST_REQ_PI_SIZE
                + HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_SIZE
                + HsmDtcmMemMap::SELF_TEST_RESP_CI_SIZE
                + HsmDtcmMemMap::SELF_TEST_RESP_PI_SIZE
                + HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_SIZE
                + HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_SIZE
                + HsmDtcmMemMap::FIPS_APPROVED_SIZE
                + 3 // Added 3 padding as FIPS APPROVED is not aligned to 4 bytes
                + HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_SIZE
                + HsmDtcmMemMap::NEGATIVE_KIND_SIZE
                + 3 // Added 3 padding as NEGATIVE KIND is not aligned to 4 bytes
                + HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_SIZE
                + HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_SIZE
                + HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_SIZE
                + HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_SIZE
                + HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_SIZE
                + HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_SIZE
        );
    }

    #[test]
    fn test_perf_log_base() {
        assert_eq!(HsmDtcmMemMap::PERF_LOG_BASE_SIZE, 0xF400);
        assert_eq!(HsmDtcmMemMap::PERF_LOG_BASE_OFFSET, 0x2003_0000);
        assert_eq!(HsmDtcmMemMap::PERF_LOG_BASE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::PERF_LOG_BASE_OFFSET + HsmDtcmMemMap::PERF_LOG_BASE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_core_run_status() {
        assert_eq!(HsmDtcmMemMap::CORE_RUN_STATUS_SIZE, 4);
        assert_eq!(HsmDtcmMemMap::CORE_RUN_STATUS_OFFSET, 0x2003_F800);
        assert_eq!(HsmDtcmMemMap::CORE_RUN_STATUS_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::CORE_RUN_STATUS_OFFSET + HsmDtcmMemMap::CORE_RUN_STATUS_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_crashdump_base() {
        assert_eq!(HsmDtcmMemMap::CRASHDUMP_BASE_SIZE, 0x0400);
        assert_eq!(HsmDtcmMemMap::CRASHDUMP_BASE_OFFSET, 0x2003_F400);
        assert_eq!(HsmDtcmMemMap::CRASHDUMP_BASE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::CRASHDUMP_BASE_OFFSET + HsmDtcmMemMap::CRASHDUMP_BASE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_soft_aes_resp_ci() {
        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_CI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_CI_OFFSET, 0x2003_F804);

        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SOFT_AES_RESP_CI_OFFSET + HsmDtcmMemMap::SOFT_AES_RESP_CI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_soft_aes_resp_pi() {
        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_PI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_PI_OFFSET, 0x2003_F808);

        assert_eq!(HsmDtcmMemMap::SOFT_AES_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SOFT_AES_RESP_PI_OFFSET + HsmDtcmMemMap::SOFT_AES_RESP_PI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_request_ci() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_CI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_CI_OFFSET, 0x2003_F80C);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_REQ_CI_OFFSET + HsmDtcmMemMap::SELF_TEST_REQ_CI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_request_pi() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_PI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_PI_OFFSET, 0x2003_F810);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_REQ_PI_OFFSET + HsmDtcmMemMap::SELF_TEST_REQ_PI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_req_queue() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_SIZE, 0x8);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_OFFSET, 0x2003_F814);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_OFFSET + HsmDtcmMemMap::SELF_TEST_REQ_QUEUE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_ci() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_CI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_CI_OFFSET, 0x2003_F81C);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_RESP_CI_OFFSET + HsmDtcmMemMap::SELF_TEST_RESP_CI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_pi() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_PI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_PI_OFFSET, 0x2003_F820);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_RESP_PI_OFFSET + HsmDtcmMemMap::SELF_TEST_RESP_PI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_queue() {
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_SIZE, 0x10);
        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_OFFSET, 0x2003_F824);

        assert_eq!(HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_OFFSET + HsmDtcmMemMap::SELF_TEST_RESP_QUEUE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_negative_self_test_id() {
        assert_eq!(HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_SIZE, 0x04);
        assert_eq!(HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_OFFSET, 0x2003_F834);

        assert_eq!(HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_OFFSET + HsmDtcmMemMap::NEGATIVE_SELF_TEST_ID_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_fips_approved() {
        assert_eq!(HsmDtcmMemMap::FIPS_APPROVED_SIZE, 0x41);
        assert_eq!(HsmDtcmMemMap::FIPS_APPROVED_OFFSET, 0x2003_F838);

        assert_eq!(HsmDtcmMemMap::FIPS_APPROVED_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::FIPS_APPROVED_OFFSET + HsmDtcmMemMap::FIPS_APPROVED_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_negative_kind() {
        assert_eq!(HsmDtcmMemMap::NEGATIVE_KIND_SIZE, 0x01);
        assert_eq!(HsmDtcmMemMap::NEGATIVE_KIND_OFFSET, 0x2003_F87C);

        assert_eq!(HsmDtcmMemMap::NEGATIVE_KIND_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::NEGATIVE_KIND_OFFSET + HsmDtcmMemMap::NEGATIVE_KIND_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_corr_ecc_err_intr_count() {
        assert_eq!(HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_OFFSET, 0x2003_F880);

        assert_eq!(HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_OFFSET
                + HsmDtcmMemMap::CORR_ECC_ERR_INTR_COUNT_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_ci() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_OFFSET, 0x2003_F884);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_OFFSET + HsmDtcmMemMap::GET_BULK_KEY_REQ_CI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_pi() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_OFFSET, 0x2003_F888);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_OFFSET + HsmDtcmMemMap::GET_BULK_KEY_REQ_PI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_queue() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_SIZE, 0x8);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET, 0x2003_F88C);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET
                + HsmDtcmMemMap::GET_BULK_KEY_REQ_QUEUE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_ci() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_OFFSET, 0x2003_F894);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_OFFSET + HsmDtcmMemMap::GET_BULK_KEY_RESP_CI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_pi() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_SIZE, 0x4);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_OFFSET, 0x2003_F898);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_OFFSET + HsmDtcmMemMap::GET_BULK_KEY_RESP_PI_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_queue() {
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_SIZE, 0x48);
        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET, 0x2003_F89C);

        assert_eq!(HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET
                + HsmDtcmMemMap::GET_BULK_KEY_RESP_QUEUE_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }

    #[test]
    fn test_rsvd() {
        assert_eq!(HsmDtcmMemMap::_RSVD_SIZE, 0x71C);
        assert_eq!(HsmDtcmMemMap::_RSVD_OFFSET, 0x2003_F8E4);

        assert_eq!(HsmDtcmMemMap::_RSVD_OFFSET % 4, 0);
        sa::const_assert!(
            HsmDtcmMemMap::_RSVD_OFFSET + HsmDtcmMemMap::_RSVD_SIZE
                <= HsmDtcmMemMap::BASE_ADDRESS + HsmDtcmMemMap::LENGTH
        );
    }
}
