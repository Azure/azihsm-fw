// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_alloc::*;
use mcr_cpu::CpuId;
use mcr_crypto_cdma_io::*;
use mcr_crypto_rng::*;
use mcr_crypto_softaes::*;
use mcr_doe::*;
use mcr_dtcm_controller::DtcmController;
use mcr_error::McrResult;
use mcr_gdma_controller::*;
use mcr_gsram_controller::GsramController;
use mcr_interrupt_controller::*;
use mcr_io_controller::*;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_itcm_controller::ItcmController;

use mcr_logging::*;
use mcr_mailbox_controller::MailboxController;
use mcr_mailbox_controller::MailboxControllerTrait;
use mcr_mailbox_controller::MailboxId;
use mcr_mem_map::*;
use mcr_msix_controller::*;
use mcr_pcie_controller::*;
use mcr_queue_controller::*;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::NegKind;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::PreopsNegativeTest;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_simplex::*;
use mcr_soc::*;
use mcr_tcon::*;
use mcr_types::*;
use zeroize::Zeroize;

use crate::error;
use crate::error::AdminErr;
use crate::function::*;
use crate::preop_cdma_io::*;

pub(crate) trait AdminEnvTrait: Clone {
    type DmaChannel: GdmaChannelTrait + Clone;
    type DmaHeap: DmaHeapTrait + Clone;
    type FunctionMgr: FunctionMgrTrait;
    type IoChannel: IoChannelTrait + Clone;
    type IoController: IoControllerTrait + Clone;
    type IpcChannel: IpcMessageChannelTrait + Clone;
    type IpcEventChannel: IpcEventChannelTrait + Clone;
    type MsixController: MsixControllerTrait + Clone;
    type PcieController: PcieControllerTrait + Clone;
    type PcieDoe: PcieDoeTrait + Clone;
    type SocInfo: SocInfoTrait + Clone;
    type Tcon: TconTrait + Clone;
    type QueueDeleteResp: SimplexPipeTrait<QueueDeleteResponse> + Clone;
    type SoftAesReq: SimplexPipeTrait<SoftAesOffloadReq> + Clone;
    type SoftAesResp: SimplexPipeTrait<SoftAesOffloadResp> + Clone;
    type SelfTestReq: SimplexPipeTrait<SelfTestReqPacket> + Clone;
    type SelfTestResp: SimplexPipeTrait<SelfTestRespPacket> + Clone;
    type AesGcmIvQueue: SimplexPipeTrait<AesGcmIV> + Clone;
    type AesGcmReqQueue: SimplexPipeTrait<AesGcmReqEntry> + Clone;
    type AesGcmRespQueue: SimplexPipeTrait<AesGcmRespEntry> + Clone;
    type SoftAes: SoftAesTrait + Clone;
    type CdmaIo: CdmaIoTrait + Clone;
    type Rng: RngTrait + Clone;

    /// Get the PCIe Controller
    fn pcie_cntrl(&self) -> &Self::PcieController;

    /// Get the PCIe DOE
    fn pcie_doe(&self) -> &Self::PcieDoe;

    /// Get the Msix Controller
    fn msix_cntrl(&self) -> &Self::MsixController;

    /// Get the CP IO Controller
    fn io_controller(&self) -> &Self::IoController;

    /// Get the FP IO Controller
    fn fp_io_controller(&self) -> &Self::IoController;

    /// Get the IO Channel
    fn io_channel(&self) -> &Self::IoChannel;

    /// Get cp-admin and fp IPC message channel
    fn admin_to_fp_ipc_channel(&self) -> &Self::IpcChannel;

    /// Get FP IPC event channel
    fn fp_ipc_event_channel(&self) -> &Self::IpcEventChannel;

    /// Get cp-admin and hsm IPC message channel
    fn hsm_ipc_channel(&self) -> &Self::IpcChannel;

    /// Get HSM IPC event channel
    fn hsm_ipc_event_channel(&self) -> &Self::IpcEventChannel;

    /// Get cp-admin and hsp IPC message channel
    fn admin_to_hsp_ipc_channel(&self) -> &Self::IpcChannel;

    /// Get hsm to cp-admin IPC message channel
    fn hsm_to_admin_ipc_channel(&self) -> &Self::IpcChannel;

    /// Get hsp to cp-admin IPC message channel
    fn hsp_to_admin_ipc_channel(&self) -> &Self::IpcChannel;

    /// Get hsp to cp-admin stop interface IPC message channel
    fn hsp_to_admin_stop_interface_ipc_channel(&self) -> &Self::IpcChannel;

    // Get the Function Manager
    fn function_mgr(&self) -> &Self::FunctionMgr;

    /// Get the DMA channel
    fn dma_channel(&self) -> &Self::DmaChannel;

    /// Get DMA Heap
    fn dma_heap(&self) -> &Self::DmaHeap;

    /// SoC information
    fn soc_info(&self) -> &Self::SocInfo;

    /// Get the timestamp counter
    fn tcon_tsc(&self) -> u64;

    /// Get the simplex pipe to get queue delete indication
    fn deferred_queue_delete_pipe(&self) -> &Self::QueueDeleteResp;

    /// Get the SoftAes request from IO core
    fn soft_aes_req(&self) -> &Self::SoftAesReq;

    /// Send an SoftAes response to IO core
    fn soft_aes_resp(&self) -> &Self::SoftAesResp;

    /// Set core liveliness indicator
    fn update_core_liveliness(&self);

    /// Get the self-test request pipe
    fn self_test_req(&self) -> &Self::SelfTestReq;

    /// Get the self-test response pipe
    fn self_test_resp(&self) -> &Self::SelfTestResp;

    /// Get Soft AES instance
    fn soft_aes(&self) -> &Self::SoftAes;

    /// Get Cdma IO instance
    fn cdma_io(&self) -> &Self::CdmaIo;

    // Cdma IO self test key table
    fn self_test_key_table(&self) -> &[Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE];

    /// Notify self test failure to SP
    ///
    /// # Arguments
    ///
    /// * `test_id` - SelfTest identified by SelfTest enum
    fn notify_self_test_failure(&self, test_id: SelfTest);

    /// Return the HSM partition persistent store address
    fn hsm_part_persistent_store_addr(&self) -> usize;

    /// Get GCM IV Queue pipe
    fn aes_gcm_iv_queue(&self) -> &Self::AesGcmIvQueue;

    /// Get GCM Request queue pipe
    fn aes_gcm_req_queue(&self) -> &Self::AesGcmReqQueue;

    /// Get GCM Response queue pipe
    fn aes_gcm_resp_queue(&self) -> &Self::AesGcmRespQueue;

    /// Get RNG instance
    fn rng(&self) -> &Self::Rng;

    /// Pause Queue Controller
    fn pause_queue_controller(&self);

    /// Resume Queue Controller
    fn resume_queue_controller(&self);
}

/// Admin Event Handler Environment
#[derive(Clone)]
pub(crate) struct AdminEnv {
    pcie_cntrl: PcieController,
    pcie_doe: PcieDoe,
    msix_cntrl: MsixController,
    io_channel: IoChannel,
    io_cntrl: IoController,
    fp_io_cntrl: IoController,
    admin_to_fp_ipc_channel: IpcMessageChannel,
    fp_ipc_event_channel: IpcEventChannel,
    _fp_io_channel_0: IoProxyChannel,
    _fp_io_channel_1: IoProxyChannel,
    hsm_ipc_channel: IpcMessageChannel,
    hsm_ipc_event_channel: IpcEventChannel,
    _hsm_io_channel: IoProxyChannel,
    admin_to_hsp_ipc_channel: IpcMessageChannel,
    hsm_to_admin_ipc_channel: IpcMessageChannel,
    hsp_to_admin_ipc_channel: IpcMessageChannel,
    hsp_to_admin_stop_interface_ipc_channel: IpcMessageChannel,
    function_mgr: FunctionMgr<QueueController>,
    dma_channel: GdmaChannel,
    dma_heap: DmaHeap,
    soc_info: SocInfo,
    deferred_queue_delete_pipe: SimplexPipe<QueueDeleteResponse>,
    soft_aes_req: SimplexPipe<SoftAesOffloadReq>,
    soft_aes_resp: SimplexPipe<SoftAesOffloadResp>,
    self_test_req: SimplexPipe<SelfTestReqPacket>,
    self_test_resp: SimplexPipe<SelfTestRespPacket>,
    soft_aes: SoftAes,
    cdma_io: CdmaIo,
    self_test_key_table: [Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE],
    aes_gcm_iv_queue: SimplexPipe<AesGcmIV>,
    aes_gcm_req_queue: SimplexPipe<AesGcmReqEntry>,
    aes_gcm_resp_queue: SimplexPipe<AesGcmRespEntry>,
    rng: Rng,
}

impl AdminEnvTrait for AdminEnv {
    type DmaChannel = GdmaChannel;
    type DmaHeap = DmaHeap;
    type FunctionMgr = FunctionMgr<QueueController>;
    type IoChannel = IoChannel;
    type IoController = IoController;
    type IpcChannel = IpcMessageChannel;
    type IpcEventChannel = IpcEventChannel;
    type MsixController = MsixController;
    type PcieController = PcieController;
    type PcieDoe = PcieDoe;
    type SocInfo = SocInfo;
    type Tcon = Tcon;
    type QueueDeleteResp = SimplexPipe<QueueDeleteResponse>;
    type SoftAesReq = SimplexPipe<SoftAesOffloadReq>;
    type SoftAesResp = SimplexPipe<SoftAesOffloadResp>;
    type SelfTestReq = SimplexPipe<SelfTestReqPacket>;
    type SelfTestResp = SimplexPipe<SelfTestRespPacket>;
    type SoftAes = SoftAes;
    type CdmaIo = CdmaIo;
    type AesGcmIvQueue = SimplexPipe<AesGcmIV>;
    type AesGcmReqQueue = SimplexPipe<AesGcmReqEntry>;
    type AesGcmRespQueue = SimplexPipe<AesGcmRespEntry>;
    type Rng = Rng;

    fn pcie_cntrl(&self) -> &Self::PcieController {
        &self.pcie_cntrl
    }

    fn pcie_doe(&self) -> &Self::PcieDoe {
        &self.pcie_doe
    }

    fn msix_cntrl(&self) -> &Self::MsixController {
        &self.msix_cntrl
    }

    fn io_controller(&self) -> &Self::IoController {
        &self.io_cntrl
    }

    fn fp_io_controller(&self) -> &Self::IoController {
        &self.fp_io_cntrl
    }

    fn io_channel(&self) -> &Self::IoChannel {
        &self.io_channel
    }

    fn admin_to_fp_ipc_channel(&self) -> &Self::IpcChannel {
        &self.admin_to_fp_ipc_channel
    }

    fn fp_ipc_event_channel(&self) -> &Self::IpcEventChannel {
        &self.fp_ipc_event_channel
    }

    fn hsm_ipc_channel(&self) -> &Self::IpcChannel {
        &self.hsm_ipc_channel
    }

    fn hsm_ipc_event_channel(&self) -> &Self::IpcEventChannel {
        &self.hsm_ipc_event_channel
    }

    fn admin_to_hsp_ipc_channel(&self) -> &Self::IpcChannel {
        &self.admin_to_hsp_ipc_channel
    }

    fn hsp_to_admin_ipc_channel(&self) -> &Self::IpcChannel {
        &self.hsp_to_admin_ipc_channel
    }

    fn hsp_to_admin_stop_interface_ipc_channel(&self) -> &Self::IpcChannel {
        &self.hsp_to_admin_stop_interface_ipc_channel
    }

    fn hsm_to_admin_ipc_channel(&self) -> &Self::IpcChannel {
        &self.hsm_to_admin_ipc_channel
    }

    fn function_mgr(&self) -> &Self::FunctionMgr {
        &self.function_mgr
    }

    fn dma_channel(&self) -> &Self::DmaChannel {
        &self.dma_channel
    }

    fn dma_heap(&self) -> &Self::DmaHeap {
        &self.dma_heap
    }

    fn soc_info(&self) -> &Self::SocInfo {
        &self.soc_info
    }

    fn tcon_tsc(&self) -> u64 {
        Tcon::tsc()
    }

    fn deferred_queue_delete_pipe(&self) -> &Self::QueueDeleteResp {
        &self.deferred_queue_delete_pipe
    }

    fn soft_aes_req(&self) -> &Self::SoftAesReq {
        &self.soft_aes_req
    }

    fn soft_aes_resp(&self) -> &Self::SoftAesResp {
        &self.soft_aes_resp
    }

    fn update_core_liveliness(&self) {
        Self::update_core_liveness();
    }

    fn self_test_req(&self) -> &Self::SelfTestReq {
        &self.self_test_req
    }

    fn self_test_resp(&self) -> &Self::SelfTestResp {
        &self.self_test_resp
    }

    fn soft_aes(&self) -> &Self::SoftAes {
        &self.soft_aes
    }

    fn cdma_io(&self) -> &Self::CdmaIo {
        &self.cdma_io
    }

    fn self_test_key_table(&self) -> &[Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE] {
        &self.self_test_key_table
    }

    fn aes_gcm_iv_queue(&self) -> &Self::AesGcmIvQueue {
        &self.aes_gcm_iv_queue
    }

    fn aes_gcm_req_queue(&self) -> &Self::AesGcmReqQueue {
        &self.aes_gcm_req_queue
    }

    fn aes_gcm_resp_queue(&self) -> &Self::AesGcmRespQueue {
        &self.aes_gcm_resp_queue
    }

    fn rng(&self) -> &Self::Rng {
        &self.rng
    }

    #[allow(unused_variables)]
    fn notify_self_test_failure(&self, test_id: SelfTest) {
        error!("Failed the self test {:?}", test_id as u32);

        // Notify the HSP about the error using the mailbox
        let mbx_err = MailboxController::create(MailboxId::Mailbox0);
        mbx_err.trigger_mbx_err();

        #[allow(clippy::empty_loop)]
        loop {}
    }

    fn hsm_part_persistent_store_addr(&self) -> usize {
        GsRamMemMap::hsm_part_persistent_store().as_ptr() as usize
    }

    fn pause_queue_controller(&self) {
        QueueController::pause();
    }

    fn resume_queue_controller(&self) {
        QueueController::resume();
    }
}

/// Admin Event Handler Environment
impl AdminEnv {
    /// Create an instance of Admin Event Handler Environment
    ///
    /// # Returns
    ///
    /// * `AdminEnv` - An instance of Admin Event Handler Environment or an appropriate error code
    pub(crate) fn new() -> McrResult<Self> {
        #[cfg(feature = "fips_validation_hooks")]
        Self::check_for_preop_cast_hooks();

        let soc_info = SocInfo::default();

        let reset_type = soc_info.reset_type();
        let fwupdate = reset_type == SocResetType::FwUpdateWarmReset;

        // Initialize TCON wakeup timer 1.
        Tcon::disable_wakeup_timer1();

        // Reset GDMA hardware and disable all GDMA interrupts
        GdmaController::reset_gdma();

        // Enable tcon wakeup 1 timer interrupt that serves a crash trigger mechanism
        // from a crashing SoC core to all other SoC cores.
        InterruptController::default().clear(Interrupt::tcon_wakeup1_irq);
        InterruptController::default().enable(Interrupt::tcon_wakeup1_irq);

        // Cross-core fault detection and recovery
        DtcmController::int_en(CpuId::Hsm);
        InterruptController::default().clear(Interrupt::cp1_dtcm_err_irq);
        InterruptController::default().enable(Interrupt::cp1_dtcm_err_irq);

        // Initialize GSRAM ECC Controller
        GsramController::global_gsram_ecc_init();

        // Clear and enable GSRAM ECC interrupt
        InterruptController::default().clear(Interrupt::gsram_irq);
        InterruptController::default().enable(Interrupt::gsram_irq);

        // ITCM ECC error IRQ register
        ItcmController::int_en(CpuId::Admin);
        InterruptController::default().clear(Interrupt::cp0_itcm_err_irq);
        InterruptController::default().enable(Interrupt::cp0_itcm_err_irq);

        // Enable GDMA error interrupts
        InterruptController::default().clear(Interrupt::gdma_err_irq);
        InterruptController::default().enable(Interrupt::gdma_err_irq);

        // Enable UCD IO Inbound error interrupts
        InterruptController::default().clear(Interrupt::ucd_ib_err_irq);
        InterruptController::default().enable(Interrupt::ucd_ib_err_irq);

        // Enable UCD IO Outbound error interrupts
        InterruptController::default().clear(Interrupt::ucd_ob_err_irq);
        InterruptController::default().enable(Interrupt::ucd_ob_err_irq);

        // Reset the following IPs in the Manticore SoC during WarmReset due to RAS & impactful
        // update path.
        //
        // Note: For POR, these IPs are designed to be reset of the SoC reset. For
        // FirmwareUpdateWarm, all the SoC hardware IPs are expected to retain their
        // state.
        if !fwupdate {
            // Note: 1SP zeroizes all GSRAM on Power On Reset.
            let cdma_key_vault = CdmaMemMap::key_vault();
            cdma_key_vault.zeroize();
        }
        soc_info.reset_gdma();

        let io_cntrl = if fwupdate {
            IoController::new(IoControllerId::Core0)
        } else {
            IoController::new_with_enable(IoControllerId::Core0)?
        };

        let fp_io_cntrl = if fwupdate {
            IoController::new(IoControllerId::Core1)
        } else {
            IoController::new_with_enable(IoControllerId::Core1)?
        };

        let ipc_cntrl = IpcController::new(IpcIntBlock::IntBlock0, Interrupt::IpcSgiCore0);

        let fp_ipc_event_channel = ipc_cntrl.create_event_channel(
            IpcChannelId::AdminToFpIoCore,
            Self::admin_to_fp_ipc_event_channel_config(),
        );

        let hsm_ipc_event_channel = ipc_cntrl.create_event_channel(
            IpcChannelId::AdminToHsmIoCore,
            Self::admin_to_hsm_ipc_event_channel_config(),
        );

        let function_mgr = match reset_type {
            SocResetType::Por => FunctionMgr::new(
                QueueController::from_id_with_init,
                GsRamMemMap::admin_pcie_resource_table(),
                GsRamMemMap::admin_pcie_resource_table_crc(),
            )?,
            SocResetType::FwUpdateWarmReset | SocResetType::WarmReset => FunctionMgr::new(
                QueueController::from_id,
                GsRamMemMap::admin_pcie_resource_table(),
                GsRamMemMap::admin_pcie_resource_table_crc(),
            )?,
        };

        let dma_cntrl = GdmaController::new_with_enable()?;
        let dma_channel =
            dma_cntrl.create_channel(GdmaChannelId::Channel0, Self::dma_channel_config())?;

        // Enable GDMA error interrupt
        GdmaController::enable_gdma_err_int();

        let heap_region = GsRamMemMap::admin_heap();

        let dma_heap = DmaHeap::new(heap_region.as_ptr() as usize, heap_region.len());

        let pcie_doe = PcieDoe::new(GsRamMemMap::doe_buffer());

        let msix_cntrl = MsixController::new();
        if reset_type == SocResetType::Por {
            msix_cntrl.enable();
        };

        if !fwupdate {
            QueueController::global_init();
        }

        let io_channel = if fwupdate {
            io_cntrl.open_channel(Self::io_channel_config())?
        } else {
            io_cntrl.create_channel(Self::io_channel_config())?
        };

        let admin_to_hsp_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::AdminToHsp,
            Self::admin_to_hsp_ipc_channel_config(),
        );

        let hsp_to_admin_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HspToAdmin,
            Self::hsp_to_admin_ipc_channel_config(),
        );

        let hsp_to_admin_stop_interface_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HspToAdmin,
            Self::hsp_to_admin_stop_interface_ipc_channel_config(),
        );

        let in_buf = GsRamMemMap::aes_bulk_self_test_src_buf();
        let out_buf = GsRamMemMap::aes_bulk_self_test_dest_buf();
        let cdma_io_key_table = &mut CdmaMemMap::key_vault()[SELF_TEST_KV_BASE_OFFSET
            ..SELF_TEST_KV_BASE_OFFSET + (MAX_KEYS_PER_TABLE * KEY_SIZE_IN_DWORDS)];
        let cdma_io = CdmaIo::new(cdma_io_key_table, in_buf, out_buf)?;

        let queue_delete_notification = SimplexPipe::new(SimplexPipeConfig {
            queue: GsRamMemMap::queue_delete_req_queue(),
            ci: GsRamMemMap::queue_delete_req_ci(),
            pi: GsRamMemMap::queue_delete_req_pi(),
        });

        let soft_aes_req = SimplexPipe::new(SimplexPipeConfig {
            queue: GsRamMemMap::soft_aes_req_queue(),
            ci: GsRamMemMap::soft_aes_req_ci(),
            pi: GsRamMemMap::soft_aes_req_pi(),
        });

        let soft_aes_resp = SimplexPipe::new(SimplexPipeConfig {
            queue: GsRamMemMap::soft_aes_resp_queue(),
            ci: SocMemMap::soft_aes_resp_ci(),
            pi: SocMemMap::soft_aes_resp_pi(),
        });

        let self_test_req = SimplexPipe::new(SimplexPipeConfig {
            queue: SocMemMap::self_test_req_queue(),
            ci: SocMemMap::self_test_req_ci(),
            pi: SocMemMap::self_test_req_pi(),
        });

        let self_test_resp = SimplexPipe::new(SimplexPipeConfig {
            queue: SocMemMap::self_test_resp_queue(),
            ci: SocMemMap::self_test_resp_ci(),
            pi: SocMemMap::self_test_resp_pi(),
        });

        let aes_gcm_iv_queue = SimplexPipe::new(SimplexPipeConfig {
            queue: SocMemMap::aes_gcm_iv_queue(),
            ci: SocMemMap::aes_gcm_iv_queue_head(),
            pi: SocMemMap::aes_gcm_iv_queue_tail(),
        });

        let aes_gcm_req_queue = SimplexPipe::new(SimplexPipeConfig {
            queue: SocMemMap::aes_gcm_req_queue(),
            ci: SocMemMap::aes_gcm_req_queue_head(),
            pi: SocMemMap::aes_gcm_req_queue_tail(),
        });

        let aes_gcm_resp_queue = SimplexPipe::new(SimplexPipeConfig {
            queue: SocMemMap::aes_gcm_resp_queue(),
            ci: SocMemMap::aes_gcm_resp_queue_head(),
            pi: SocMemMap::aes_gcm_resp_queue_tail(),
        });

        // IMPORTANT: No ELBI DOE registers should be read or written to prior to PcieController::new()
        // which sets PCIE_TOP_REG.PERST_N_DIS = true. Doing so results in an infinite ROM boot loop.
        let pcie_cntrl = PcieController::new_with_phy_init()?;

        let soft_aes = SoftAes::new();

        Tcon::init_wakeup_timer0();

        if soc_info.reset_type() == SocResetType::WarmReset {
            // Set the CFS bit to notify the driver to start recovery.
            function_mgr.prepare_for_warm_boot();
        }

        // Boot handshake
        let message: IpcMessageIoStateChange = Self::handle_sp_state_change_req(
            &hsp_to_admin_ipc_channel,
            IoProcessorState::PrepareRelease,
        )?;

        let (_hsm_io_channel, admin_to_hsm_ipc_channel, hsm_to_admin_ipc_channel) =
            Self::make_hsm_channels(&io_cntrl, &ipc_cntrl)?;

        let (_fp_io_channel_0, _fp_io_channel_1, admin_to_fp_ipc_channel) =
            Self::make_fp_channels(&fp_io_cntrl, &ipc_cntrl)?;

        Self::send_state_change_rsp(&hsp_to_admin_ipc_channel, message)?;

        let message: IpcMessageIoStateChange =
            Self::handle_sp_state_change_req(&hsp_to_admin_ipc_channel, IoProcessorState::Release)?;

        Self::set_io_state(&admin_to_hsm_ipc_channel, IoProcessorState::Start)?;

        Self::set_io_state(&admin_to_fp_ipc_channel, IoProcessorState::Start)?;

        // Wait for HSM to reach RUN state before responding to HSP for Release message
        Self::wait_for_hsm_run_state()?;

        Self::send_state_change_rsp(&hsp_to_admin_ipc_channel, message)?;

        // Init RNG after boot handshake to avoid handshake delays
        let rng = Rng::new_without_calibration();

        let mut env = Self {
            pcie_cntrl,
            pcie_doe,
            msix_cntrl,
            io_channel,
            io_cntrl,
            fp_io_cntrl,
            _fp_io_channel_0,
            _fp_io_channel_1,
            admin_to_fp_ipc_channel,
            fp_ipc_event_channel,
            hsm_ipc_channel: admin_to_hsm_ipc_channel,
            hsm_ipc_event_channel,
            hsm_to_admin_ipc_channel,
            _hsm_io_channel,
            admin_to_hsp_ipc_channel,
            hsp_to_admin_ipc_channel,
            hsp_to_admin_stop_interface_ipc_channel,
            function_mgr,
            dma_channel,
            dma_heap,
            soc_info,
            deferred_queue_delete_pipe: queue_delete_notification,
            soft_aes_req,
            soft_aes_resp,
            self_test_req,
            self_test_resp,
            soft_aes,
            cdma_io,
            self_test_key_table: [None; MAX_KEYS_PER_TABLE],
            aes_gcm_iv_queue,
            aes_gcm_req_queue,
            aes_gcm_resp_queue,
            rng,
        };

        // Run the Pre Operation self tests for FIPS
        env.preops_cast()?;

        if fwupdate {
            env.io_cntrl.resume_inbound();
            env.fp_io_cntrl.resume_inbound();
            QueueController::resume();
        }

        Ok(env)
    }

    fn wait_for_hsm_run_state() -> McrResult<()> {
        let start_tsc = Tcon::tsc();
        // Timeout after 0.5 second
        let timeout_cycles = Tcon::tsc_freq_hz() / 2;
        let elapsed_cycles = start_tsc + timeout_cycles as u64;

        loop {
            const RUN: u32 = IoProcessorBootState::Run.0;
            match GsRamMemMap::boot_status().get() {
                RUN => break,
                _ => {
                    if Tcon::tsc() > elapsed_cycles {
                        Err(AdminErr::HsmBootTimeout)?
                    }
                }
            }
        }

        Ok(())
    }

    fn io_channel_config() -> IoChannelConfig {
        // Admin Io channel configuration
        IoChannelConfig {
            channel_id: IoChannelId::Channel2,
            rx_queue: GsRamMemMap::admin_rx_queue(),
            rx_queue_pi: GsRamMemMap::admin_rx_queue_shadow_pi(),
            rx_free_list: GsRamMemMap::admin_rx_free_list(),
            rx_entry_pool_info: RxEntryPoolInfo::EntryPool(GsRamMemMap::admin_sqe_pool()),
            tx_queue: GsRamMemMap::admin_tx_queue(),
            tx_queue_pi: GsRamMemMap::admin_tx_queue_shadow_pi(),
            tx_free_list: GsRamMemMap::admin_tx_free_list(),
            tx_entry_pool_info: TxEntryPoolInfo::EntryPool(GsRamMemMap::admin_cqe_pool()),
            enable_irq: true,
        }
    }

    fn admin_to_fp_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::admin_to_fp_ipc_tx_queue(),
                ci: PsRamMemMap::admin_to_fp_ipc_tx_queue_ci(),
                pi: PsRamMemMap::admin_to_fp_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::admin_to_fp_ipc_rx_queue(),
                ci: PsRamMemMap::admin_to_fp_ipc_rx_queue_ci(),
                pi: PsRamMemMap::admin_to_fp_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor0,
            receive_message_descriptor: IpcDescriptor::Descriptor1,
        };
        // For queues with FP core, we zeroize both tx and rx indices.
        // The communication between Admin and FP starts with boot handshake.
        // There will be no response before Admin sends data in TX queue. Hence,
        // it is safe to set rx pointers to 0 as well.
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config.rx_queue.ci.set(0);
        config.rx_queue.pi.set(0);
        config
    }

    fn admin_to_fp_ipc_event_channel_config() -> IpcEventChannelConfig {
        let send_event_mask = 1 << IpcDescriptor::Descriptor17 as u32;
        let receive_event_mask = 1 << IpcDescriptor::Descriptor18 as u32;

        IpcEventChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            send_event_mask,
            receive_event_mask,
        }
    }

    fn admin_to_hsm_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsm_ipc_tx_queue(),
                ci: GsRamMemMap::admin_to_hsm_ipc_tx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsm_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsm_ipc_rx_queue(),
                ci: GsRamMemMap::admin_to_hsm_ipc_rx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsm_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor30,
            receive_message_descriptor: IpcDescriptor::Descriptor31,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    fn hsm_to_admin_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_admin_ipc_rx_queue(),
                ci: GsRamMemMap::hsm_to_admin_ipc_rx_queue_ci(),
                pi: GsRamMemMap::hsm_to_admin_ipc_rx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_admin_ipc_tx_queue(),
                ci: GsRamMemMap::hsm_to_admin_ipc_tx_queue_ci(),
                pi: GsRamMemMap::hsm_to_admin_ipc_tx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor13,
            receive_message_descriptor: IpcDescriptor::Descriptor12,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    fn admin_to_hsm_ipc_event_channel_config() -> IpcEventChannelConfig {
        let send_event_mask = 1 << IpcDescriptor::Descriptor28 as u32;
        let receive_event_mask = 1 << IpcDescriptor::Descriptor29 as u32;

        IpcEventChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            send_event_mask,
            receive_event_mask,
        }
    }

    fn admin_to_hsp_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsp_ipc_tx_queue(),
                ci: GsRamMemMap::admin_to_hsp_ipc_tx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsp_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsp_ipc_rx_queue(),
                ci: GsRamMemMap::admin_to_hsp_ipc_rx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsp_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor20,
            receive_message_descriptor: IpcDescriptor::Descriptor21,
        };

        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    // IPC Channel Configuration for HSP to Admin Request Channel
    fn hsp_to_admin_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsp_to_admin_ipc_rx_queue(),
                ci: GsRamMemMap::hsp_to_admin_ipc_rx_queue_ci(),
                pi: GsRamMemMap::hsp_to_admin_ipc_rx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsp_to_admin_ipc_tx_queue(),
                ci: GsRamMemMap::hsp_to_admin_ipc_tx_queue_ci(),
                pi: GsRamMemMap::hsp_to_admin_ipc_tx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor23,
            receive_message_descriptor: IpcDescriptor::Descriptor22,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    // IPC Channel Configuration for HSP to Admin Stop Interface Request Channel
    fn hsp_to_admin_stop_interface_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock0,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsp_to_admin_stop_interface_ipc_rx_queue(),
                ci: GsRamMemMap::hsp_to_admin_stop_interface_ipc_rx_queue_ci(),
                pi: GsRamMemMap::hsp_to_admin_stop_interface_ipc_rx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsp_to_admin_stop_interface_ipc_tx_queue(),
                ci: GsRamMemMap::hsp_to_admin_stop_interface_ipc_tx_queue_ci(),
                pi: GsRamMemMap::hsp_to_admin_stop_interface_ipc_tx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor9,
            receive_message_descriptor: IpcDescriptor::Descriptor8,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    // Get DMA channel configuration
    fn dma_channel_config() -> GdmaChannelConfig {
        GdmaChannelConfig {
            tx_queue: GsRamMemMap::admin_gdma_tx_queue(),
            tx_queue_ci: GsRamMemMap::admin_gdma_tx_queue_shadow_pi(),
            rx_queue: GsRamMemMap::admin_gdma_rx_queue(),
            rx_queue_pi: GsRamMemMap::admin_gdma_rx_queue_shadow_pi(),
        }
    }

    fn make_fp_channels(
        io_cntrl: &IoController,
        ipc_cntrl: &IpcController,
    ) -> McrResult<(IoProxyChannel, IoProxyChannel, IpcMessageChannel)> {
        let ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::AdminToFpIoCore,
            Self::admin_to_fp_ipc_channel_config(),
        );

        loop {
            const DONE: u32 = IoProcessorBootState::Done.0;
            match PsRamMemMap::fp_boot_status().get() {
                DONE => break,
                _ => continue,
            }
        }

        // Send state change message to normal boot
        Self::set_io_state(&ipc_channel, IoProcessorState::NormalBoot)?;

        const FP_RX_ENTRY_LEN: usize = 128;
        const FP_TX_ENTRY_LEN: usize = 32;
        // send ucd query request and get ucd query response for FP IO Channel_0
        let io_channel_0 = Self::query_ucd_info(
            &ipc_channel,
            io_cntrl,
            IoChannelId::Channel0,
            FP_RX_ENTRY_LEN,
            FP_TX_ENTRY_LEN,
            false,
        )?;

        // send ucd query request and get ucd query response for FP IO Channel_1
        let io_channel_1 = Self::query_ucd_info(
            &ipc_channel,
            io_cntrl,
            IoChannelId::Channel1,
            FP_RX_ENTRY_LEN,
            FP_TX_ENTRY_LEN,
            false,
        )?;

        Ok((io_channel_0, io_channel_1, ipc_channel))
    }

    fn make_hsm_channels(
        io_cntrl: &IoController,
        ipc_cntrl: &IpcController,
    ) -> McrResult<(IoProxyChannel, IpcMessageChannel, IpcMessageChannel)> {
        let admin_to_hsm_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::AdminToHsmIoCore,
            Self::admin_to_hsm_ipc_channel_config(),
        );

        let hsm_to_admin_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HsmIoCoreToAdmin,
            Self::hsm_to_admin_ipc_channel_config(),
        );

        loop {
            const DONE: u32 = IoProcessorBootState::Done.0;
            match GsRamMemMap::boot_status().get() {
                DONE => break,
                _ => continue,
            }
        }

        // Send state change message to normal boot
        Self::set_io_state(&admin_to_hsm_ipc_channel, IoProcessorState::NormalBoot)?;

        // send ucd query request and get ucd query response
        let io_channel = Self::query_ucd_info(
            &admin_to_hsm_ipc_channel,
            io_cntrl,
            IoChannelId::Channel3,
            core::mem::size_of::<IoRxEntry>(),
            core::mem::size_of::<IoTxEntry>(),
            true,
        )?;

        Ok((
            io_channel,
            admin_to_hsm_ipc_channel,
            hsm_to_admin_ipc_channel,
        ))
    }

    fn set_io_state(ipc_channel: &IpcMessageChannel, state: IoProcessorState) -> McrResult<()> {
        let message = IpcMessageIoStateChange {
            state,
            ..Default::default()
        };

        ipc_channel.send_request(0, message.encode())?;

        let ipc_message = Self::wait_for_response(ipc_channel);

        let message: IpcMessageIoStateChange = IpcMessageDecoder::decode(ipc_message)?;

        match message.header.status() {
            0 => Ok(()),
            _ => Err(AdminErr::InvalidStateChangeIpcResponse.into()),
        }
    }

    fn query_ucd_info(
        ipc_channel: &IpcMessageChannel,
        io_ctrl: &IoController,
        channel_id: IoChannelId,
        rx_entry_len: usize,
        tx_entry_len: usize,
        enable_irq: bool,
    ) -> McrResult<IoProxyChannel> {
        let message = IpcMessageUcdQuery {
            info: UcdQueryInfo {
                ctrl_id: io_ctrl.id(),
                channel_id,
                ..Default::default()
            },
            ..Default::default()
        };

        ipc_channel.send_request(0, message.encode())?;

        let ipc_message = Self::wait_for_response(ipc_channel);

        let message: IpcMessageUcdQuery = IpcMessageDecoder::decode(ipc_message)?;

        match message.header.status() {
            0 => Self::create_proxy_io_channel(
                message,
                io_ctrl,
                channel_id,
                rx_entry_len,
                tx_entry_len,
                enable_irq,
            ),
            _ => Err(AdminErr::InvalidUcdQueryIpcResponse.into()),
        }
    }

    fn create_proxy_io_channel(
        message: IpcMessageUcdQuery,
        io_ctrl: &IoController,
        channel_id: IoChannelId,
        rx_entry_len: usize,
        tx_entry_len: usize,
        enable_irq: bool,
    ) -> McrResult<IoProxyChannel> {
        let len = message.info.queue_len as usize;

        let io_channel_config = IoChannelConfig {
            channel_id,
            rx_queue: mem_addr_to_slice(message.info.rx_queue_addr as usize, len),
            rx_queue_pi: mem_addr_to_volatile_ptr(message.info.rx_pi_addr),
            rx_free_list: mem_addr_to_slice(message.info.rx_free_list_addr as usize, len),
            rx_entry_pool_info: RxEntryPoolInfo::Size(rx_entry_len),
            tx_queue: mem_addr_to_slice(message.info.tx_queue_addr as usize, len),
            tx_queue_pi: mem_addr_to_volatile_ptr(message.info.tx_pi_addr),
            tx_free_list: mem_addr_to_slice(message.info.tx_free_list_addr as usize, len),
            tx_entry_pool_info: TxEntryPoolInfo::Size(tx_entry_len),
            enable_irq,
        };

        if SocInfo::default().reset_type() == SocResetType::FwUpdateWarmReset {
            io_ctrl.open_proxy_channel(channel_id)
        } else {
            io_ctrl.create_proxy_channel(io_channel_config)
        }
    }

    /// Executes pre-operational self-tests for Admin
    fn preops_cast(&mut self) -> McrResult<()> {
        let mut aes_fp_self_test =
            PreOpAesFpSelfTest::new(&self.cdma_io, &self.soft_aes, &self.admin_to_fp_ipc_channel)?;

        // Clear self test key vault
        aes_fp_self_test.clear_self_test_vault()?;

        for self_test in mcr_self_test::SelfTest::all() {
            match self_test {
                SelfTest::AesEcb => {
                    self.handle_test_result(
                        self.soft_aes.aes_ecb_256_decrypt_self_test(),
                        self_test,
                    )?;
                }
                SelfTest::AesGcmAlignedAndUnalignedData => {
                    self.handle_test_result(aes_fp_self_test.aes_gcm_test(), self_test)?;
                }
                SelfTest::AesGcmAlignedData => {
                    self.handle_test_result(
                        aes_fp_self_test.aes_gcm_test_aligned_data(),
                        self_test,
                    )?;
                }
                SelfTest::AesGcmAadNoAlignedData => {
                    self.handle_test_result(
                        aes_fp_self_test.aes_gcm_test_aad_no_aligned_data(),
                        self_test,
                    )?;
                }
                SelfTest::AesUnwrapWithPadding => {
                    self.handle_test_result(
                        self.soft_aes.aes_256_key_unwrap_self_test(),
                        self_test,
                    )?;
                }
                SelfTest::AesXtsNegEnc => {
                    self.handle_test_result(aes_fp_self_test.aes_xts_neg_enc_test(), self_test)?;
                }
                SelfTest::AesXtsNegDec => {
                    self.handle_test_result(aes_fp_self_test.aes_xts_neg_dec_test(), self_test)?;
                }
                _ => (),
            }
        }

        // Store the self test key table for later use
        self.self_test_key_table = aes_fp_self_test.cdma_io_self_test_key_table();

        Ok(())
    }

    /// Helper function to handle test results with consistent error mapping
    fn handle_test_result(&self, result: Result<(), u32>, test_type: &SelfTest) -> McrResult<()> {
        match result {
            Ok(()) => Ok(()),
            Err(err) => {
                error!(
                    "Preop-Self test failed for {:?} with error {}",
                    *test_type as u32, err
                );

                Err(err)
            }
        }
    }

    pub(crate) fn wait_for_response(ipc_channel: &IpcMessageChannel) -> IpcMessage {
        loop {
            if let Some(message) = ipc_channel.poll_message() {
                break message;
            }
        }
    }

    fn wait_for_request(ipc_channel: &IpcMessageChannel) -> IpcMessage {
        loop {
            if let Some(message) = ipc_channel.receive_message() {
                break message;
            }
        }
    }

    fn handle_sp_state_change_req(
        ipc_channel: &IpcMessageChannel,
        expected_state: IoProcessorState,
    ) -> McrResult<IpcMessageIoStateChange> {
        let ipc_message = Self::wait_for_request(ipc_channel);
        let mut message: IpcMessageIoStateChange = IpcMessageDecoder::decode(ipc_message)?;

        if message.state != expected_state {
            message.header.set_status(1);
        } else {
            message.header.set_status(0);
        }

        message.header.set_response(true);

        Ok(message)
    }

    fn send_state_change_rsp(
        ipc_channel: &IpcMessageChannel,
        message: IpcMessageIoStateChange,
    ) -> McrResult<()> {
        ipc_channel.send_response(message.encode())?;

        Ok(())
    }

    fn update_core_liveness() {
        AdminDtcmMemMap::core_run_status().set(1u32);
    }

    #[cfg(feature = "fips_validation_hooks")]
    fn check_for_preop_cast_hooks() {
        // Initialize the cast failure for self-test
        if let Some(x) = SocMemMap::negative_self_test_id().first_mut() {
            *x = None;
        }
        if let Some(x) = SocMemMap::negative_kind().first_mut() {
            *x = NegKind::None;
        }

        // Check if any of pre-operational self-test or health test is chosen to be failed
        let (st_opt, nk) = {
            if let Some(preop) = GsRamMemMap::negative_self_test().first_mut() {
                let st = preop.get_preops_negative_self_test();
                let nk = preop.get_preops_negative_kind();

                // One-shot: reset the whole struct so no stale bits remain
                *preop = PreopsNegativeTest::new();

                (st, nk)
            } else {
                (None, NegKind::None)
            }
        };

        // Priority: if negative_kind is set, use it, else use test_id
        if nk != NegKind::None {
            if let Some(dst) = SocMemMap::negative_kind().first_mut() {
                *dst = nk;
            }
        } else if let Some(st) = st_opt {
            if let Some(dst) = SocMemMap::negative_self_test_id().first_mut() {
                *dst = Some(st);
            }
        }
    }

    pub fn initialize_debug_log_sender() {
        let admin_debug_log_sender = DebugLogSender::new(
            SimplexPipeConfig {
                queue: GsRamMemMap::cp0_debug_log_buffer_queue(),
                ci: GsRamMemMap::cp0_debug_log_ring_buffer_head(),
                pi: GsRamMemMap::cp0_debug_log_ring_buffer_tail(),
            },
            GsRamMemMap::cp0_debug_log_ring_buffer_size(),
            GsRamMemMap::cp0_debug_log_buffer_sender_overflows(),
        );
        unsafe {
            DEBUG_LOG_SENDER = Some(admin_debug_log_sender);
        }
    }
}
