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

#[derive(Clone, Copy, Debug)]
enum TestEvent {
    AcquireResource,
    ResourceReady,
    DropAndReacquireResource,
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
        println!("tag[{:?}] State: {:?}, Event: {:?}", tag, self.state, event);
        match (self.state, event) {
            (TestState::Init, TestEvent::AcquireResource) => {
                if self.handle_acquire(tag) {
                    self.state = TestState::Intermediate;
                } else {
                    self.state = TestState::WaitingForResource;
                }
                Err(TestErr::Pending)
            }
            (TestState::WaitingForResource, TestEvent::ResourceReady) => {
                // we expect the resource to be acquired here.
                assert!(self.handle_acquire(tag));
                println!("tag[{:?}] resource acquired", tag);

                self.state = TestState::Test;
                Err(TestErr::Pending)
            }
            (TestState::Intermediate, TestEvent::DropAndReacquireResource) => {
                println!("tag[{:?}] dropping and reacquiring resource", tag);
                self.handle_drop();
                assert!(self.resource.is_none());

                if self.handle_acquire(tag) {
                    println!("tag[{:?}] resource reacquired", tag);
                    self.state = TestState::Test;
                } else {
                    self.state = TestState::WaitingForResource;
                }
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
        println!("Tag[{:?}] acquire_resource", tag);
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

    fn handle_acquire(&mut self, tag: TagId) -> bool {
        self.resource = self.context.acquire(tag);
        if let Some(r) = &self.resource {
            r.map(|_| {});
            true
        } else {
            false
        }
    }

    fn handle_drop(&mut self) {
        let _ = self.resource.take();
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

    fn cleanup_event(&self, _instance_id: usize) -> Self::Event {
        todo!()
    }

    fn cleanup_ctx(&mut self, _instance_id: usize) {
        unimplemented!()
    }
}

impl Default for TestResource {
    fn default() -> Self {
        Self { resource_count: 1 }
    }
}

#[test]
fn test_reacquire_multiple() {
    // Test Scenario:
    // 1. Fsm1 acquires resource and goes to intermediate state.
    // 2. Fsm2 tries to acquire resource and goes to waiting state.
    // 3. Fsm1 drops and reacquires resource and goes to waiting state.
    // 4. Fsm2 is woken up and acquires resource and runs to completion making the resource available.
    // 5. Fsm1 is woken up and reacquires resource and runs to completion.

    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let context = TestContext::new(scheduler.clone());

    let tag1 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();

    let tag2 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag2.is_some());
    let id2 = tag2.unwrap();

    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| fsm.state),
        Some(TestState::Intermediate)
    );

    scheduler.on_event(TestEvent::AcquireResource, id2);
    assert_eq!(
        scheduler.map(id2, |fsm| fsm.state),
        Some(TestState::WaitingForResource)
    );

    scheduler.on_event(TestEvent::DropAndReacquireResource, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| fsm.state),
        Some(TestState::WaitingForResource)
    );

    // After the above call, when FSM1 returns pending, the scheduler should pull FSM2 from the
    // the ready queue and call acquire_resources for it. This would have pushed FSM2 to Test state.
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::End, id2);
    assert_eq!(scheduler.map(id2, |fsm| fsm.state), None);

    // This should make the scheduler to call acquire_resources for FSM1 which should have enqueued
    // ResourceReady to the state machine pushing the state to Test.
    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), None);
}

#[test]
fn test_reacquire_single() {
    // Test Scenario:
    // 1. Fsm1 acquires resource
    // 2. Fsm1 drops and reacquires resource. Since there is no one else in the waiting queue, it should succeed immediately.

    let scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let context = TestContext::new(scheduler.clone());

    let tag1 = scheduler.alloc(TestFsm::new(context.clone()));
    assert!(tag1.is_some());
    let id1 = tag1.unwrap();

    scheduler.on_event(TestEvent::AcquireResource, id1);
    assert_eq!(
        scheduler.map(id1, |fsm| fsm.state),
        Some(TestState::Intermediate)
    );

    // When there is no one else in the waitiers queue, the reacquire should succeed immediately.
    scheduler.on_event(TestEvent::DropAndReacquireResource, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), Some(TestState::Test));

    scheduler.on_event(TestEvent::End, id1);
    assert_eq!(scheduler.map(id1, |fsm| fsm.state), None);
}
