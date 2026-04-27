// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::ComboFsm;
use crate::fsm::HsmFsmEventRecorder;
use crate::resource::HspIpcChannelResource;
use crate::CmdResource;
use crate::CmdResourceRef;
use crate::CmdScheduler;
use crate::HsmFsmResourceId;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();

    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_get_cert_len()
        .times(1)
        .returning(|_| Some(1024));

    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::HspIpcChannel));
}

#[test]
fn test_acquire_hsp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::HspIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel)
    );
}

#[test]
fn test_get_ak_cert() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_get_cert_len().times(1).return_const(Some(1024));

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, _| Ok(()));

    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetCertificateCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetCertificate);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert!(!resp.data.certificate.is_empty());
}

#[test]
fn test_get_certificate_with_resource_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_get_cert_len().times(1).return_const(Some(1024));

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, _| Err(HsmErr::Pending));

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, cert_ctx| {
            cert_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });

    let cert_buf = [1u8; 2048];
    part.expect_end_get_cert().times(1).returning(move |ctx| {
        ctx.cert_buf = Some(cert_buf.as_slice().into());

        Ok(())
    });

    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetCertificateCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetCertificate);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert!(!resp.data.certificate.is_empty());
}

#[test]
fn test_get_certificate_with_cert_sizes_gone() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_get_cert_len().times(1).return_const(None);

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, _| Err(HsmErr::InvalidCertificate));

    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidCertificate)
    );
}

#[test]
fn test_invalid_state_for_cert() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_get_cert_len().times(1).return_const(Some(1024));

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, _| Err(HsmErr::Pending));

    part.expect_begin_get_cert()
        .times(1)
        .returning(move |_, _| Err(HsmErr::Pending));

    let cert_id = 1;
    let req =
        encode_buf::<DdiGetCertificateCmdReq, _>(&cmd_certificate_req(cert_id), &heap).unwrap();
    let mut cmd = GetCertificateCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

fn cmd_certificate_req(cert_id: u8) -> DdiGetCertificateCmdReq {
    DdiGetCertificateCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetCertificate,
            sess_id: None,
        },
        data: DdiGetCertificateReq {
            slot_id: 0,
            cert_id,
        },
    }
}

fn acquire_ipc_channnel(
) -> Option<CmdResourceRef<HspIpcChannelResource<MockIpcMessageChannel>, ComboFsm<MockEnv>>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        HspIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );

    resource.acquire(TagId::default(), ())
}
