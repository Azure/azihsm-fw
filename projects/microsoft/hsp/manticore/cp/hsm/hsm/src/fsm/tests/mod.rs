// Copyright (c) Microsoft Corporation. All rights reserved.

mod harness;

mod aes_enc_dec_tests;
mod aes_gen_key_tests;
mod attest_key_tests;
mod change_pin_tests;
mod close_session_tests;
mod delete_key_tests;
#[cfg(feature = "mcr_test_hooks")]
mod der_key_import_tests;
mod ecc_gen_key_tests;
mod ecc_sign_tests;
mod ecdh_key_exchange_tests;
mod establish_credential_tests;
mod flush_session_tests;
mod get_api_rev_tests;
mod get_cert_chain_info_tests;
mod get_certificate_tests;
mod get_device_info_tests;
mod get_establish_cred_encryption_key_tests;
#[cfg(feature = "fips_validation_hooks")]
mod get_priv_key_tests;
#[cfg(feature = "fips_validation_hooks")]
mod get_rng_tests;
mod get_sealed_bk3_tests;
mod get_session_encryption_key_tests;
mod get_unwrapping_key_tests;
mod hkdf_derive_tests;
mod hmac_tests;
mod hsm_fsm_tests;
mod init_bk3_tests;
mod kbkdf_derive_tests;
mod open_key_tests;
mod open_session_tests;
mod page_alloc;
mod part_init_tests;
#[cfg(feature = "fips_validation_hooks")]
mod raw_key_import_tests;
mod reopen_session_tests;
mod res_cleanup_fsm_test;
mod rsa_mod_exp_crt_tests;
mod rsa_mod_exp_no_crt_tests;
#[cfg(feature = "fips_validation_hooks")]
mod rsa_unwrap_kek_tests;
mod rsa_unwrap_tests;
mod set_sealed_bk3_tests;
#[cfg(feature = "fips_validation_hooks")]
mod sha_tests;
#[cfg(feature = "fips_validation_hooks")]
mod soft_aes_tests;
mod test_action_tests;
mod unmask_key_tests;
mod unsupported_tests;

use mcr_crypto_pka::*;
use mcr_ddi_mbor::*;
use mcr_ddi_types::*;
use mcr_types::*;

use self::page_alloc::Page;
use crate::error::*;
use crate::event::*;
use crate::fsm::attest_key::AttestKeyCmd;
use crate::fsm::delete_key::DeleteKeyCmd;
use crate::fsm::get_api_rev::GetApiRevCmd;
use crate::fsm::get_cert_chain_info::GetCertChainInfoCmd;
use crate::fsm::get_certificate::GetCertificateCmd;
use crate::fsm::get_device_info::GetDeviceInfoCmd;
#[cfg(feature = "fips_validation_hooks")]
use crate::fsm::get_priv_key::GetPrivKeyCmd;
use crate::fsm::get_unwrapping_key::GetUnwrappingKeyCmd;
use crate::fsm::open_key::OpenKeyCmd;
use crate::fsm::rsa_mod_exp::RsaModExpCmd;
use crate::fsm::rsa_unwrap::RsaUnwrapCmd;
#[cfg(feature = "fips_validation_hooks")]
use crate::fsm::rsa_unwrap_kek::RsaUnwrapKekTestCmd;
use crate::fsm::unsupported::UnsupportedCmd;
use crate::fsm::HsmFsmEventRecorder;
use crate::fsm::OpenSessionCmd;
use crate::fsm::{decode_buf, encode_buf, HsmCmdTrait};
use crate::heap::HsmDmaAllocTrait;
use crate::mock::*;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::AesBulk256Cmd;
use crate::partition::EccKeyPct;
use crate::partition::GetEncryptionKeyOut;
use crate::partition::RsaCrtParamCalcState;
use crate::partition::RsaCrtParamComputeCmd;
use crate::partition::RsaPrivKeyCrt;
use crate::partition::RsaSize;
use crate::resource::FpIpcChannelResource;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;
use crate::TagId;

/// Encode a command to a page
///
/// # Arguments
///
/// * `cmd` - Command to encode
///
/// # Returns
///
/// * Encoded page
fn ddi_encode_page<T>(cmd: &T) -> Page
where
    T: MborEncode + MborLen,
{
    let mut page = Page::new().unwrap();

    let mut encoder = MborEncoder::new(page.slice_mut());
    cmd.mbor_encode(&mut encoder).unwrap();

    let rem_len = encoder.remaining();
    page.set_len(page.cap() - rem_len);

    page
}

/// Decode a command from a page
///
/// # Arguments
///
/// * `page` - Page to decode
///
/// # Returns
///
/// * Decoded command
fn ddi_decode_page<'a, T>(page: &'a Page) -> T
where
    T: mcr_ddi_mbor::MborDecode<'a>,
{
    let mut decoder = mcr_ddi_mbor::MborDecoder::new(page.slice());
    T::mbor_decode(&mut decoder).unwrap()
}

/// Sets the expectations for key agreement ECC PCT execution
///
/// # Arguments
///
/// * `part` - mutable reference to the partition
fn set_ecc_pct_key_agreement_expectations(part: &mut MockPartition) {
    part.expect_begin_ecc_pct_validation().once().returning(
        move |_tag, _key_id, _usage, _pub_key| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
            let engine = resource.acquire(TagId::default(), None);

            let key_agreement_buffer_size =
                (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + (PkaEccCurve::MAX_LEN * 2);
            let op_dma_buf = MockDmaAlloc::new(key_agreement_buffer_size);

            let priv_key_blob = &[0u8; 96];
            let pub_key_blob = &[0u8; 136];

            let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);
            let pub_key_data_len = PkaEccCurve::MAX_LEN * 2;

            let mut key_blob_dma_buf = MockDmaAlloc::new(pub_key_data_len + priv_key_size);

            key_blob_dma_buf.as_ref_mut()[pub_key_data_len..pub_key_data_len + priv_key_size]
                .copy_from_slice(&priv_key_blob[..priv_key_size]);
            let priv_key_blob = IoMemRange::from(
                &key_blob_dma_buf.as_ref()[pub_key_data_len..pub_key_data_len + priv_key_size],
            );

            key_blob_dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(pub_key_blob);
            let pub_key_blob = IoMemRange::from(&key_blob_dma_buf.as_ref()[..pub_key_data_len]);

            let sha = MockSha::new();
            let engine: Box<dyn PctEngine> =
                Box::new(PctEngineImpl::<MockEnv>::new(engine.unwrap(), sha));

            let ecc_key_pct = EccKeyPct::new(
                priv_key_blob,
                pub_key_blob,
                key_blob_dma_buf,
                PkaEccCurve::Ecc384,
                op_dma_buf,
                engine,
            );

            Ok(ecc_key_pct)
        },
    );
    part.expect_continue_ecc_pct_validation()
        .times(3)
        .returning(move |_tag, _op| Ok(()));
    part.expect_end_ecc_pct_validation()
        .once()
        .returning(|_tag, _op| Ok(true));
    part.expect_is_pct_final_state()
        .times(3)
        .returning(|_| false);
    part.expect_is_pct_final_state().once().returning(|_| true);
}

pub(crate) fn import_der_crt_key() -> HsmResult<RsaCrtParamComputeCmd<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(1));

    Ok(RsaCrtParamComputeCmd {
        tag: TagId::default(),
        priv_key_crt: RsaPrivKeyCrt {
            rsa_type: RsaSize::Rsa2k,
            p: vec![].into(),
            q: vec![].into(),
            dp: vec![].into(),
            dq: vec![].into(),
            n: vec![].into(),
            n1q: None,
            n2p: None,
            e: vec![].into(),
            coefficient: vec![].into(),
        },
        rsa_op_data: Some(PkaRsaCmd {
            rsa_type: PkaRsaSize::Rsa2k,
        }),
        mont_in_q_inv_mod_p: None,
        n1q: None,
        p_inv_mod_q: None,
        mont_in_p_full: None,
        n2p: None,
        engine_ref: engine.unwrap(),
        state: RsaCrtParamCalcState::Idle,
    })
}

pub(crate) fn import_der_aesbulk256_key() -> HsmResult<AesBulk256Cmd<MockEnv>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        FpIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );
    let channel = resource.acquire(TagId::default(), ());

    Ok(AesBulk256Cmd::DerKeyImport(
        Default::default(),
        0,
        channel.unwrap(),
    ))
}
