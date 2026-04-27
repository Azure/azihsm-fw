// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;

/// IO Queue Manager
pub(crate) struct IoQueueMgr {
    /// IO queues
    io_queues: Vec<Option<IoQueue>>,
}

impl Default for IoQueueMgr {
    fn default() -> Self {
        let mut io_queues = Vec::with_capacity(HSM_IO_QUEUE_SIZE);
        for _ in 0..HSM_IO_QUEUE_SIZE {
            io_queues.push(None);
        }

        Self { io_queues }
    }
}

impl IoQueueMgr {
    /// Enable the IO queue
    pub fn enable_io_queue(&mut self, sq_id: DevSqId, cq_id: DevCqId) {
        let ioq_index = usize::from(sq_id) - HSM_IO_QUEUE_BASE;
        if self.io_queues[ioq_index].is_none() {
            self.io_queues[ioq_index] = Some(IoQueue::new(sq_id, cq_id));
        }
    }

    /// Disable the IO queue
    pub fn disable_io_queue(
        &mut self,
        sq_id: DevSqId,
        delete_ctx: Option<IoQueueDeleteContext>,
    ) -> bool {
        let ioq_index = usize::from(sq_id) - HSM_IO_QUEUE_BASE;
        let mut io_queue = self.io_queues[ioq_index].take();
        let mut pending_disable = false;

        if let Some(io_queue) = io_queue.as_mut() {
            // There is atleast one reference to the IO queue used by a command
            if io_queue.ref_cnt() > 1 {
                pending_disable = true;
                io_queue.set_delete_context(delete_ctx);
            }
            io_queue.invalidate();
        }

        pending_disable
    }

    /// Disable All the Io Queues
    pub fn disable_all_io_queues(&mut self, delete_ctx: Option<IoQueueDeleteContext>) -> bool {
        let mut pending_disable = false;

        self.io_queues.iter_mut().for_each(|q| {
            if let Some(q) = q.as_mut() {
                // There is atleast one reference to the IO queue used by a command
                if q.ref_cnt() > 1 {
                    pending_disable = true;
                    q.set_delete_context(delete_ctx.clone());
                }
                q.invalidate();
            }

            *q = None;
        });

        pending_disable
    }

    /// Get the IO queue
    pub fn io_queue(&self, sq_id: DevSqId) -> Option<IoQueue> {
        let ioq_index = usize::from(sq_id) - HSM_IO_QUEUE_BASE;
        self.io_queues[ioq_index].clone()
    }
}
