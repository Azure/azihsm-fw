// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::AesBulk256Cmd;
use crate::fsm::FlushSessionCmd;
use crate::fsm::HsmFsmEventRecorder;
use crate::fsm::HsmFsmResourceId;
use crate::resource::FpIpcChannelResource;

#[test]
fn test_invalid_event() {
    let part = MockPartition::new();

    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(10));
    assert!(cmd.retry());
}

#[test]
fn test_flush_session_no_session() {
    let mut part = MockPartition::new();
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::SessionNotFound));

    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SessionNotFound)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(10));
    assert!(cmd.retry());
}

#[test]
fn test_flush_session() {
    let mut part = MockPartition::new();
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Ok(flush_session_cmd().unwrap()));
    part.expect_end_close_user_session()
        .once()
        .returning(|_| Ok(()));
    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_close_app_session_on_engine_ready() {
    let mut part = MockPartition::new();
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Ok(flush_session_cmd().unwrap()));
    part.expect_end_close_user_session()
        .once()
        .returning(|_| Ok(()));

    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_close_app_session_on_engine_ready_err_pending() {
    let mut part = MockPartition::new();
    part.expect_close_user_session()
        .times(0)
        .returning(|_| Ok(()));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    part.expect_end_close_user_session()
        .times(0)
        .returning(|_| Ok(()));

    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let part = MockPartition::new();
    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_requires_resource() {
    let part = MockPartition::new();
    let cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
}

#[test]
fn test_close_app_session_invalid_state() {
    let mut part = MockPartition::new();
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));

    let mut cmd = FlushSessionCmd::<MockEnv>::new(10, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

pub(crate) fn flush_session_cmd() -> HsmResult<AesBulk256Cmd<MockEnv>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        FpIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );
    let channel = resource.acquire(TagId::default(), ());

    Ok(AesBulk256Cmd::CloseAppSession(0, channel.unwrap()))
}
