// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::HkdfDeriveCmd;
use crate::fsm::HsmFsmResourceId;
use crate::partition::*;
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
    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert!(!cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let req = MockDmaAlloc::new(10);

    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_hkdf_derive()
        .once()
        .returning(|_, _, _, _, _, _, _, _| Ok(key_id()));
    app_session.expect_delete_key().once().returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert!(!cmd.retry());
}

#[test]
fn test_rollback_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });

    app_session
        .expect_hkdf_derive()
        .once()
        .returning(|_, _, _, _, _, _, _, _| Ok(key_id()));
    app_session
        .expect_delete_key()
        .once()
        .returning(|_| Err(HsmErr::InvalidKeyIndex));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::InvalidKeyIndex));
    assert!(!cmd.retry());
}

#[test]
fn test_hkdf_derive() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_hkdf_derive()
        .once()
        .returning(|_, _, _, _, _, _, _, _| Ok(key_id()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    validate_response(resp, false);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_hkdf_derive_aesbulk256_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_kdf_aesbulk256_derive()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
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

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    validate_response(resp, true);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_hkdf_derive_aesbulk256_key_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_kdf_aesbulk256_derive()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
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

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
    validate_response(resp, true);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_hkdf_derive_aesbulk256_key_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
fn test_hkdf_derive_aesbulk256_key_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
    let app_session = MockUserSession::new();
    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_aesbulk256_rollback_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_kdf_aesbulk256_derive()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
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

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    validate_response(resp, true);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

#[test]
fn test_aesbulk256_rollback_during_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_hkdf_aesbulk256_derive()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_kdf_aesbulk256_derive()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_begin_rollback_aesbulk256_key()
        .once()
        .returning(|_, _, _| Ok(()));
    app_session
        .expect_end_rollback_aesbulk256_key()
        .once()
        .returning(|_| Ok(()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
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

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&aesbulk256_cmd_req(), &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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
    validate_response(resp, true);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

#[test]
fn test_hkdf_derive_some_salt_info() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_hkdf_derive()
        .once()
        .returning(|_, _, _, _, _, _, _, _| Ok(key_id()));
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd_req = cmd_req();
    let salt = MborByteArray::new_with_len([0u8; 256].as_ptr(), 256);
    let info = MborByteArray::new_with_len([0u8; 256].as_ptr(), 256);
    cmd_req.data.salt = Some(salt);
    cmd_req.data.info = Some(info);

    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req, &heap).unwrap();
    let mut cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    validate_response(resp, false);

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiHkdfDeriveCmdReq, _>(&cmd_req(), &heap).unwrap();
    let app_session = MockUserSession::new();
    let cmd = HkdfDeriveCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);

    assert!(!cmd.requires_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel));
}

fn cmd_req() -> DdiHkdfDeriveCmdReq {
    DdiHkdfDeriveCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::HkdfDerive,
            sess_id: Some(SessionId::default()),
        },
        data: DdiHkdfDeriveReq {
            key_id: 0,
            hash_algorithm: DdiHashAlgorithm::Sha256,
            salt: None,
            info: None,
            key_type: DdiKeyType::Aes256,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_derive(true)
                    .with_session(true),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
            key_length: None,
        },
    }
}

fn aesbulk256_cmd_req() -> DdiHkdfDeriveCmdReq {
    DdiHkdfDeriveCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::HkdfDerive,
            sess_id: Some(SessionId::default()),
        },
        data: DdiHkdfDeriveReq {
            key_id: 0,
            hash_algorithm: DdiHashAlgorithm::Sha256,
            salt: None,
            info: None,
            key_type: DdiKeyType::AesGcmBulk256,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_derive(true)
                    .with_session(true),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
            key_length: None,
        },
    }
}

fn gen_aesbulk256_key_cmd() -> HsmResult<AesBulk256Cmd<MockEnv>> {
    let mock_ipc_message_channel = MockIpcMessageChannel::new();
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(
        FpIpcChannelResource::new(mock_ipc_message_channel),
        scheduler,
        1,
    );
    let channel = resource.acquire(TagId::default(), ());

    Ok(AesBulk256Cmd::DerKeyImport(
        Default::default(),
        key_id(),
        channel.unwrap(),
        SecureByteArray::<32>::new([0u8; 32]),
    ))
}

fn validate_response(resp: Option<MockDmaAlloc>, aes_bulk256: bool) {
    let resp = decode_buf::<DdiHkdfDeriveCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, key_id());
    match aes_bulk256 {
        true => assert_eq!(resp.data.bulk_key_id.unwrap(), Default::default()),
        false => assert!(resp.data.bulk_key_id.is_none()),
    }
}

fn key_id() -> u16 {
    1
}
