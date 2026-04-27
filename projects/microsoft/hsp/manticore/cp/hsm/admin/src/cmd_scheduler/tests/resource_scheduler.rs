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
    AcquireResource,
    ResourceReady,
    End,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    Test,
    WaitingForResource,
    Done,
}

struct TestFsm {
    state: TestState,
    resource: Option<CmdResourceRef<TestResource, TestFsm>>,
    context: TestContext,
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

impl CmdFsm for TestFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (TestState::Init, TestEvent::AcquireResource) => {
                self.resource = self.context.acquire(tag);
                if let Some(r) = &self.resource {
                    r.map(|_| {});
                    self.state = TestState::Test;
                } else {
                    self.state = TestState::WaitingForResource;
                }
                Err(TestErr::Pending)
            }
            (TestState::WaitingForResource, TestEvent::ResourceReady) => {
                self.state = TestState::Test;
                Err(TestErr::Pending)
            }
            (TestState::Test, TestEvent::End) => {
                self.resource = None;
                self.state = TestState::Done;
                Ok(())
            }
            _ => unreachable!(),
        }
    }

    fn acquire_resource(&mut self, tag: TagId, _res_id: Self::ResourceId) -> Self::Event {
        self.resource = self.context.acquire(tag);
        assert!(self.resource.is_some());
        println!("acquire_resource");
        TestEvent::ResourceReady
    }
}

impl TestFsm {
    fn new(context: TestContext) -> Self {
        Self {
            state: TestState::Init,
            resource: None,
            context,
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
fn test_serial() {
    let scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let context = TestContext::new(scheduler.clone());
    let tag1 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();
    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));
    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), None);

    let tag2 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag2.is_some());
    let id2 = tag2.unwrap();
    scheduler.on_event(TestEvent::AcquireResource, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));
    scheduler.on_event(TestEvent::End, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), None);
}

#[test]
fn test_parallel() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let context = TestContext::new(scheduler.clone());

    let tag1 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();

    let tag2 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag2.is_some());
    let id2 = tag2.unwrap();

    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::AcquireResource, id2);
    assert_eq!(
        scheduler.map(id2, |fsm| fsm.state),
        Some(TestState::WaitingForResource)
    );

    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), None);

    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::End, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), None);
}
