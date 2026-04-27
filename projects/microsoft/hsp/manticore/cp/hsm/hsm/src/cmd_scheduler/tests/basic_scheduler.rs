// Copyright (c) Microsoft Corporation. All rights reserved.

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
    assert_eq!(scheduler.occupancy_list(), 1u128 << id);
    assert_eq!(scheduler.access_list(), 0);
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    assert_eq!(scheduler.access_list(), 1u128 << id);
    scheduler.on_event(TestEvent::End, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
    assert_eq!(scheduler.access_list(), 1u128 << id);
}

#[test]
fn test_multi_basic() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(65, 1, TestRecorder {});

    let mut occupancy_list: u128 = 0;
    for _ in 0..65 {
        let tag = scheduler.alloc(TestFsm::default());
        let id = tag.unwrap();
        occupancy_list |= 1u128 << id;
        assert_eq!(scheduler.occupancy_list(), occupancy_list);
        ids.push(id);
    }

    let tag = scheduler.alloc(TestFsm::default());
    assert!(tag.is_none());
    assert_eq!(scheduler.occupancy_list(), occupancy_list);

    let mut access_list: u128 = 0;
    for id in ids {
        scheduler.on_event(TestEvent::Start, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
        access_list |= 1u128 << id;
        assert_eq!(scheduler.access_list(), access_list);
        scheduler.on_event(TestEvent::End, id);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
        access_list |= 1u128 << id;
        assert_eq!(scheduler.access_list(), access_list);
        occupancy_list &= !(1 << id);

        assert_eq!(scheduler.occupancy_list(), occupancy_list);
    }
    assert_eq!(scheduler.occupancy_list(), 0);
}

#[test]
#[should_panic]
fn test_fsm_acquire_panic() {
    let mut fsm = TestFsm::default();
    fsm.acquire_resource(0, ());
}
