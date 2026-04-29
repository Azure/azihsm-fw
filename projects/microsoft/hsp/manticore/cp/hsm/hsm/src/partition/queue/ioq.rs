// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;

use mcr_types::DevCqId;
use mcr_types::DevSqId;

use crate::*;

/// IoQueue
#[derive(Clone)]
pub(crate) struct IoQueue {
    rimpl: Rc<RefCell<IoQueueImpl>>,
}

impl IoQueue {
    /// Create a new instance of `IoQueue`
    ///
    /// # Arguments
    ///
    /// * `sq_id` - Submission queue id
    /// * `cq_id` - Completion queue id
    ///
    /// # Returns
    ///
    /// * Instance of `IoQueue`
    pub fn new(sq_id: DevSqId, cq_id: DevCqId) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(IoQueueImpl::new(sq_id, cq_id))),
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
    pub fn valid(&self) -> bool {
        self.rimpl.borrow().valid()
    }

    /// Invalidate the queue
    pub fn invalidate(&mut self) {
        self.rimpl.borrow_mut().invalidate()
    }

    /// Put a new delete context to this IO queue
    pub fn set_delete_context(&self, ctx: Option<IoQueueDeleteContext>) {
        self.rimpl.borrow_mut()._delete_ctx = ctx;
    }

    /// Take the delete context
    pub fn take_delete_ctx(&self) -> Option<IoQueueDeleteContext> {
        self.rimpl.borrow_mut()._delete_ctx.take()
    }

    /// Reference count
    pub fn ref_cnt(&self) -> usize {
        Rc::strong_count(&self.rimpl)
    }
}

/// IO Queue implementation
struct IoQueueImpl {
    sq_id: DevSqId,
    cq_id: DevCqId,
    valid: bool,
    _delete_ctx: Option<IoQueueDeleteContext>,
}

impl IoQueueImpl {
    /// Create a new instance of `IoQueue`
    fn new(sq_id: DevSqId, cq_id: DevCqId) -> Self {
        Self {
            sq_id,
            cq_id,
            valid: true,
            _delete_ctx: None,
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
        self.valid = false
    }
}

/// IO Queue Delete context
#[derive(Clone)]
pub struct IoQueueDeleteContext {
    rimpl: Rc<RefCell<IoQueueDeleteContextImpl>>,
}

impl IoQueueDeleteContext {
    /// Create new instance of queue delete context
    ///
    /// # Arguments
    ///
    /// * `tag` - Target tag ID this, queue delete context belongs to
    /// * `migrate` - Indicate if this queue delete context is for migration
    ///
    /// # Returns
    ///
    /// * Instance of IoQueueDeleteContext
    pub fn new(tag: u16, migrate: bool) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(IoQueueDeleteContextImpl::new(tag, migrate))),
        }
    }

    /// Get the tag present in the Io Queue Delete Context
    ///
    /// # Returns
    ///
    /// * TagId
    pub fn tag(&self) -> TagId {
        self.rimpl.borrow().target_tag
    }

    /// Get the ref count of the Io Queue Delete Context
    ///
    /// # Returns
    ///
    /// * No of outstanding reference count on the Io Queue Delete Context
    pub fn ref_cnt(&self) -> usize {
        Rc::strong_count(&self.rimpl)
    }

    /// Check if this Io Queue Delete Context is for migration
    pub fn is_migration(&self) -> bool {
        self.rimpl.borrow().migrate
    }
}

/// Io Queue Delete Context Implementation
pub struct IoQueueDeleteContextImpl {
    target_tag: TagId,
    migrate: bool,
}

impl IoQueueDeleteContextImpl {
    fn new(tag: TagId, migrate: bool) -> Self {
        Self {
            target_tag: tag,
            migrate,
        }
    }
}
