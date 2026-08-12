// Copyright (c) Microsoft Corporation. All rights reserved.

use core::alloc::Layout;
use core::ops::Range;
use core::ptr::NonNull;

use mcr_cpu::*;
use mcr_crypto_aes::*;
use mcr_crypto_pka::*;
use mcr_crypto_rng::*;
use mcr_crypto_sha::*;
use mcr_ddi_types::DdiApiRev;
use mcr_ddi_types::DdiEncryptedEstablishCredential;
use mcr_ddi_types::DdiEncryptedPin;
use mcr_ddi_types::DdiEncryptedSessionCredential;
use mcr_ddi_types::DdiHashAlgorithm;
use mcr_ddi_types::DdiKeyProperties;
use mcr_ddi_types::DdiKeyType;
use mcr_ddi_types::DdiKeyUsage;
use mcr_ddi_types::DdiRsaCryptoPadding;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestAction;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestActionPinPolicyConfig;
use mcr_error::McrResult;
use mcr_gdma_controller::*;
use mcr_io_controller::*;
use mcr_ipc_controller::*;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::CrashType;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::SocCpuId;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ipc_message::StackErrorType;
#[cfg(feature = "mcr_manual_test_hooks")]
use mcr_ipc_message::TdispInterruptInfo;
use mcr_logging::DebugLogSenderTrait;
use mcr_mailbox_controller::MailboxControllerTrait;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_simplex::*;
use mcr_tcon::*;
use mcr_types::*;

use crate::cmd_scheduler::*;
use crate::env::*;
use crate::error::HsmResult;
use crate::heap::*;
use crate::partition::pct_engine::PctEngine;
use crate::partition::store::EntryAttributes;
use crate::partition::*;
use crate::x509::Ecdsa384Signature;

cfg_if::cfg_if! {
    if #[cfg(test)] {
        use mockall::*;
        use mockall::predicate::*;
    }
}

mod env {
    use super::*;

    mock! {
        pub(crate) MockEnv {}

        impl HsmEnvTrait for MockEnv {
            type Hal = MockHal;
            type Partition = MockPartition;
            type UserSession = MockUserSession;

            fn hal(&self) -> &<MockMockEnv as HsmEnvTrait>::Hal;

            fn partition(&self, pfn: PcieFunction) -> <MockMockEnv as HsmEnvTrait>::Partition;

            fn prepare_for_shutdown(&self);

            fn pka_engine(&self) -> &PkaEngine<Self>;

        }

        impl Clone for MockEnv {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockMailboxController{}

        impl MailboxControllerTrait for MockMailboxController {
            fn init(&self);

            fn trigger_mbx_err(&self);
        }

        impl Clone for MockMailboxController {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockHal {}

        impl HsmHalTrait for MockHal {
            type IoChannel = MockIoChannel;
            type DmaChannel = MockDmaChannel;
            type IpcMessageChannel = MockIpcMessageChannel;
            type IpcEventChannel = MockIpcEventChannel;
            type DmaHeap = MockDmaHeap;
            type Aes = MockAes;
            type Rng = MockRng;
            type Pka = MockPka;
            type Sha = MockSha;
            type CpuInfo = MockCpuInfo;
            type Tcon = MockTcon;
            type QueueDeleteResp = MockSimplexPipe<QueueDeleteResponse>;
            type SoftAesReq = MockSimplexPipe<SoftAesOffloadReq>;
            type SoftAesResp = MockSimplexPipe<SoftAesOffloadResp>;
            type SelfTestReq=MockSimplexPipe<SelfTestReqPacket>;
            type SelfTestResp=MockSimplexPipe<SelfTestRespPacket>;
            type MailboxController = MockMailboxController;

            fn io_channel(&self) -> &<MockMockHal as HsmHalTrait>::IoChannel;

            fn dma_channel(&self) -> &<MockMockHal as HsmHalTrait>::DmaChannel;

            fn dma_heap(&self) -> &<MockMockHal as HsmHalTrait>::DmaHeap;

            fn admin_ipc_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcMessageChannel;

            fn ipc_event_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcEventChannel;

            fn hsm_to_admin_ipc_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcMessageChannel;

            fn aes(&self) -> &<MockMockHal as HsmHalTrait>::Aes;

            fn rng(&self) -> &<MockMockHal as HsmHalTrait>::Rng;

            fn sha(&self) -> &<MockMockHal as HsmHalTrait>::Sha;

            fn pka(&self) -> &Vec<<MockMockHal as HsmHalTrait>::Pka>;

            fn hsm_to_fp_ipc_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcMessageChannel;

            fn hsp_ipc_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcMessageChannel;

            fn fp_to_hsm_ipc_channel(&self) -> &<MockMockHal as HsmHalTrait>::IpcMessageChannel;

            fn part_persistent_store_addr(&self) -> usize;

            fn vault_addr(&self) -> usize;

            fn cpu_info(&self) -> &<MockMockHal as HsmHalTrait>::CpuInfo;

            fn cdma_vault_addr(&self) -> usize;

            fn cdma_vault_meta_data(&self) -> usize;

            fn bks_table_addr(&self) -> usize;

            fn tcon_tsc(&self) -> u64;

            fn resource_table(&self) -> &[Resource];

            fn partition_data_store_addr(&self) -> usize;

            fn alias_key_len(&self) -> u32;

            fn alias_key(&self) -> &[u8];

            fn alias_cert_len(&self) -> usize;

            fn alias_cert(&self) -> &[u8];

            fn queue_delete_notification(&self) -> &MockSimplexPipe<QueueDeleteResponse>;

            fn soft_aes_req(&self) -> &MockSimplexPipe<SoftAesOffloadReq>;

            fn soft_aes_resp(&self) -> &MockSimplexPipe<SoftAesOffloadResp>;

            fn update_core_liveliness(&self);

            fn self_test_req(&self) -> &MockSimplexPipe<SelfTestReqPacket>;

            fn self_test_resp(&self) -> &MockSimplexPipe<SelfTestRespPacket>;

            fn notify_self_test_failure(&self, test_id: SelfTest);

            fn notify_pct_validation_failure(&self, err: u32);

            fn is_fips_approved(&self, pfn:PcieFunction) -> bool;

            #[cfg(feature = "fips_validation_hooks")]
            fn toggle_fips_approved_state(&self, pfn: PcieFunction);

            #[cfg(feature = "mcr_test_hooks")]
            fn get_corr_ecc_err_intr_count(&self) -> Option<u32>;

            #[cfg(feature = "mcr_test_hooks")]
            fn set_corr_ecc_err_intr_count(&self, count: u32) -> McrResult<()>;
        }

        impl Clone for MockHal {
            fn clone(&self) -> Self;
        }
    }
}

mod partition {
    use super::*;

    mock! {
        pub(crate) MockPartition {}

        impl HsmPartition for MockPartition {
            type Env = MockEnv;
            type UserSession = MockUserSession;

            fn min_api_rev(&self) -> DdiApiRev;

            fn max_api_rev(&self) -> DdiApiRev;

            fn enable(&self);

            fn disable(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

            fn reset(&self);

            fn begin_migrate(&self, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

            fn end_migrate(&self);

            fn set_resource_mask(&self, mask: u128);

            fn resource_mask(&self) -> u128;

            fn enabled(&self) -> bool;

            fn enable_io_queue(&self, sq_id: DevSqId, cq_id: DevCqId);

            fn disable_io_queue(&self, sq_id: DevSqId, delete_ctx: Option<IoQueueDeleteContext>) -> bool;

            fn io_queue(&self, sq_id: DevSqId) -> Option<IoQueue>;

            fn clear_unwrapping_key(&mut self) -> HsmResult<()>;

            fn set_unwrapping_key_required(&self, required: bool);

            fn unwrapping_key_id(&self) -> Option<KeyId>;

            fn is_unwrapping_key_pct_verified(&self) -> bool;

            fn mark_unwrapping_key_pct_verified(&mut self);

            fn store_data(&self);

            fn get_alias_cert(&self) -> IoMemRange;

            fn get_alias_cert_len(&self) -> usize;

            fn get_cert_len(&self, cert_id: u8) -> Option<usize>;

            fn begin_generate_partition_identifiers(
                &self,
                tag: TagId,
            ) -> HsmResult<GetPartitionIdCtx>;

            fn continue_generate_partition_identifiers(
                &self,
                tag: TagId,
                ctx: GetPartitionIdCtx,
            ) -> HsmResult<PartitionIdGenResult>;

            fn end_generate_partition_identifiers(&self, km: PartitionIdGenResult) -> HsmResult<()>;

            fn begin_get_establish_cred_encryption_key(
                &self,
                tag: TagId,
            ) -> HsmResult<GetEstablishCredEncryptionKeyCtx<MockEnv>>;

            fn end_get_establish_cred_encryption_key(
                &self,
                tag: TagId,
                ctx: GetEstablishCredEncryptionKeyCtx<MockEnv>
            ) -> HsmResult<GetEstablishCredEncryptionKeyOut>;

            fn begin_establish_credential(
                &self,
                tag: TagId,
                pub_key: &IoMemRange,
                pota_pub_key: &IoMemRange,
            ) -> HsmResult<EstablishCredentialCtx<MockEnv>>;

            fn continue_establish_credential(
                &self,
                ctx: EstablishCredentialCtx<MockEnv>,
                pub_key: &IoMemRange,
                pota_pub_key: &IoMemRange,
                pota_sig: &IoMemRange,
            ) -> HsmResult<EstablishCredentialCtx<MockEnv>>;

            fn end_establish_credential(
                &self,
                ctx: EstablishCredentialCtx<MockEnv>,
                encrypted_credential: &DdiEncryptedEstablishCredential,
            ) -> HsmResult<()>;

            fn verify_nonce(&self, nonce: [u8; 32]) -> HsmResult<()>;

            fn verify_cred_is_not_set(&self) -> HsmResult<()>;

            fn set_cert_chain_lengths_info(&self, info: Option<GetCertChainLengthsInfo>);

            fn rollback_open_session(&self, id: SessionId, is_reopen: bool) -> HsmResult<()>;

            fn close_user_session(&self, id: SessionId) -> HsmResult<()>;

            fn delete_user_session(&self, id: SessionId);

            fn begin_open_user_session(
                &self,
                tag: TagId,
                pub_key: &IoMemRange
            ) -> HsmResult<OpenSessionCtx<MockEnv>>;

            fn continue_open_user_session(
                &self,
                ctx: OpenSessionCtx<MockEnv>,
                pub_key: &IoMemRange,
            ) -> HsmResult<OpenSessionCtx<MockEnv>>;

            #[allow(clippy::too_many_arguments)]
            fn end_open_user_session<'a>(
                &self,
                ctx: OpenSessionCtx<MockEnv>,
                rev: DdiApiRev,
                encrypted_credential: &DdiEncryptedSessionCredential,
                reopen_sess_id: Option<u16>,
                bk_session_buf: &mut [u8],
                mk_session_buf: &mut [u8],
                bmk_session: Option<&'a [u8]>,
            ) -> HsmResult<MockUserSession>;

            fn begin_ecc_pct_validation(
                &self,
                tag: TagId,
                key_id: KeyId,
                usage: EccKeyUsage,
                public_key: PkaEccPublicKey,
            ) -> HsmResult<EccKeyPct<MockEnv>>;

            fn begin_ecc_pct_validation_raw(
                &self,
                tag: TagId,
                usage: EccKeyUsage,
                public_key: &PkaEccPublicKey,
                priv_d: &[u8],
            ) -> HsmResult<EccKeyPct<MockEnv>>;

            fn begin_ecc_pct_validation_with_engine(
                &self,
                tag: TagId,
                usage: EccKeyUsage,
                public_key: &PkaEccPublicKey,
                priv_d: &[u8],
                engine_ref: PkaEngineRef<MockEnv>,
            ) -> HsmResult<EccKeyPct<MockEnv>>;


            fn continue_ecc_pct_validation(&self, tag: TagId, ecc_key_pct: &mut EccKeyPct<MockEnv>) -> HsmResult<()>;

            fn end_ecc_pct_validation(&self, tag: TagId, ecc_key_pct: &mut EccKeyPct<MockEnv>) -> HsmResult<bool>;

            fn is_pct_final_state(&self, pct_op: &EccKeyPct<MockEnv>) -> bool;

            fn authorize_user_with_pin_policy(&self, id: &AppId, pin: &AppPin) -> HsmResult<()>;

            fn user_session(&self, id: SessionId, allow_disabled: bool) -> HsmResult<MockUserSession>;

            fn begin_close_user_session(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                id: SessionId,
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn end_close_user_session(&self, op: &AesBulk256Cmd<MockEnv>) -> HsmResult<()>;

            fn begin_get_session_encryption_key(
                &self,
                tag: TagId,
            ) -> HsmResult<GetSessionEncryptionKeyCtx<MockEnv>>;

            fn end_get_session_encryption_key(
                &self,
                tag: TagId,
                ctx: GetSessionEncryptionKeyCtx<MockEnv>,
            ) -> HsmResult<GetSessionEncryptionKeyOut>;

            fn verify_cred_is_set(&self) -> HsmResult<()>;

            fn delete_internal_key(&self, key_id: KeyId) -> HsmResult<()>;

            fn unset_establish_cred_encryption_key_id(&self);

            fn unset_session_encryption_key_id(&self);

            fn is_fips_approved(&self) -> bool;

            fn notify_pct_validation_failure(&self, err: u32);

            fn set_vm_launch_guid(&self, guid: &VmLaunchGuid);

            fn vm_launch_guid(&self) -> VmLaunchGuid;

            fn begin_get_dev_id_cert_chain_info(
                &self,
                tag: TagId,
                get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<MockEnv>,
            ) -> HsmResult<()>;

            fn end_get_dev_id_cert_chain_info(
                &self,
                get_cert_chain_lengths_ctx: &mut GetCertLengthsContext<MockEnv>,
            ) -> HsmResult<()>;

            fn update_cert_chain_lengths_info(
                &self,
                cert_info: &mut GetCertChainLengthsInfo,
            ) -> HsmResult<()>;

            fn get_ecdsa384_signature_from_buffer(
                &self,
                signature_buffer: &[u8],
            ) -> HsmResult<Ecdsa384Signature>;

            fn begin_get_cert(
                &self,
                tag: TagId,
                get_cert_ctx: &mut GetCertContext<MockEnv>,
            ) -> HsmResult<()>;

            fn end_get_cert(&self, get_cert_ctx: &mut GetCertContext<MockEnv>) -> HsmResult<()>;

            fn get_partition_id_private_key_blob(&self) -> Option<&'static [u8]>;

            fn get_raw_alias_key(&self) -> HsmResult<SecureByteVec>;

            fn begin_generate_pid_cert(&self, tag: TagId, key_blob: &[u8]) -> HsmResult<CertSignContext<MockEnv>>;

            fn end_generate_pid_cert(&self, tag: TagId, cert_sign_ctx: &CertSignContext<MockEnv>) -> HsmResult<()>;

            fn is_partition_cert_valid(&self) -> bool;

            fn set_partition_cert_valid(&self, valid: bool);

            fn partition_cert(&self) -> IoMemRange;

            fn partition_cert_length(&self) -> u32;

            fn set_partition_cert_length(&self, len: u32) -> HsmResult<()>;

            fn generate_bk_boot(&self, bk_boot: &mut [u8]) -> HsmResult<()>;

            fn mask_bk3(&self, bk3: &[u8], masking_key: &[u8], output_len: &mut usize, output_buf: &mut [u8]) -> HsmResult<()>;

            fn mask_bk_boot(&self,bk_boot: &[u8], output_len: &mut usize,output_buf: &mut [u8]) -> HsmResult<()>;

            fn get_masked_bk_boot_len(&self) -> u32;

            fn set_masked_bk_boot_len(&self, len: u32);

            fn sealed_bk3(&self) -> IoMemRange;

            fn get_sealed_bk3_len(&self) -> u32;

            fn set_sealed_bk3_len(&self, len: u32);

            fn unmask_bk3(&self, masked_bk3: &[u8], bk3: &mut [u8]) -> HsmResult<()>;

            fn generate_and_store_bk3_session(&self, bk3: &[u8]) -> HsmResult<()>;

            fn generate_bk(&self, bk3: &[u8], pota_pub_key: &[u8], bk: &mut [u8]) -> HsmResult<()>;

            fn generate_new_mk_and_import(&self) -> HsmResult<()>;

            fn generate_bmk(&self, bk: &[u8], bmk_len: &mut usize, bmk_out: &mut [u8]) -> HsmResult<()>;

            fn import_mk_from_bmk(&self, bk3: &[u8], pota_pub_key: &[u8], bmk: &[u8]) -> HsmResult<()>;

            fn is_partition_provisioned(&self) -> bool;

            fn begin_signature_with_part_priv_key(
                &self,
                tag: TagId,
                key_data: &IoMemRange,
                signature: &IoMemRange,
            ) -> HsmResult<KeySignContext<MockEnv>>;

            fn end_signature_with_key_blob(
                &self,
                tag: TagId,
                sign_ctx: KeySignContext<MockEnv>,
            ) -> HsmResult<()>;

            fn masked_bk_boot(&self) -> IoMemRange;

            fn needs_renegotiation(&self, sess_id: u16) -> bool;

            fn unmask_unwrapping_key_and_import(&self, masked_uk: &[u8]) -> HsmResult<()>;

            fn generate_bmk_session(
                &self,
                bk: &[u8],
                smk: &[u8],
                bmk_len: &mut usize,
                bmk_out: &mut [u8],
            ) -> HsmResult<()>;

            fn flush_session(&self, session_id: u16);

            #[cfg(feature = "fips_validation_hooks")]
            fn toggle_fips_approved_state(&self);

            #[cfg(feature = "fips_validation_hooks")]
            fn clear_bk3_info(&self);

            #[cfg(feature = "mcr_test_hooks")]
            fn set_test_hook_to_trigger_level2_abort(&self, level2_trigger: bool);

            #[cfg(feature = "mcr_test_hooks")]
            fn test_hook_to_trigger_level2_abort(&self) ->bool;

            #[cfg(feature = "mcr_test_hooks")]
            fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

            #[cfg(feature = "mcr_test_hooks")]
            fn override_pin_policy_context(&self, pin_policy_config: DdiTestActionPinPolicyConfig);

            #[cfg(feature = "mcr_test_hooks")]
            fn clear_pin_policy(&self);

            #[cfg(feature = "mcr_test_hooks")]
            fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

            #[cfg(feature = "fips_validation_hooks")]
            fn inject_rng_hw_failure (&self, rng_hw_self_test_id: u32);

            #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
            fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8>;

            #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
            fn reset_unwrapping_key_pct(&self);

            #[cfg(all(feature = "mcr_test_hooks", feature = "mcr_test_hooks_cdma_ecc_err"))]
            fn get_corr_ecc_err_intr_count(&self) -> Option<u32>;

            #[cfg(feature = "mcr_test_hooks")]
            fn set_current_svn(&self, svn: u64) -> HsmResult<()>;

            fn clear_credentials(&self) -> HsmResult<()>;

            fn clear_provisioning_state(&self) -> HsmResult<()>;

        }

        impl Clone for MockPartition {
            fn clone(&self) -> Self;
        }
    }

    mock! {

        pub(crate) MockUserSession {}

        impl HsmUserSession for MockUserSession{
            type Env = MockEnv;

            fn app_vault_id(&self) -> AppVaultId;

            fn app_id(&self) -> AppId;

            #[allow(clippy::too_many_arguments)]
            fn open_key_zc(
                &mut self,
                tag: TagId,
                key_tag: u16,
                key_id: Option<KeyId>,
                key_kind: Option<EntryKind>,
                phase: OpenKeyPhase,
                is_unwrapping_key: bool,
                ecc_op: &mut Option<EccGenPubKeyCmd<MockEnv>>,
                pub_key: &IoMemRange,
            ) -> HsmResult<OpenKeyData>;

            #[cfg(feature = "fips_validation_hooks")]
            fn get_priv_key(
                &mut self,
                key_id: u16,
                key_data: &mut IoMemRange,
            ) -> HsmResult<()>;

            fn get_key_kind(
                &self,
                key_id: KeyId,
            ) -> HsmResult<EntryKind>;

            #[cfg(feature = "fips_validation_hooks")]
            fn get_key_length(&self, key_id: KeyId) -> HsmResult<u16>;

            fn delete_key(&self, key_id: KeyId) -> HsmResult<()>;

            fn aes_gen_key(
                &self,
                tag: Option<u16>,
                kind: AesKeyKind,
                usage: AesKeyUsage,
                availability: KeyAvailability,
            ) -> HsmResult<AesKey>;

            fn aes_enc_dec<'a>(&self, tag: TagId, key: AesKeyIn<'a>, input: &AesEncDecIn<'a>) -> HsmResult<()>;

            fn begin_aes_key_unwrap(&self, tag: TagId, kek: &[u8], inout: &[u8]) -> HsmResult<()>;

            fn end_aes_key_unwrap(&self, tag: TagId) -> HsmResult<Range<usize>>;

            #[cfg(feature = "fips_validation_hooks")]
            fn begin_soft_aes(
                &self,
                tag: TagId,
                key: &[u8],
                inout: &[u8],
                op: SoftAesOp,
            ) -> HsmResult<()>;

            #[cfg(feature = "fips_validation_hooks")]
            fn end_soft_aes(&self, tag: TagId) -> HsmResult<Range<usize>>;

            fn begin_ecc_gen_key(
                &self,
                tag: TagId,
                key_tag: Option<u16>,
                curve: EccCurve,
                usage: EccKeyUsage,
                availability: KeyAvailability,
            ) -> HsmResult<EccGenKey<MockEnv>>;

            fn end_ecc_gen_key(&self, tag: TagId, op: EccGenKey<MockEnv>) -> HsmResult<EccGenKeyOut>;

            fn begin_ecc_sign_zc<'a>(
                &self,
                tag: TagId,
                key_in: EccKeyIn<'a>,
                digest: &IoMemRange,
                digest_algo: DdiHashAlgorithm,
                signature: &IoMemRange,
            ) -> HsmResult<EccSign<MockEnv>>;

            fn end_ecc_sign_zc(&self, tag: TagId, op: EccSign<MockEnv>) -> HsmResult<()>;

            fn begin_ecc_gen_pub_key(
                &self,
                tag: TagId,
                key_id: KeyId,
            ) -> HsmResult<EccGenPubKeyCmd<MockEnv>>;

            fn continue_ecc_gen_pub_key_zc(
                &self,
                op: EccGenPubKeyCmd<MockEnv>,
                pub_key: &IoMemRange,
            ) -> HsmResult<EccGenPubKeyCmd<MockEnv>>;

            fn begin_ecdh_compute_with_pub_key_validation(
                &self,
                tag: TagId,
                key_id: KeyId,
                target_key_type: DdiKeyType,
                pub_key: &IoMemRange
            ) -> HsmResult<EcdhComputeCmd<MockEnv>>;

            fn continue_ecdh_compute_zc(
                &self,
                op: EcdhComputeCmd<MockEnv>,
                pub_key: &IoMemRange,
            ) -> HsmResult<EcdhComputeCmd<MockEnv>>;

            fn end_ecdh_compute(
                &self,
                op: EcdhComputeCmd<MockEnv>,
                key_usage: DdiKeyUsage,
                key_tag: Option<u16>,
                key_availabilty: KeyAvailability,
            ) -> HsmResult<KeyId>;

            fn end_ecc_gen_pub_key_zc(&self, op: EccGenPubKeyCmd<MockEnv>) -> HsmResult<()>;

            fn begin_ecc_pct_validation(
                &self,
                tag: TagId,
                key_id: KeyId,
                usage: EccKeyUsage,
                public_key: PkaEccPublicKey,
            ) -> HsmResult<EccKeyPct<MockEnv>>;

            fn continue_ecc_pct_validation(
                &self,
                tag: TagId,
                ecc_key_pct: &mut EccKeyPct<MockEnv>,
            ) -> HsmResult<()>;

            fn end_ecc_pct_validation(
                &self,
                tag: TagId,
                ecc_key_pct: &mut EccKeyPct<MockEnv>,
            ) -> HsmResult<bool>;

            fn is_pct_final_state(&self, op: &EccKeyPct<MockEnv>) -> bool;

            fn begin_ecc_structural_validation(
                &self,
                tag: TagId,
                key_id: KeyId,
                entry_usage: DdiKeyUsage,
                pub_key_blob: Vec<u8>,
            ) -> HsmResult<EccStructuralValidationCmd<MockEnv>>;

            fn continue_ecc_structural_validation(
                &self,
                op: EccStructuralValidationCmd<MockEnv>,
            ) -> HsmResult<EccStructuralValidationCmd<MockEnv>>;

            fn end_ecc_structural_validation(
                &mut self,
                op: EccStructuralValidationCmd<MockEnv>,
            ) -> HsmResult<()>;

            fn begin_rsa_mod_exp_zc(
                &self,
                tag: TagId,
                key_id: KeyId,
                usage: Option<RsaKeyUsage>,
                input: &IoMemRange,
                output: &IoMemRange,
            ) -> HsmResult<RsaModExp<MockEnv>>;

            fn end_rsa_mod_exp_zc(&self, tag: TagId, op: RsaModExp<MockEnv>) -> HsmResult<()>;

            fn begin_rsa_pct_validation(
                &self,
                tag: TagId,
                key_id: KeyId,
                usage: RsaKeyUsage,
                rsa_type: PkaRsaSize,
                n: &mut [u8],
                e: &[u8],
            ) -> HsmResult<RsaPctValidationCmd<MockEnv>> ;

            fn continue_rsa_pct_validation(
                &self,
                mut op: RsaPctValidationCmd<MockEnv>,
            ) -> HsmResult<RsaPctValidationCmd<MockEnv>>;

            fn end_rsa_pct_validation(&self, mut op: RsaPctValidationCmd<MockEnv>) -> HsmResult<bool>;

            fn is_rsa_pct_final_state(&self, pct_op: &RsaPctValidationCmd<MockEnv>) -> bool;

            fn begin_rsa_unwrap_mod_exp_zc(
                &self,
                tag: TagId,
                key_id: KeyId,
                input: &IoMemRange,
                output: &IoMemRange,
                usage: Option<RsaKeyUsage>,
            ) -> HsmResult<RsaModExp<MockEnv>>;

            fn end_rsa_unwrap_mod_exp_zc(&self, tag: TagId, op: RsaModExp<MockEnv>) -> HsmResult<()>;

            fn decode_oaep_kek(
            &self,
                unwrapped_data: &[u8],
                padding: DdiRsaCryptoPadding,
                hash_alg: DdiHashAlgorithm,
            ) -> HsmResult<SecureByteVec>;

            #[cfg(feature = "fips_validation_hooks")]
            fn get_random_number(&self, rng_number: &mut IoMemRange) -> HsmResult<()>;

            fn sha_single_block_zc(
                &self,
                mode: ShaType,
                buffer: &IoMemRange,
                output_buffer: &mut IoMemRange,
            ) -> HsmResult<()>;

            #[cfg(feature = "fips_validation_hooks")]
            fn import_raw_key(
                &self,
                key_type: DdiKeyType,
                key_properties: DdiKeyProperties,
                key_tag: Option<u16>,
                raw_key: &[u8],
            ) -> HsmResult<KeyId>;

            fn import_der_key(
                &self,
                entry_class: EntryClass,
                entry_usage: DdiKeyUsage,
                entry_tag: Option<u16>,
                entry_availability: KeyAvailability,
                der: &[u8],
            ) -> HsmResult<ImportDerKeyResult>;

            fn begin_import_der_crt_key(
                &self,
                tag: TagId,
                der: &[u8],
            ) -> HsmResult<(RsaCrtParamComputeCmd<MockEnv>, Vec<u8>)>;

            fn continue_import_der_crt_key(
                &self,
                op: RsaCrtParamComputeCmd<MockEnv>,
            ) -> HsmResult<RsaCrtParamComputeCmd<MockEnv>>;

            fn end_import_der_crt_key(
                &self,
                op: RsaCrtParamComputeCmd<MockEnv>,
                entry_usage: DdiKeyUsage,
                entry_tag: Option<u16>,
                entry_availability: KeyAvailability,
            ) -> HsmResult<(KeyId, DdiKeyType)>;

            fn get_unwrapping_key(
                &self,
                tag: TagId,
                key_id: Option<KeyId>,
                pfn: PcieFunction,
            ) -> HsmResult<GetUnwrappingKeyCtx>;

            fn begin_compute_rsa_crt_params(
                &self,
                tag: TagId,
                priv_key_crt: RsaPrivKeyCrt,
            ) -> HsmResult<RsaCrtParamComputeCmd<MockEnv>>;

            fn continue_compute_rsa_crt_params(
                &self,
                tag: TagId,
                op: RsaCrtParamComputeCmd<MockEnv>,
            ) -> HsmResult<RsaCrtParamComputeCmd<MockEnv>>;

            fn end_compute_rsa_crt_params(
                &self,
                op: RsaCrtParamComputeCmd<MockEnv>,
            ) -> HsmResult<RsaPrivKeyCrt>;

            fn hmac(&self, key_id: KeyId, msg: &[u8], output: &mut IoMemRange) -> HsmResult<()>;

            fn var_hmac(&self, key_id: KeyId, msg: &[u8], output_buffer: &mut IoMemRange) -> HsmResult<()>;

            #[allow(clippy::too_many_arguments)]
            fn hkdf_derive(
                &self,
                key_id: KeyId,
                salt: &[u8],
                info: &[u8],
                hash_algo: DdiHashAlgorithm,
                key_type: DdiKeyType,
                key_properties: DdiKeyProperties,
                key_tag: Option<u16>,
                key_len: Option<u8>,
            ) -> HsmResult<KeyId>;

            #[allow(clippy::too_many_arguments)]
            fn kbkdf_derive(
                &self,
                key_id: KeyId,
                label: &[u8],
                context: &[u8],
                hash_algo: DdiHashAlgorithm,
                key_type: DdiKeyType,
                key_properties: DdiKeyProperties,
                key_tag: Option<u16>,
                key_len: Option<u8>,
            ) -> HsmResult<KeyId>;

            fn begin_hkdf_aesbulk256_derive(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                salt: &[u8],
                info: &[u8],
                kdf_info: KdfInfo,
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn begin_kbkdf_aesbulk256_derive(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                label: &[u8],
                context: &[u8],
                kdf_info: KdfInfo,
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn end_kdf_aesbulk256_derive(&self, op: &AesBulk256Cmd<MockEnv>) -> HsmResult<()>;

            #[allow(clippy::too_many_arguments)]
            fn begin_import_der_aesbulk256_key(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                entry_usage: DdiKeyUsage,
                entry_tag: Option<u16>,
                key_type: DdiKeyType,
                entry_availability: KeyAvailability,
                der: &[u8],
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            #[allow(clippy::too_many_arguments)]
            fn unmask_import_der_aesbulk256_key(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                entry_usage: DdiKeyUsage,
                entry_tag: Option<u16>,
                key_type: DdiKeyType,
                original_attributes: &EntryAttributes,
                der: &[u8],
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn end_import_der_aesbulk256_key(
                &self,
                op: &AesBulk256Cmd<MockEnv>,
            ) -> HsmResult<()>;

            fn begin_delete_aesbulk256_key(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                key_id: KeyId,
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn end_delete_aesbulk256_key(
                &self,
                op: &AesBulk256Cmd<MockEnv>,
            ) -> HsmResult<()>;

            fn begin_aesbulk256_gen_key(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                key_tag: Option<u16>,
                key_type: DdiKeyType,
                availability: KeyAvailability,
            ) -> HsmResult<AesBulk256Cmd<MockEnv>>;

            fn end_aesbulk256_gen_key(
                &self,
                op: &AesBulk256Cmd<MockEnv>,
            ) -> HsmResult<()>;

            fn begin_rollback_aesbulk256_key(
                &self,
                tag: TagId,
                pfn: PcieFunction,
                op: &AesBulk256Cmd<MockEnv>,
            ) -> HsmResult<()>;

            fn end_rollback_aesbulk256_key(&self, op: &AesBulk256Cmd<MockEnv>) -> HsmResult<()>;

            fn begin_change_pin(&self, tag: TagId) -> HsmResult<ChangePinCmdCtx<MockEnv>>;

            fn continue_change_pin(
                &self,
                ctx: ChangePinCmdCtx<MockEnv>,
                pub_key: &IoMemRange,
            ) -> HsmResult<ChangePinCmdCtx<MockEnv>>;

            fn end_change_pin(
                &self,
                ctx: ChangePinCmdCtx<MockEnv>,
                encrypted_pin: &DdiEncryptedPin,
            ) -> HsmResult<()>;

            fn notify_pct_validation_failure(&self, err: u32);

            #[cfg(feature = "mcr_manual_test_hooks")]
            fn send_tdisp_interrupt_request(&self, tag: TagId, info: TdispInterruptInfo) -> HsmResult<()>;

            #[cfg(feature = "mcr_test_hooks")]
            fn send_crashdump_request(&self, tag: TagId, cpu_id: SocCpuId, crash_type: CrashType) -> HsmResult<()>;

            #[cfg(feature = "mcr_test_hooks")]
            fn send_stack_validation_request(&self, tag: TagId, cpu_id: SocCpuId, error_type: StackErrorType) -> HsmResult<()>;

            #[cfg(feature = "mcr_test_hooks")]
            fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

            #[cfg(feature = "mcr_test_hooks")]
            fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction>;

            #[cfg(feature = "mcr_test_hooks")]
            fn begin_neg_self_test_req(&self, neg_self_test: SelfTest, tag: TagId) -> HsmResult<()>;

            #[cfg(feature = "mcr_test_hooks")]
            fn end_neg_self_test_resp(&self, tag: TagId) -> HsmResult<()>;

            #[cfg(feature = "fips_validation_hooks")]
            fn force_pka_instance(&self, pka_instance: Option<usize>);

            #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
            fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8>;

            fn get_masked_key_len_from_vault<'a>(
                &self,
                key_label: &[u8],
                key_id: KeyId,
                pub_data: Option<&'a [u8]>,
            ) -> HsmResult<usize>;

            fn get_masked_key_len(&self, metadata_len: usize, encrypted_key_len: usize)
                -> HsmResult<usize>;

            fn mask_key_from_vault<'a>(
                &self,
                key_label: &[u8],
                key_id: KeyId,
                pub_data: Option<&'a [u8]>,
                masked_key: &mut [u8],
            ) -> HsmResult<()>;

            fn mask_bulk_key(
                &self,
                key_label: &[u8],
                key_id: KeyId,
                raw_key: &[u8],
                masked_key: &mut [u8],
            ) -> HsmResult<()>;

            fn get_masked_bulk_key_len(
                &self,
                key_label: &[u8],
                key_id: KeyId,
                raw_key_len: usize,
            ) -> HsmResult<usize>;

            fn mask_key(
                &self,
                metadata: &[u8],
                masking_key: &[u8],
                padded_key_buffer: &[u8],
                masked_key: &mut [u8],
            ) -> HsmResult<()>;

            fn unmask_key(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyRawResult<MockEnv>>;

            fn unmask_key_and_import(&self, masked_key: &[u8]) -> HsmResult<UnmaskedKeyResult<MockEnv>>;
        }

        impl HsmSession for MockUserSession {
            fn id(&self) -> SessionId;

            fn physical_session_id(&self) -> SessionId;

            fn api_rev(&self) -> DdiApiRev;

            fn invalidate(&mut self);

            fn valid(&self) -> bool;
        }

        impl Clone for MockUserSession {
            fn clone(&self) -> Self;
        }
    }
}

mod channel {
    use super::*;

    mock! {
        pub(crate) MockIoChannel {}

        impl IoChannelTrait for MockIoChannel {

            fn begin_recv(&self) -> Option<IoRxDesc>;

            fn end_recv(&self, addr: u32, sq_id: DevSqId);

            fn begin_send<'a>(&self, desc: &IoTxDesc<'a>) -> McrResult<()>;

            fn peek_tag(&self) -> Option<u16>;

            fn end_send(&self) -> Option<IoTxCompleteDesc>;
        }

        impl Clone for MockIoChannel {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockDmaChannel {}

        impl GdmaChannelTrait for MockDmaChannel {
            fn begin_txn(&self, txn: &mut DmaTxnDesc) -> McrResult<()>;

            fn peek_tag(&self) -> Option<u16>;

            fn end_txn(&self) -> Option<DmaTxnCompletionDesc>;
        }

        impl Clone for MockDmaChannel {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockIpcMessageChannel {}

        impl IpcMessageChannelTrait for MockIpcMessageChannel {
            fn send_request(
                &self,
                _tag: u16,
                _message: mcr_ipc_controller::IpcMessage,
            ) -> McrResult<()>;

            fn send_response(
                &self,
                _message: mcr_ipc_controller::IpcMessage,
            ) -> McrResult<()>;

            fn receive_message(&self) -> Option<mcr_ipc_controller::IpcMessage>;

            fn peek_tag(&self) -> Option<u16> ;

            fn poll_message(&self) -> Option<mcr_ipc_controller::IpcMessage>;
        }

        impl Clone for MockIpcMessageChannel {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockIpcEventChannel {}

        impl IpcEventChannelTrait for MockIpcEventChannel {
            fn begin_event(&self, tag: u16, event_id: IpcDescriptor, event: u32) -> McrResult<()>;

            fn end_event(&self, event_id: IpcDescriptor, event: u32) -> McrResult<()>;

            fn peek_tag(&self) -> Option<u16>;

            fn receive_event(&self, event_id: IpcDescriptor) -> Option<u32>;
        }

        impl Clone for MockIpcEventChannel {
            fn clone(&self) -> Self;
        }
    }
}

mod heap {
    use super::*;

    pub(crate) struct MockDmaAlloc {
        vec: Vec<u8>,
    }

    impl HsmDmaAllocTrait for MockDmaAlloc {
        fn as_ref(&self) -> &[u8] {
            &self.vec
        }

        fn as_ref_mut(&mut self) -> &mut [u8] {
            &mut self.vec
        }

        fn len(&self) -> usize {
            self.vec.len()
        }
    }

    impl MockDmaAlloc {
        pub fn new(size: usize) -> Self {
            Self { vec: vec![0; size] }
        }
    }

    mock! {
        pub(crate) MockDmaHeap {}

        impl HsmDmaHeapTrait for MockDmaHeap {
            type Alloc = MockDmaAlloc;

            fn allocate(&self, len: usize) -> Option<<Self as HsmDmaHeapTrait>::Alloc>;

            fn allocate_from_pool(&self, len: usize) -> Option<<Self as HsmDmaHeapTrait>::Alloc>;

            fn size(&self) -> usize;

            fn free(&self) -> usize;

            fn deallocate(&self, ptr: NonNull<u8>, layout: Layout, size: usize);
        }

        impl Clone for MockDmaHeap {
            fn clone(&self) -> Self;
        }
    }
}

mod crypto {
    use super::*;
    mock! {
        pub(crate) MockAes {}

        impl AesTrait for MockAes {
            fn encrypt_decrypt<'a>(&self, cmd_info: &AesCommand<'a>) -> McrResult<()>;

            fn aes_cbc_self_test(
                &self,
                self_test_input: &mut [u8],
                self_test_output: &mut [u8],
                self_test_iv: &mut [u8],
            ) -> McrResult<()>;
        }

        impl Clone for MockAes {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockRng {}

        impl RngTrait for MockRng {
            fn bytes(&self, data: &mut [u8]);

            fn self_test(&self) -> McrResult<()>;

            #[cfg(feature = "fips_validation_hooks")]
            fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32);
        }

        impl Clone for MockRng {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockPka {}

        impl PkaTrait for MockPka {
            fn peek_tag(&self) -> Option<u16>;

            fn begin_ecc_gen_key(&self, tag: u16, curve: PkaEccCurve) -> McrResult<PkaEccCmd>;

            fn end_ecc_gen_key(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccKeyPair>;

            fn begin_ecc_sign_zc(
                &self,
                tag: u16,
                curve: PkaEccCurve,
                priv_key: &[u8],
                digest: &IoMemRange,
                signature: &IoMemRange,
            ) -> McrResult<PkaEccCmd>;

            fn end_ecc_sign_zc(&self, tag: u16) -> McrResult<()>;

            fn begin_ecc_verify_zc(
                &self,
                tag: u16,
                curve: PkaEccCurve,
                pub_key: &IoMemRange,
                digest: &IoMemRange,
                signature: &IoMemRange,
            ) -> McrResult<()>;

            fn end_ecc_verify_zc(&self, tag: u16) -> McrResult<bool>;

            fn begin_ecc_point_validation_zc(
                &self,
                tag: u16,
                curve: PkaEccCurve,
                pub_key: &IoMemRange,
            ) -> McrResult<()>;

            fn end_ecc_point_validation_zc(&self, tag: u16) -> McrResult<bool>;

            fn begin_ecc_gen_pub_key_zc(
                &self,
                tag: u16,
                curve: PkaEccCurve,
                private_key: &[u8],
                pub_key: &IoMemRange,
            ) -> McrResult<PkaEccCmd>;

            fn end_ecc_gen_pub_key_zc(&self, tag: u16, op: PkaEccCmd) -> McrResult<()>;

            fn begin_montgomery_constant_calculation(
                &self,
                tag: u16,
                curve: PkaEccCurve,
            ) -> McrResult<()>;

            fn end_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()>;

            fn begin_ecdh_compute_zc(
                &self,
                tag: u16,
                curve: PkaEccCurve,
                private_key: &[u8],
                public_key: &IoMemRange,
            ) -> McrResult<PkaEccCmd>;

            fn end_ecdh_compute(&self, tag: u16, op: PkaEccCmd) -> McrResult<PkaEccSecretValue>;

            fn begin_rsa_private_key_op_zc(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                priv_key: &[u8],
                data: &IoMemRange,
                result: &IoMemRange,
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_private_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

            fn begin_rsa_public_key_op_zc(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                public_key: &IoMemRange,
                data: &IoMemRange,
                result: &IoMemRange,
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_public_key_op_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

            fn begin_rsa_private_key_op_crt_zc(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                crt_param1: &[u8],
                crt_param2: &[u8],
                data: &IoMemRange,
                result: &IoMemRange,
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_private_key_op_crt_zc(&self, tag: u16, op: PkaRsaCmd) -> McrResult<()>;

            fn begin_rsa_montgomery_constant_calculation(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                modulus_be: &[u8],
            ) -> McrResult<()>;

            fn end_rsa_montgomery_constant_calculation(&self, tag: u16) -> McrResult<()>;

            fn begin_rsa_montgomery_in(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                data_be: &[u8],
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_montgomery_in(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData>;

            fn begin_rsa_modular_inverse(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                data_be: &[u8],
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_modular_inverse(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaMontData>;

            fn begin_rsa_montgomery_out(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                data_be: &[u8],
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_montgomery_out(&self, tag: u16, op: PkaRsaCmd) -> McrResult<PkaRsaData>;

            fn begin_rsa_modular_multiplication(
                &self,
                tag: u16,
                rsa_type: PkaRsaSize,
                value1: &[u8],
                value2: &[u8],
            ) -> McrResult<PkaRsaCmd>;

            fn end_rsa_modular_multiplication(
                &self,
                tag: u16,
                op: PkaRsaCmd,
            ) -> McrResult<PkaRsaMontData>;

            fn ecdsa_self_test(&self) -> McrResult<()>;

            fn ecdh_self_test(&self) -> McrResult<()>;

            fn rsa_mod_exp_self_test(&self) -> McrResult<Vec<u8>>;

            fn rsa_mod_exp_crt_self_test(&self) -> McrResult<()>;

            fn begin_memory_wipe(&self, tag: u16) -> McrResult<()>;

            fn end_memory_wipe(&self, tag: u16) -> McrResult<()>;
        }

        impl Clone for MockPka {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockPctEngine {}

        impl PctEngine for MockPctEngine {
            fn peek_tag(&self) -> Option<TagId>;

            fn begin_montgomery_constant_calculation(
                &mut self,
                tag: u16,
                curve: PkaEccCurve,
            ) -> McrResult<()>;

            fn end_montgomery_constant_calculation(&mut self, tag: TagId) -> McrResult<()>;

            fn begin_ecdh_compute_zc(&mut self, tag: TagId, curve: PkaEccCurve,
                                     priv_blob: &[u8], pub_blob: &IoMemRange) -> McrResult<PkaEccCmd>;

            fn end_ecdh_compute(&self, tag: TagId, cmd: PkaEccCmd) -> McrResult<PkaEccSecretValue>;

            fn begin_ecc_sign_zc(&mut self, tag: TagId, curve: PkaEccCurve,
                                 priv_blob: &[u8], digest: &IoMemRange, sig_out: &IoMemRange) -> McrResult<PkaEccCmd>;

            fn end_ecc_sign_zc(&mut self, tag: TagId) -> McrResult<()>;

            fn begin_ecc_verify_zc(&mut self, tag: TagId, curve: PkaEccCurve,
                                   pub_blob: &IoMemRange, digest: &IoMemRange, sig: &IoMemRange) -> McrResult<()>;

            fn end_ecc_verify_zc(&self, tag: TagId) ->McrResult<bool>;

            fn sha_single_block_zc(&mut self, mode: ShaMode, input: &IoMemRange, output: &mut IoMemRange) -> McrResult<()>;
        }
    }

    mock! {
        pub(crate) MockSha {}

        impl ShaTrait for MockSha {
            fn digest_zc<'a>(&self, command_info: &ShaDigestCmdInfoZc<'a>) -> McrResult<()>;

            fn hmac(
                &self,
                key: &[u8],
                data: &[u8],
                sha_mode: ShaMode,
                in_buf: &mut IoMemRange,
                out_buf: &mut IoMemRange,
            ) -> McrResult<()>;

            fn hkdf<'a>(
                &self,
                hkdf_info: HkdfInfo<'a>,
                sha_mode: ShaMode,
                prk_buf: &mut IoMemRange,
                in_buf: &mut IoMemRange,
                output: &mut [u8],
            ) -> McrResult<()>;

            fn kbkdf_counter_hmac<'a>(
                &self,
                kbkdf_info: KbkdfInfo<'a>,
                sha_mode: ShaMode,
                in_buf: &mut IoMemRange,
                output: &mut [u8],
            ) -> McrResult<()>;

            fn hkdf_self_test_256(&self) -> McrResult<()>;

            fn kbkdf_self_test_512(&self) -> McrResult<()>;

            fn decode_oaep_kek(
                &self,
                unwrapped_data: &[u8],
                hash_alg: HashAlgorithm,
            ) -> McrResult<SecureByteVec>;

            fn decode_oaep_kek_self_test(
                &self,
                unwrapped_data: &[u8],
                hash_alg: HashAlgorithm,
            ) -> McrResult<()>;
        }


        impl Clone for MockSha {
            fn clone(&self) -> Self;
        }
    }
}

mod cpu_info {
    use super::*;

    mock! {
        pub(crate) MockCpuInfo {}

        impl CpuInfoTrait for MockCpuInfo {
            fn run_fp_io_cores(&self, run: bool);
        }

        impl Clone for MockCpuInfo {
            fn clone(&self) -> Self;
        }
    }
}

mod debug_log_sender {
    use super::*;

    mock! {
        pub(crate) MockDebugLogSender {}

        impl DebugLogSenderTrait for MockDebugLogSender {

            fn send(&self, debug_log_entry: DebugLogEntryParameters);
        }


        impl Clone for MockDebugLogSender {
            fn clone(&self) -> Self;
        }
    }
}

mod tcon {
    use super::*;

    mock! {
        pub(crate) MockTcon {}

        impl TconTrait for MockTcon {
            fn tsc() -> u64;
        }

        impl Clone for MockTcon {
            fn clone(&self) -> Self;
        }
    }
}

mod simplex {
    use super::*;

    mock! {
        pub(crate) MockSimplexPipe<T: Clone + Copy> {}

        impl<T: Copy + Clone> SimplexPipeTrait<T> for MockSimplexPipe<T> {

            fn send(&self, msg: T) -> McrResult<()>;

            fn recv(&self) -> Option<T>;

            fn peek(&self) -> Option<T>;

            fn is_empty(&self) -> bool;

            fn is_full(&self) -> bool;

            fn empty_slot_count(&self) -> usize;
        }

        impl<T: Clone + Copy> Clone for MockSimplexPipe<T> {
            fn clone(&self) -> Self;
        }
    }
}

#[mockall_double::double]
pub(crate) use channel::MockDmaChannel;
#[mockall_double::double]
pub(crate) use channel::MockIoChannel;
#[mockall_double::double]
pub(crate) use channel::MockIpcEventChannel;
#[mockall_double::double]
pub(crate) use channel::MockIpcMessageChannel;
#[mockall_double::double]
pub(crate) use cpu_info::MockCpuInfo;
#[mockall_double::double]
pub(crate) use crypto::MockAes;
#[mockall_double::double]
pub(crate) use crypto::MockPka;
#[mockall_double::double]
pub(crate) use crypto::MockRng;
#[mockall_double::double]
pub(crate) use crypto::MockSha;
#[mockall_double::double]
pub(crate) use env::MockEnv;
#[mockall_double::double]
pub(crate) use env::MockHal;
#[mockall_double::double]
pub(crate) use env::MockMailboxController;
pub(crate) use heap::MockDmaAlloc;
#[mockall_double::double]
pub(crate) use heap::MockDmaHeap;
#[mockall_double::double]
pub(crate) use partition::MockPartition;
#[mockall_double::double]
pub(crate) use partition::MockUserSession;
#[mockall_double::double]
pub(crate) use simplex::MockSimplexPipe;
#[mockall_double::double]
pub(crate) use tcon::MockTcon;
