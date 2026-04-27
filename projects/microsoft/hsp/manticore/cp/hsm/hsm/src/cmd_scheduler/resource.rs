// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::collections::VecDeque;
use alloc::rc::Rc;
use core::cell::Ref;
use core::cell::RefCell;
use mcr_crypto_pka::PkaTrait;

use crate::*;

/// Command Resource Information
pub trait CmdResourceInfo {
    type Id;
    type Resource;
    type Event;
    type Context: Clone;

    /// Resource ID
    fn id(&self) -> Self::Id;

    /// Get the current resource count
    fn count(&self) -> usize;

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
    fn find_ctx<F>(&self, predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool;

    /// Self test if the resource is operating as expected
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance ID for this which self-test to be performed.
    /// * `test` - A function closure to be called to complete the self test.
    ///
    /// # Notes
    ///
    /// This trait implements a default implementation of self_test, to override the implementaiton
    /// only if the resrouce is expected to perform self test.
    fn self_test<FnTest>(&mut self, _instance_id: usize, _test: FnTest)
    where
        FnTest: Fn(&dyn PkaTrait) + 'static,
    {
    }

    /// Get the cleanup event
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance Id
    fn cleanup_event(&self, _instance_id: usize) -> Self::Event {
        unimplemented!()
    }

    /// Cleanup the resource context
    ///
    /// # Arguments
    /// * `instance_id` - The resource instance ID
    ///
    /// # Notes
    /// This function is used to cleanup the resource context.
    fn cleanup_ctx(&mut self, _instance_id: usize) {
        unimplemented!()
    }

    /// Override the resource to a single instance
    ///
    /// # Arguments
    ///
    /// * `range` - The range
    ///
    /// # Notes
    ///
    /// This function is used to override the to a single instance.
    #[cfg(feature = "fips_validation_hooks")]
    fn operate_on_fixed_resource(&mut self, _instance: Option<usize>) {}
}

/// Command Resource Reference
pub struct CmdResourceRef<
    Res: CmdResourceInfo<Id = Fsm::ResourceId, Event = Fsm::Event>,
    Fsm: CmdFsm,
> {
    rimpl: Rc<RefCell<CmdResourceImpl<Res, Fsm>>>,
    instance_id: usize,
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId, Event = Fsm::Event>, Fsm: CmdFsm>
    CmdResourceRef<Res, Fsm>
{
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

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId, Event = Fsm::Event>, Fsm: CmdFsm> Drop
    for CmdResourceRef<Res, Fsm>
where
    Res: CmdResourceInfo,
    Fsm: CmdFsm,
{
    /// Executes the destructor for this type.
    fn drop(&mut self) {
        self.rimpl.borrow_mut().cleanup(self.instance_id);
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

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId, Event = Fsm::Event>, Fsm: CmdFsm>
    CmdResource<Res, Fsm>
{
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

    /// Find the context
    ///
    /// # Arguments
    ///
    /// * `predicate` - The predicate
    ///
    /// # Returns
    ///
    /// * `Option<Res::Context>` - The context
    pub fn find_ctx<F>(&self, predicate: F) -> Option<Res::Context>
    where
        F: Fn(&Res::Context) -> bool,
    {
        self.rimpl.borrow().find_ctx(predicate)
    }

    /// Self test if the resource is operating as expected
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance ID for this which self-test to be performed.
    /// * `test` - A function closure to be called to complete the seld test.
    pub fn self_test<FnTest>(&self, instance_id: usize, test: FnTest)
    where
        FnTest: Fn(&dyn PkaTrait) + 'static,
    {
        self.rimpl.borrow_mut().self_test(instance_id, test)
    }

    /// Cleanup the fsm
    ///
    /// # Arguments
    ///
    /// * `tag` - The tag for the async cleanup fs
    pub fn set_cleanup_fsm(&self, tag: TagId) {
        self.rimpl.borrow_mut().set_cleanup_fsm(tag);
    }

    /// Wake the next waiter
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance ID
    pub fn wakeup(&self, instance_id: usize) {
        self.rimpl.borrow_mut().wakeup(instance_id);
    }

    /// Operate on fixed resource by limiting the resource to a single instance
    ///
    /// # Arguments
    ///
    /// * `instance` - The resource instance ID
    #[cfg(feature = "fips_validation_hooks")]
    pub fn operate_on_fixed_resource(&self, instance: Option<usize>) {
        self.rimpl
            .borrow_mut()
            .resource
            .operate_on_fixed_resource(instance)
    }
}

/// Command Resource Implementation
pub struct CmdResourceImpl<Res: CmdResourceInfo, Fsm: CmdFsm> {
    /// Resources
    resource: Res,

    /// Waiters
    waiters: VecDeque<TagId>,

    /// Scheduler
    scheduler: CmdScheduler<Fsm>,

    /// Tag for the async cleanup fsm
    cleanup_fsm: Option<TagId>,
}

impl<Res: CmdResourceInfo<Id = Fsm::ResourceId, Event = Fsm::Event>, Fsm: CmdFsm>
    CmdResourceImpl<Res, Fsm>
{
    /// Create a new command resource
    fn new(resource: Res, scheduler: CmdScheduler<Fsm>, max_waiters: usize) -> Self {
        Self {
            resource,
            scheduler,
            waiters: VecDeque::with_capacity(max_waiters),
            cleanup_fsm: None,
        }
    }

    /// Acquire the resource
    fn acquire(&mut self, tag: TagId, ctx: Res::Context) -> Option<usize> {
        let transferred = self
            .scheduler
            .rsrc_ownership_transferred(self.resource.id());
        let count = self.resource.count();

        if count > 1 || (count == 1 && !transferred) {
            self.resource.set(ctx)
        } else {
            self.waiters.push_back(tag);
            None
        }
    }

    /// Cleanup
    fn cleanup(&mut self, instance_id: usize) {
        if let Some(tag) = self.cleanup_fsm {
            // Cleanup the resource context
            self.resource.cleanup_ctx(instance_id);

            // Cleanup the resource
            self.scheduler
                .cleanup(self.resource.cleanup_event(instance_id), tag);
        } else {
            self.wakeup(instance_id);
        }
    }

    /// Wake the next waiter
    fn wakeup(&mut self, instance_id: usize) {
        self.resource.clear(instance_id);
        if let Some(tag) = self.waiters.pop_front() {
            self.scheduler.wakeup(tag, self.resource.id());
        }
    }

    /// Get the resource
    fn resource(&self, instance_id: usize) -> &Res::Resource {
        self.resource.resource(instance_id)
    }

    /// Find the context
    fn find_ctx<F>(&self, predicate: F) -> Option<Res::Context>
    where
        F: Fn(&Res::Context) -> bool,
    {
        self.resource.find_ctx(predicate)
    }

    /// Self test if the resource is operating as expected
    ///
    /// # Arguments
    ///
    /// * `instance_id` - The resource instance ID for this which self-test to be performed.
    /// * `test` - A function closure to be called to complete the seld test.
    fn self_test<FnTest>(&mut self, instance_id: usize, test: FnTest)
    where
        FnTest: Fn(&dyn PkaTrait) + 'static,
    {
        self.resource.self_test(instance_id, test)
    }

    /// Cleanup the fsm
    fn set_cleanup_fsm(&mut self, tag: TagId) {
        self.cleanup_fsm = Some(tag);
    }
}
