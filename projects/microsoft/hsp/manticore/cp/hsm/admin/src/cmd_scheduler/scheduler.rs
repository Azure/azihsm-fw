// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;

use super::CmdTimer;
use super::TagId;

pub trait CmdFsmError {
    fn pending(&self) -> bool;
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

    /// Get the timer for this command, if it has one.
    ///
    /// # Returns
    ///
    /// A reference to the timer, if the command has one.
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        None
    }
}

/// Command Scheduler
pub struct CmdScheduler<Fsm: CmdFsm> {
    /// Command Scheduler Implementation
    rimpl: Rc<RefCell<CmdSchedulerImpl<Fsm>>>,

    /// Ready Queue
    #[allow(clippy::type_complexity)]
    ready_queue: Rc<RefCell<Vec<(TagId, Fsm::ResourceId)>>>,
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

    /// Allocate a static command that never gets deallocated
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

    /// Enable the scheduler empty notification
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `event` - Event ID that will be used to indicate the scheduler is empty
    ///
    pub fn set_scheduler_empty_notification(&self, tag: TagId, event: Fsm::Event) {
        self.rimpl
            .borrow_mut()
            .set_scheduler_empty_notification(tag, event);
    }

    /// On Event
    ///
    /// # Arguments
    ///
    /// * `id` - Event ID
    /// * `tag` - Tag ID
    pub fn on_event(&self, id: Fsm::Event, tag: TagId) {
        self.rimpl.borrow_mut().on_event(id, tag);

        self.check_ready_queue_and_scheduler_empty();
    }

    /// On Tick
    ///
    /// # Arguments
    ///
    /// * `timeout_event` - The event type signaling timeout to FSMs.
    pub fn on_tick(&mut self, timeout_event: Fsm::Event) {
        self.rimpl.borrow_mut().on_tick(timeout_event);

        self.check_ready_queue_and_scheduler_empty();
    }

    /// Maps a `FnOnce` closue on `Fsm` if the tag is valid
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
        F: FnOnce(&mut Fsm) -> T,
    {
        self.rimpl.borrow_mut().fsm(tag).map(f)
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

    fn check_ready_queue_and_scheduler_empty(&self) {
        // Store the values from the iterator to a local vector
        let mut ready_queue_copy = Vec::with_capacity(self.ready_queue.borrow().len());
        for (tag, res_id) in self.ready_queue.borrow_mut().drain(..) {
            ready_queue_copy.push((tag, res_id));
        }

        for (tag, res_id) in ready_queue_copy.drain(..) {
            self.rimpl.borrow_mut().on_ready(tag, res_id);
        }

        // Check and notify the scheduler if it is empty
        self.rimpl.borrow_mut().check_scheduler_empty();
    }
}

impl<Fsm: CmdFsm> Clone for CmdScheduler<Fsm> {
    /// Returns a copy of the value.
    fn clone(&self) -> Self {
        Self {
            rimpl: self.rimpl.clone(),
            ready_queue: self.ready_queue.clone(),
        }
    }
}

/// Command Slot
struct CmdSlot<Fsm: CmdFsm> {
    /// Finite State Machine and Tag
    fsm: Option<(Fsm, TagId)>,
}

impl<Fsm: CmdFsm> CmdSlot<Fsm> {
    fn on_event(
        &mut self,
        event: Fsm::Event,
        recorder: &mut Fsm::Recorder,
        free_tags: &mut Vec<u16>,
    ) {
        if let Some(fsm) = self.fsm.as_mut() {
            let tag = fsm.1;

            recorder.on_event_start(event, tag);
            let result = fsm.0.on_event(event, tag);
            recorder.on_event_end(event, tag, result);

            let reset = if result.is_ok() {
                true
            } else {
                !result.err().unwrap().pending()
            };

            if reset {
                self.fsm = None;
                free_tags.push(tag);
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

    /// Total static slots
    slots_static_count: usize,

    /// Scheduler empty notification tagID
    scheduler_empty_tag: Option<TagId>,

    /// Scheduler empty notification event
    scheduler_empty_event: Option<Fsm::Event>,
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
            slots_static_count: 0,
            scheduler_empty_tag: None,
            scheduler_empty_event: None,
        }
    }

    /// Allocate a command
    fn alloc(&mut self, fsm: Fsm) -> Option<TagId> {
        let tag = self.free_tags.pop()?;

        let cmd = &mut self.slots[tag as usize];

        let id = tag as TagId;
        cmd.fsm = Some((fsm, id));

        Some(id)
    }

    /// Allocate a static command that never gets deallocated
    fn alloc_static(&mut self, fsm: Fsm) -> Option<TagId> {
        let tag = self.alloc(fsm);
        if tag.is_some() {
            self.slots_static_count += 1;
        }

        tag
    }

    /// Set the scheduler empty notification
    fn set_scheduler_empty_notification(&mut self, tag: TagId, event: Fsm::Event) {
        self.scheduler_empty_tag = Some(tag);
        self.scheduler_empty_event = Some(event);
    }

    /// On Event
    fn on_event(&mut self, event: Fsm::Event, tag: TagId) {
        let index = tag as usize;
        let cmd = &mut self.slots[index];
        cmd.on_event(event, &mut self.recorder, &mut self.free_tags)
    }

    /// On Ready
    fn on_ready(&mut self, tag: TagId, res_id: Fsm::ResourceId) {
        let index = tag as usize;
        let cmd = &mut self.slots[index];

        if let Some(fsm) = cmd.fsm.as_mut() {
            let event = fsm.0.acquire_resource(tag, res_id);
            cmd.on_event(event, &mut self.recorder, &mut self.free_tags)
        }
    }

    /// On Tick
    fn on_tick(&mut self, timeout_event: Fsm::Event) {
        for cmd in self.slots.iter_mut().filter(|c| c.fsm.is_some()) {
            let fsm = cmd.fsm.as_mut().unwrap();
            if let Some(timer) = fsm.0.get_timer() {
                if timer.tick() {
                    cmd.on_event(timeout_event, &mut self.recorder, &mut self.free_tags);
                }
            }
        }
    }

    /// On Scheduler Empty
    fn check_scheduler_empty(&mut self) {
        if let Some(tag) = self.scheduler_empty_tag {
            if let Some(event) = self.scheduler_empty_event {
                if self.slots.len() - self.free_tags.len() == self.slots_static_count {
                    self.on_event(event, tag);
                }
            }
        }
    }

    /// Get the command's finite state machine
    #[cfg(test)]
    fn fsm(&mut self, tag: TagId) -> Option<&mut Fsm> {
        use core::ops::IndexMut;

        let index = tag as usize;
        let cmd = self.slots.index_mut(index);

        if let Some(fsm) = cmd.fsm.as_mut() {
            let (ref mut f, _) = fsm;
            Some(f)
        } else {
            None
        }
    }
}
