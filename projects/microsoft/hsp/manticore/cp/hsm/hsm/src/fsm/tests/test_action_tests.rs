// Copyright (c) Microsoft Corporation. All rights reserved.

#[cfg(feature = "mcr_test_hooks")]
use mcr_self_test::SelfTest;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::test_action::TestActionCmd;
use crate::fsm::{HsmFsmResourceId, SessionId};

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::Level1SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_level1_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::Level1SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_level1_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::Level1SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd)
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_set_level2_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::SetLevel2SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    part.expect_set_test_hook_to_trigger_level2_abort()
        .times(1)
        .returning(|_| ());
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_set_level2_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::SetLevel2SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd)
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_clear_level2_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ClearLevel2SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    part.expect_set_test_hook_to_trigger_level2_abort()
        .times(1)
        .returning(|_| ());
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiTestActionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::TestAction);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_clear_level2_skip_io_test_action_cmd() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ClearLevel2SkipIo,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd)
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    let resp = cmd.take_response();
    assert!(resp.is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_test_action_cmd_invalidate_cert_size_cache() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::InvalidateCertSizeCache,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    part.expect_set_cert_chain_lengths_info()
        .times(1)
        .returning(move |_| ());

    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_some());
}

#[test]
fn test_acquire_hsm_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::HsmToAdminIpcChannel));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::HsmToAdminIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::HsmToAdminIpcChannel)
    );
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState),
    );
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_some());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState),
    );
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_trigger_crash_in_hsm() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    #[allow(unused_mut)]
    let mut part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_some());
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_trigger_crash_in_hsm() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Hsm,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_trigger_crash_in_admin_after_resource_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Admin,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());

    user_session
        .expect_send_crashdump_request()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::Pending));
    user_session
        .expect_send_crashdump_request()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HsmToAdminIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::Pending),
    );

    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_trigger_crash_in_admin() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Admin,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    user_session
        .expect_send_crashdump_request()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );

    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_trigger_crash_in_admin() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Admin,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd),
    );
    assert!(cmd.take_response().is_none());
}

#[cfg(feature = "mcr_test_hooks")]
#[test]
fn test_trigger_crash_in_fp() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Fp2,
            }),
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    user_session
        .expect_send_crashdump_request()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );

    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[cfg(not(feature = "mcr_test_hooks"))]
#[test]
fn test_trigger_crash_in_fp() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerCrash,
            Some(DdiTestActionCrashReqInfo {
                crash_type: DdiTestActionCrashType::HardFault,
                cpu_id: DdiTestActionSocCpuId::Fp2,
            }),
            None,
            Some(DdiTestActionPinPolicyConfig {
                delay_increment: None,
                state: None,
                delay: None,
                allowed_attempts: None,
                lockout_delay: None,
            }),
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedCmd),
    );
    assert!(cmd.take_response().is_none());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_execute_negative_self_test() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ExecuteNegativeSelfTest,
            None,
            Some(1),
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    user_session
        .expect_begin_neg_self_test_req()
        .times(1)
        .returning(|_, _| Ok(()));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_execute_negative_test_invalid_argument() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ExecuteNegativeSelfTest,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_execute_negative_test_invalid_test_id() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ExecuteNegativeSelfTest,
            None,
            Some(SelfTest::SelfTestCompleted as u32 + 1),
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_execute_negative_test_with_valid_ipc_response() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ExecuteNegativeSelfTest,
            None,
            Some(1),
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    user_session
        .expect_begin_neg_self_test_req()
        .times(1)
        .returning(|_, _| Ok(()));

    user_session
        .expect_end_neg_self_test_resp()
        .times(1)
        .returning(|_| Ok(()));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::AdminToHsmIpcResponse, TagId::default()),
        Ok(()),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_some());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_execute_negative_test_with_invalid_ipc_receive_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ExecuteNegativeSelfTest,
            None,
            Some(1),
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let mut user_session = MockUserSession::new();
    user_session
        .expect_id()
        .times(1)
        .return_const(SessionId::default());
    user_session
        .expect_begin_neg_self_test_req()
        .times(1)
        .returning(|_, _| Ok(()));

    user_session
        .expect_end_neg_self_test_resp()
        .times(1)
        .returning(|_| Err(HsmErr::IpcResponseError));

    let part = MockPartition::new();
    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending),
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::AdminToHsmIpcResponse, TagId::default()),
        Err(HsmErr::IpcResponseError),
    );
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.take_response().is_none());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_pin_policy_override_context() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::PinPolicyOverride,
            None,
            None,
            Some(DdiTestActionPinPolicyConfig {
                delay_increment: Some(2),
                state: Some(false),
                delay: Some(35),
                allowed_attempts: Some(35),
                lockout_delay: Some(35),
            }),
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();

    let mut part = MockPartition::new();
    part.expect_override_pin_policy_context()
        .times(1)
        .return_const(());

    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert!(cmd.take_response().is_some());
}

#[test]
#[cfg(feature = "mcr_test_hooks")]
fn test_pin_policy_clear_context() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::PinPolicyClear,
            None,
            None,
            None,
            None,
            None,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();

    let mut part = MockPartition::new();
    part.expect_clear_pin_policy().times(1).return_const(());

    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert!(cmd.take_response().is_some());
}

#[test]
#[cfg(feature = "fips_validation_hooks")]
fn test_force_pka_instance() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::ForcePkaInstance,
            None,
            None,
            None,
            Some(1),
            None,
            None,
        ),
        &heap,
    )
    .unwrap();

    let mut user_session = MockUserSession::new();
    user_session
        .expect_force_pka_instance()
        .once()
        .returning(|_| ());

    let mut cmd = TestActionCmd::<MockEnv>::new(
        req,
        heap,
        user_session,
        MockPartition::new(),
        PcieFunction::Pf,
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert!(cmd.take_response().is_some());
}

#[test]
#[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
fn test_neg_pct_skip_cnt() {
    let cnt = Some(0);
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiTestActionCmdReq, _>(
        &cmd_req(
            DdiTestAction::TriggerNegativePctFailure,
            None,
            None,
            None,
            None,
            cnt,
            None,
        ),
        &heap,
    )
    .unwrap();
    let user_session = MockUserSession::new();

    let mut part = MockPartition::new();
    part.expect_neg_pct_skip_cnt()
        .times(1)
        .returning(move |cnt| cnt);

    let mut cmd = TestActionCmd::<MockEnv>::new(req, heap, user_session, part, PcieFunction::Pf);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(()),
    );
    assert!(cmd.take_response().is_some());
}

fn cmd_req(
    action: DdiTestAction,
    crash_info: Option<DdiTestActionCrashReqInfo>,
    neg_test_id: Option<u32>,
    pin_policy_config: Option<DdiTestActionPinPolicyConfig>,
    force_pka_instance: Option<u8>,
    neg_pct_skip_cnt: Option<u8>,
    ecc_error_info: Option<DdiTestActionEccErrorInfo>,
) -> DdiTestActionCmdReq {
    DdiTestActionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::TestAction,
            sess_id: Some(SessionId::default()),
        },
        data: DdiTestActionReq {
            action,
            crash_info,
            neg_test_id,
            pin_policy_config,
            force_pka_instance,
            neg_pct_skip_cnt,
            ecc_error_info,
            tdisp_interrupt_type: None,
            updated_svn: None,
            gdma_error_type: None,
        },
    }
}
