// Copyright (c) Microsoft Corporation. All rights reserved.

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
fn test_basic() {
    let scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::default());
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    scheduler.on_event(TestEvent::End, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
}

#[test]
fn test_mult_basic() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});

    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        ids.push(tag.unwrap());
    }

    let tag = scheduler.alloc(TestFsm::default());
    assert!(tag.is_none());

    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    }
}

#[test]
#[should_panic]
fn test_fsm_acquire_panic() {
    let mut fsm = TestFsm::default();
    fsm.acquire_resource(0, ());
}
