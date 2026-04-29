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

enum Fsm {
    Test(TestFsm),
    Cleanup(TestCleanupFsm),
}

/// Combo FSM that can switch between Test and Cleanup FSMs
struct ComboTestFsm {
    fsm: Fsm,
}

impl CmdFsm for ComboTestFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    fn on_event(&mut self, event: Self::Event, tag: TagId) -> Result<(), Self::Error> {
        match &mut self.fsm {
            Fsm::Test(fsm) => fsm.on_event(event, tag),
            Fsm::Cleanup(fsm) => fsm.on_event(event, tag),
        }
    }

    fn acquire_resource(&mut self, tag: TagId, _res_id: Self::ResourceId) -> Self::Event {
        match &mut self.fsm {
            Fsm::Test(fsm) => fsm.acquire_resource(tag, ()),
            Fsm::Cleanup(fsm) => fsm.acquire_resource(tag, ()),
        }
    }
}

impl ComboTestFsm {
    fn new_test(fsm: TestFsm) -> Self {
        Self {
            fsm: Fsm::Test(fsm),
        }
    }

    fn new_cleanup(fsm: TestCleanupFsm) -> Self {
        Self {
            fsm: Fsm::Cleanup(fsm),
        }
    }
}

#[derive(Clone, Copy)]
enum TestEvent {
    AcquireResource,
    ResourceReady,

    #[allow(dead_code)]
    ResourceCleanup(usize),

    #[allow(dead_code)]
    ResourceCleanupDone(usize),
    DropResource,
    End,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    WaitingForResource,
    Intermediate,
    Test,
    Done,
}

/// Test FSM that uses a resource and drops it
struct TestFsm {
    state: TestState,
    resource: Option<CmdResourceRef<TestResource, ComboTestFsm>>,
    context: TestContext,
}

#[derive(Clone)]
struct TestContext {
    rimpl: Rc<RefCell<TestContextImpl>>,
}

impl TestContext {
    fn new(scheduler: CmdScheduler<ComboTestFsm>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(TestContextImpl::new(scheduler))),
        }
    }

    fn acquire(&self, tag: TagId) -> Option<CmdResourceRef<TestResource, ComboTestFsm>> {
        self.rimpl.borrow_mut().resource.acquire(tag, ())
    }

    fn set_cleanup(&self, tag: TagId) {
        self.rimpl.borrow_mut().set_cleanup(tag);
    }
}

struct TestContextImpl {
    resource: CmdResource<TestResource, ComboTestFsm>,
}

impl TestContextImpl {
    fn new(scheduler: CmdScheduler<ComboTestFsm>) -> Self {
        Self {
            resource: CmdResource::new(TestResource::default(), scheduler, 1),
        }
    }

    fn set_cleanup(&mut self, tag: TagId) {
        self.resource.set_cleanup_fsm(tag);
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
                    self.state = TestState::Intermediate;
                } else {
                    self.state = TestState::WaitingForResource;
                }
                Err(TestErr::Pending)
            }
            (TestState::WaitingForResource, TestEvent::ResourceReady) => {
                self.state = TestState::Test;
                Err(TestErr::Pending)
            }
            (TestState::Intermediate, TestEvent::DropResource) => {
                let _ = self.resource.take();
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

/// Resource Cleanup FSM
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestCleanupState {
    WaitingForCleanupRequest,
    WaitingForCleanupDone,
}

struct TestCleanupFsm {
    state: TestCleanupState,
}

impl CmdFsm for TestCleanupFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        match event {
            TestEvent::ResourceCleanup(_) => {
                self.state = TestCleanupState::WaitingForCleanupDone;
                Err(TestErr::Pending)
            }
            TestEvent::ResourceCleanupDone(_) => {
                self.state = TestCleanupState::WaitingForCleanupRequest;
                Err(TestErr::PendingAndDrainReady)
            }
            _ => Err(TestErr::Pending),
        }
    }
}

impl TestCleanupFsm {
    fn new() -> Self {
        Self {
            state: TestCleanupState::WaitingForCleanupRequest,
        }
    }
}

struct TestResource {
    resource_count: usize,
}

impl CmdResourceInfo for TestResource {
    type Id = ();
    type Resource = ();
    type Event = TestEvent;
    type Context = ();

    fn id(&self) -> Self::Id {}

    fn count(&self) -> usize {
        self.resource_count
    }

    fn set(&mut self, _ctx: Self::Context) -> Option<usize> {
        self.resource_count -= 1;
        Some(0)
    }

    fn clear(&mut self, _idx: usize) {
        self.resource_count += 1;
    }

    fn resource(&self, _idx: usize) -> &Self::Resource {
        &()
    }

    fn find_ctx<F>(&self, _predicate: F) -> Option<Self::Context>
    where
        F: Fn(&Self::Context) -> bool,
    {
        todo!()
    }

    fn cleanup_event(&self, instance_id: usize) -> Self::Event {
        TestEvent::ResourceCleanup(instance_id)
    }

    fn cleanup_ctx(&mut self, _instance_id: usize) {}
}

impl Default for TestResource {
    fn default() -> Self {
        Self { resource_count: 1 }
    }
}

#[test]
fn test_resource_with_cleanup() {
    let scheduler = CmdScheduler::<ComboTestFsm>::new(2, 1, TestRecorder {});

    let cleanup_tag = scheduler.alloc_static(ComboTestFsm::new_cleanup(TestCleanupFsm::new()));
    assert!(cleanup_tag.is_some());
    let cleanup_id = cleanup_tag.unwrap();

    let context = TestContext::new(scheduler.clone());
    context.set_cleanup(cleanup_id);
    let tag1 = scheduler.alloc(ComboTestFsm::new_test(TestFsm::new(context.clone())));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();
    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| {
            if let Fsm::Test(fsm) = &fsm.fsm {
                fsm.state
            } else {
                panic!("Expected Test FSM");
            }
        }),
        Some(TestState::Intermediate)
    );

    scheduler.on_event(TestEvent::DropResource, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| {
            if let Fsm::Test(fsm) = &fsm.fsm {
                fsm.state
            } else {
                panic!("Expected Test FSM");
            }
        }),
        Some(TestState::Test)
    );

    // Check that the cleanup FSM is in the waiting state
    assert_eq!(
        scheduler.map(cleanup_id, |fsm| {
            if let Fsm::Cleanup(fsm) = &fsm.fsm {
                fsm.state
            } else {
                panic!("Expected Cleanup FSM");
            }
        }),
        Some(TestCleanupState::WaitingForCleanupDone)
    );

    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| {
            if let Fsm::Test(fsm) = &fsm.fsm {
                fsm.state
            } else {
                panic!("Expected Test FSM");
            }
        }),
        None
    );

    // Finally check that the cleanup FSM is done cleaning up on CleanupDone event.
    scheduler.on_event(TestEvent::ResourceCleanupDone(0), cleanup_id);
    assert_eq!(
        scheduler.map(cleanup_id, |fsm| {
            if let Fsm::Cleanup(fsm) = &fsm.fsm {
                fsm.state
            } else {
                panic!("Expected Cleanup FSM");
            }
        }),
        Some(TestCleanupState::WaitingForCleanupRequest)
    );
}
