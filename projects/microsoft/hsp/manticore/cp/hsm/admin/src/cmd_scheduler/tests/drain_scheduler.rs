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
    Stop,
    SchedulerEmptyEvent,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TestState {
    Init,
    EmptyDetected,
}

struct TestDynamicFsm {
    state: TestState,
}

impl TestDynamicFsm {
    fn new() -> Self {
        Self {
            state: TestState::Init,
        }
    }
}

struct TestStaticFsm {
    state: TestState,
}

impl TestStaticFsm {
    fn new() -> Self {
        Self {
            state: TestState::Init,
        }
    }
}

impl CmdFsm for TestDynamicFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    // CmdFsm trait implementation
    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        match event {
            TestEvent::Start => {
                self.state = TestState::Init;
                Err(TestErr::Pending)
            }
            TestEvent::Stop => Ok(()),
            _ => {
                unreachable!()
            }
        }
    }
}

impl CmdFsm for TestStaticFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    // CmdFsm trait implementation
    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        if let TestEvent::SchedulerEmptyEvent = event {
            self.state = TestState::EmptyDetected
        }

        Err(TestErr::Pending)
    }
}

enum TestFsm {
    TestStatic(TestStaticFsm),
    TestDynamic(TestDynamicFsm),
}

impl TestFsm {
    fn get_state(&self) -> TestState {
        match self {
            TestFsm::TestStatic(fsm) => fsm.state,
            TestFsm::TestDynamic(fsm) => fsm.state,
        }
    }
}

impl CmdFsm for TestFsm {
    type Error = TestErr;
    type ResourceId = ();
    type Event = TestEvent;
    type Recorder = TestRecorder;

    // CmdFsm trait implementation
    fn on_event(&mut self, event: Self::Event, _tag: TagId) -> Result<(), Self::Error> {
        match self {
            TestFsm::TestStatic(fsm) => fsm.on_event(event, _tag),
            TestFsm::TestDynamic(fsm) => fsm.on_event(event, _tag),
        }
    }
}

#[test]
fn test_static_notification_disabled() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_some());
    let id = tag.unwrap();
    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(
        scheduler.map(id, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id);
    assert_eq!(
        scheduler.map(id, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );
}

#[test]
fn test_static_notification_enabled() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_some());
    let id = tag.unwrap();

    scheduler.set_scheduler_empty_notification(id, TestEvent::SchedulerEmptyEvent);

    scheduler.on_event(TestEvent::Start, id);
    assert_eq!(
        scheduler.map(id, |fsm| fsm.get_state()),
        Some(TestState::EmptyDetected)
    );

    scheduler.on_event(TestEvent::Stop, id);
    assert_eq!(
        scheduler.map(id, |fsm| fsm.get_state()),
        Some(TestState::EmptyDetected)
    );
}

#[test]
fn test_combined_notification_disabled() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_some());
    let id_static = tag.unwrap();

    let tag = scheduler.alloc(TestFsm::TestDynamic(TestDynamicFsm::new()));
    assert!(tag.is_some());
    let id_dynamic = tag.unwrap();

    scheduler.on_event(TestEvent::Start, id_static);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Start, id_dynamic);
    assert_eq!(
        scheduler.map(id_dynamic, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_static);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_dynamic);
    assert_eq!(scheduler.map(id_dynamic, |fsm| fsm.get_state()), None);
}

#[test]
fn test_combined_notification_enabled() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_some());
    let id_static = tag.unwrap();

    scheduler.set_scheduler_empty_notification(id_static, TestEvent::SchedulerEmptyEvent);

    let tag = scheduler.alloc(TestFsm::TestDynamic(TestDynamicFsm::new()));
    assert!(tag.is_some());
    let id_dynamic = tag.unwrap();

    scheduler.on_event(TestEvent::Start, id_dynamic);
    assert_eq!(
        scheduler.map(id_dynamic, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );
    scheduler.on_event(TestEvent::Start, id_static);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_static);
    assert_eq!(
        scheduler.map(id_dynamic, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_dynamic);
    assert_eq!(scheduler.map(id_dynamic, |fsm| fsm.get_state()), None);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::EmptyDetected)
    );
}

#[test]
fn test_combined_notification_enabled_with_overflow() {
    let scheduler = CmdScheduler::<TestFsm>::new(2, 1, TestRecorder {});
    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_some());
    let id_static = tag.unwrap();

    scheduler.set_scheduler_empty_notification(id_static, TestEvent::SchedulerEmptyEvent);

    let tag = scheduler.alloc(TestFsm::TestDynamic(TestDynamicFsm::new()));
    assert!(tag.is_some());
    let id_dynamic = tag.unwrap();

    let tag = scheduler.alloc_static(TestFsm::TestStatic(TestStaticFsm::new()));
    assert!(tag.is_none());

    scheduler.on_event(TestEvent::Start, id_dynamic);
    assert_eq!(
        scheduler.map(id_dynamic, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );
    scheduler.on_event(TestEvent::Start, id_static);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_static);
    assert_eq!(
        scheduler.map(id_dynamic, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::Init)
    );

    scheduler.on_event(TestEvent::Stop, id_dynamic);
    assert_eq!(scheduler.map(id_dynamic, |fsm| fsm.get_state()), None);
    assert_eq!(
        scheduler.map(id_static, |fsm| fsm.get_state()),
        Some(TestState::EmptyDetected)
    );
}
