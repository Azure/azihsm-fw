// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use crate::error::HsmErr;
use crate::error::HsmResult;
use crate::TagId;

pub trait CmdFsmError {
    fn pending(&self) -> bool;
    fn drain_ready(&self) -> bool;
}

/// Event Recorder Trait
pub trait CmdFsmEventRecorder: Clone {
    type Error: CmdFsmError + Copy;
    type Event: Copy;

    /// On start event
    ///
    /// # Arguments
    ///
    /// * `event` - Event
    /// * `tag`   - Tag
    #[allow(unused_variables)]
    fn on_event_start(&self, event: Self::Event, tag: TagId) {}

    /// On end event
    ///
    /// # Arguments
    ///
    /// * `event`  - Event
    /// * `tag`    - Tag
    /// * `result` - Event processing result
    #[allow(unused_variables)]
    fn on_event_end(&self, event: Self::Event, tag: TagId, result: Result<(), Self::Error>) {}
}

/// Command Finite State Machine Trait
pub trait CmdFsm {
    type Error: CmdFsmError + Copy;
    type ResourceId: Copy + PartialEq;
    type Event: Copy;
    type Recorder: CmdFsmEventRecorder<Error = Self::Error, Event = Self::Event>;

    /// On Event
    ///
    /// # Arguments
    ///
    /// * `event` - Event
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * `Result<(), Self::Error>` - Result of the event processing
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error>;

    /// Acquire a resource
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `res_id` - Resource ID
    ///
    /// # Returns
    ///
    /// * `Self::Event` - Event to wake the state machine with
    #[allow(unused_variables)]
    fn acquire_resource(&mut self, tag: TagId, res_id: Self::ResourceId) -> Self::Event {
        unimplemented!()
    }
}

/// Command Scheduler
pub struct CmdScheduler<Fsm: CmdFsm> {
    /// Command Scheduler Implementation
    rimpl: Rc<RefCell<CmdSchedulerImpl<Fsm>>>,

    /// Ready Queue
    #[allow(clippy::type_complexity)]
    ready_queue: Rc<RefCell<Vec<(TagId, Fsm::ResourceId)>>>,

    /// Cleanup Queue
    #[allow(clippy::type_complexity)]
    cleanup_queue: Rc<RefCell<Vec<(TagId, Fsm::Event)>>>,
}

impl<Fsm: CmdFsm> CmdScheduler<Fsm> {
    /// Create a new instance of `CmdScheduler`
    ///
    /// # Arguments
    ///
    /// * `cmd_count` - Number of commands
    /// * `max_wake_count` - Maximum number of commands woken up by a command
    /// * `recorder` -
    //
    /// # Returns
    ///
    /// * `CmdScheduler` - New instance of `CmdScheduler`
    pub fn new(cmd_count: usize, max_wake_count: usize, recorder: Fsm::Recorder) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CmdSchedulerImpl::new(cmd_count, recorder))),
            ready_queue: Rc::new(RefCell::new(Vec::with_capacity(max_wake_count))),
            cleanup_queue: Rc::new(RefCell::new(Vec::with_capacity(max_wake_count))),
        }
    }

    /// Allocate a command
    ///
    /// # Arguments
    ///
    /// * `fsm` - Command Finite State Machine
    ///
    /// # Returns
    ///
    /// * `Option<TagId>` - Tag ID if the command was allocated, `None` otherwise
    pub fn alloc(&self, fsm: Fsm) -> Option<TagId> {
        self.rimpl.borrow_mut().alloc(fsm)
    }

    /// Allocate a static command
    ///
    /// # Arguments
    ///
    /// * `fsm` - Command Finite State Machine
    ///
    /// # Returns
    ///
    /// * `Option<TagId>` - Tag ID if the command was allocated, `None` otherwise
    pub fn alloc_static(&self, fsm: Fsm) -> Option<TagId> {
        self.rimpl.borrow_mut().alloc_static(fsm)
    }

    /// On Event
    ///
    /// # Arguments
    ///
    /// * `id` - Event ID
    /// * `tag` - Tag ID
    pub fn on_event(&self, event: Fsm::Event, tag: TagId) {
        self.rimpl.borrow_mut().on_event(event, tag);

        let mut cleanup_queue_copy = Vec::with_capacity(self.cleanup_queue.borrow().len());
        for (tag, event) in self.cleanup_queue.borrow_mut().drain(..) {
            cleanup_queue_copy.push((tag, event));
        }

        for (tag, event) in cleanup_queue_copy.drain(..) {
            self.rimpl.borrow_mut().on_cleanup(tag, event);
        }

        // Store the values from the iterator
        let mut ready_queue_copy = Vec::with_capacity(self.ready_queue.borrow().len());
        for (tag, res_id) in self.ready_queue.borrow_mut().drain(..) {
            ready_queue_copy.push((tag, res_id));
        }

        for (tag, res_id) in ready_queue_copy.drain(..) {
            self.rimpl.borrow_mut().on_ready(tag, res_id);
        }
    }

    pub fn cleanup(&self, event: Fsm::Event, tag: TagId) {
        self.cleanup_queue.borrow_mut().push((tag, event));
    }

    /// On Tick
    ///
    /// # Arguments
    ///
    /// * `timeout_event` - The event type signaling timeout to FSMs.
    pub fn on_tick(&self, _timeout_event: Fsm::Event) {
        #[cfg(not(feature = "disable_io_timeout_handling"))]
        self.rimpl.borrow_mut().on_tick(_timeout_event);
    }

    /// Maps a `FnOnce` closure on `Fsm` if the tag is valid
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `f` - `FnOnce` closure
    ///
    /// # Returns
    ///
    /// * `Option<T>` - Result of the closure if the tag is valid, `None` otherwise
    #[cfg(test)]
    pub fn map<F, T>(&self, tag: TagId, f: F) -> Option<T>
    where
        F: FnOnce(&Fsm) -> T,
    {
        self.rimpl.borrow().fsm(tag).map(f)
    }

    /// Wakeup a command
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID of the command to wakeup
    /// * `res_id` - Resource ID that caused the wakeup
    pub(crate) fn wakeup(&self, tag: u16, res_id: Fsm::ResourceId) {
        self.ready_queue.borrow_mut().push((tag, res_id));
    }

    /// Checks if the resource is already in the ready queue.
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID of the command to wakeup
    /// * `res_id` - Resource ID that caused the wakeup
    pub(crate) fn rsrc_ownership_transferred(&self, res_id: Fsm::ResourceId) -> bool {
        for entry in self.ready_queue.borrow().iter() {
            if entry.1 == res_id {
                return true;
            }
        }

        false
    }

    /// Drain the commands from the scheduler and call the closure once the scheduler is empty.
    ///
    /// # Arguments
    ///
    /// * `io_drain` - Closure to be invoked when the drain scenario is detected
    ///
    /// # Returns
    ///
    /// * Ok(()) if the drain can be completed, Err() if the drain cannot be accepted at this time
    ///
    /// # Notes
    ///
    /// `FnDrain` is a closure function that is expected to perform synchronous operation, as the
    /// command scheduler itself does not provide any facility to handle asynchronous states and
    /// should be managed by the caller if required at all.
    ///
    /// # Warning
    ///
    /// Do NOT pass a closure that could submit an asynchronous operation and expect handling of
    /// completion of asynchronous events within the scheduler.
    pub(crate) fn drain<FnDrain>(&self, io_drain: FnDrain) -> HsmResult<()>
    where
        FnDrain: Fn() + 'static,
    {
        self.rimpl.borrow_mut().drain(io_drain)
    }

    /// Get the occupancy list mantained by this scheduler instance
    ///
    /// # Returns
    ///
    /// * u128 - A u128 mask that represents the occupancy list in the scheduler
    #[cfg(test)]
    pub(crate) fn occupancy_list(&self) -> u128 {
        self.rimpl.borrow().occupancy_list
    }

    /// Get the access list mantained by this scheduler instance
    ///
    /// # Returns
    ///
    /// * u128 - A u128 mask that represents the access list in the scheduler
    #[cfg(test)]
    pub(crate) fn access_list(&self) -> u128 {
        self.rimpl.borrow().access_list
    }
}

impl<Fsm: CmdFsm> Clone for CmdScheduler<Fsm> {
    /// Returns a copy of the value.
    fn clone(&self) -> Self {
        Self {
            rimpl: self.rimpl.clone(),
            ready_queue: self.ready_queue.clone(),
            cleanup_queue: self.cleanup_queue.clone(),
        }
    }
}

/// Command Slot
struct CmdSlot<Fsm: CmdFsm> {
    /// Finite State Machine and Tag
    fsm: Option<(Fsm, TagId)>,
}

impl<Fsm: CmdFsm> CmdSlot<Fsm> {
    /// On command event
    fn on_event(
        &mut self,
        event: Fsm::Event,
        recorder: &mut Fsm::Recorder,
        free_tags: &mut Vec<u16>,
        occupancy_list: &mut u128,
        access_list: &mut u128,
        fn_drain: &mut Option<Box<dyn Fn()>>,
    ) {
        if let Some(fsm) = self.fsm.as_mut() {
            let tag = fsm.1;

            // Set the access list to indicate that this command FSM is active
            *access_list |= 1 << tag;

            recorder.on_event_start(event, tag);
            let result = fsm.0.on_event(event, tag);
            recorder.on_event_end(event, tag, result);

            // `reset` flag is used to indicate if the command FSM is de-commissioned which
            // is when a command FSM returns Ok or Err but not pending. Static FSMs
            // need to be kept around even after it indicates drain ready. `static_fsm` flag is
            // used to indicate if the `reset` flag is set by a Static FSM. This is only true for Static FSM but
            // HSM FSM returns this as false when it sets the `reset` flag.
            let (reset, static_fsm) = match result {
                Ok(_) => (true, false),
                Err(e) => (!e.pending(), e.drain_ready()),
            };

            if reset {
                if !static_fsm {
                    self.fsm = None;
                    free_tags.push(tag);
                }

                // Clear the occupancy list to indicate that this command FSM is de-commissioned
                *occupancy_list &= !(1 << tag);

                if *occupancy_list == 0 {
                    if let Some(drain_fn) = fn_drain.take() {
                        drain_fn()
                    }
                }
            }
        }
    }
}

impl<Fsm: CmdFsm> Default for CmdSlot<Fsm> {
    /// Returns the "default value" for a type.
    fn default() -> Self {
        Self { fsm: None }
    }
}

/// Command Scheduler Implementation
struct CmdSchedulerImpl<Fsm: CmdFsm> {
    /// Scheduling Slots
    slots: Vec<CmdSlot<Fsm>>,

    /// Tags
    free_tags: Vec<u16>,

    /// Trace Function
    recorder: Fsm::Recorder,

    /// Command occupancy list in the scheduler's command slot
    occupancy_list: u128,

    /// Command slots that processed an event from the last timer expiry (active commands)
    access_list: u128,

    /// Drain closure to be invoked if a drain is pending
    fn_drain: Option<Box<dyn Fn()>>,
}

impl<Fsm: CmdFsm> CmdSchedulerImpl<Fsm> {
    /// Create a new instance of `CmdSchedulerImpl`
    fn new(size: usize, recorder: Fsm::Recorder) -> Self {
        let mut cmds = Vec::with_capacity(size);
        let mut free_tags = Vec::with_capacity(size);

        for index in 0..size as u16 {
            cmds.push(CmdSlot::default());
            free_tags.push(index);
        }
        Self {
            slots: cmds,
            free_tags,
            recorder,
            occupancy_list: Default::default(),
            access_list: Default::default(),
            fn_drain: None,
        }
    }

    /// Allocate a command
    fn alloc(&mut self, fsm: Fsm) -> Option<TagId> {
        let tag = self.free_tags.pop()?;

        let cmd = &mut self.slots[tag as usize];

        let id = tag as TagId;
        cmd.fsm = Some((fsm, id));

        // Set the occupancy list to indicate that this command FSM is newly commissioned
        self.occupancy_list |= 1 << id;

        Some(id)
    }

    /// Allocate a static command
    fn alloc_static(&mut self, fsm: Fsm) -> Option<TagId> {
        let tag = self.free_tags.pop()?;

        let cmd = &mut self.slots[tag as usize];

        let id = tag as TagId;
        cmd.fsm = Some((fsm, id));

        Some(id)
    }

    /// On Event
    fn on_event(&mut self, event: Fsm::Event, tag: TagId) {
        let index = tag as usize;
        let cmd = &mut self.slots[index];

        cmd.on_event(
            event,
            &mut self.recorder,
            &mut self.free_tags,
            &mut self.occupancy_list,
            &mut self.access_list,
            &mut self.fn_drain,
        )
    }

    /// On Ready
    fn on_ready(&mut self, tag: TagId, res_id: Fsm::ResourceId) {
        let index = tag as usize;
        let cmd = &mut self.slots[index];

        if let Some(fsm) = cmd.fsm.as_mut() {
            let event = fsm.0.acquire_resource(tag, res_id);

            // Mark the access list to indicate that this cmd FSM is active
            self.access_list |= 1 << index;

            cmd.on_event(
                event,
                &mut self.recorder,
                &mut self.free_tags,
                &mut self.occupancy_list,
                &mut self.access_list,
                &mut self.fn_drain,
            )
        }
    }

    /// On Clenup
    fn on_cleanup(&mut self, tag: TagId, event: Fsm::Event) {
        let index = tag as usize;
        let cmd = &mut self.slots[index];

        cmd.on_event(
            event,
            &mut self.recorder,
            &mut self.free_tags,
            &mut self.occupancy_list,
            &mut self.access_list,
            &mut self.fn_drain,
        )
    }

    /// On Tick
    #[cfg(not(feature = "disable_io_timeout_handling"))]
    fn on_tick(&mut self, timeout_event: Fsm::Event) {
        let mut inactive_list = self.occupancy_list ^ self.access_list;
        while inactive_list != 0 {
            let index = inactive_list.trailing_zeros() as usize;
            let cmd = &mut self.slots[index];
            cmd.on_event(
                timeout_event,
                &mut self.recorder,
                &mut self.free_tags,
                &mut self.occupancy_list,
                &mut self.access_list,
                &mut self.fn_drain,
            );
            inactive_list &= !(1 << index);
        }

        self.access_list = 0;
    }

    /// Get the command's finite state machine
    #[cfg(test)]
    fn fsm(&self, tag: TagId) -> Option<&Fsm> {
        let index = tag as usize;
        let cmd = &self.slots[index];

        if let Some(fsm) = cmd.fsm.as_ref() {
            Some(&fsm.0)
        } else {
            None
        }
    }

    /// Request to drain all IO requests and issue callback immediately or in deferred manner
    fn drain<FnDrain>(&mut self, io_drain: FnDrain) -> HsmResult<()>
    where
        FnDrain: Fn() + 'static,
    {
        if self.fn_drain.is_some() {
            Err(HsmErr::DrainBusy)?
        }

        if self.occupancy_list == 0 {
            io_drain()
        } else {
            self.fn_drain = Some(Box::new(io_drain));
        }

        Ok(())
    }
}
