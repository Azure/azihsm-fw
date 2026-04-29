// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;

use crate::*;

/// TestEnv
#[derive(Clone)]
pub(crate) struct TestEnv {
    rimpl: Rc<RefCell<TestEnvImpl>>,
}

impl TestEnv {
    fn new() -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(TestEnvImpl::new())),
        }
    }

    fn set_drained(&self, value: bool) {
        self.rimpl.borrow_mut().set_drained(value)
    }

    fn drained(&self) -> bool {
        self.rimpl.borrow().drained()
    }
}

struct TestEnvImpl {
    drained: bool,
}

impl TestEnvImpl {
    fn new() -> Self {
        Self { drained: false }
    }

    fn set_drained(&mut self, value: bool) {
        self.drained = value
    }

    fn drained(&self) -> bool {
        self.drained
    }
}

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
    Start,
    End,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    Test,
    Done,
}

struct TestFsm {
    state: TestState,
}

impl Default for TestFsm {
    fn default() -> Self {
        Self {
            state: TestState::Init,
        }
    }
}

impl CmdFsm for TestFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (TestState::Init, TestEvent::Start) => {
                self.state = TestState::Test;
                Err(TestErr::Pending)
            }
            (TestState::Test, TestEvent::End) => {
                self.state = TestState::Done;
                Ok(())
            }
            _ => unreachable!(),
        }
    }
}

#[test]
fn test_deferred_drain() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let test_env = TestEnv::new();

    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    let env = test_env.clone();
    let closure = move || env.set_drained(true);
    let result = scheduler.drain(closure);
    assert!(result.is_ok());

    assert!(!test_env.drained());

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }

    assert!(test_env.drained());
}

#[test]
fn test_immediate_drain() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let test_env = TestEnv::new();

    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }

    let env = test_env.clone();
    let closure = move || env.set_drained(true);
    let result = scheduler.drain(closure);
    assert!(result.is_ok());

    assert!(test_env.drained());
}

#[test]
fn test_drain_while_there_is_an_active_drain() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let test_env = TestEnv::new();
    let test_env2 = TestEnv::new();

    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    let env = test_env.clone();
    let closure = move || env.set_drained(true);
    let result = scheduler.drain(closure);
    assert!(result.is_ok());

    assert!(!test_env.drained());

    let second_env = test_env2.clone();
    let closure = move || second_env.set_drained(true);
    let result = scheduler.drain(closure);
    assert_eq!(result.err(), Some(HsmErr::DrainBusy));

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }

    assert!(test_env.drained());
}

#[test]
fn test_drain_followed_by_another_drain() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let test_env = TestEnv::new();
    let test_env2 = TestEnv::new();

    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    let env = test_env.clone();
    let closure = move || env.set_drained(true);
    let result = scheduler.drain(closure);
    assert!(result.is_ok());

    assert!(!test_env.drained());

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }

    assert!(test_env.drained());

    let mut ids = vec![];
    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    let second_env = test_env2.clone();
    let closure = move || second_env.set_drained(true);
    let result = scheduler.drain(closure);
    assert!(result.is_ok());

    assert!(!test_env2.drained());

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }

    assert!(test_env2.drained());
}
