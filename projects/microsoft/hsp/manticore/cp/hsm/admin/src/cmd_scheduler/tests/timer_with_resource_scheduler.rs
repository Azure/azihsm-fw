// Copyright (c) Microsoft Corporation. All rights reserved.

use std::cell::RefCell;
use std::rc::Rc;

use crate::*;

#[derive(Clone)]
struct TestRecorder {}

#[derive(PartialEq, Eq, Clone, Copy)]
enum TestErr {
    Pending,
}

impl CmdFsmError for TestErr {
    fn pending(&self) -> bool {
        *self == Self::Pending
    }
}

impl CmdFsmEventRecorder for TestRecorder {
    type Error = TestErr;
    type Event = TestEvent;
}

#[derive(Clone, Copy)]
enum TestEvent {
    Start, // The start event will trigger the FSM to acquire resources, only if successful the timer will start
    ResourceReady, // Resource is ready, start the timer
    Timeout, // Once timeout, the resource will be released
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    WaitingForResource,
    Test,
    Done,
}

struct TestFsm {
    state: TestState,
    timer: CmdTimer,
    start_timer: u8,
    resource: Option<CmdResourceRef<TestResource, TestFsm>>,
    context: TestContext,
}

impl TestFsm {
    fn new(start_timer: u8, context: TestContext) -> Self {
        Self {
            state: TestState::Init,
            timer: CmdTimer::new(),
            start_timer,
            resource: None,
            context,
        }
    }
}

impl CmdFsm for TestFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    // Timer trait implementation
    fn get_timer(&mut self) -> Option<&mut CmdTimer> {
        Some(&mut self.timer)
    }

    // CmdFsm trait implementation
    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (TestState::Init, TestEvent::Start) => {
                self.resource = self.context.acquire(tag);
                if self.resource.is_some() {
                    self.state = TestState::Test;
                    self.timer.start(self.start_timer);
                } else {
                    self.state = TestState::WaitingForResource;
                }

                Err(TestErr::Pending)
            }
            (TestState::WaitingForResource, TestEvent::ResourceReady) => {
                assert!(self.resource.is_some());

                self.state = TestState::Test;
                self.timer.start(self.start_timer);

                Err(TestErr::Pending)
            }
            (TestState::Test, TestEvent::Timeout) => {
                self.state = TestState::Done;
                self.resource.take();

                Err(TestErr::Pending)
            }
            _ => unreachable!(),
        }
    }

    fn acquire_resource(&mut self, tag: TagId, _res_id: Self::ResourceId) -> Self::Event {
        assert!(self.resource.is_none());
        self.resource = self.context.acquire(tag);
        assert!(self.resource.is_some());
        TestEvent::ResourceReady
    }
}

#[derive(Clone)]
struct TestContext {
    rimpl: Rc<RefCell<TestContextImpl>>,
}

impl TestContext {
    fn new(scheduler: CmdScheduler<TestFsm>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(TestContextImpl::new(scheduler))),
        }
    }

    fn acquire(&self, tag: TagId) -> Option<CmdResourceRef<TestResource, TestFsm>> {
        self.rimpl.borrow_mut().resource.acquire(tag, ())
    }
}

struct TestContextImpl {
    resource: CmdResource<TestResource, TestFsm>,
}

impl TestContextImpl {
    fn new(scheduler: CmdScheduler<TestFsm>) -> Self {
        Self {
            resource: CmdResource::new(TestResource::default(), scheduler, 1),
        }
    }
}

#[derive(Default)]
struct TestResource {}

impl CmdResourceInfo for TestResource {
    type Id = ();
    type Resource = ();
    type Context = ();

    fn id(&self) -> Self::Id {}

    fn max_count(&self) -> usize {
        1
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {}

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &()
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        todo!()
    }
}

#[test]
fn test_timer_with_resource_scheduler() {
    let expected_ticks = 3;
    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let context = TestContext::new(scheduler.clone());
    let tag = scheduler.alloc(TestFsm::new(expected_ticks, context.clone()));

    assert!(tag.is_some());
    let id = tag.unwrap();

    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    assert_eq!(scheduler.map(id, |fsm| fsm.resource.is_some()), Some(true));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_tick(TestEvent::Timeout);

    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Done));
    assert_eq!(scheduler.map(id, |fsm| fsm.resource.is_some()), Some(false));
}

#[test]
fn test_timer_with_resource_scheduler_multiple_fsms() {
    let expected_ticks = 3;

    let mut scheduler = CmdScheduler::<TestFsm>::new(2, 2, TestRecorder {});
    let context = TestContext::new(scheduler.clone());
    let tag1 = scheduler.alloc(TestFsm::new(expected_ticks, context.clone()));
    let tag2 = scheduler.alloc(TestFsm::new(expected_ticks, context.clone()));

    assert!(tag1.is_some());
    assert!(tag2.is_some());
    let id1 = tag1.unwrap();
    let id2 = tag2.unwrap();
    scheduler.on_event(TestEvent::Start, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));
    assert_eq!(scheduler.map(id1, |fsm| fsm.resource.is_some()), Some(true));

    scheduler.on_event(TestEvent::Start, id2);
    assert_eq!(
        scheduler.map(id2, |fsm| fsm.state),
        Some(TestState::WaitingForResource)
    );
    assert_eq!(
        scheduler.map(id2, |fsm| fsm.resource.is_some()),
        Some(false)
    );

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));
        assert_eq!(
            scheduler.map(id2, |fsm| fsm.state),
            Some(TestState::WaitingForResource)
        );
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Done));
    assert_eq!(
        scheduler.map(id1, |fsm| fsm.resource.is_some()),
        Some(false)
    );
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));
    assert_eq!(scheduler.map(id2, |fsm| fsm.resource.is_some()), Some(true));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Done));
    assert_eq!(
        scheduler.map(id2, |fsm| fsm.resource.is_some()),
        Some(false)
    );
}
