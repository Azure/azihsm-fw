// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::PcieFunction;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::AesBulk256Cmd;
use crate::fsm::EntryKind;
use crate::fsm::HsmFsmEventRecorder;
use crate::fsm::HsmFsmResourceId;
use crate::resource::FpIpcChannelResource;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(10);

    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    heap.expect_allocate_from_pool().once().returning(|_| None);

    let mut app_session = MockUserSession::new();
    app_session.expect_delete_key().once().returning(|_| Ok(()));
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(move |_| Ok(EntryKind::Aes256));

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_delete_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_delete_key().once().returning(|_| Ok(()));
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(move |_| Ok(EntryKind::Aes256));

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
    assert!(cmd.rollback(TagId::default()).is_ok());

    let resp = decode_buf::<DdiDeleteKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_delete_key_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_delete_key()
        .once()
        .returning(|_| Err(HsmErr::CannotDeleteKeyInUse));
    app_session.expect_id().once().returning(Default::default);
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(move |_| Ok(EntryKind::Aes256));

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::CannotDeleteKeyInUse)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(cmd.retry());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_delete_key().once().returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(move |_| Ok(EntryKind::Aes256));
    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_aesbulk256_delete_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(aesbulk256_delete_key_cmd().unwrap()));
    app_session
        .expect_end_delete_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::AesGcmBulk256));
    app_session.expect_id().times(1).returning(Default::default);

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDeleteKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_aesbulk256_delete_key_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(aesbulk256_delete_key_cmd().unwrap()));
    app_session
        .expect_end_delete_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::AesGcmBulk256));
    app_session.expect_id().times(1).returning(Default::default);

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiDeleteKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let session = MockUserSession::new();
    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_aesbulk256_delete_key_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::AesGcmBulk256));
    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
fn test_aesbulk256_delete_key_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_delete_aesbulk256_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::AesGcmBulk256));
    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let req = encode_buf::<DdiDeleteKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let app_session = MockUserSession::new();
    let cmd = DeleteKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert!(!cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
}

pub(crate) fn aesbulk256_delete_key_cmd() -> HsmResult<AesBulk256Cmd<MockEnv>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        FpIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );
    let channel = resource.acquire(TagId::default(), ());

    Ok(AesBulk256Cmd::DeleteKey(
        Default::default(),
        0,
        channel.unwrap(),
    ))
}

fn cmd_req() -> DdiDeleteKeyCmdReq {
    DdiDeleteKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::DeleteKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiDeleteKeyReq { key_id: 10u16 },
    }
}
