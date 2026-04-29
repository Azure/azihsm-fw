// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_cdma_io::MAX_KEYS_PER_TABLE;
use mcr_self_test::SelfTestReqPacket;
use mcr_self_test::SelfTestRespPacket;
use mcr_types::AesBulk256KeyId;
use mcr_types::AesGcmReqEntry;
use mcr_types::AesGcmRespEntry;
use mcr_types::GetBulkKeyReqEntry;
use mcr_types::GetBulkKeyRespEntry;
use mcr_types::QueueDeleteResponse;

use crate::mock::*;

#[derive(Default)]
pub(crate) struct AdminFsmTest {
    pcie_cntrl: Option<MockPcieController>,
    pcie_doe: Option<MockPcieDoe>,
    msix_cntrl: Option<MockMsixController>,
    admin_to_fp_ipc_channel: Option<MockIpcMessageChannel>,
    fp_ipc_event_channel: Option<MockIpcEventChannel>,
    hsm_ipc_channel: Option<MockIpcMessageChannel>,
    hsm_ipc_event_channel: Option<MockIpcEventChannel>,
    admin_to_hsp_ipc_channel: Option<MockIpcMessageChannel>,
    hsp_to_admin_ipc_channel: Option<MockIpcMessageChannel>,
    hsp_to_admin_stop_interface_ipc_channel: Option<MockIpcMessageChannel>,
    function_mgr: Option<MockFunctionMgr>,
    io_channel: Option<MockIoChannel>,
    dma_heap: Option<MockDmaHeap>,
    dma_channel: Option<MockDmaChannel>,
    io_cntrl: Option<MockIoController>,
    fp_io_cntrl: Option<MockIoController>,
    deferred_queue_delete_pipe: Option<MockSimplexPipe<QueueDeleteResponse>>,
    self_test_request: Option<MockSimplexPipe<SelfTestReqPacket>>,
    self_test_response: Option<MockSimplexPipe<SelfTestRespPacket>>,
    soft_aes: Option<MockSoftAes>,
    cdma_io: Option<MockCdmaIo>,
    self_test_key_table: Option<[Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE]>,
    aes_gcm_req_queue: Option<MockSimplexPipe<AesGcmReqEntry>>,
    aes_gcm_resp_queue: Option<MockSimplexPipe<AesGcmRespEntry>>,
    get_bulk_key_req_queue: Option<MockSimplexPipe<GetBulkKeyReqEntry>>,
    get_bulk_key_resp_queue: Option<MockSimplexPipe<GetBulkKeyRespEntry>>,
}

impl AdminFsmTest {
    pub fn pcie_cntrl(&mut self) -> &mut MockPcieController {
        if self.pcie_cntrl.is_none() {
            self.pcie_cntrl = Some(MockPcieController::new())
        }

        self.pcie_cntrl.as_mut().unwrap()
    }

    pub fn pcie_doe(&mut self) -> &mut MockPcieDoe {
        if self.pcie_doe.is_none() {
            self.pcie_doe = Some(MockPcieDoe::new())
        }

        self.pcie_doe.as_mut().unwrap()
    }

    pub fn msix_cntrl(&mut self) -> &mut MockMsixController {
        if self.msix_cntrl.is_none() {
            self.msix_cntrl = Some(MockMsixController::new())
        }

        self.msix_cntrl.as_mut().unwrap()
    }

    pub fn io_channel(&mut self) -> &mut MockIoChannel {
        if self.io_channel.is_none() {
            self.io_channel = Some(MockIoChannel::new())
        }

        self.io_channel.as_mut().unwrap()
    }

    pub fn admin_to_fp_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.admin_to_fp_ipc_channel.is_none() {
            self.admin_to_fp_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.admin_to_fp_ipc_channel.as_mut().unwrap()
    }

    pub fn fp_ipc_event_channel(&mut self) -> &mut MockIpcEventChannel {
        if self.fp_ipc_event_channel.is_none() {
            self.fp_ipc_event_channel = Some(MockIpcEventChannel::new())
        }

        self.fp_ipc_event_channel.as_mut().unwrap()
    }

    pub fn hsm_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.hsm_ipc_channel.is_none() {
            self.hsm_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.hsm_ipc_channel.as_mut().unwrap()
    }

    pub fn hsm_ipc_event_channel(&mut self) -> &mut MockIpcEventChannel {
        if self.hsm_ipc_event_channel.is_none() {
            self.hsm_ipc_event_channel = Some(MockIpcEventChannel::new())
        }

        self.hsm_ipc_event_channel.as_mut().unwrap()
    }

    pub fn admin_to_hsp_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.admin_to_hsp_ipc_channel.is_none() {
            self.admin_to_hsp_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.admin_to_hsp_ipc_channel.as_mut().unwrap()
    }

    pub fn hsp_to_admin_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.hsp_to_admin_ipc_channel.is_none() {
            self.hsp_to_admin_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.hsp_to_admin_ipc_channel.as_mut().unwrap()
    }

    pub fn hsp_to_admin_stop_interface_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.hsp_to_admin_stop_interface_ipc_channel.is_none() {
            self.hsp_to_admin_stop_interface_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.hsp_to_admin_stop_interface_ipc_channel
            .as_mut()
            .unwrap()
    }

    pub fn function_mgr(&mut self) -> &mut MockFunctionMgr {
        if self.function_mgr.is_none() {
            self.function_mgr = Some(MockFunctionMgr::new());
        }

        self.function_mgr.as_mut().unwrap()
    }

    pub fn dma_heap(&mut self) -> &mut MockDmaHeap {
        if self.dma_heap.is_none() {
            self.dma_heap = Some(MockDmaHeap::new())
        }

        self.dma_heap.as_mut().unwrap()
    }

    pub fn dma_channel(&mut self) -> &mut MockDmaChannel {
        if self.dma_channel.is_none() {
            self.dma_channel = Some(MockDmaChannel::new())
        }

        self.dma_channel.as_mut().unwrap()
    }

    pub fn io_controller(&mut self) -> &mut MockIoController {
        if self.io_cntrl.is_none() {
            self.io_cntrl = Some(MockIoController::new())
        }

        self.io_cntrl.as_mut().unwrap()
    }

    pub fn fp_io_controller(&mut self) -> &mut MockIoController {
        if self.fp_io_cntrl.is_none() {
            self.fp_io_cntrl = Some(MockIoController::new())
        }

        self.fp_io_cntrl.as_mut().unwrap()
    }

    pub fn deferred_queue_delete_pipe(&mut self) -> &mut MockSimplexPipe<QueueDeleteResponse> {
        if self.deferred_queue_delete_pipe.is_none() {
            self.deferred_queue_delete_pipe = Some(MockSimplexPipe::new())
        }

        self.deferred_queue_delete_pipe.as_mut().unwrap()
    }

    pub fn self_test_req(&mut self) -> &mut MockSimplexPipe<SelfTestReqPacket> {
        if self.self_test_request.is_none() {
            self.self_test_request = Some(MockSimplexPipe::new())
        }

        self.self_test_request.as_mut().unwrap()
    }

    pub fn self_test_resp(&mut self) -> &mut MockSimplexPipe<SelfTestRespPacket> {
        if self.self_test_response.is_none() {
            self.self_test_response = Some(MockSimplexPipe::new())
        }

        self.self_test_response.as_mut().unwrap()
    }

    pub fn soft_aes(&mut self) -> &mut MockSoftAes {
        if self.soft_aes.is_none() {
            self.soft_aes = Some(MockSoftAes::new())
        }

        self.soft_aes.as_mut().unwrap()
    }

    pub fn cdma_io(&mut self) -> &mut MockCdmaIo {
        if self.cdma_io.is_none() {
            self.cdma_io = Some(MockCdmaIo::new())
        }

        self.cdma_io.as_mut().unwrap()
    }

    pub fn self_test_key_table(&mut self) -> &mut [Option<AesBulk256KeyId>; MAX_KEYS_PER_TABLE] {
        if self.self_test_key_table.is_none() {
            self.self_test_key_table = Some([None; MAX_KEYS_PER_TABLE])
        }

        self.self_test_key_table.as_mut().unwrap()
    }

    pub fn aes_gcm_req_queue(&mut self) -> &mut MockSimplexPipe<AesGcmReqEntry> {
        if self.aes_gcm_req_queue.is_none() {
            self.aes_gcm_req_queue = Some(MockSimplexPipe::new())
        }
        self.aes_gcm_req_queue.as_mut().unwrap()
    }

    pub fn aes_gcm_resp_queue(&mut self) -> &mut MockSimplexPipe<AesGcmRespEntry> {
        if self.aes_gcm_resp_queue.is_none() {
            self.aes_gcm_resp_queue = Some(MockSimplexPipe::new())
        }
        self.aes_gcm_resp_queue.as_mut().unwrap()
    }

    pub fn get_bulk_key_req_queue(&mut self) -> &mut MockSimplexPipe<GetBulkKeyReqEntry> {
        if self.get_bulk_key_req_queue.is_none() {
            self.get_bulk_key_req_queue = Some(MockSimplexPipe::new())
        }
        self.get_bulk_key_req_queue.as_mut().unwrap()
    }

    pub fn get_bulk_key_resp_queue(&mut self) -> &mut MockSimplexPipe<GetBulkKeyRespEntry> {
        if self.get_bulk_key_resp_queue.is_none() {
            self.get_bulk_key_resp_queue = Some(MockSimplexPipe::new())
        }
        self.get_bulk_key_resp_queue.as_mut().unwrap()
    }

    pub fn env(&mut self) -> MockAdminEnvTrait {
        let mut env = MockAdminEnvTrait::new();

        if let Some(pcie_cntrl) = self.pcie_cntrl.take() {
            env.expect_pcie_cntrl().return_const(pcie_cntrl);
        }

        if let Some(pcie_doe) = self.pcie_doe.take() {
            env.expect_pcie_doe().return_const(pcie_doe);
        }

        if let Some(msix_cntrl) = self.msix_cntrl.take() {
            env.expect_msix_cntrl().return_const(msix_cntrl);
        }

        if let Some(io_channel) = self.io_channel.take() {
            env.expect_io_channel().return_const(io_channel);
        }

        if let Some(admin_to_fp_ipc_channel) = self.admin_to_fp_ipc_channel.take() {
            env.expect_admin_to_fp_ipc_channel()
                .return_const(admin_to_fp_ipc_channel);
        }

        if let Some(fp_ipc_event_channel) = self.fp_ipc_event_channel.take() {
            env.expect_fp_ipc_event_channel()
                .return_const(fp_ipc_event_channel);
        }

        if let Some(hsm_ipc_channel) = self.hsm_ipc_channel.take() {
            env.expect_hsm_ipc_channel().return_const(hsm_ipc_channel);
        }

        if let Some(hsm_ipc_event_channel) = self.hsm_ipc_event_channel.take() {
            env.expect_hsm_ipc_event_channel()
                .return_const(hsm_ipc_event_channel);
        }

        if let Some(admin_to_hsp_ipc_channel) = self.admin_to_hsp_ipc_channel.take() {
            env.expect_admin_to_hsp_ipc_channel()
                .return_const(admin_to_hsp_ipc_channel);
        }

        if let Some(hsp_to_admin_ipc_channel) = self.hsp_to_admin_ipc_channel.take() {
            env.expect_hsp_to_admin_ipc_channel()
                .return_const(hsp_to_admin_ipc_channel);
        }

        if let Some(hsp_to_admin_stop_interface_ipc_channel) =
            self.hsp_to_admin_stop_interface_ipc_channel.take()
        {
            env.expect_hsp_to_admin_stop_interface_ipc_channel()
                .return_const(hsp_to_admin_stop_interface_ipc_channel);
        }

        if let Some(function_mgr) = self.function_mgr.take() {
            env.expect_function_mgr().return_const(function_mgr);
        }

        if let Some(dma_heap) = self.dma_heap.take() {
            env.expect_dma_heap().return_const(dma_heap);
        }

        if let Some(dma_channel) = self.dma_channel.take() {
            env.expect_dma_channel().return_const(dma_channel);
        }

        if let Some(io_cntrl) = self.io_cntrl.take() {
            env.expect_io_controller().return_const(io_cntrl);
        }

        if let Some(fp_io_cntrl) = self.fp_io_cntrl.take() {
            env.expect_fp_io_controller().return_const(fp_io_cntrl);
        }

        if let Some(deferred_queue_delete_pipe) = self.deferred_queue_delete_pipe.take() {
            env.expect_deferred_queue_delete_pipe()
                .return_const(deferred_queue_delete_pipe);
        }

        if let Some(self_test_request) = self.self_test_request.take() {
            env.expect_self_test_req().return_const(self_test_request);
        }

        if let Some(self_test_response) = self.self_test_response.take() {
            env.expect_self_test_resp().return_const(self_test_response);
        }

        if let Some(soft_aes) = self.soft_aes.take() {
            env.expect_soft_aes().return_const(soft_aes);
        }

        if let Some(cdma_io) = self.cdma_io.take() {
            env.expect_cdma_io().return_const(cdma_io);
        }

        if let Some(self_test_key_table) = self.self_test_key_table.take() {
            env.expect_self_test_key_table()
                .return_const(self_test_key_table);
        }

        if let Some(aes_gcm_req_queue) = self.aes_gcm_req_queue.take() {
            env.expect_aes_gcm_req_queue()
                .return_const(aes_gcm_req_queue);
        }

        if let Some(aes_gcm_resp_queue) = self.aes_gcm_resp_queue.take() {
            env.expect_aes_gcm_resp_queue()
                .return_const(aes_gcm_resp_queue);
        }

        if let Some(get_bulk_key_req_queue) = self.get_bulk_key_req_queue.take() {
            env.expect_get_bulk_key_req_queue()
                .return_const(get_bulk_key_req_queue);
        }

        if let Some(get_bulk_key_resp_queue) = self.get_bulk_key_resp_queue.take() {
            env.expect_get_bulk_key_resp_queue()
                .return_const(get_bulk_key_resp_queue);
        }

        env
    }
}
