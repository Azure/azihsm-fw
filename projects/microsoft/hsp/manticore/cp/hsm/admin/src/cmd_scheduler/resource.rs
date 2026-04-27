// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::collections::VecDeque;
use alloc::rc::Rc;
use core::cell::Ref;
use core::cell::RefCell;

use crate::*;

/// Command Resource Information
pub trait CmdResourceInfo {
    type Id;
    type Resource;
    type Context: Clone;

    /// Resource ID
    fn id(&self) -> Self::Id;

    /// Maximum number of commands
    fn max_count(&self) -> usize;

    /// Acquire the resource
    ///
    /// # Arguments
    ///
    /// # Returns
    ///
    /// * `Option<usize>` - The resource instance ID.
    fn set(&mut self, ctx: Self::Context) -> Option<usize>;

    /// Release the resource
    ///
    /// # Arguments
    /// * `instance_id` - The resource instance ID.
    fn clear(&mut self, instance_id: usize);

    /// Get the resource
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance ID.
    ///
    /// # Returns
    ///
    /// * `&Self::Resource` - The resource
    fn resource(&self, instance_id: usize) -> &Self::Resource;

    /// Find the context
    ///
    /// # Arguments
    ///
    /// * `predicate` - The predicate
    ///
    /// # Returns
    ///
    /// * `Option<Self::Context>` - The context
    #[allow(unused)]
    fn find_ctx<F>(&self, predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool;
}

/// Command Resource Reference
pub struct CmdResourceRef<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> {
    rimpl: Rc<RefCell<CmdResourceImpl<Res, Fsm>>>,
    instance_id: usize,
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> CmdResourceRef<Res, Fsm> {
    /// Create a new `CmdResourceRef`
    fn new(rimpl: Rc<RefCell<CmdResourceImpl<Res, Fsm>>>, instance_id: usize) -> Self {
        Self { rimpl, instance_id }
    }

    pub fn map<F, T>(&self, closure: F) -> T
    where
        F: FnOnce(&Res::Resource) -> T,
    {
        closure(self.rimpl.borrow_mut().resource(self.instance_id))
    }

    /// Get the Resource
    pub fn deref(&self) -> Ref<'_, Res::Resource> {
        Ref::map(self.rimpl.borrow(), |r| r.resource(self.instance_id))
    }
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> Drop for CmdResourceRef<Res, Fsm>
where
    Res: CmdResourceInfo,
    Fsm: CmdFsm,
{
    /// Executes the destructor for this type.
    fn drop(&mut self) {
        self.rimpl.borrow_mut().release(self.instance_id);
    }
}

/// Command Resource
pub struct CmdResource<Res: CmdResourceInfo, Fsm: CmdFsm> {
    rimpl: Rc<RefCell<CmdResourceImpl<Res, Fsm>>>,
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> Clone for CmdResource<Res, Fsm> {
    fn clone(&self) -> Self {
        Self {
            rimpl: self.rimpl.clone(),
        }
    }
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> CmdResource<Res, Fsm> {
    /// Create a new command resource
    ///
    /// # Arguments
    ///
    /// * `resource` - The resource
    /// * `scheduler` - The scheduler
    /// * `max_waiters` - The maximum number of waiters
    ///
    /// # Returns
    ///
    /// * `CmdResource` - The command resource
    pub fn new(resource: Res, scheduler: CmdScheduler<Fsm>, max_waiters: usize) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CmdResourceImpl::new(
                resource,
                scheduler,
                max_waiters,
            ))),
        }
    }

    /// Acquire the resource
    ///
    /// # Arguments
    ///
    /// * `tag` - The tag
    ///
    /// # Returns
    ///
    /// * `Option<CmdResourceRef<Res, Fsm>>` - The resource reference
    pub fn acquire(&self, tag: TagId, ctx: Res::Context) -> Option<CmdResourceRef<Res, Fsm>> {
        self.rimpl
            .borrow_mut()
            .acquire(tag, ctx)
            .map(|id| CmdResourceRef::new(self.rimpl.clone(), id))
    }
}

/// Command Resource Implementation
pub struct CmdResourceImpl<Res: CmdResourceInfo, Fsm: CmdFsm> {
    /// Resources
    resource: Res,

    /// Current resource count
    count: usize,

    /// Waiters
    waiters: VecDeque<TagId>,

    /// Scheduler
    scheduler: CmdScheduler<Fsm>,
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId>, Fsm: CmdFsm> CmdResourceImpl<Res, Fsm> {
    /// Create a new command resource
    fn new(resource: Res, scheduler: CmdScheduler<Fsm>, max_waiters: usize) -> Self {
        Self {
            count: resource.max_count(),
            resource,
            scheduler,
            waiters: VecDeque::with_capacity(max_waiters),
        }
    }

    /// Acquire the resource
    fn acquire(&mut self, tag: TagId, ctx: Res::Context) -> Option<usize> {
        let transferred = self
            .scheduler
            .rsrc_ownership_transferred(self.resource.id());
        if self.count > 1 || (self.count == 1 && !transferred) {
            self.count -= 1;
            self.resource.set(ctx)
        } else {
            self.waiters.push_back(tag);
            None
        }
    }

    /// Release the resource
    fn release(&mut self, instance_id: usize) {
        self.count += 1;
        self.resource.clear(instance_id);
        if let Some(tag) = self.waiters.pop_front() {
            self.scheduler.wakeup(tag, self.resource.id());
        }
    }

    /// Get the resource
    fn resource(&self, instance_id: usize) -> &Res::Resource {
        self.resource.resource(instance_id)
    }
}
