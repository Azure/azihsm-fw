// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_types::DevCqId;
use mcr_types::DevSqId;

/// AdminQueue
#[derive(Clone)]
pub(crate) struct AdminQueue {
    /// AdminQueue implementation
    rimpl: Rc<RefCell<AdminQueueImpl>>,
}

impl AdminQueue {
    /// Create a new instance of `AdminQueue`
    pub fn new(sq_id: DevSqId, cq_id: DevCqId) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(AdminQueueImpl::new(sq_id, cq_id))),
        }
    }

    /// Get the submission queue id
    pub fn sq_id(&self) -> DevSqId {
        self.rimpl.borrow().sq_id()
    }

    /// Get the completion queue id
    pub fn cq_id(&self) -> DevCqId {
        self.rimpl.borrow().cq_id()
    }

    /// Check if the queue is valid
    ///
    /// # Returns
    ///
    /// * `true` if the queue is valid
    pub fn valid(&self) -> bool {
        self.rimpl.borrow().valid()
    }

    /// Invalidate the queue
    pub fn invalidate(&self) {
        self.rimpl.borrow_mut().invalidate()
    }
}

/// AdminQueue implementation
struct AdminQueueImpl {
    /// Device Submission queue Id
    sq_id: DevSqId,

    /// Device Completion queue Id
    cq_id: DevCqId,

    /// Valid flag
    valid: bool,
}

impl AdminQueueImpl {
    /// Create a new instance of `AdminQueueImpl`
    fn new(sq_id: DevSqId, cq_id: DevCqId) -> Self {
        Self {
            sq_id,
            cq_id,
            valid: true,
        }
    }

    /// Get the submission queue id
    fn sq_id(&self) -> DevSqId {
        self.sq_id
    }

    /// Get the completion queue id
    fn cq_id(&self) -> DevCqId {
        self.cq_id
    }

    /// Check if the queue is valid
    fn valid(&self) -> bool {
        self.valid
    }

    /// Invalidate the queue
    fn invalidate(&mut self) {
        self.valid = false;
    }
}
