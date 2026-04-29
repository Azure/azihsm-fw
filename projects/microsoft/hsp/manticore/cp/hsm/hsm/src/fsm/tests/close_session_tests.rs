// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::AesBulk256Cmd;
use crate::fsm::CloseSessionCmd;
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
    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Ok(close_app_session_cmd().unwrap()));
    part.expect_end_close_user_session()
        .once()
        .returning(|_| Ok(()));
    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_close_app_session_no_session() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .once()
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let ddi_req = DdiCloseSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::CloseSession,
            sess_id: None,
        },
        data: DdiCloseSessionReq {},
    };

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&ddi_req, &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SessionExpected)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_close_app_session_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::CannotUseDefaultCredentials));

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::CannotUseDefaultCredentials)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_close_app_session() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Ok(close_app_session_cmd().unwrap()));
    part.expect_end_close_user_session()
        .once()
        .returning(|_| Ok(()));
    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);

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

    let resp = decode_buf::<DdiCloseSessionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_close_app_session_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Ok(close_app_session_cmd().unwrap()));
    part.expect_end_close_user_session()
        .once()
        .returning(|_| Ok(()));

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
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
fn test_close_app_session_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
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

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
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
fn test_close_app_session_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_close_user_session()
        .times(0)
        .returning(|_| Ok(()));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
}

#[test]
fn test_close_app_session_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_needs_renegotiation()
        .once()
        .returning(|_| false);
    part.expect_close_user_session()
        .times(0)
        .returning(|_| Ok(()));
    part.expect_begin_close_user_session()
        .once()
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);
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

#[test]
fn test_close_app_session_needs_renegotiation() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_needs_renegotiation().once().returning(|_| true);
    part.expect_delete_user_session().once().returning(|_| ());

    let req = encode_buf::<DdiCloseSessionCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = CloseSessionCmd::<MockEnv>::new(req, heap, part, PcieFunction::Vf0);

    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiCloseSessionCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

pub(crate) fn close_app_session_cmd() -> HsmResult<AesBulk256Cmd<MockEnv>> {
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

fn cmd_req() -> DdiCloseSessionCmdReq {
    DdiCloseSessionCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::CloseSession,
            sess_id: Some(SessionId::default()),
        },
        data: DdiCloseSessionReq {},
    }
}
