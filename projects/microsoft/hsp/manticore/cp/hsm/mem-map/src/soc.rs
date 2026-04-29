// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_mem_map_derive::mem_map;
use mcr_self_test::NegKind;
use mcr_self_test::SelfTest;
use mcr_self_test::{SelfTestReqPacket, SelfTestRespPacket};
use mcr_types::*;

/// SOC memmap to access CP Admin/HSM and GSRAM
#[mem_map(address = 0x6000_0000, length = 0xA0000000)]
pub struct SocMemMap {
    #[field(mutable = true, offset = 0x0063F804, volatile = true)]
    soft_aes_resp_ci: u32,

    // CP1 dTCM - SoftAes response producer index
    #[field(mutable = true, volatile = true)]
    soft_aes_resp_pi: u32,

    /// CP1 dTCM - Self test request consumer index
    #[field(mutable = true, volatile = true)]
    self_test_req_ci: u32,

    /// CP1 dTCM - Self test request producer index
    #[field(mutable = true, volatile = true)]
    self_test_req_pi: u32,

    /// Self test request queue
    #[field(cardinality = 2, mutable = true)]
    self_test_req_queue: SelfTestReqPacket,

    /// CP1 dTCM - Self test request consumer index
    #[field(mutable = true, volatile = true)]
    self_test_resp_ci: u32,

    /// CP1 dTCM - Self test request producer index
    #[field(mutable = true, volatile = true)]
    self_test_resp_pi: u32,

    /// Self test request response queue
    #[field(cardinality = 2, mutable = true)]
    self_test_resp_queue: SelfTestRespPacket,

    /// Negative Self Test Identifier
    #[field(cardinality = 0x1, mutable = true)]
    negative_self_test_id: Option<SelfTest>,

    /// FIPS approval status for HSM core
    #[field(cardinality = 65, mutable = true)]
    fips_approved: bool,

    /// negative health-test kind (0=None, 1=RngRct, 2=RngApt)
    #[field(cardinality = 0x1, mutable = true)]
    negative_kind: NegKind,

    /// Get Bulk Key request consumer index
    #[field(mutable = true, volatile = true, offset = 0x0063_F884)]
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

    /// AES GCM IV queue buffer size
    #[field(alignment = 0x4, mutable = true, volatile = true, offset = 0x43221A1C)]
    aes_gcm_iv_queue_buffer_size: u32,

    /// AES GCM IV queue head
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_iv_queue_head: u32,

    /// AES GCM IV queue tail
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_iv_queue_tail: u32,

    /// AES GCM IV queue sender overflows
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_iv_queue_sender_overflows: u32,

    /// AES GCM IV queue
    #[field(cardinality = 131, mutable = true)]
    aes_gcm_iv_queue: AesGcmIV,

    /// FIPS approval status for FP2 DTCM
    #[field(cardinality = 65, mutable = true, offset = 0x43421C60)]
    fp_fips_approved: bool,

    /// AES GCM request queue buffer size
    #[field(alignment = 0x4, mutable = true, volatile = true, offset = 0x43421CB0)]
    aes_gcm_req_queue_buffer_size: u32,

    /// AES GCM request queue head
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_req_queue_head: u32,

    /// AES GCM request queue tail
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_req_queue_tail: u32,

    /// AES GCM request queue sender overflows
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_req_queue_sender_overflows: u32,

    /// AES GCM request queue
    #[field(cardinality = 521, mutable = true)]
    aes_gcm_req_queue: AesGcmReqEntry,

    /// AES GCM response queue buffer size
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_resp_queue_buffer_size: u32,

    /// AES GCM response queue head
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_resp_queue_head: u32,

    /// AES GCM response queue tail
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_resp_queue_tail: u32,

    /// AES GCM response queue sender overflows
    #[field(alignment = 0x4, mutable = true, volatile = true)]
    aes_gcm_resp_queue_sender_overflows: u32,

    /// AES GCM response queue
    #[field(cardinality = 521, mutable = true)]
    aes_gcm_resp_queue: AesGcmRespEntry,
}

/// Compile-time assertions for SocMemMap
///
/// These assertions are only valid for 32-bit targets and are moved from the unit tests
/// to ensure they are evaluated during firmware build.
#[cfg(target_pointer_width = "32")]
const _: () = {
    static_assertions::const_assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_SIZE, 0x1048);
    static_assertions::const_assert_eq!(
        SocMemMap::AES_GCM_RESP_QUEUE_BUFFER_SIZE_OFFSET,
        0xA342_2D08
    );
    static_assertions::const_assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_HEAD_OFFSET, 0xA342_2D0C);
    static_assertions::const_assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_TAIL_OFFSET, 0xA342_2D10);
    static_assertions::const_assert_eq!(
        SocMemMap::AES_GCM_RESP_QUEUE_SENDER_OVERFLOWS_OFFSET,
        0xA342_2D14
    );
    static_assertions::const_assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_OFFSET, 0xA342_2D18);
};

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_soft_aes_resp_ci() {
        assert_eq!(SocMemMap::SOFT_AES_RESP_CI_SIZE, 0x4);
        assert_eq!(SocMemMap::SOFT_AES_RESP_CI_OFFSET, 0x6063_F804);

        assert_eq!(SocMemMap::SOFT_AES_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SOFT_AES_RESP_CI_OFFSET + SocMemMap::SOFT_AES_RESP_CI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_soft_aes_resp_pi() {
        assert_eq!(SocMemMap::SOFT_AES_RESP_PI_SIZE, 0x4);
        assert_eq!(SocMemMap::SOFT_AES_RESP_PI_OFFSET, 0x6063_F808);

        assert_eq!(SocMemMap::SOFT_AES_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SOFT_AES_RESP_PI_OFFSET + SocMemMap::SOFT_AES_RESP_PI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_request_ci() {
        assert_eq!(SocMemMap::SELF_TEST_REQ_CI_SIZE, 0x4);
        assert_eq!(SocMemMap::SELF_TEST_REQ_CI_OFFSET, 0x6063_F80C);

        assert_eq!(SocMemMap::SELF_TEST_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_REQ_CI_OFFSET + SocMemMap::SELF_TEST_REQ_CI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_request_pi() {
        assert_eq!(SocMemMap::SELF_TEST_REQ_PI_SIZE, 0x4);
        assert_eq!(SocMemMap::SELF_TEST_REQ_PI_OFFSET, 0x6063_F810);

        assert_eq!(SocMemMap::SELF_TEST_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_REQ_PI_OFFSET + SocMemMap::SELF_TEST_REQ_PI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_req_queue() {
        assert_eq!(SocMemMap::SELF_TEST_REQ_QUEUE_SIZE, 0x8);
        assert_eq!(SocMemMap::SELF_TEST_REQ_QUEUE_OFFSET, 0x6063_F814);

        assert_eq!(SocMemMap::SELF_TEST_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_REQ_QUEUE_OFFSET + SocMemMap::SELF_TEST_REQ_QUEUE_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_ci() {
        assert_eq!(SocMemMap::SELF_TEST_RESP_CI_SIZE, 0x4);
        assert_eq!(SocMemMap::SELF_TEST_RESP_CI_OFFSET, 0x6063_F81C);

        assert_eq!(SocMemMap::SELF_TEST_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_RESP_CI_OFFSET + SocMemMap::SELF_TEST_RESP_CI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_pi() {
        assert_eq!(SocMemMap::SELF_TEST_RESP_PI_SIZE, 0x4);
        assert_eq!(SocMemMap::SELF_TEST_RESP_PI_OFFSET, 0x6063_F820);

        assert_eq!(SocMemMap::SELF_TEST_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_RESP_PI_OFFSET + SocMemMap::SELF_TEST_RESP_PI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_self_test_resp_queue() {
        assert_eq!(SocMemMap::SELF_TEST_RESP_QUEUE_SIZE, 0x10);
        assert_eq!(SocMemMap::SELF_TEST_RESP_QUEUE_OFFSET, 0x6063_F824);

        assert_eq!(SocMemMap::SELF_TEST_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::SELF_TEST_RESP_QUEUE_OFFSET + SocMemMap::SELF_TEST_RESP_QUEUE_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_negative_self_test_id() {
        assert_eq!(SocMemMap::NEGATIVE_SELF_TEST_ID_SIZE, 0x4);
        assert_eq!(SocMemMap::NEGATIVE_SELF_TEST_ID_OFFSET, 0x6063_F834);

        assert_eq!(SocMemMap::NEGATIVE_SELF_TEST_ID_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::NEGATIVE_SELF_TEST_ID_OFFSET + SocMemMap::NEGATIVE_SELF_TEST_ID_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_hs_fips_approved() {
        assert_eq!(SocMemMap::FIPS_APPROVED_SIZE, 0x41);
        assert_eq!(SocMemMap::FIPS_APPROVED_OFFSET, 0x6063_F838);

        assert_eq!(SocMemMap::FIPS_APPROVED_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::FIPS_APPROVED_OFFSET + SocMemMap::FIPS_APPROVED_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_negative_kind() {
        assert_eq!(SocMemMap::NEGATIVE_KIND_SIZE, 0x1);
        assert_eq!(SocMemMap::NEGATIVE_KIND_OFFSET, 0x6063_F87C);

        assert_eq!(SocMemMap::NEGATIVE_KIND_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::NEGATIVE_KIND_OFFSET + SocMemMap::NEGATIVE_KIND_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_ci() {
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_CI_SIZE, 0x4);
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_CI_OFFSET, 0x6063_F884);

        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_CI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_REQ_CI_OFFSET + SocMemMap::GET_BULK_KEY_REQ_CI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_pi() {
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_PI_SIZE, 0x4);
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_PI_OFFSET, 0x6063_F888);

        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_PI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_REQ_PI_OFFSET + SocMemMap::GET_BULK_KEY_REQ_PI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_req_queue() {
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_QUEUE_SIZE, 0x8);
        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET, 0x6063_F88C);

        assert_eq!(SocMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_REQ_QUEUE_OFFSET + SocMemMap::GET_BULK_KEY_REQ_QUEUE_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_ci() {
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_CI_SIZE, 0x4);
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_CI_OFFSET, 0x6063_F894);

        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_CI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_RESP_CI_OFFSET + SocMemMap::GET_BULK_KEY_RESP_CI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_pi() {
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_PI_SIZE, 0x4);
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_PI_OFFSET, 0x6063_F898);

        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_PI_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_RESP_PI_OFFSET + SocMemMap::GET_BULK_KEY_RESP_PI_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_get_bulk_key_resp_queue() {
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_QUEUE_SIZE, 0x48);
        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET, 0x6063_F89C);

        assert_eq!(SocMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::GET_BULK_KEY_RESP_QUEUE_OFFSET + SocMemMap::GET_BULK_KEY_RESP_QUEUE_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_iv_queue_buffer_size() {
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_BUFFER_SIZE_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_BUFFER_SIZE_OFFSET, 0xA322_1A1C);

        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_BUFFER_SIZE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_IV_QUEUE_BUFFER_SIZE_OFFSET
                + SocMemMap::AES_GCM_IV_QUEUE_BUFFER_SIZE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_iv_queue_head() {
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_HEAD_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_HEAD_OFFSET, 0xA322_1A20);

        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_HEAD_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_IV_QUEUE_HEAD_OFFSET + SocMemMap::AES_GCM_IV_QUEUE_HEAD_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_iv_queue_tail() {
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_TAIL_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_TAIL_OFFSET, 0xA322_1A24);
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_TAIL_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_IV_QUEUE_TAIL_OFFSET + SocMemMap::AES_GCM_IV_QUEUE_TAIL_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_iv_queue_sender_overflows() {
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_SENDER_OVERFLOWS_SIZE, 0x4);
        assert_eq!(
            SocMemMap::AES_GCM_IV_QUEUE_SENDER_OVERFLOWS_OFFSET,
            0xA322_1A28
        );
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_SENDER_OVERFLOWS_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_IV_QUEUE_SENDER_OVERFLOWS_OFFSET
                + SocMemMap::AES_GCM_IV_QUEUE_SENDER_OVERFLOWS_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_iv_queue() {
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_SIZE, 0x624);
        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_OFFSET, 0xA322_1A2C);

        assert_eq!(SocMemMap::AES_GCM_IV_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_IV_QUEUE_OFFSET + SocMemMap::AES_GCM_IV_QUEUE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_fp_fips_approved() {
        assert_eq!(SocMemMap::FP_FIPS_APPROVED_SIZE, 0x41);
        assert_eq!(SocMemMap::FP_FIPS_APPROVED_OFFSET, 0xA342_1C60);

        assert_eq!(SocMemMap::FP_FIPS_APPROVED_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::FP_FIPS_APPROVED_OFFSET + SocMemMap::FP_FIPS_APPROVED_SIZE
                < SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_req_queue_buffer_size() {
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_BUFFER_SIZE_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_BUFFER_SIZE_OFFSET, 0xA342_1CB0);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_BUFFER_SIZE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_REQ_QUEUE_BUFFER_SIZE_OFFSET
                + SocMemMap::AES_GCM_REQ_QUEUE_BUFFER_SIZE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_req_queue_head() {
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_HEAD_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_HEAD_OFFSET, 0xA342_1CB4);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_HEAD_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_REQ_QUEUE_HEAD_OFFSET + SocMemMap::AES_GCM_REQ_QUEUE_HEAD_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_req_queue_tail() {
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_TAIL_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_TAIL_OFFSET, 0xA342_1CB8);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_TAIL_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_REQ_QUEUE_TAIL_OFFSET + SocMemMap::AES_GCM_REQ_QUEUE_TAIL_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_req_queue_sender_overflows() {
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_SENDER_OVERFLOWS_SIZE, 0x4);
        assert_eq!(
            SocMemMap::AES_GCM_REQ_QUEUE_SENDER_OVERFLOWS_OFFSET,
            0xA342_1CBC
        );
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_SENDER_OVERFLOWS_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_REQ_QUEUE_SENDER_OVERFLOWS_OFFSET
                + SocMemMap::AES_GCM_REQ_QUEUE_SENDER_OVERFLOWS_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_req_queue() {
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_OFFSET, 0xA342_1CC0);
        assert_eq!(SocMemMap::AES_GCM_REQ_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_REQ_QUEUE_OFFSET + SocMemMap::AES_GCM_REQ_QUEUE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_resp_queue_buffer_size() {
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_BUFFER_SIZE_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_BUFFER_SIZE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_RESP_QUEUE_BUFFER_SIZE_OFFSET
                + SocMemMap::AES_GCM_RESP_QUEUE_BUFFER_SIZE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_resp_queue_head() {
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_HEAD_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_HEAD_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_RESP_QUEUE_HEAD_OFFSET + SocMemMap::AES_GCM_RESP_QUEUE_HEAD_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_resp_queue_tail() {
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_TAIL_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_TAIL_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_RESP_QUEUE_TAIL_OFFSET + SocMemMap::AES_GCM_RESP_QUEUE_TAIL_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_resp_queue_sender_overflows() {
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_SENDER_OVERFLOWS_SIZE, 0x4);
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_SENDER_OVERFLOWS_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_RESP_QUEUE_SENDER_OVERFLOWS_OFFSET
                + SocMemMap::AES_GCM_RESP_QUEUE_SENDER_OVERFLOWS_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }

    #[test]
    fn test_aes_gcm_resp_queue() {
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_SIZE, 0x824);
        assert_eq!(SocMemMap::AES_GCM_RESP_QUEUE_OFFSET % 4, 0);
        sa::const_assert!(
            SocMemMap::AES_GCM_RESP_QUEUE_OFFSET + SocMemMap::AES_GCM_RESP_QUEUE_SIZE
                <= SocMemMap::BASE_ADDRESS + SocMemMap::LENGTH
        );
    }
}
