// Copyright (c) Microsoft Corporation. All rights reserved.

use std::cell::RefCell;
use std::rc::Rc;

use crate::*;

#[derive(Clone)]
struct TestRecorder {}

#[derive(PartialEq, Eq, Clone, Copy)]
enum TestErr {
    Pending,
    PendingAndDrainReady,
}

impl CmdFsmError for TestErr {
    fn pending(&self) -> bool {
        *self == Self::Pending
    }

    fn drain_ready(&self) -> bool {
        *self == Self::PendingAndDrainReady
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
    fn new(count: usize, scheduler: CmdScheduler<TestFsm>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(TestContextImpl::new(count, scheduler))),
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
    fn new(count: usize, scheduler: CmdScheduler<TestFsm>) -> Self {
        Self {
            resource: CmdResource::new(TestResource::new(count), scheduler, 1),
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
struct TestResource {
    free_list: Vec<usize>,
}

impl TestResource {
    pub fn new(count: usize) -> Self {
        // Populate the entries based on inner.size()
        let free_list = (0..count).collect();
        Self { free_list }
    }
}

impl CmdResourceInfo for TestResource {
    type Id = ();
    type Resource = Self;
    type Event = TestEvent;
    type Context = ();

    fn id(&self) -> Self::Id {}

    fn count(&self) -> usize {
        self.free_list.len()
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        let index = self.free_list.pop()?;
        Some(index)
    }

    fn clear(&mut self, instance_id: usize) {
        //push it back in the free List
        self.free_list.push(instance_id);
    }

    fn resource(&self, _idx: usize) -> &Self::Resource {
        self
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        todo!()
    }

    fn cleanup_event(&self, _instance_id: usize) -> Self::Event {
        todo!()
    }
}

#[test]
fn test_serial() {
    let scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let context = TestContext::new(2, scheduler.clone());

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

    let tag3 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag3.is_some());
    let id3 = tag3.unwrap();
    scheduler.on_event(TestEvent::AcquireResource, id3);
    assert_eq!(scheduler.map(id3, |fsm| fsm.state), Some(TestState::Test));
    scheduler.on_event(TestEvent::End, id3);
    assert_eq!(scheduler.map(id3, |fsm| fsm.state), None);

    let tag4 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag4.is_some());
    let id4 = tag4.unwrap();
    scheduler.on_event(TestEvent::AcquireResource, id4);
    assert_eq!(scheduler.map(id4, |fsm| fsm.state), Some(TestState::Test));
    scheduler.on_event(TestEvent::End, id4);
    assert_eq!(scheduler.map(id4, |fsm| fsm.state), None);
}

#[test]
fn test_parallel() {
    let scheduler = CmdScheduler::<TestFsm>::new(3, 1, TestRecorder {});
    let context = TestContext::new(2, scheduler.clone());

    let tag1 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();

    let tag2 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag2.is_some());
    let id2 = tag2.unwrap();

    let tag3 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag3.is_some());
    let id3 = tag3.unwrap();

    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::AcquireResource, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::AcquireResource, id3);
    assert_eq!(
        scheduler.map(id3, |fsm| fsm.state),
        Some(TestState::WaitingForResource)
    );

    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), None);

    assert_eq!(scheduler.map(id3, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::End, id3);
    assert_eq!(scheduler.map(id3, |fsm| fsm.state), None);

    scheduler.on_event(TestEvent::End, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), None);
}
