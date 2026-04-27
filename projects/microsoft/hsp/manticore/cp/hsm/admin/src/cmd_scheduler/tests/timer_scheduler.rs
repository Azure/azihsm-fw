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
    Timeout,
    CancelTimer,
    Stop,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    Test,
    Timeout,
    Done,
}

struct TestFsm {
    state: TestState,
    timer: CmdTimer,
    start_timer: Option<u8>,
    reset_timer: Option<u8>,
    has_timer: bool,
}

impl TestFsm {
    fn new(start_timer: Option<u8>, reset_timer: Option<u8>, has_timer: bool) -> Self {
        Self {
            state: TestState::Init,
            timer: CmdTimer::new(),
            start_timer,
            reset_timer,
            has_timer,
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
        if self.has_timer {
            Some(&mut self.timer)
        } else {
            None
        }
    }

    // CmdFsm trait implementation
    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        match (self.state, event) {
            (TestState::Init, TestEvent::Start) => {
                self.state = TestState::Test;
                if let Some(start_timer) = self.start_timer {
                    self.timer.start(start_timer);
                }

                Err(TestErr::Pending)
            }
            (TestState::Test, TestEvent::Timeout) => {
                self.state = TestState::Timeout;

                if let Some(reset_timer) = self.reset_timer {
                    self.timer.start(reset_timer);
                    Err(TestErr::Pending)
                } else {
                    Ok(())
                }
            }
            (TestState::Timeout, TestEvent::Timeout) => {
                self.state = TestState::Done;
                Err(TestErr::Pending)
            }
            (TestState::Test | TestState::Timeout, TestEvent::CancelTimer) => {
                self.timer.stop();
                Err(TestErr::Pending)
            }
            (_, TestEvent::Stop) => Ok(()),
            _ => unreachable!(),
        }
    }
}

#[test]
fn test_timer_scheduler_not_implemented() {
    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::new(None, None, false));
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..3 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_event(TestEvent::Stop, id);
}

#[test]
fn test_timer_scheduler_not_used() {
    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::new(None, None, true));
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..3 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_event(TestEvent::Stop, id);
}

#[test]
fn test_timer_scheduler_one_time() {
    let expected_ticks = 3;

    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::new(Some(expected_ticks), None, true));

    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), None);
}

#[test]
fn test_timer_scheduler_reset() {
    let expected_ticks = 3;
    let reset_ticks = 2;

    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::new(Some(expected_ticks), Some(reset_ticks), true));
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Timeout));

    for _ in 0..reset_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Timeout));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Done));

    scheduler.on_event(TestEvent::Stop, id);
}

#[test]
fn test_timer_scheduler_cancel() {
    let expected_ticks = 3;

    let mut scheduler = CmdScheduler::<TestFsm>::new(1, 1, TestRecorder {});
    let tag = scheduler.alloc(TestFsm::new(Some(expected_ticks), None, true));
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_event(TestEvent::CancelTimer, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..3 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_event(TestEvent::Stop, id);
}

#[test]
fn test_timer_scheduler_multiple_fsms() {
    let expected_ticks = 3;
    let reset_ticks = 2;

    let mut scheduler = CmdScheduler::<TestFsm>::new(3, 3, TestRecorder {});
    let _ = scheduler.alloc(TestFsm::new(None, None, false));
    let tag = scheduler.alloc(TestFsm::new(Some(expected_ticks), Some(reset_ticks), true));
    let _ = scheduler.alloc(TestFsm::new(None, None, false));

    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));

    for _ in 0..expected_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Test));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Timeout));

    for _ in 0..reset_ticks - 1 {
        scheduler.on_tick(TestEvent::Timeout);
        assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Timeout));
    }

    scheduler.on_tick(TestEvent::Timeout);
    assert_eq!(scheduler.map(id, |fsm| fsm.state), Some(TestState::Done));

    scheduler.on_event(TestEvent::Stop, id);
}
