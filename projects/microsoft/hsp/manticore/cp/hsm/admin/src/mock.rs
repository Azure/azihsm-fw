// Copyright (c) Microsoft Corporation. All rights reserved.

cfg_if::cfg_if! {
    if #[cfg(test)] {
        use mockall::*;
        use mockall::predicate::*;
    }
}

use core::alloc::Layout;
use core::ops::Range;
use core::ptr::NonNull;

use mcr_alloc::*;
use mcr_crypto_cdma_io::*;
use mcr_crypto_rng::*;
use mcr_crypto_softaes::SoftAesTrait;
use mcr_doe::*;
use mcr_error::McrResult;
use mcr_gdma_controller::*;
use mcr_io_controller::IoChannelTrait;
use mcr_io_controller::IoControllerTrait;
use mcr_ipc_controller::*;
use mcr_logging::DebugLogSenderTrait;
use mcr_msix_controller::*;
use mcr_pcie_controller::*;
use mcr_queue_controller::*;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_simplex::SimplexPipeTrait;
use mcr_soc::*;
use mcr_tcon::*;
use mcr_types::*;

use crate::error::AdminErr;
use crate::function::*;
use crate::AdminEnvTrait;

mod controller {
    use super::*;

    mock! {
        pub(crate) MockAdminEnvTrait {}

        impl AdminEnvTrait for MockAdminEnvTrait {
            type DmaChannel = MockDmaChannel;
            type DmaHeap = MockDmaHeap;
            type FunctionMgr = MockFunctionMgr;
            type IoChannel=MockIoChannel;
            type IoController=MockIoController;
            type IpcChannel=MockIpcMessageChannel;
            type IpcEventChannel=MockIpcEventChannel;
            type MsixController=MockMsixController;
            type PcieController=MockPcieController;
            type PcieDoe=MockPcieDoe;
            type SocInfo=MockSocInfo;
            type Tcon=MockTcon;
            type QueueDeleteResp=MockSimplexPipe<QueueDeleteResponse>;
            type SoftAesReq=MockSimplexPipe<SoftAesOffloadReq>;
            type SoftAesResp=MockSimplexPipe<SoftAesOffloadResp>;
            type SelfTestReq=MockSimplexPipe<SelfTestReqPacket>;
            type SelfTestResp=MockSimplexPipe<SelfTestRespPacket>;
            type SoftAes=MockSoftAes;
            type CdmaIo=MockCdmaIo;
            type AesGcmIvQueue=MockSimplexPipe<AesGcmIV>;
            type AesGcmReqQueue=MockSimplexPipe<AesGcmReqEntry>;
            type AesGcmRespQueue=MockSimplexPipe<AesGcmRespEntry>;
            type GetBulkKeyReqQueue=MockSimplexPipe<GetBulkKeyReqEntry>;
            type GetBulkKeyRespQueue=MockSimplexPipe<GetBulkKeyRespEntry>;
            type Rng=MockRng;

            fn pcie_cntrl(&self) -> &controller::MockPcieController;

            fn pcie_doe(&self) -> &controller::MockPcieDoe;

            fn msix_cntrl(&self) -> &controller::MockMsixController;

            fn io_controller(&self) -> &controller::MockIoController;

            fn fp_io_controller(&self) -> &controller::MockIoController;

            fn io_channel(&self) -> &controller::MockIoChannel;

            fn admin_to_fp_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn fp_ipc_event_channel(&self) -> &controller::MockIpcEventChannel;

            fn hsm_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn hsm_ipc_event_channel(&self) -> &controller::MockIpcEventChannel;

            fn admin_to_hsp_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn hsp_to_admin_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn hsp_to_admin_stop_interface_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn hsm_to_admin_ipc_channel(&self) -> &controller::MockIpcMessageChannel;

            fn function_mgr(&self) -> &controller::MockFunctionMgr;

            fn dma_channel(&self) -> &controller::MockDmaChannel;

            fn dma_heap(&self) -> &controller::MockDmaHeap;

            fn soc_info(&self) -> &controller::MockSocInfo;

            fn tcon_tsc(&self) -> u64;

            fn deferred_queue_delete_pipe(&self) -> &controller::MockSimplexPipe<QueueDeleteResponse>;

            fn soft_aes_req(&self) -> &controller::MockSimplexPipe<SoftAesOffloadReq>;

            fn soft_aes_resp(&self) -> &controller::MockSimplexPipe<SoftAesOffloadResp>;

            fn update_core_liveliness(&self);

            fn self_test_req(&self) -> &controller::MockSimplexPipe<SelfTestReqPacket>;

            fn self_test_resp(&self) -> &controller::MockSimplexPipe<SelfTestRespPacket>;

            fn soft_aes(&self) -> &controller::MockSoftAes;

            fn cdma_io(&self) -> &controller::MockCdmaIo;

            fn self_test_key_table(&self) -> &[Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE];

            fn aes_gcm_iv_queue(&self) -> &controller::MockSimplexPipe<AesGcmIV>;

            fn aes_gcm_req_queue(&self) -> &controller::MockSimplexPipe<AesGcmReqEntry>;

            fn aes_gcm_resp_queue(&self) -> &controller::MockSimplexPipe<AesGcmRespEntry>;

            fn get_bulk_key_req_queue(&self) -> &controller::MockSimplexPipe<GetBulkKeyReqEntry>;

            fn get_bulk_key_resp_queue(&self) -> &controller::MockSimplexPipe<GetBulkKeyRespEntry>;

            fn rng(&self) -> &controller::MockRng;

            fn notify_self_test_failure(&self, test_id: SelfTest);

            fn hsm_part_persistent_store_addr(&self) -> usize;

            fn pause_queue_controller(&self);

            fn resume_queue_controller(&self);
        }

        impl Clone for MockAdminEnvTrait {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock SoftAes
        pub(crate) MockSoftAes {}

        impl SoftAesTrait for MockSoftAes {
            fn key_unwrap_inplace(&self, kek: &[u8], input: &mut [u8]) -> McrResult<Range<usize>>;

            fn ecb_decrypt(&self, key: &[u8], inout: &mut [u8]) -> McrResult<Range<usize>>;

            #[allow(clippy::too_many_arguments)]
            fn aes_gcm_tag_correction<'a>(
                &self,
                encrypt: bool,
                key: &'a [u8],
                iv: &'a [u8; 12],
                aad_len: u64,
                text_len: u64,
                aad: Option<&'a [u8]>,
                bad_tag: Option<&'a [u8; 16]>,
                unaligned_input_block: &'a [u8],
                aligned_input_len: usize,
                output: &'a mut [u8],
            ) -> McrResult<[u8; 16]>;

            fn aes_ecb_256_decrypt_self_test(&self) -> McrResult<()>;

            fn aes_256_key_unwrap_self_test(&self) -> McrResult<()>;
        }

        impl Clone for MockSoftAes {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock DebugLogSender
        pub(crate) MockDebugLogSender {}

        impl DebugLogSenderTrait for MockDebugLogSender {

            fn send(&self, debug_log_entry: DebugLogEntryParameters);
        }


        impl Clone for MockDebugLogSender {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Cdma IO
        pub(crate) MockCdmaIo {}

        impl CdmaIoTrait for MockCdmaIo {
            fn import_key(&self, key_slice: &[u32], vault_id: u8) -> McrResult<AesBulk256KeyId>;

            fn delete_key(&self, key_id: AesBulk256KeyId) -> McrResult<()>;

            fn clear_key_vault(&self) -> McrResult<()>;

            fn get_entry(&self, key_id: AesBulk256KeyId) -> McrResult<SecureByteArray<32>>;

            fn begin_enc_dec(
                &self,
                tag_id: u16,
                cdma_io_config: &CdmaIoConfig,
                input_text: &[u8],
            ) -> McrResult<()>;

            fn end_enc_dec(
                &self,
                tag_id: u16,
                cdma_io_config: &CdmaIoConfig,
                output_text: &mut [u8],
            ) -> McrResult<Option<GcmTag>>;

            fn zeroize_buffers(&self);
        }

        impl Clone for MockCdmaIo {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Rng
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
        /// Mock PCie controller
        pub(crate) MockPcieController {}

        impl PcieControllerTrait for MockPcieController {
            fn perst_up(&self) -> McrResult<()>;

            fn perst_down(&self);

            fn link_status(&self) -> McrResult<PcieLinkStatus>;

            fn complete_flr(&self, _pfn: PcieFunction);
        }

        impl Clone for MockPcieController {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock PCIe DOE Interface
        pub(crate) MockPcieDoe {}

        impl PcieDoeTrait for MockPcieDoe {
            fn recv(&self) -> McrResult<()>;

            fn end_recv(&self) -> McrResult<()>;

            fn send(&self) -> McrResult<()>;

            fn end_send(&self);

            fn abort(&self);

            fn reset(&self);

            fn set_err(&self);

            fn set_busy(&self, value: bool);

            fn buffer_addr(&self) -> MemoryAddr;
        }

        impl Clone for MockPcieDoe {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Msix controller
        pub(crate) MockMsixController {}

        impl MsixControllerTrait for MockMsixController {
            fn enable_pcie_fn(&self, pfn: PcieFunction);
        }

        impl Clone for MockMsixController {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock IO Controller
        pub(crate) MockIoController {}

        impl IoControllerTrait for MockIoController {

            fn pause_inbound(&self);

            fn resume_inbound(&self);
        }

        impl Clone for MockIoController {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock IO Channel
        pub(crate) MockIoChannel {}

        impl IoChannelTrait for MockIoChannel {

            fn begin_recv(&self) -> Option<mcr_io_controller::IoRxDesc>;

            fn end_recv(&self, addr: u32, sq_id: DevSqId);

            fn begin_send<'a>(&self, desc: &mcr_io_controller::IoTxDesc<'a>) -> McrResult<()>;

            fn peek_tag(&self) -> Option<u16>;

            fn end_send(&self) -> Option<mcr_io_controller::IoTxCompleteDesc>;
        }

        impl Clone for MockIoChannel {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Ipc Message Channel
        pub(crate) MockIpcMessageChannel {}

        impl IpcMessageChannelTrait for MockIpcMessageChannel {
            fn send_request(
                &self,
                _tag: u16,
                _message: mcr_ipc_controller::IpcMessage,
            ) -> McrResult<()>;

            fn send_response(&self, message: IpcMessage) -> McrResult<()>;

            fn receive_message(&self) -> Option<mcr_ipc_controller::IpcMessage>;

            fn peek_tag(&self) -> Option<u16>;

            fn poll_message(&self) -> Option<mcr_ipc_controller::IpcMessage>;
        }

        impl Clone for MockIpcMessageChannel {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Ipc Event Channel
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

    mock! {
        /// Mock Queue Controller
        pub(crate) MockQueueController{}

        impl QueueControllerTrait for MockQueueController {

            fn from_id_with_init(cntrl: QueueCntrlId) ->Self;

            fn from_id(cntrl: QueueCntrlId) -> Self;

            fn id(&self) -> QueueCntrlId;

            fn enabled(&self) -> bool;

            fn ready(&self) -> bool;

            fn enable(&self);

            fn disable(&self);

            fn clear_enable(&self);

            fn set_enable(&self);

            fn reset(&self);

            fn create_asq(&self, dev_sq: DevSqId, host_sq: HostSqId) -> McrResult<()>;

            fn create_acq(&self, dev_cq: DevCqId, host_cq: HostCqId) -> McrResult<()>;

            fn create_sq(&self, dev_sq: DevSqId, host_sq: HostSqId, mem: QueueMem) -> McrResult<()>;

            fn delete_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

            fn create_cq(
                &self,
                dev_cq: DevCqId,
                host_cq: HostCqId,
                mem: QueueMem,
                irq: Option<u16>,
            ) -> McrResult<()>;

            fn delete_cq(&self, dev_cq: DevCqId, host_cq: HostCqId);

            fn set_cfs(&self);

            fn host_register_info(&self) -> ControllerLmInfo;

            fn restore_host_register_info(&self, info: &ControllerLmInfo);

            fn sq_info(&self, dev_sq: DevSqId, host_sq: HostSqId) -> LmSqInfo;

            fn restore_sq_info(&self, dev_sq: DevSqId, info: &LmSqInfo) -> McrResult<()>;

            fn cq_info(&self, dev_cq: DevCqId, host_cq: HostCqId) -> LmCqInfo;

            fn restore_cq_info(&self, dev_cq: DevCqId, info: &LmCqInfo) -> McrResult<()>;

            fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

            fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);
        }

        impl Clone for MockQueueController{
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Function
        pub(crate) MockFunction {}

        impl FunctionTrait for MockFunction {
            fn query_state_change(&self) -> CntrlStateChangeAction;

            fn cntrl_id(&self) -> QueueCntrlId;

            fn enable(&self) -> McrResult<()>;

            fn disable(&self);

            fn clear_enable(&self);

            fn reset(&self);

            fn ready(&self) -> bool;

            fn enabled(&self) -> bool;

            fn set_res_cnt(&self, cnt: u32);

            fn res_cnt(&self) -> u32;

            fn res_mask(&self) -> [u8; 16];

            fn create_cq(&self, host_cq: HostCqId, mem: QueueMem, irq: Option<u16>) -> Result<(), AdminErr>;

            fn delete_cq(&self, host_cq: HostCqId) -> Result<(), AdminErr>;

            fn create_sq(
                &self,
                host_sq: HostSqId,
                host_cq: HostCqId,
                mem: QueueMem,
            ) -> Result<(DevSqId, DevCqId), AdminErr>;

            fn delete_sq(&self, host_sq: HostSqId) -> Result<(DevSqId, DevCqId), AdminErr>;

            fn set_cfs(&self);

            fn dev_sq(&self, host_sq: HostSqId) -> Result<DevSqId, AdminErr>;

            fn admin_queue(&self) -> Option<AdminQueue>;

            fn save_lm_context(
                &self,
                buf: &mut VmLiveMigrationInfo,
                session_allocation_mask: u8,
                masked_bk_boot: &MaskedBkBoot,
                sealed_bk3: &SealedBk3,
            );

            fn restore_lm_context(&self, buf: &mut VmLiveMigrationInfo) -> Result<(), AdminErr>;

            fn get_enabled_sq_info(&self) -> Vec<(HostSqId, DevSqId, DevCqId)>;

            fn enable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

            fn disable_sq(&self, dev_sq: DevSqId, host_sq: HostSqId);

            fn complete_live_migration(&self);
        }

        impl Clone for MockFunction {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        /// Mock Function Manager
        pub(crate) MockFunctionMgr {}

        impl FunctionMgrTrait for MockFunctionMgr {

            type Function = crate::mock::MockFunction;

            fn reset(&self);

            fn function(&self, func: PcieFunction) -> crate::mock::MockFunction;

            fn set_res_cnt(&self, func: PcieFunction, cnt: u32) -> Result<u32, AdminErr>;

            fn prepare_for_warm_boot(&self);
        }
    }

    mock! {
        /// Mock DMA channel
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
        /// Mock SoC information
        pub(crate) MockSocInfo {}

        impl SocInfoTrait for MockSocInfo {
            fn fw_version(&self) -> [u8; 32];

            fn id(&self) -> [u8; 32];

            fn svn(&self) -> [u8; 8];

            fn reset_type(&self) -> SocResetType;

            fn reset_gdma(&self);

            fn reset_nvme(&self);

            #[cfg(feature = "fips_validation_hooks")]
            fn set_negative_cast_hooks(&self, id: SelfTest);
        }

        impl Clone for MockSocInfo {
            fn clone(&self) -> Self;
        }
    }

    mock! {
        pub(crate) MockTcon {}

        impl TconTrait for MockTcon {
            fn tsc() -> u64;
        }

        impl Clone for MockTcon {
            fn clone(&self) -> Self;
        }
    }

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

        impl<T: Copy + Clone> Clone for MockSimplexPipe<T> {
            fn clone(&self) -> Self;
        }
    }
}

mod heap {
    use super::*;

    pub(crate) struct MockDmaAlloc {
        vec: Vec<u8>,
    }

    impl DmaAllocTrait for MockDmaAlloc {
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

        impl DmaHeapTrait for MockDmaHeap {
            type Alloc = MockDmaAlloc;

            fn allocate(&self, len: usize) -> Option<<Self as DmaHeapTrait>::Alloc>;

            fn size(&self) -> usize;

            fn free(&self) -> usize;

            fn deallocate(&self, ptr: NonNull<u8>, layout: Layout);
        }

        impl Clone for MockDmaHeap {
            fn clone(&self) -> Self;
        }
    }
}

#[mockall_double::double]
pub(crate) use controller::MockAdminEnvTrait;
#[mockall_double::double]
pub(crate) use controller::MockCdmaIo;
#[mockall_double::double]
pub(crate) use controller::MockDmaChannel;
#[mockall_double::double]
pub(crate) use controller::MockFunction;
#[mockall_double::double]
pub(crate) use controller::MockFunctionMgr;
#[mockall_double::double]
pub(crate) use controller::MockIoChannel;
#[mockall_double::double]
pub(crate) use controller::MockIoController;
#[mockall_double::double]
pub(crate) use controller::MockIpcEventChannel;
#[mockall_double::double]
pub(crate) use controller::MockIpcMessageChannel;
#[mockall_double::double]
pub(crate) use controller::MockMsixController;
#[mockall_double::double]
pub(crate) use controller::MockPcieController;
#[mockall_double::double]
pub(crate) use controller::MockPcieDoe;
#[mockall_double::double]
pub(crate) use controller::MockQueueController;
#[mockall_double::double]
pub(crate) use controller::MockRng;
#[mockall_double::double]
pub(crate) use controller::MockSimplexPipe;
#[mockall_double::double]
pub(crate) use controller::MockSocInfo;
#[mockall_double::double]
pub(crate) use controller::MockSoftAes;
#[mockall_double::double]
pub(crate) use controller::MockTcon;
pub(crate) use heap::MockDmaAlloc;
#[mockall_double::double]
pub(crate) use heap::MockDmaHeap;
