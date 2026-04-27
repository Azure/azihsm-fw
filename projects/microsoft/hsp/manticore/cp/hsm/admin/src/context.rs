// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_cdma_io::MAX_KEYS_PER_TABLE;
use mcr_self_test::SelfTest;
use mcr_types::AesBulk256KeyId;
use mcr_types::HsmPartPersistentStore;
use mcr_types::MAX_PCIE_FUNCTIONS;

use crate::env::AdminEnvTrait;
use crate::fsm::AdminFsm;
use crate::resource::AdminToFpIpcChannel;
use crate::resource::CastIdle;
use crate::resource::DoeIdle;
use crate::resource::HsmIpcChannel;
use crate::resource::HspIpcChannel;
use crate::resource::TdispIdle;
use crate::CmdResource;
use crate::CmdScheduler;

#[derive(Clone)]
pub(crate) struct AdminFsmContext<E: AdminEnvTrait + 'static> {
    env: E,
    admin_to_fp_ipc_channel: CmdResource<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>>,
    hsm_ipc_channel: CmdResource<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>>,
    admin_to_hsp_ipc_channel: CmdResource<HspIpcChannel<E::IpcChannel>, AdminFsm<E>>,
    cast_idle: CmdResource<CastIdle, AdminFsm<E>>,
    doe_idle: CmdResource<DoeIdle, AdminFsm<E>>,
    tdisp_idle: CmdResource<TdispIdle, AdminFsm<E>>,
}

impl<E: AdminEnvTrait> AdminFsmContext<E> {
    pub fn new(env: E, scheduler: CmdScheduler<AdminFsm<E>>) -> Self {
        Self {
            admin_to_fp_ipc_channel: CmdResource::new(
                AdminToFpIpcChannel::new(env.admin_to_fp_ipc_channel().clone()),
                scheduler.clone(),
                1,
            ),
            hsm_ipc_channel: CmdResource::new(
                HsmIpcChannel::new(env.hsm_ipc_channel().clone()),
                scheduler.clone(),
                1,
            ),
            admin_to_hsp_ipc_channel: CmdResource::new(
                HspIpcChannel::new(env.admin_to_hsp_ipc_channel().clone()),
                scheduler.clone(),
                1,
            ),
            cast_idle: CmdResource::new(CastIdle::new(), scheduler.clone(), 1),
            doe_idle: CmdResource::new(DoeIdle::new(), scheduler.clone(), 1),
            tdisp_idle: CmdResource::new(TdispIdle::new(), scheduler, 1),
            env,
        }
    }

    pub fn pcie_cntrl(&self) -> &E::PcieController {
        self.env.pcie_cntrl()
    }

    pub fn pcie_doe(&self) -> &E::PcieDoe {
        self.env.pcie_doe()
    }

    pub fn msix_cntrl(&self) -> &E::MsixController {
        self.env.msix_cntrl()
    }

    pub fn io_controller(&self) -> &E::IoController {
        self.env.io_controller()
    }

    pub fn fp_io_controller(&self) -> &E::IoController {
        self.env.fp_io_controller()
    }

    pub fn io_channel(&self) -> &E::IoChannel {
        self.env.io_channel()
    }

    pub fn admin_to_fp_ipc_channel(
        &self,
    ) -> &CmdResource<AdminToFpIpcChannel<E::IpcChannel>, AdminFsm<E>> {
        &self.admin_to_fp_ipc_channel
    }

    pub fn fp_ipc_event_channel(&self) -> &E::IpcEventChannel {
        self.env.fp_ipc_event_channel()
    }

    pub fn hsm_ipc_channel(&self) -> &CmdResource<HsmIpcChannel<E::IpcChannel>, AdminFsm<E>> {
        &self.hsm_ipc_channel
    }

    pub fn hsm_ipc_event_channel(&self) -> &E::IpcEventChannel {
        self.env.hsm_ipc_event_channel()
    }

    pub fn admin_to_hsp_ipc_channel(
        &self,
    ) -> &CmdResource<HspIpcChannel<E::IpcChannel>, AdminFsm<E>> {
        &self.admin_to_hsp_ipc_channel
    }

    pub fn hsp_to_admin_ipc_channel(&self) -> &E::IpcChannel {
        self.env.hsp_to_admin_ipc_channel()
    }

    pub fn hsp_to_admin_stop_interface_ipc_channel(&self) -> &E::IpcChannel {
        self.env.hsp_to_admin_stop_interface_ipc_channel()
    }

    pub fn hsm_to_admin_ipc_channel(&self) -> &E::IpcChannel {
        self.env.hsm_to_admin_ipc_channel()
    }

    pub fn function_mgr(&self) -> &E::FunctionMgr {
        self.env.function_mgr()
    }

    pub fn dma_channel(&self) -> &E::DmaChannel {
        self.env.dma_channel()
    }

    pub fn dma_heap(&self) -> &E::DmaHeap {
        self.env.dma_heap()
    }

    pub fn soc_info(&self) -> &E::SocInfo {
        self.env.soc_info()
    }

    pub fn tcon_tsc(&self) -> u64 {
        self.env.tcon_tsc()
    }

    pub fn queue_delete_notification(&self) -> &E::QueueDeleteResp {
        self.env.deferred_queue_delete_pipe()
    }

    pub fn soft_aes_req(&self) -> &E::SoftAesReq {
        self.env.soft_aes_req()
    }

    pub fn soft_aes_resp(&self) -> &E::SoftAesResp {
        self.env.soft_aes_resp()
    }

    pub fn update_core_liveliness(&self) {
        self.env.update_core_liveliness()
    }

    pub fn self_test_req(&self) -> &E::SelfTestReq {
        self.env.self_test_req()
    }

    pub fn self_test_resp(&self) -> &E::SelfTestResp {
        self.env.self_test_resp()
    }

    pub fn soft_aes(&self) -> &E::SoftAes {
        self.env.soft_aes()
    }

    pub fn cdma_io(&self) -> &E::CdmaIo {
        self.env.cdma_io()
    }

    pub fn self_test_key_table(&self) -> &[Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE] {
        self.env.self_test_key_table()
    }

    pub fn aes_gcm_iv_queue(&self) -> &E::AesGcmIvQueue {
        self.env.aes_gcm_iv_queue()
    }

    pub fn aes_gcm_req_queue(&self) -> &E::AesGcmReqQueue {
        self.env.aes_gcm_req_queue()
    }

    pub fn aes_gcm_resp_queue(&self) -> &E::AesGcmRespQueue {
        self.env.aes_gcm_resp_queue()
    }

    pub fn get_bulk_key_req_queue(&self) -> &E::GetBulkKeyReqQueue {
        self.env.get_bulk_key_req_queue()
    }

    pub fn get_bulk_key_resp_queue(&self) -> &E::GetBulkKeyRespQueue {
        self.env.get_bulk_key_resp_queue()
    }

    pub fn rng(&self) -> &E::Rng {
        self.env.rng()
    }

    pub fn cast_idle(&self) -> &CmdResource<CastIdle, AdminFsm<E>> {
        &self.cast_idle
    }

    pub fn doe_idle(&self) -> &CmdResource<DoeIdle, AdminFsm<E>> {
        &self.doe_idle
    }

    pub fn tdisp_idle(&self) -> &CmdResource<TdispIdle, AdminFsm<E>> {
        &self.tdisp_idle
    }

    pub fn notify_self_test_failure(&self, test_id: SelfTest) {
        self.env.notify_self_test_failure(test_id);
    }

    pub fn hsm_part_persistent_store_addr(
        &self,
        pfn_index: usize,
    ) -> &'static mut HsmPartPersistentStore {
        let persistent_store: &'static mut [HsmPartPersistentStore] =
            mcr_mem_map::mem_addr_to_slice(
                self.env.hsm_part_persistent_store_addr(),
                MAX_PCIE_FUNCTIONS,
            );

        &mut persistent_store[pfn_index]
    }

    pub fn pause_queue_controller(&self) {
        self.env.pause_queue_controller();
    }

    pub fn resume_queue_controller(&self) {
        self.env.resume_queue_controller();
    }
}
