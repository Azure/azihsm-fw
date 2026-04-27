// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::{cmd_scheduler::*, HsmFsmEventRecorder};
use core::cell::RefCell;
use std::rc::Rc;

use crate::fsm::res_cleanup_fsm::HsmResCleanupFsm;
use crate::resource::{HsmFsmResourceId, PkaResource};

#[test]
fn test_invalid_event() {
    let mut cmd = HsmResCleanupFsm::new(Rc::new(RefCell::new(MockEnv::new())));
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_resource_not_pka() {
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::FpIpcChannel, 0),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_reource_cleanup_success() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut hal = MockHal::new();
    let mut pka = Vec::new();
    for i in 0..16 {
        let mut pka_instance = MockPka::new();
        if i == TEST_PKA_INSTANCE {
            pka_instance
                .expect_begin_memory_wipe()
                .once()
                .return_const(Ok(()));
            pka_instance
                .expect_end_memory_wipe()
                .once()
                .return_const(Ok(()));
        }
        pka.push(pka_instance);
    }

    hal.expect_pka().times(..).return_const(pka);
    env.borrow_mut().expect_hal().times(..).return_const(hal);

    let mut pka = Vec::new();
    for _ in 0..16 {
        pka.push(MockPka::new());
    }
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let pka_rsrc = PkaResource::new(pka);
    let pka_engine = CmdResource::new(pka_rsrc, scheduler, 16);
    env.borrow_mut()
        .expect_pka_engine()
        .times(1)
        .return_const(pka_engine);

    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(TEST_PKA_INSTANCE), TagId::default()),
        Err(HsmErr::DrainReady)
    );
}

#[test]
fn test_resource_cleanup_begin_mem_wipe_failed() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut hal = MockHal::new();
    let mut pka = Vec::new();
    for i in 0..16 {
        let mut pka_instance = MockPka::new();
        if i == TEST_PKA_INSTANCE {
            pka_instance
                .expect_begin_memory_wipe()
                .once()
                .return_const(Err(0xFFFF_FFFF));
        }
        pka.push(pka_instance);
    }

    hal.expect_pka().times(..).return_const(pka);
    env.borrow_mut().expect_hal().times(..).return_const(hal);

    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::PkaMemoryWipeFailed)
    );
}

#[test]
fn test_resource_cleanup_end_mem_wipe_failed() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut hal = MockHal::new();
    let mut pka = Vec::new();
    for i in 0..16 {
        let mut pka_instance = MockPka::new();
        if i == TEST_PKA_INSTANCE {
            pka_instance
                .expect_begin_memory_wipe()
                .once()
                .return_const(Ok(()));
            pka_instance
                .expect_end_memory_wipe()
                .once()
                .return_const(Err(0xFFFF_FFFF));
        }
        pka.push(pka_instance);
    }

    hal.expect_pka().times(..).return_const(pka);
    env.borrow_mut().expect_hal().times(..).return_const(hal);

    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(TEST_PKA_INSTANCE), TagId::default()),
        Err(HsmErr::PkaMemoryWipeFailed)
    );
}

#[test]
#[should_panic]
fn test_resource_cleanup_pka_error() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut hal = MockHal::new();
    let mut pka = Vec::new();
    for i in 0..16 {
        let mut pka_instance = MockPka::new();
        if i == TEST_PKA_INSTANCE {
            pka_instance
                .expect_begin_memory_wipe()
                .once()
                .return_const(Ok(()));
            pka_instance
                .expect_end_memory_wipe()
                .once()
                .return_const(Ok(()));
        }
        pka.push(pka_instance);
    }

    hal.expect_pka().times(..).return_const(pka);
    env.borrow_mut().expect_hal().times(..).return_const(hal);

    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaError(TEST_PKA_INSTANCE), TagId::default()),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_unexpected_pka_instance_on_cleanup() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut cmd = HsmResCleanupFsm::new(env);
    cmd.pka_state = 1u16 << TEST_PKA_INSTANCE;
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_unexpected_pka_instance_on_cleanup_done() {
    const TEST_PKA_INSTANCE: usize = 1;
    let env = Rc::new(RefCell::new(MockEnv::new()));
    let mut hal = MockHal::new();
    let mut pka = Vec::new();
    for i in 0..16 {
        let mut pka_instance = MockPka::new();
        if i == TEST_PKA_INSTANCE {
            pka_instance
                .expect_begin_memory_wipe()
                .once()
                .return_const(Ok(()));
        }
        pka.push(pka_instance);
    }

    hal.expect_pka().times(..).return_const(pka);
    env.borrow_mut().expect_hal().times(..).return_const(hal);

    let mut cmd = HsmResCleanupFsm::new(env);
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceCleanup(HsmFsmResourceId::Pka, TEST_PKA_INSTANCE),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::PkaDone(TEST_PKA_INSTANCE + 1),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
}
