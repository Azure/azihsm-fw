// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec;
use alloc::vec::Vec;

use fsm::ComboFsm;
use mcr_cpu::*;
use mcr_crypto_aes::*;
use mcr_crypto_pka::*;
use mcr_crypto_rng::*;
use mcr_crypto_sha::*;
use mcr_dtcm_controller::DtcmController;
use mcr_error::McrResult;
use mcr_gdma_controller::*;
use mcr_interrupt_controller::Interrupt;
use mcr_interrupt_controller::InterruptController;
use mcr_interrupt_controller::*;
use mcr_io_controller::*;
use mcr_ipc_controller::*;
use mcr_ipc_message::*;
use mcr_itcm_controller::ItcmController;
use mcr_logging::*;
use mcr_mailbox_controller::MailboxController;
use mcr_mailbox_controller::MailboxControllerTrait;
use mcr_mailbox_controller::MailboxId;
use mcr_mem_map::CdmaMemMap;
use mcr_mem_map::GsRamMemMap;
use mcr_mem_map::HsmDtcmMemMap;
use mcr_mem_map::PsRamMemMap;
use mcr_mem_map::SocMemMap;
use mcr_self_test::SelfTest;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_simplex::*;
use mcr_soc::*;
use mcr_tcon::Tcon;
use mcr_tcon::TconTrait;
use mcr_types::*;

use crate::cmd_scheduler::CmdScheduler;
use crate::error;
use crate::error::HsmErr;
use crate::fsm;
use crate::heap::*;
use crate::partition::*;

pub(crate) trait HsmEnvTrait: Clone {
    type Hal: HsmHalTrait;
    type Partition: HsmPartition<UserSession = Self::UserSession> + Clone;
    type UserSession: HsmUserSession<Env = Self>;

    /// Get the hardware absraction layer
    fn hal(&self) -> &Self::Hal;

    /// Get the partition belong to a given pcie function
    fn partition(&self, pfn: PcieFunction) -> Self::Partition;

    /// Prepare for graceful shutdown
    fn prepare_for_shutdown(&self);

    /// Pka engine resource
    fn pka_engine(&self) -> &PkaEngine<Self>;
}

#[derive(Clone)]
pub(crate) struct HsmEnv {
    hal: HsmHal,
    partition_mgr: HsmPartitionMgr<Self>,
    pka_engine: PkaEngine<Self>,
}

impl HsmEnvTrait for HsmEnv {
    type Hal = HsmHal;
    type Partition = Partition<Self>;
    type UserSession = UserSession<Self>;

    /// Get the hardware absraction layer
    fn hal(&self) -> &Self::Hal {
        &self.hal
    }

    /// Get the partition
    fn partition(&self, pfn: PcieFunction) -> Self::Partition {
        self.partition_mgr.partition(pfn)
    }

    /// Prepare for graceful shutdown
    fn prepare_for_shutdown(&self) {
        let store = GsRamMemMap::hsm_partition_table().get_mut(0).unwrap();
        *store = HsmPartDataStore::default();

        self.partition_mgr.prepare_for_shutdown()
    }

    /// Pka engine resource
    fn pka_engine(&self) -> &PkaEngine<Self> {
        &self.pka_engine
    }
}

impl HsmEnv {
    /// Create an instance of Hsm Environment
    pub fn new(scheduler: &CmdScheduler<ComboFsm<Self>>) -> McrResult<Self> {
        let hal = HsmHal::new()?;
        let ctx = PartEnv::new(hal.clone(), scheduler.clone());
        let reset_type = SocInfo::default().reset_type();

        let partition_mgr = if reset_type == SocResetType::FwUpdateWarmReset {
            HsmPartitionMgr::new(|pfn| Partition::restore(pfn, ctx.clone()))
        } else if reset_type == SocResetType::WarmReset {
            HsmPartitionMgr::new(|pfn| Partition::new_with_resource_table(pfn, ctx.clone()))
        } else {
            HsmPartitionMgr::new(|pfn| Partition::new(pfn, ctx.clone()))
        };

        Ok(Self {
            partition_mgr,
            hal,
            pka_engine: ctx.pka_engine().clone(),
        })
    }
}

/// Hsm Environment
pub(crate) trait HsmHalTrait: Clone {
    type DmaChannel: GdmaChannelTrait + Clone;
    type DmaHeap: HsmDmaHeapTrait + Clone;
    type IoChannel: IoChannelTrait + Clone;
    type IpcMessageChannel: IpcMessageChannelTrait + Clone;
    type IpcEventChannel: IpcEventChannelTrait + Clone;
    type Aes: AesTrait + Clone;
    type Rng: RngTrait + Clone;
    type Sha: ShaTrait + Clone;
    type Pka: PkaTrait + Clone;
    type CpuInfo: CpuInfoTrait + Clone;
    type Tcon: TconTrait + Clone;
    type QueueDeleteResp: SimplexPipeTrait<QueueDeleteResponse> + Clone;
    type SoftAesReq: SimplexPipeTrait<SoftAesOffloadReq> + Clone;
    type SoftAesResp: SimplexPipeTrait<SoftAesOffloadResp> + Clone;
    type SelfTestReq: SimplexPipeTrait<SelfTestReqPacket> + Clone;
    type SelfTestResp: SimplexPipeTrait<SelfTestRespPacket> + Clone;
    type MailboxController: MailboxControllerTrait + Clone;

    /// Get IO Channel
    fn io_channel(&self) -> &Self::IoChannel;

    /// Get DMA Channel
    fn dma_channel(&self) -> &Self::DmaChannel;

    /// Get DMA Heap
    fn dma_heap(&self) -> &Self::DmaHeap;

    /// Get Admin to Hsm request IPC channel
    fn admin_ipc_channel(&self) -> &Self::IpcMessageChannel;

    /// Get IPC event channel
    fn ipc_event_channel(&self) -> &Self::IpcEventChannel;

    /// Get AES object
    fn aes(&self) -> &Self::Aes;

    /// Get RNG object
    fn rng(&self) -> &Self::Rng;

    /// Get SHA object
    fn sha(&self) -> &Self::Sha;

    /// Get PKA object
    fn pka(&self) -> &Vec<Self::Pka>;

    /// Get HSM to FP IPC channel
    fn hsm_to_fp_ipc_channel(&self) -> &Self::IpcMessageChannel;

    /// Get HSM to HSP IPC Channel
    fn hsp_ipc_channel(&self) -> &Self::IpcMessageChannel;

    /// Get FP to HSM Request IPC channel
    fn fp_to_hsm_ipc_channel(&self) -> &Self::IpcMessageChannel;

    /// Get HSM to Admin IPC Channel
    fn hsm_to_admin_ipc_channel(&self) -> &Self::IpcMessageChannel;

    /// Get the Partition Persistent Store Base Address
    fn part_persistent_store_addr(&self) -> usize;

    /// Vault address
    fn vault_addr(&self) -> usize;

    /// Get CPU info object
    #[allow(unused)]
    fn cpu_info(&self) -> &Self::CpuInfo;

    /// CDMA Vault address
    fn cdma_vault_addr(&self) -> usize;

    /// Meta data address to manage CDMA Vault
    fn cdma_vault_meta_data(&self) -> usize;

    /// BKS table address
    fn bks_table_addr(&self) -> usize;

    /// Get the timestamp counter
    #[allow(unused)]
    fn tcon_tsc(&self) -> u64;

    /// Get the Resource table
    fn resource_table(&self) -> &[Resource];

    /// Get the parition data store
    fn partition_data_store_addr(&self) -> usize;

    /// Alias key address
    fn alias_key_len(&self) -> u32;

    /// Alias key data
    fn alias_key(&self) -> &[u8];

    /// Alias certificate length
    fn alias_cert_len(&self) -> usize;

    /// Alias certificate data
    fn alias_cert(&self) -> &[u8];

    /// Get the simplex pipe to get queue delete indication
    fn queue_delete_notification(&self) -> &Self::QueueDeleteResp;

    /// Get the simplex pipe to send SoftAes request to admin core
    fn soft_aes_req(&self) -> &Self::SoftAesReq;

    /// Get the simplex pipe to send SoftAes request to admin core
    fn soft_aes_resp(&self) -> &Self::SoftAesResp;

    /// Set core liveliness indicator
    fn update_core_liveliness(&self);

    /// Get the self test request pipe
    fn self_test_req(&self) -> &Self::SelfTestReq;

    /// Get the self test response pipe
    fn self_test_resp(&self) -> &Self::SelfTestResp;

    /// Trigger Self test error notification other cores
    fn notify_self_test_failure(&self, test_id: SelfTest);

    /// Trigger PCT error notification to HSP
    fn notify_pct_validation_failure(&self, err: u32);

    /// FIPS certification status
    fn is_fips_approved(&self, pfn: PcieFunction) -> bool;

    /// Toggle FIPS Approved status
    #[cfg(feature = "fips_validation_hooks")]
    fn toggle_fips_approved_state(&self, pfn: PcieFunction);

    /// Get corrected ECC Error Interrupt count
    #[cfg(feature = "mcr_test_hooks")]
    fn get_corr_ecc_err_intr_count(&self) -> Option<u32>;

    /// Set corrected ECC Error Interrupt count
    #[cfg(feature = "mcr_test_hooks")]
    fn set_corr_ecc_err_intr_count(&self, count: u32) -> McrResult<()>;
}

/// Hsm Environment
#[derive(Clone)]
pub(crate) struct HsmHal {
    admin_ipc_channel: IpcMessageChannel,
    dma_channel: GdmaChannel,
    dma_heap: HsmDmaHeap,
    io_channel: IoChannel,
    ipc_event_channel: IpcEventChannel,
    aes: Aes,
    rng: Rng,
    sha: Sha,
    pka: Vec<Pka<InterruptController>>,
    hsm_to_fp_ipc_channel: IpcMessageChannel,
    hsp_ipc_channel: IpcMessageChannel,
    fp_to_hsm_ipc_channel: IpcMessageChannel,
    hsm_to_admin_ipc_channel: IpcMessageChannel,
    #[allow(unused)]
    cpu_info: CpuInfo,
    queue_delete_notification: SimplexPipe<QueueDeleteResponse>,
    soft_aes_req: SimplexPipe<SoftAesOffloadReq>,
    soft_aes_resp: SimplexPipe<SoftAesOffloadResp>,
    self_test_req: SimplexPipe<SelfTestReqPacket>,
    self_test_resp: SimplexPipe<SelfTestRespPacket>,
}

impl HsmHalTrait for HsmHal {
    type DmaChannel = GdmaChannel;
    type DmaHeap = HsmDmaHeap;
    type IoChannel = IoChannel;
    type IpcMessageChannel = IpcMessageChannel;
    type IpcEventChannel = IpcEventChannel;
    type Aes = Aes;
    type Rng = Rng;
    type Pka = Pka<InterruptController>;
    type Sha = Sha;
    type CpuInfo = CpuInfo;
    type Tcon = Tcon;
    type QueueDeleteResp = SimplexPipe<QueueDeleteResponse>;
    type SoftAesReq = SimplexPipe<SoftAesOffloadReq>;
    type SoftAesResp = SimplexPipe<SoftAesOffloadResp>;
    type SelfTestReq = SimplexPipe<SelfTestReqPacket>;
    type SelfTestResp = SimplexPipe<SelfTestRespPacket>;
    type MailboxController = MailboxController;

    /// Get IO Channel
    fn io_channel(&self) -> &Self::IoChannel {
        &self.io_channel
    }

    /// Get DMA Channel
    fn dma_channel(&self) -> &Self::DmaChannel {
        &self.dma_channel
    }

    /// Get DMA Heap
    fn dma_heap(&self) -> &Self::DmaHeap {
        &self.dma_heap
    }

    /// Get Admin to Hsm request IPC channel
    fn admin_ipc_channel(&self) -> &Self::IpcMessageChannel {
        &self.admin_ipc_channel
    }

    /// Get IPC event channel
    fn ipc_event_channel(&self) -> &Self::IpcEventChannel {
        &self.ipc_event_channel
    }

    /// Get AES engine.
    fn aes(&self) -> &Self::Aes {
        &self.aes
    }

    /// Get RNG engine.
    fn rng(&self) -> &Self::Rng {
        &self.rng
    }

    /// Get PKA engine.
    fn pka(&self) -> &Vec<Self::Pka> {
        &self.pka
    }

    /// Get SHA engine.
    fn sha(&self) -> &Self::Sha {
        &self.sha
    }

    /// Get HSM to FP IPC channel
    fn hsm_to_fp_ipc_channel(&self) -> &Self::IpcMessageChannel {
        &self.hsm_to_fp_ipc_channel
    }

    /// Get HSM to HSP IPC Channel
    fn hsp_ipc_channel(&self) -> &Self::IpcMessageChannel {
        &self.hsp_ipc_channel
    }

    /// Get FP to HSM IPC channel
    fn fp_to_hsm_ipc_channel(&self) -> &Self::IpcMessageChannel {
        &self.fp_to_hsm_ipc_channel
    }

    /// Get HSM to Admin IPC Channel
    fn hsm_to_admin_ipc_channel(&self) -> &Self::IpcMessageChannel {
        &self.hsm_to_admin_ipc_channel
    }

    /// Get the Partition Persistent Store Base Address
    fn part_persistent_store_addr(&self) -> usize {
        GsRamMemMap::hsm_part_persistent_store().as_ptr() as usize
    }

    /// Vault address
    fn vault_addr(&self) -> usize {
        GsRamMemMap::key_vault().as_ptr() as usize
    }

    /// Get CPU info.
    fn cpu_info(&self) -> &Self::CpuInfo {
        &self.cpu_info
    }

    /// CDMA Vault address
    fn cdma_vault_addr(&self) -> usize {
        CdmaMemMap::key_vault().as_ptr() as usize
    }

    /// Meta data address to manage CDMA Vault
    fn cdma_vault_meta_data(&self) -> usize {
        GsRamMemMap::cdma_key_vault_meta_data().as_ptr() as usize
    }

    /// BKS table address
    fn bks_table_addr(&self) -> usize {
        GsRamMemMap::bks_table().as_ptr() as usize
    }

    /// Get the timestamp counter
    fn tcon_tsc(&self) -> u64 {
        Tcon::tsc()
    }

    /// Get the Resource table
    fn resource_table(&self) -> &[Resource] {
        GsRamMemMap::admin_pcie_resource_table()
    }

    /// Get the parition data store
    fn partition_data_store_addr(&self) -> usize {
        GsRamMemMap::hsm_partition_table().as_ptr() as usize
    }

    /// Alias key length
    fn alias_key_len(&self) -> u32 {
        GsRamMemMap::alias_key_length().get()
    }

    /// Alias key length
    fn alias_key(&self) -> &'static [u8] {
        GsRamMemMap::alias_key()
    }

    /// Alias certificate length
    fn alias_cert_len(&self) -> usize {
        GsRamMemMap::alias_cert_length().get() as usize
    }

    /// Alias certificate data
    fn alias_cert(&self) -> &[u8] {
        GsRamMemMap::alias_cert()
    }

    /// Get the simplex pipe to get queue delete indication
    fn queue_delete_notification(&self) -> &Self::QueueDeleteResp {
        &self.queue_delete_notification
    }

    /// Get the simplex pipe to send SoftAes request to admin core
    fn soft_aes_req(&self) -> &Self::SoftAesReq {
        &self.soft_aes_req
    }

    /// Get the simplex pipe to send SoftAes request to admin core
    fn soft_aes_resp(&self) -> &Self::SoftAesResp {
        &self.soft_aes_resp
    }

    /// Set core liveliness indicator
    fn update_core_liveliness(&self) {
        HsmDtcmMemMap::core_run_status().set(1u32);
    }

    /// Get the self test request pipe
    fn self_test_req(&self) -> &Self::SelfTestReq {
        &self.self_test_req
    }

    /// Get the self test response pipe
    fn self_test_resp(&self) -> &Self::SelfTestResp {
        &self.self_test_resp
    }

    /// Trigger Self test error notification other cores
    #[allow(unused_variables)]
    fn notify_self_test_failure(&self, test_id: SelfTest) {
        error!("Failed the self test {:?}", test_id as u32);

        Self::notify_failure();
    }

    /// Trigger PCT error notification to HSP
    #[allow(unused_variables)]
    fn notify_pct_validation_failure(&self, err: u32) {
        error!("PCT Validation Failure: {:?}. System Reset Required! ", err);

        Self::notify_failure();
    }

    /// FIPS approved status
    fn is_fips_approved(&self, pfn: PcieFunction) -> bool {
        HsmDtcmMemMap::fips_approved()[pfn.0 as usize]
    }

    /// Toggle FIPS Approved status
    #[cfg(feature = "fips_validation_hooks")]
    fn toggle_fips_approved_state(&self, pfn: PcieFunction) {
        let hsm_fips_approved = HsmDtcmMemMap::fips_approved();
        let fp_fips_approved = SocMemMap::fp_fips_approved();

        // Update the HSM FIPS approved status
        let current = hsm_fips_approved[pfn.0 as usize];
        hsm_fips_approved[pfn.0 as usize] = !current;

        // Update the FP FIPS approved status
        let current = fp_fips_approved[pfn.0 as usize];
        fp_fips_approved[pfn.0 as usize] = !current;
    }

    /// Get corrected ECC Error Interrupt count
    #[cfg(feature = "mcr_test_hooks")]
    fn get_corr_ecc_err_intr_count(&self) -> Option<u32> {
        Some(
            HsmDtcmMemMap::corr_ecc_err_intr_count()
                .first()
                .copied()
                .unwrap_or(0),
        )
    }

    /// Set corrected ECC Error Interrupt count
    #[cfg(feature = "mcr_test_hooks")]
    fn set_corr_ecc_err_intr_count(&self, count: u32) -> McrResult<()> {
        if let Some(val) = HsmDtcmMemMap::corr_ecc_err_intr_count().get_mut(0) {
            *val = count;
            Ok(())
        } else {
            Err(HsmErr::InvalidMemoryMapEntry as u32)
        }
    }
}

impl HsmHal {
    /// Create an instance of Hsm Environment
    pub fn new() -> McrResult<Self> {
        let ipc_cntrl = IpcController::new(IpcIntBlock::IntBlock1, Interrupt::IpcSgiCore1);

        let admin_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::AdminToHsmIoCore,
            Self::admin_to_hsm_ipc_channel_config(),
        );

        // Disable TCON wakeup timer 1.
        Tcon::disable_wakeup_timer1();

        let rng = Rng::new(Self::rng_calibration_config());
        InterruptController::default().clear(Interrupt::rng_error_irq);
        InterruptController::default().enable(Interrupt::rng_error_irq);

        Self::begin_bootstrap(&admin_ipc_channel)?;

        // Enable tcon wakeup 1 timer interrupt that serves a crash trigger mechanism
        // from a crashing SoC core to all other SoC cores.
        //
        // Note: IMPORTANT - Enable this interrupt only after the bootstrap with Admin is complete.
        InterruptController::default().clear(Interrupt::tcon_wakeup1_irq);
        InterruptController::default().enable(Interrupt::tcon_wakeup1_irq);

        let hsm_to_fp_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HsmIoCoreToFpIoCore,
            Self::hsm_to_fp_ipc_channel_config(),
        );

        let hsp_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HsmIoCoreToHsp,
            Self::hsm_to_hsp_ipc_channel_config(),
        );

        let hsm_to_admin_ipc_channel = ipc_cntrl.create_message_channel(
            IpcChannelId::HsmIoCoreToAdmin,
            Self::hsm_to_admin_ipc_channel_config(),
        );

        let fp_to_hsm_ipc_channel = ipc_cntrl
            .create_message_channel(IpcChannelId::FpToHsm, Self::fp_to_hsm_ipc_channel_config());

        let dma_cntrl = GdmaController::new();
        let dma_channel =
            dma_cntrl.create_channel(GdmaChannelId::Channel1, Self::dma_channel_config())?;

        let io_cntrl = IoController::new(IoControllerId::Core0);
        let io_channel = io_cntrl.open_channel(Self::io_channel_config())?;

        let heap_region = GsRamMemMap::hsm_heap();
        let dma_heap = HsmDmaHeap::new(heap_region.as_ptr() as usize, heap_region.len());

        let intc = InterruptController::default();

        // Cross-core fault detection and recovery
        DtcmController::int_en(CpuId::Admin);
        InterruptController::default().clear(Interrupt::cp0_dtcm_err_irq);
        InterruptController::default().enable(Interrupt::cp0_dtcm_err_irq);

        // ITCM ECC error IRQ register
        ItcmController::int_en(CpuId::Hsm);
        InterruptController::default().clear(Interrupt::cp1_itcm_err_irq);
        InterruptController::default().enable(Interrupt::cp1_itcm_err_irq);

        let aes = Aes::new(Self::get_mut_buffer(GsRamMemMap::aes_cmd_buffer)?);

        let sha_cmd_buffer = Self::get_mut_buffer(GsRamMemMap::sha_cmd_buffer)?;
        let sha_out_buffer = Self::get_mut_buffer(GsRamMemMap::sha_out_buffer)?;
        let sha_self_test_buffer = Self::get_mut_buffer(GsRamMemMap::sha_self_test_buffer)?;
        let sha = Sha::new(sha_cmd_buffer, sha_out_buffer, sha_self_test_buffer);

        Pka::<InterruptController>::global_init(Self::get_mut_buffer(
            GsRamMemMap::pka_ecc_const_buffer,
        )?)?;
        let ecc_const = Self::get_buffer(GsRamMemMap::pka_ecc_const_buffer)?;
        let mut pka_vec = vec![];

        for (index, ((cmd, output), input)) in GsRamMemMap::pka_cmd_buffer()
            .iter_mut()
            .zip(GsRamMemMap::pka_output_buffer().iter_mut())
            .zip(GsRamMemMap::pka_input_buffer().iter_mut())
            .enumerate()
        {
            let idx = index
                .try_into()
                .map_err(|_| HsmErr::InvalidMemoryMapEntry)?;
            // There can be only one outstanding self test execution at a time across all the PKA
            // instances. The buffer is shared across all the PKA instances.
            let pka_self_test_buffer = Self::get_mut_buffer(GsRamMemMap::pka_self_test_buffer)?;

            pka_vec.push(Pka::new(
                PkaInstanceId(idx),
                cmd,
                output,
                input,
                &ecc_const[..],
                intc.clone(),
                pka_self_test_buffer,
            ));
        }

        let ipc_event_channel = ipc_cntrl.create_event_channel(
            IpcChannelId::AdminToHsmIoCore,
            Self::ipc_event_channel_config(),
        );

        let cpu_info = CpuInfo {};

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
            ci: HsmDtcmMemMap::soft_aes_resp_ci(),
            pi: HsmDtcmMemMap::soft_aes_resp_pi(),
        });

        let self_test_req = SimplexPipe::new(SimplexPipeConfig {
            queue: HsmDtcmMemMap::self_test_req_queue(),
            ci: HsmDtcmMemMap::self_test_req_ci(),
            pi: HsmDtcmMemMap::self_test_req_pi(),
        });

        let self_test_resp = SimplexPipe::new(SimplexPipeConfig {
            queue: HsmDtcmMemMap::self_test_resp_queue(),
            ci: HsmDtcmMemMap::self_test_resp_ci(),
            pi: HsmDtcmMemMap::self_test_resp_pi(),
        });

        // Check if the HSM is FIPS approved status
        // and set the FIPS certification state in the memory map.
        Self::populate_fips_certification_state()?;

        let env = Self {
            admin_ipc_channel,
            dma_channel,
            dma_heap,
            io_channel,
            ipc_event_channel,
            aes,
            rng,
            sha,
            pka: pka_vec,
            hsm_to_fp_ipc_channel,
            hsp_ipc_channel,
            hsm_to_admin_ipc_channel,
            fp_to_hsm_ipc_channel,
            cpu_info,
            queue_delete_notification,
            soft_aes_req,
            soft_aes_resp,
            self_test_req,
            self_test_resp,
        };

        // Run the Pre Operation self tests for FIPS
        env.preops_cast()?;

        Self::end_bootstrap();

        Ok(env)
    }

    // Get IO channel configuration
    fn io_channel_config() -> IoChannelConfig {
        IoChannelConfig {
            channel_id: IoChannelId::Channel3,
            rx_queue: GsRamMemMap::hsm_rx_queue(),
            rx_queue_pi: GsRamMemMap::hsm_rx_queue_shadow_pi(),
            rx_free_list: GsRamMemMap::hsm_rx_free_list(),
            rx_entry_pool_info: RxEntryPoolInfo::EntryPool(GsRamMemMap::hsm_sqe_pool()),
            tx_queue: GsRamMemMap::hsm_tx_queue(),
            tx_queue_pi: GsRamMemMap::hsm_tx_queue_shadow_pi(),
            tx_free_list: GsRamMemMap::hsm_tx_free_list(),
            tx_entry_pool_info: TxEntryPoolInfo::EntryPool(GsRamMemMap::hsm_cqe_pool()),
            enable_irq: true,
        }
    }

    // Get DMA channel configuration
    fn dma_channel_config() -> GdmaChannelConfig {
        GdmaChannelConfig {
            tx_queue: GsRamMemMap::hsm_gdma_tx_queue(),
            tx_queue_ci: GsRamMemMap::hsm_gdma_tx_queue_shadow_pi(),
            rx_queue: GsRamMemMap::hsm_gdma_rx_queue(),
            rx_queue_pi: GsRamMemMap::hsm_gdma_rx_queue_shadow_pi(),
        }
    }

    /// HSM to Admin IPC response channel configuration
    fn admin_to_hsm_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsm_ipc_rx_queue(),
                ci: GsRamMemMap::admin_to_hsm_ipc_rx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsm_ipc_rx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::admin_to_hsm_ipc_tx_queue(),
                ci: GsRamMemMap::admin_to_hsm_ipc_tx_queue_ci(),
                pi: GsRamMemMap::admin_to_hsm_ipc_tx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor31,
            receive_message_descriptor: IpcDescriptor::Descriptor30,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    /// HSM to Admin IPC request channel configuration
    fn hsm_to_admin_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_admin_ipc_tx_queue(),
                ci: GsRamMemMap::hsm_to_admin_ipc_tx_queue_ci(),
                pi: GsRamMemMap::hsm_to_admin_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_admin_ipc_rx_queue(),
                ci: GsRamMemMap::hsm_to_admin_ipc_rx_queue_ci(),
                pi: GsRamMemMap::hsm_to_admin_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor12,
            receive_message_descriptor: IpcDescriptor::Descriptor13,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    /// HSM to FP IPC request channel configuration
    fn hsm_to_fp_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            tx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::hsm_to_fp_ipc_tx_queue(),
                ci: PsRamMemMap::hsm_to_fp_ipc_tx_queue_ci(),
                pi: PsRamMemMap::hsm_to_fp_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::hsm_to_fp_ipc_rx_queue(),
                ci: PsRamMemMap::hsm_to_fp_ipc_rx_queue_ci(),
                pi: PsRamMemMap::hsm_to_fp_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor15,
            receive_message_descriptor: IpcDescriptor::Descriptor16,
        };

        // For queues with FP core, we zeroize both tx and rx indices.
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config.rx_queue.ci.set(0);
        config.rx_queue.pi.set(0);
        config
    }

    /// HSM to HSP IPC request channel configuration
    fn hsm_to_hsp_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            tx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_hsp_ipc_tx_queue(),
                ci: GsRamMemMap::hsm_to_hsp_ipc_tx_queue_ci(),
                pi: GsRamMemMap::hsm_to_hsp_ipc_tx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: GsRamMemMap::hsm_to_hsp_ipc_rx_queue(),
                ci: GsRamMemMap::hsm_to_hsp_ipc_rx_queue_ci(),
                pi: GsRamMemMap::hsm_to_hsp_ipc_rx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor24,
            receive_message_descriptor: IpcDescriptor::Descriptor25,
        };
        // Once zeroize the pi and ci of the queue that this channel owns, that is, the channel can
        // increment the pi, which is true for the Tx queue
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config
    }

    /// FP to HSM IPC request channel configuration
    fn fp_to_hsm_ipc_channel_config() -> IpcMessageChannelConfig {
        let config = IpcMessageChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            tx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::fp_to_hsm_ipc_rx_queue(),
                ci: PsRamMemMap::fp_to_hsm_ipc_rx_queue_ci(),
                pi: PsRamMemMap::fp_to_hsm_ipc_rx_queue_pi(),
            },
            rx_queue: IpcMessageQueueConfig {
                queue: PsRamMemMap::fp_to_hsm_ipc_tx_queue(),
                ci: PsRamMemMap::fp_to_hsm_ipc_tx_queue_ci(),
                pi: PsRamMemMap::fp_to_hsm_ipc_tx_queue_pi(),
            },
            send_message_descriptor: IpcDescriptor::Descriptor11,
            receive_message_descriptor: IpcDescriptor::Descriptor10,
        };

        // For queues with FP core, we zeroize both tx and rx indices.
        config.tx_queue.ci.set(0);
        config.tx_queue.pi.set(0);
        config.rx_queue.ci.set(0);
        config.rx_queue.pi.set(0);
        config
    }

    fn ipc_event_channel_config() -> IpcEventChannelConfig {
        let send_event_mask = 1 << IpcDescriptor::Descriptor29 as u32;
        let receive_event_mask = 1 << IpcDescriptor::Descriptor28 as u32;

        IpcEventChannelConfig {
            int_block: IpcIntBlock::IntBlock1,
            send_event_mask,
            receive_event_mask,
        }
    }

    // Get RNG calibration configuration.
    fn rng_calibration_config() -> RngCalibration {
        mcr_crypto_rng::RngCalibration {
            clk_div: 0x60u8,
            cutoff: mcr_crypto_rng::RngCalibrationCutoff::new()
                .with_clk_div_msb(0)
                .with_repcnt(0x29)
                .with_apt(0x318)
                .with_chisq(0x82),
        }
    }

    fn handle_io_state_change(
        ipc_channel: &IpcMessageChannel,
        expect: IoProcessorState,
    ) -> McrResult<()> {
        let ipc_message = Self::wait_for_request(ipc_channel);

        let mut message: IpcMessageIoStateChange = IpcMessageDecoder::decode(ipc_message)?;

        if message.state != expect {
            message.header.set_status(1);
        }

        message.header.set_status(0);
        message.header.set_response(true);

        ipc_channel.send_response(message.encode())?;

        Ok(())
    }

    fn handle_ucd_query(ipc_channel: &IpcMessageChannel) -> McrResult<()> {
        let ipc_message = Self::wait_for_request(ipc_channel);

        // Populate the free list entry with entry pool addresses for the hardware to consume
        let rx_free_list = GsRamMemMap::hsm_rx_free_list();
        if SocInfo::default().reset_type() != SocResetType::FwUpdateWarmReset {
            let rx_entry_pool = GsRamMemMap::hsm_sqe_pool();
            for (fl_entry, rx_entry) in rx_free_list.iter_mut().zip(rx_entry_pool.iter()) {
                fl_entry.lo = rx_entry as *const IoRxEntry as u32;
                fl_entry.hi = 0
            }
        }

        let mut message: IpcMessageUcdQuery = IpcMessageDecoder::decode(ipc_message)?;

        message.info.queue_len = GsRamMemMap::hsm_sqe_pool().len() as u32;
        message.info.rx_queue_addr = GsRamMemMap::hsm_rx_queue().as_ptr() as u32;
        message.info.rx_pi_addr = GsRamMemMap::hsm_rx_queue_shadow_pi().as_ptr() as u32;
        message.info.rx_free_list_addr = GsRamMemMap::hsm_rx_free_list().as_ptr() as u32;
        message.info.tx_queue_addr = GsRamMemMap::hsm_tx_queue().as_ptr() as u32;
        message.info.tx_pi_addr = GsRamMemMap::hsm_tx_queue_shadow_pi().as_ptr() as u32;
        message.info.tx_free_list_addr = GsRamMemMap::hsm_tx_free_list().as_ptr() as u32;

        message.header.set_status(0);
        message.header.set_response(true);

        ipc_channel.send_response(message.encode())
    }

    fn wait_for_request(ipc_channel: &IpcMessageChannel) -> IpcMessage {
        loop {
            if let Some(message) = ipc_channel.receive_message() {
                break message;
            }
        }
    }

    /// Begin HSM bootstrapping
    fn begin_bootstrap(ipc_channel: &IpcMessageChannel) -> McrResult<()> {
        const DONE: u32 = IoProcessorBootState::Done.0;
        GsRamMemMap::boot_status().set(DONE);

        Self::handle_io_state_change(ipc_channel, IoProcessorState::NormalBoot)?;

        Self::handle_ucd_query(ipc_channel)?;

        Self::handle_io_state_change(ipc_channel, IoProcessorState::Start)?;

        Ok(())
    }

    /// End HSM bootstrapping
    fn end_bootstrap() {
        const RUN: u32 = IoProcessorBootState::Run.0;
        GsRamMemMap::boot_status().set(RUN);
    }

    fn get_mut_buffer<T>(buffer: fn() -> &'static mut [T]) -> McrResult<&'static mut T> {
        let buf = buffer().first_mut().ok_or(HsmErr::InvalidMemoryMapEntry)?;

        Ok(buf)
    }

    fn get_buffer<T>(buffer: fn() -> &'static mut [T]) -> McrResult<&'static T> {
        let buf = buffer().first().ok_or(HsmErr::InvalidMemoryMapEntry)?;

        Ok(buf)
    }

    /// Executes pre-operational self-tests for HSM
    fn preops_cast(&self) -> McrResult<()> {
        for self_test in mcr_self_test::SelfTest::all() {
            match self_test {
                SelfTest::Hkdf => {
                    self.handle_test_result(self.sha.hkdf_self_test_256(), self_test)?;
                }
                SelfTest::Kbkdf => {
                    self.handle_test_result(self.sha.kbkdf_self_test_512(), self_test)?;
                }
                SelfTest::Rsa2KModExpEngineInstance0
                | SelfTest::Rsa2KModExpEngineInstance1
                | SelfTest::Rsa2KModExpEngineInstance2
                | SelfTest::Rsa2KModExpEngineInstance3
                | SelfTest::Rsa2KModExpEngineInstance4
                | SelfTest::Rsa2KModExpEngineInstance5
                | SelfTest::Rsa2KModExpEngineInstance6
                | SelfTest::Rsa2KModExpEngineInstance7
                | SelfTest::Rsa2KModExpEngineInstance8
                | SelfTest::Rsa2KModExpEngineInstance9
                | SelfTest::Rsa2KModExpEngineInstance10
                | SelfTest::Rsa2KModExpEngineInstance11
                | SelfTest::Rsa2KModExpEngineInstance12
                | SelfTest::Rsa2KModExpEngineInstance13
                | SelfTest::Rsa2KModExpEngineInstance14
                | SelfTest::Rsa2KModExpEngineInstance15 => {
                    if let Some(instance) = self_test.get_engine_instance() {
                        let result = match self.pka[instance].rsa_mod_exp_self_test() {
                            Ok(padded_key) => {
                                // If RSA operation succeeded, try OAEP decode
                                self.sha().decode_oaep_kek_self_test(
                                    padded_key.as_ref(),
                                    HashAlgorithm::Sha256,
                                )
                            }
                            Err(e) => Err(e), // Forward the RSA error
                        };

                        self.handle_test_result(result, self_test)?;
                    }
                }
                SelfTest::Rsa2KModExpCrtEngineInstance0
                | SelfTest::Rsa2KModExpCrtEngineInstance1
                | SelfTest::Rsa2KModExpCrtEngineInstance2
                | SelfTest::Rsa2KModExpCrtEngineInstance3
                | SelfTest::Rsa2KModExpCrtEngineInstance4
                | SelfTest::Rsa2KModExpCrtEngineInstance5
                | SelfTest::Rsa2KModExpCrtEngineInstance6
                | SelfTest::Rsa2KModExpCrtEngineInstance7
                | SelfTest::Rsa2KModExpCrtEngineInstance8
                | SelfTest::Rsa2KModExpCrtEngineInstance9
                | SelfTest::Rsa2KModExpCrtEngineInstance10
                | SelfTest::Rsa2KModExpCrtEngineInstance11
                | SelfTest::Rsa2KModExpCrtEngineInstance12
                | SelfTest::Rsa2KModExpCrtEngineInstance13
                | SelfTest::Rsa2KModExpCrtEngineInstance14
                | SelfTest::Rsa2KModExpCrtEngineInstance15 => {
                    if let Some(instance) = self_test.get_engine_instance() {
                        self.handle_test_result(
                            self.pka[instance].rsa_mod_exp_crt_self_test(),
                            self_test,
                        )?;
                    } else {
                        Err(HsmErr::MissingSelfTestEngineInstance as u32)?;
                    }
                }
                SelfTest::EcdsaEngineInstance0
                | SelfTest::EcdsaEngineInstance1
                | SelfTest::EcdsaEngineInstance2
                | SelfTest::EcdsaEngineInstance3
                | SelfTest::EcdsaEngineInstance4
                | SelfTest::EcdsaEngineInstance5
                | SelfTest::EcdsaEngineInstance6
                | SelfTest::EcdsaEngineInstance7
                | SelfTest::EcdsaEngineInstance8
                | SelfTest::EcdsaEngineInstance9
                | SelfTest::EcdsaEngineInstance10
                | SelfTest::EcdsaEngineInstance11
                | SelfTest::EcdsaEngineInstance12
                | SelfTest::EcdsaEngineInstance13
                | SelfTest::EcdsaEngineInstance14
                | SelfTest::EcdsaEngineInstance15 => {
                    if let Some(instance) = self_test.get_engine_instance() {
                        self.handle_test_result(self.pka[instance].ecdsa_self_test(), self_test)?;
                    } else {
                        Err(HsmErr::MissingSelfTestEngineInstance as u32)?;
                    }
                }
                SelfTest::EcdhEngineInstance0
                | SelfTest::EcdhEngineInstance1
                | SelfTest::EcdhEngineInstance2
                | SelfTest::EcdhEngineInstance3
                | SelfTest::EcdhEngineInstance4
                | SelfTest::EcdhEngineInstance5
                | SelfTest::EcdhEngineInstance6
                | SelfTest::EcdhEngineInstance7
                | SelfTest::EcdhEngineInstance8
                | SelfTest::EcdhEngineInstance9
                | SelfTest::EcdhEngineInstance10
                | SelfTest::EcdhEngineInstance11
                | SelfTest::EcdhEngineInstance12
                | SelfTest::EcdhEngineInstance13
                | SelfTest::EcdhEngineInstance14
                | SelfTest::EcdhEngineInstance15 => {
                    if let Some(instance) = self_test.get_engine_instance() {
                        self.handle_test_result(self.pka[instance].ecdh_self_test(), self_test)?;
                    } else {
                        Err(HsmErr::MissingSelfTestEngineInstance as u32)?;
                    }
                }
                SelfTest::AesCbc => {
                    self.handle_test_result(self.execute_aes_cbc_self_test(), self_test)?;
                }
                SelfTest::Rng => {
                    self.handle_test_result(self.rng.self_test(), self_test)?;
                }
                _ => (),
            }
        }

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

    /// Helper to execute the AES CBC self test
    fn execute_aes_cbc_self_test(&self) -> Result<(), u32> {
        let mut aes_self_test_input = self
            .dma_heap
            .allocate(AES_SELF_TEST_INPUT_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let mut aes_self_test_output = self
            .dma_heap
            .allocate(AES_SELF_TEST_OUTPUT_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;
        let mut aes_self_test_iv = self
            .dma_heap
            .allocate(AES_SELF_TEST_IV_BUF_MAX_SIZE_BYTES)
            .ok_or(HsmErr::DmaAllocFailure)?;

        self.aes().aes_cbc_self_test(
            aes_self_test_input.as_ref_mut(),
            aes_self_test_output.as_ref_mut(),
            aes_self_test_iv.as_ref_mut(),
        )
    }

    fn populate_fips_certification_state() -> McrResult<()> {
        // Get FIPS approval status from measurements
        let fips_approved = GsRamMemMap::por_measurements()
            .first()
            .ok_or(HsmErr::InvalidPorMeasurementDataAccess)?
            .fips_approved;

        // Directly compare to create boolean
        let fips_approved = fips_approved == 1;

        // Get the FIPS approved status from the memory map for both HSM and FP cores
        let hsm_fips_approved = HsmDtcmMemMap::fips_approved();
        let fp_fips_approved = SocMemMap::fp_fips_approved();

        // Enumerate over the HSM and FP FIPS approved status and poppulate the current FIPS
        // apprroved status as populated from FW package
        for (hsm_fips_approval_sts, fp_fips_approval_sts) in hsm_fips_approved
            .iter_mut()
            .zip(fp_fips_approved.iter_mut())
        {
            *hsm_fips_approval_sts = fips_approved;
            *fp_fips_approval_sts = fips_approved
        }

        Ok(())
    }

    pub fn initialize_debug_log_sender() {
        let hsm_debug_log_sender = DebugLogSender::new(
            SimplexPipeConfig {
                queue: GsRamMemMap::cp1_debug_log_buffer_queue(),
                ci: GsRamMemMap::cp1_debug_log_ring_buffer_head(),
                pi: GsRamMemMap::cp1_debug_log_ring_buffer_tail(),
            },
            GsRamMemMap::cp1_debug_log_ring_buffer_size(),
            GsRamMemMap::cp1_debug_log_buffer_sender_overflows(),
        );
        unsafe {
            DEBUG_LOG_SENDER = Some(hsm_debug_log_sender);
        }
    }

    /// Notify HSP of failure
    fn notify_failure() {
        // Notify the HSP about the error using the mailbox
        let mbx_err = MailboxController::create(MailboxId::Mailbox0);
        mbx_err.trigger_mbx_err();

        #[allow(clippy::empty_loop)]
        loop {}
    }
}
