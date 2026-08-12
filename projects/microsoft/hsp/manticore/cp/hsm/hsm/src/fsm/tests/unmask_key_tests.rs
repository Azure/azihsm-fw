// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;
use zerocopy::IntoBytes;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::unmask_key::UnmaskKeyCmd;
use crate::fsm::HsmFsmResourceId;
use crate::heap::HsmDmaHeapTrait;
use crate::partition::store::EntryAttributes;
use crate::partition::{
    EntryKind, ImportDerKeyResult, UnmaskedKeyImportResult, UnmaskedKeyRawResult, UnmaskedKeyResult,
};

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(SessionId::default);
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let part = MockPartition::new();

    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(SessionId::default);
    app_session
        .expect_delete_key()
        .times(0)
        .returning(|_| Ok(()));

    let req = MockDmaAlloc::new(10);

    let part = MockPartition::new();

    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: Some(import_result(DdiKeyType::Aes128)),
                raw_result: None,
            })
        });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session.expect_delete_key().once().returning(|_| Ok(()));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let session = MockUserSession::new();

    let part = MockPartition::new();

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let session = MockUserSession::new();
    let part = MockPartition::new();
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_unmask_non_bulk_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: Some(import_result(DdiKeyType::Aes128)),
                raw_result: None,
            })
        });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiUnmaskKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_unmask_key_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| Err(HsmErr::MaskedKeyDecodeFailed));
    app_session.expect_id().once().returning(Default::default);

    let part = MockPartition::new();
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::MaskedKeyDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(3)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: Some(import_result(DdiKeyType::Aes128)),
                raw_result: None,
            })
        });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_unmask_bulk_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiUnmaskKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_unmask_bulk_key_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiUnmaskKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
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
    let session = MockUserSession::new();
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let part = MockPartition::new();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_unmask_bulk_key_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let part = MockPartition::new();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
fn test_unmask_bulk_key_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Err(HsmErr::InvalidArgument));

    let part = MockPartition::new();
    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_unmask_bulk_key_rollback_on_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiUnmaskKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

#[test]
fn test_unmask_bulk_key_rollback_during_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session.expect_api_rev().once().returning(api_rev);
    app_session
        .expect_unmask_key_and_import()
        .once()
        .returning(|_| {
            Ok(UnmaskedKeyResult {
                import_result: None,
                raw_result: Some(raw_result()),
            })
        });
    app_session
        .expect_unmask_import_der_aesbulk256_key()
        .once()
        .returning(|_, _, _, _, _, _, _| Ok(import_der_aesbulk256_key().unwrap()));
    app_session
        .expect_end_import_der_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_bulk_key_len()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_bulk_key()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().once().returning(|| true);

    let req = encode_buf::<DdiUnmaskKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = UnmaskKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(cmd.rollback(TagId::default()), Ok(()));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiUnmaskKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

fn cmd_req() -> DdiUnmaskKeyCmdReq {
    DdiUnmaskKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(api_rev()),
            op: DdiOp::UnmaskKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiUnmaskKeyReq {
            masked_key: MborByteArray::new_with_len(MASKED_KEY.as_ptr(), MASKED_KEY.len()),
        },
    }
}

fn import_result(key_type: DdiKeyType) -> UnmaskedKeyImportResult {
    UnmaskedKeyImportResult {
        import_result: ImportDerKeyResult {
            priv_key_id: 1,
            pub_key_data: None,
            key_type,
        },
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    }
}

fn raw_result() -> UnmaskedKeyRawResult<MockEnv> {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate()
        .once()
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut entry_attributes = EntryAttributes::default();
    entry_attributes.common.flags.set_encrypt(true);
    entry_attributes.common.flags.set_decrypt(true);

    UnmaskedKeyRawResult {
        metadata: DdiMaskedKeyMetadata {
            svn: Some(1),
            key_type: DdiKeyType::AesGcmBulk256,
            key_attributes: DdiMaskedKeyAttributes {
                blob: entry_attributes.as_bytes().try_into().unwrap(),
            },
            bks2_index: Some(0), // BKS2_0 is used currently.
            key_tag: Some(1),
            key_label: MborByteArray::new_with_len(
                [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                DDI_MAX_KEY_LABEL_LENGTH,
            ),
            key_length: EntryKind::try_from(DdiKeyType::AesGcmBulk256)
                .map(|entry| entry.raw_key_blob_size() as u16)
                .unwrap(),
        },
        decrypted_key: heap.allocate(32).unwrap(),
    }
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

const MASKED_KEY: [u8; 32] = [0x01; 32];
