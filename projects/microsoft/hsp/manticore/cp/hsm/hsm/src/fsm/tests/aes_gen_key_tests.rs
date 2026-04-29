// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::aes_gen_key::AesGenKeyCmd;
use crate::fsm::HsmFsmEventRecorder;
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

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
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
    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    let part = MockPartition::new();

    let req = MockDmaAlloc::new(10);

    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
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
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_aes_gen_key()
        .once()
        .returning(|_key_tag, _key_size, _key_usage, _avail| {
            Ok(AesKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)))
        });
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    session.expect_delete_key().once().returning(|_| Ok(()));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_rollback_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_aes_gen_key()
        .once()
        .returning(|_key_tag, _key_size, _key_usage, _avail| {
            Ok(AesKey::new(VaultKey::new(KeyStore::new(0, 1u128), 0)))
        });
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_delete_key()
        .once()
        .returning(|_| Err(HsmErr::InvalidKeyIndex));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::InvalidKeyIndex));
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_aes_gen_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_aes_gen_key()
        .once()
        .returning(|_key_tag, _key_size, _key_usage, _avail| {
            Ok(AesKey::new(VaultKey::new(
                KeyStore::new(0, 1u128),
                key_id(),
            )))
        });
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, key_id());
}

#[test]
fn test_aes_gen_key_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);
    session
        .expect_aes_gen_key()
        .once()
        .returning(|_key_tag, _key_size, _key_usage, _avail| Err(HsmErr::VaultNotFound));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::VaultNotFound)
    );
    let resp = cmd.take_response();
    assert!(resp.is_none());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_aes_gen_key_invalid_key_usage() {
    let mut cmd: DdiAesGenerateKeyCmdReq = cmd_req();
    cmd.data.key_properties.key_metadata.set_sign(true);
    cmd.data.key_properties.key_metadata.set_verify(true);
    cmd.data.key_properties.key_metadata.set_encrypt(false);
    cmd.data.key_properties.key_metadata.set_decrypt(false);

    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut session = MockUserSession::new();
    session.expect_id().once().returning(SessionId::default);

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd, &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidPermissions)
    );
    let resp = cmd.take_response();
    assert!(resp.is_none());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut session = MockUserSession::new();
    session.expect_id().times(2).returning(SessionId::default);
    session
        .expect_aes_gen_key()
        .once()
        .returning(|_key_tag, _key_size, _key_usage, _avail| {
            Ok(AesKey::new(VaultKey::new(
                KeyStore::new(0, 1u128),
                key_id(),
            )))
        });
    session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, key_id());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_aesbulk256_gen_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_aesbulk256_gen_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session.expect_id().times(1).returning(Default::default);

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_aesbulk256_gen_key_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_aesbulk256_gen_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session.expect_id().times(1).returning(Default::default);

    #[cfg(feature = "mcr_test_hooks")]
    app_session
        .expect_cmd_fsm_test_action()
        .times(1)
        .returning(|_| None);

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_aesbulk256_gen_key_on_engine_ready_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::Pending));
    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel),
            TagId::default()
        ) == Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_aesbulk256_gen_key_on_engine_ready_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Err(HsmErr::InvalidArgument));

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::InvalidArgument));
}

#[test]
fn test_acquire_fp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let session = MockUserSession::new();
    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, session, part, PcieFunction::Vf0);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::FpIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::FpIpcChannel)
    );
}

#[test]
fn test_aesbulk256_gen_key_rollback() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_aesbulk256_gen_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session.expect_id().times(1).returning(Default::default);
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

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

#[test]
fn test_aesbulk256_gen_key_rollback_during_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_aesbulk256_gen_key()
        .once()
        .returning(|_, _, _, _, _| Ok(gen_aesbulk256_key_cmd().unwrap()));
    app_session
        .expect_end_aesbulk256_gen_key()
        .once()
        .returning(|_| Ok(()));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));
    app_session.expect_id().times(1).returning(Default::default);
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

    let req = encode_buf::<DdiAesGenerateKeyCmdReq, _>(&cmd_req_aesbulk256(), &heap).unwrap();
    let mut cmd = AesGenKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Vf0);
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

    let resp = decode_buf::<DdiAesGenerateKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev, cmd_req_aesbulk256().hdr.rev);
    assert_eq!(resp.hdr.op, cmd_req_aesbulk256().hdr.op);
    assert_eq!(resp.hdr.sess_id, cmd_req_aesbulk256().hdr.sess_id);
    assert_eq!(resp.hdr.status, DdiStatus::Success);

    assert_eq!(cmd.rollback(TagId::default()), Err(HsmErr::Pending));

    assert!(cmd
        .on_event(HsmFsmEvent::FpToHsmIpcResponse, TagId::default())
        .is_ok());
}

fn cmd_req() -> DdiAesGenerateKeyCmdReq {
    DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::AesGenerateKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::Aes128,
            key_tag: None,
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

fn cmd_req_aesbulk256() -> DdiAesGenerateKeyCmdReq {
    DdiAesGenerateKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::AesGenerateKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiAesGenerateKeyReq {
            key_size: DdiAesKeySize::AesGcmBulk256Unapproved,
            key_tag: Some(1),
            key_properties: DdiTargetKeyProperties {
                key_metadata: DdiTargetKeyMetadata::default()
                    .with_encrypt(true)
                    .with_decrypt(true)
                    .with_session(false),
                key_label: MborByteArray::new_with_len(
                    [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
                    DDI_MAX_KEY_LABEL_LENGTH,
                ),
            },
        },
    }
}

pub(crate) fn gen_aesbulk256_key_cmd() -> HsmResult<AesBulk256Cmd<MockEnv>> {
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
        0,
        channel.unwrap(),
    ))
}

fn key_id() -> u16 {
    1
}
