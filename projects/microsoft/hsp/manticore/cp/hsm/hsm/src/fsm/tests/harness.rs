// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::mock::*;

#[derive(Default)]
pub(crate) struct HsmFsmTest {
    channel: Option<MockIoChannel>,
    partition: Option<MockPartition>,
    dma_heap: Option<MockDmaHeap>,
    dma_channel: Option<MockDmaChannel>,
    admin_ipc_channel: Option<MockIpcMessageChannel>,
}

impl HsmFsmTest {
    pub fn io_channel(&mut self) -> &mut MockIoChannel {
        if self.channel.is_none() {
            self.channel = Some(MockIoChannel::new())
        }

        self.channel.as_mut().unwrap()
    }

    pub fn partition(&mut self) -> &mut MockPartition {
        if self.partition.is_none() {
            self.partition = Some(MockPartition::new())
        }

        self.partition.as_mut().unwrap()
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

    #[allow(dead_code)]
    pub fn admin_ipc_channel(&mut self) -> &mut MockIpcMessageChannel {
        if self.admin_ipc_channel.is_none() {
            self.admin_ipc_channel = Some(MockIpcMessageChannel::new())
        }

        self.admin_ipc_channel.as_mut().unwrap()
    }

    pub fn env(&mut self, partition_call_count: usize) -> MockEnv {
        let mut hal = MockHal::new();

        if let Some(channel) = self.channel.take() {
            hal.expect_io_channel().return_const(channel);
        }

        if let Some(dma_channel) = self.dma_channel.take() {
            hal.expect_dma_channel().return_const(dma_channel);
        }

        if let Some(dma_heap) = self.dma_heap.take() {
            hal.expect_dma_heap().return_const(dma_heap);
        }

        if let Some(admin_ipc_channel) = self.admin_ipc_channel.take() {
            hal.expect_admin_ipc_channel()
                .return_const(admin_ipc_channel);
        }

        hal.expect_tcon_tsc().return_const(0u64);

        let mut env = MockEnv::new();

        env.expect_hal().return_const(hal);

        if let Some(partition) = self.partition.take() {
            env.expect_partition()
                .times(partition_call_count)
                .return_const(partition);
        }

        env
    }
}
