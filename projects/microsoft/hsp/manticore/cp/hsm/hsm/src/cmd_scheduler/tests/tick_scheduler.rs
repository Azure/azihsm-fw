// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::*;

#[derive(Clone)]
struct TestRecorder {}

#[derive(PartialEq, Eq, Clone, Copy)]
enum TestErr {
    Pending,
    PendingAndDrainReady,
    TimeOut,
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
    TimeOut,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    Test,
    Done,
    Err,
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
            (_, TestEvent::TimeOut) => {
                self.state = TestState::Err;
                Err(TestErr::TimeOut)
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
fn test_tick() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});

    let mut occupancy_list: u128 = 0;
    for _ in 0..2 {
        let tag = scheduler.alloc(TestFsm::default());
        let id = tag.unwrap();
        occupancy_list |= 1u128 << id;
        assert_eq!(scheduler.occupancy_list(), occupancy_list);
        assert_eq!(scheduler.access_list(), 0);
        ids.push(id);
    }

    let mut access_list: u128 = 0;
    scheduler.on_event(TestEvent::Start, ids[0]);
    access_list |= 1u128 << ids[0];
    assert_eq!(
        scheduler.map(ids[0], |fsm| fsm.state),
        Some(TestState::Test)
    );
    assert_eq!(scheduler.access_list(), access_list);
    assert_eq!(scheduler.occupancy_list(), occupancy_list);

    scheduler.on_event(TestEvent::End, ids[0]);
    occupancy_list &= !(1 << ids[0]);
    assert_eq!(scheduler.map(ids[0], |fsm| fsm.state), None);
    assert_eq!(scheduler.access_list(), 1u128 << ids[0]);
    assert_eq!(scheduler.occupancy_list(), occupancy_list);

    scheduler.on_tick(TestEvent::TimeOut);
    assert_eq!(scheduler.map(ids[1], |fsm| fsm.state), None);
    assert_eq!(scheduler.access_list(), 0);
    assert_eq!(scheduler.occupancy_list(), 0);
}

#[test]
fn test_tick_multiples() {
    let mut ids = vec![];
    let scheduler = CmdScheduler::<TestFsm>::new(65, 1, TestRecorder {});

    let mut occupancy_list: u128 = 0;
    for _ in 0..65 {
        let tag = scheduler.alloc(TestFsm::default());
        let id = tag.unwrap();
        occupancy_list |= 1u128 << id;
        assert_eq!(scheduler.occupancy_list(), occupancy_list);
        assert_eq!(scheduler.access_list(), 0);
        ids.push(id);
    }

    assert_eq!(scheduler.occupancy_list(), occupancy_list);

    let mut access_list: u128 = 0;
    for id in ids {
        if id != 64 {
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
    }

    assert_eq!(scheduler.occupancy_list(), 1u128 << 64);
    assert_eq!(scheduler.occupancy_list(), occupancy_list);
    assert_eq!(scheduler.access_list(), access_list);

    scheduler.on_tick(TestEvent::TimeOut);

    assert_eq!(scheduler.occupancy_list(), 0);
    assert_eq!(scheduler.access_list(), 0);
}
