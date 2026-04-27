// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::raw_key_import::RawKeyImportCmd;
use mcr_types::*;
use openssl::rand::rand_bytes;

// Shared constants
const SECRET_256_SIZE: usize = 32;
const SECRET_384_SIZE: usize = 48;
const SECRET_521_SIZE: usize = 68;
const RSA_2K_KEY_SIZE: usize = 516;
const RSA_3K_KEY_SIZE: usize = 772;
const RAW_KEY_BUFFER_SIZE: usize = 1028;
// Key tag
const KEY_TAG: u16 = 0x5453;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);

    let mut secret_buf = [0u8; SECRET_256_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Secret256,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().once().returning(Default::default);
    let req = MockDmaAlloc::new(1024);

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool().once().returning(|_| None);
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));

    let mut secret_buf = [0u8; SECRET_384_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Secret384,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
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
    app_session.expect_id().times(2).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut secret_buf = [0u8; SECRET_521_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(true),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Secret521,
            None,
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
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
fn test_invalid_key_type_request() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidKeyType));

    let mut secret_buf = [0u8; RSA_3K_KEY_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_unwrap(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Rsa3kPrivate,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidKeyType)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_key_usage_request() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidPermissions));

    let mut secret_buf = [0u8; RSA_2K_KEY_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_encrypt(true)
            .with_decrypt(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };

    // Note that the logic of Rsa2kPrivate can only be used as an unwrap key, stays in the app session layer
    // So we have to mock that logic on line 255
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Rsa2kPrivate,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidPermissions)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_key_availability_request() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Err(HsmErr::InvalidArgument));

    let mut secret_buf = [0u8; SECRET_521_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(true),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Secret521,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_raw_key_import_secret521() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut secret_buf = [0u8; SECRET_521_SIZE];
    rand_bytes(&mut secret_buf).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..secret_buf.len()].copy_from_slice(&secret_buf);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            secret_buf.len(),
            DdiKeyType::Secret521,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_raw_key_import_var_hmac_sha256() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut key_length_buf = [0u8; 1];
    rand_bytes(&mut key_length_buf).expect("Failed to generate random bytes");
    let key_length = (key_length_buf[0] % 33) + 32; // 32-64
    let mut var_hmac_key = vec![0u8; key_length as usize];
    rand_bytes(&mut var_hmac_key).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..var_hmac_key.len()].copy_from_slice(&var_hmac_key);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            var_hmac_key.len(),
            DdiKeyType::VarLenHmacSha256,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_raw_key_import_var_hmac_sha384() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut key_length_buf = [0u8; 1];
    rand_bytes(&mut key_length_buf).expect("Failed to generate random bytes");
    let key_length = (key_length_buf[0] % 81) + 48; // 48-128
    let mut var_hmac_key = vec![0u8; key_length as usize];
    rand_bytes(&mut var_hmac_key).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..var_hmac_key.len()].copy_from_slice(&var_hmac_key);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            var_hmac_key.len(),
            DdiKeyType::VarLenHmacSha384,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_raw_key_import_var_hmac_sha521() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let mut app_session = MockUserSession::new();
    app_session.expect_id().times(1).returning(Default::default);
    app_session
        .expect_import_raw_key()
        .once()
        .returning(|_, _, _, _| Ok(4));
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut key_length_buf = [0u8; 1];
    rand_bytes(&mut key_length_buf).expect("Failed to generate random bytes");
    let key_length = (key_length_buf[0] % 65) + 64; // 64-128
    let mut var_hmac_key = vec![0u8; key_length as usize];
    rand_bytes(&mut var_hmac_key).expect("Failed to generate random bytes");
    let mut raw_key = [0u8; RAW_KEY_BUFFER_SIZE];
    raw_key[..var_hmac_key.len()].copy_from_slice(&var_hmac_key);
    let key_properties = DdiTargetKeyProperties {
        key_metadata: DdiTargetKeyMetadata::default()
            .with_derive(true)
            .with_session(false),
        key_label: MborByteArray::new_with_len(
            [2u8; DDI_MAX_KEY_LABEL_LENGTH].as_ptr(),
            DDI_MAX_KEY_LABEL_LENGTH,
        ),
    };
    let req = encode_buf::<DdiRawKeyImportCmdReq, _>(
        &cmd_req(
            raw_key,
            var_hmac_key.len(),
            DdiKeyType::VarLenHmacSha512,
            Some(KEY_TAG),
            key_properties,
        ),
        &heap,
    )
    .unwrap();

    let mut cmd = RawKeyImportCmd::<MockEnv>::new(req, heap, app_session);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

fn cmd_req(
    raw: [u8; 1028],
    keylength: usize,
    key_kind: DdiKeyType,
    key_tag: Option<u16>,
    key_properties: DdiTargetKeyProperties,
) -> DdiRawKeyImportCmdReq {
    DdiRawKeyImportCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::RawKeyImport,
            sess_id: Some(SessionId::default()),
        },
        data: DdiRawKeyImportReq {
            raw: MborByteArray::new_with_len(raw.as_ptr(), keylength),
            key_kind,
            key_tag,
            key_properties,
        },
    }
}
