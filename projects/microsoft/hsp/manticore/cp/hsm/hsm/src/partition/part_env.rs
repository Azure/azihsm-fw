// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::ComboFsm;
use crate::resource::*;
use crate::NUM_HSM_IO_SCHEDULER_SLOT;

pub(crate) type PkaEngine<E> =
    CmdResource<PkaResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::Pka>, ComboFsm<E>>;

pub(crate) type PkaEngineRef<E> =
    CmdResourceRef<PkaResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::Pka>, ComboFsm<E>>;

type FpIpcChannel<E> = CmdResource<
    FpIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

pub(super) type FpIpcChannelRef<E> = CmdResourceRef<
    FpIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

type HspIpcChannel<E> = CmdResource<
    HspIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

pub(super) type HspIpcChannelRef<E> = CmdResourceRef<
    HspIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

type HsmToAdminIpcChannel<E> = CmdResource<
    HsmToAdminIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

#[allow(dead_code)]
pub(super) type HsmToAdminIpcChannelRef<E> = CmdResourceRef<
    HsmToAdminIpcChannelResource<<<E as HsmEnvTrait>::Hal as HsmHalTrait>::IpcMessageChannel>,
    ComboFsm<E>,
>;

pub(crate) struct PartEnv<E: HsmEnvTrait + 'static> {
    hal: E::Hal,
    scheduler: CmdScheduler<ComboFsm<E>>,
    pka: PkaEngine<E>,
    fp_ipc: FpIpcChannel<E>,
    hsp_ipc: HspIpcChannel<E>,
    hsm_to_admin_ipc: HsmToAdminIpcChannel<E>,
}

impl<E: HsmEnvTrait> Clone for PartEnv<E> {
    fn clone(&self) -> Self {
        Self {
            hal: self.hal.clone(),
            scheduler: self.scheduler.clone(),
            pka: self.pka.clone(),
            fp_ipc: self.fp_ipc.clone(),
            hsp_ipc: self.hsp_ipc.clone(),
            hsm_to_admin_ipc: self.hsm_to_admin_ipc.clone(),
        }
    }
}

impl<E: HsmEnvTrait> PartEnv<E> {
    /// Create a new HSM context
    ///
    /// # Arguments
    ///
    /// * `hal` - HAL
    /// * `scheduler` - Command scheduler
    ///
    /// # Returns
    ///
    /// * HSM context
    pub fn new(hal: E::Hal, scheduler: CmdScheduler<ComboFsm<E>>) -> Self {
        // Prepare PKA resource
        let pka_rsrc = PkaResource::new(hal.pka().clone());
        let max_pka_rsrc_waiters = NUM_HSM_IO_SCHEDULER_SLOT - pka_rsrc.count();

        // Prepare FP IPC message channel resource
        let fp_ipc_rsrc = FpIpcChannelResource::new(hal.hsm_to_fp_ipc_channel().clone());
        let max_fp_ipc_rsrc_waiters = NUM_HSM_IO_SCHEDULER_SLOT - fp_ipc_rsrc.count();

        // Prepare HSP IPC message channel resource
        let hsp_ipc_rsrc = HspIpcChannelResource::new(hal.hsp_ipc_channel().clone());
        let max_hsp_ipc_rsrc_waiters = NUM_HSM_IO_SCHEDULER_SLOT - hsp_ipc_rsrc.count();

        let hsm_to_admin_ipc_rsrc =
            HsmToAdminIpcChannelResource::new(hal.hsm_to_admin_ipc_channel().clone());
        let max_hsm_to_admin_ipc_rsrc_waiters =
            NUM_HSM_IO_SCHEDULER_SLOT - hsm_to_admin_ipc_rsrc.count();

        Self {
            pka: CmdResource::new(pka_rsrc, scheduler.clone(), max_pka_rsrc_waiters),
            fp_ipc: CmdResource::new(fp_ipc_rsrc, scheduler.clone(), max_fp_ipc_rsrc_waiters),
            hsp_ipc: CmdResource::new(hsp_ipc_rsrc, scheduler.clone(), max_hsp_ipc_rsrc_waiters),
            hsm_to_admin_ipc: CmdResource::new(
                hsm_to_admin_ipc_rsrc,
                scheduler.clone(),
                max_hsm_to_admin_ipc_rsrc_waiters,
            ),
            hal,
            scheduler,
        }
    }

    /// Get the AES engine
    ///
    /// # Returns
    ///
    /// * AES
    pub fn aes(&self) -> &<E::Hal as HsmHalTrait>::Aes {
        self.hal.aes()
    }

    /// Get the PKA engine.
    ///
    /// # Returns
    ///
    /// * Reference to PKA engine.
    pub fn pka_engine(&self) -> &PkaEngine<E> {
        &self.pka
    }

    /// Get the Random Number Generator (RNG)
    ///
    /// # Returns
    ///
    /// * RNG
    pub fn rng(&self) -> &<E::Hal as HsmHalTrait>::Rng {
        self.hal.rng()
    }

    /// Get the SHA engine
    ///
    /// # Returns
    ///
    /// * SHA
    pub fn sha(&self) -> &<E::Hal as HsmHalTrait>::Sha {
        self.hal.sha()
    }

    /// Get the DMA heap
    ///
    /// # Returns
    ///
    /// * DMA heap
    pub fn dma_heap(&self) -> &<E::Hal as HsmHalTrait>::DmaHeap {
        self.hal.dma_heap()
    }

    /// Get a reference to the Partition Persistent Store for specific Pfn Index
    ///
    /// # Arguments
    /// * `pfn_index` - pfn index of partition
    ///
    /// # Returns
    /// * Static mutable reference to the indexed partition persistent store
    pub fn part_persistent_store_ref(
        &self,
        pfn_index: usize,
    ) -> &'static mut HsmPartPersistentStore {
        let persistent_store: &'static mut [HsmPartPersistentStore] =
            mcr_mem_map::mem_addr_to_slice(
                self.hal.part_persistent_store_addr(),
                MAX_PCIE_FUNCTIONS,
            );

        &mut persistent_store[pfn_index]
    }

    /// Get the Vault address
    ///
    /// # Returns
    ///
    /// * Vault address
    pub fn vault_addr(&self) -> usize {
        self.hal.vault_addr()
    }

    /// Get the CDMA Vault address
    ///
    /// # Returns
    ///
    /// * Vault address
    pub fn cdma_vault_addr(&self) -> usize {
        self.hal.cdma_vault_addr()
    }

    /// Get the CDMA Vault meta data
    ///
    /// # Returns
    ///
    /// * Vault meta data address
    pub fn cdma_vault_meta_data(&self) -> usize {
        self.hal.cdma_vault_meta_data()
    }

    /// Get the BKS table address
    ///
    /// # Returns
    ///
    /// * BKS table address address
    pub fn bks_table_addr(&self) -> usize {
        self.hal.bks_table_addr()
    }

    /// Get the IPC Channel.
    ///
    /// # Returns
    ///
    /// * Reference to IPC channel.
    pub fn fp_ipc_channel(&self) -> &FpIpcChannel<E> {
        &self.fp_ipc
    }

    /// Get the HSP IPC Channel resource.
    ///
    /// # Returns
    ///
    /// * Reference to HSP IPC channel resource.
    pub fn hsp_ipc_channel(&self) -> &HspIpcChannel<E> {
        &self.hsp_ipc
    }

    /// Get the HSM to Admin IPC Channel resource.
    ///
    /// # Returns
    ///
    /// * Reference to HSM to Admin IPC channel resource.
    #[allow(dead_code)]
    pub fn hsm_to_admin_ipc_channel(&self) -> &HsmToAdminIpcChannel<E> {
        &self.hsm_to_admin_ipc
    }

    /// Get the parition data store address.
    ///
    /// # Returns
    ///
    /// * Absolute address of the partitions data store in memory
    pub fn data_store_addr(&self) -> usize {
        self.hal.partition_data_store_addr()
    }

    /// Get the resource table.
    ///
    /// # Returns
    ///
    /// * Reference to Resource table in memory.
    pub fn resource_table(&self) -> &[Resource] {
        self.hal.resource_table()
    }

    /// Gets the alias key length.
    ///
    /// # Returns
    ///
    /// * alias key length
    pub fn alias_key_len(&self) -> u32 {
        self.hal.alias_key_len()
    }

    /// Gets the alias key.
    ///
    /// # Returns
    ///
    /// * alias key
    pub fn alias_key(&self) -> &[u8] {
        self.hal.alias_key()
    }

    /// Gets the alias cert length.
    ///
    /// # Returns
    ///
    /// * alias cert length
    pub fn alias_cert_len(&self) -> usize {
        self.hal.alias_cert_len()
    }

    /// Gets the alias cert.
    ///
    /// # Returns
    ///
    /// * alias cert
    pub fn alias_cert(&self) -> &[u8] {
        self.hal.alias_cert()
    }

    /// Get the simplex pipe to send SoftAes request to admin core
    pub fn soft_aes_req(&self) -> &<E::Hal as HsmHalTrait>::SoftAesReq {
        self.hal.soft_aes_req()
    }

    /// Get the simplex pipe to send SoftAes request to admin core
    pub fn soft_aes_resp(&self) -> &<E::Hal as HsmHalTrait>::SoftAesResp {
        self.hal.soft_aes_resp()
    }

    /// Tcon Timer
    pub fn tcon_tsc(&self) -> u64 {
        self.hal.tcon_tsc()
    }

    /// Fips certification status
    ///
    /// # Arguments
    /// * `pfn` - PCIe function
    ///
    /// # Returns
    /// * true if FIPS Approved, false otherwise
    pub fn is_fips_approved(&self, pfn: PcieFunction) -> bool {
        self.hal.is_fips_approved(pfn)
    }

    /// Notify PCT validation failure to HSP
    ///
    /// # Arguments
    /// * `err` - Error code indicating the failure reason
    pub fn notify_pct_validation_failure(&self, err: u32) {
        self.hal.notify_pct_validation_failure(err);
    }

    /// Toggle Fips approval state
    ///
    /// # Arguments
    /// * `pfn` - PCIe function
    #[cfg(feature = "fips_validation_hooks")]
    pub fn toggle_fips_approved_state(&self, pfn: PcieFunction) {
        self.hal.toggle_fips_approved_state(pfn);
    }

    /// Get corrected ECC Error Interrupt count
    ///
    /// # Arguments
    /// * `self` - Reference to the HSM environment
    /// # Returns
    /// * Corrected ECC Error Interrupt count
    #[cfg(all(feature = "mcr_test_hooks", feature = "mcr_test_hooks_cdma_ecc_err"))]
    pub fn get_corr_ecc_err_intr_count(&self) -> Option<u32> {
        self.hal.get_corr_ecc_err_intr_count()
    }
}
