// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;
use crate::{cmd_scheduler::TagId, partition::EntryKind};

#[test]
fn test_successful_get_privkey() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::Ecc256Private));
    app_session
        .expect_get_key_length()
        .times(1)
        .returning(|_| Ok(EntryKind::Ecc256Private.raw_key_blob_size() as u16));
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);

    app_session
        .expect_get_priv_key()
        .times(1)
        .returning(|_, _| Ok(()));

    // Get Private Key Command
    let get_priv_key_req = encode_buf(&get_priv_key_cmd_req(12), &heap).unwrap();
    let mut get_priv_key_cmd = GetPrivKeyCmd::<MockEnv>::new(get_priv_key_req, heap, app_session);

    assert_eq!(
        get_priv_key_cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );
    assert!(get_priv_key_cmd.take_response().is_some());
    assert!(get_priv_key_cmd.session_id().is_some());
}

#[test]
fn test_get_privkey_aes_bulk256() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Ok(EntryKind::AesXtsBulk256));
    app_session
        .expect_api_rev()
        .times(1)
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session.expect_id().times(2).returning(Default::default);

    app_session
        .expect_get_priv_key()
        .times(1)
        .returning(|_, _| Ok(()));

    let get_priv_key_req = encode_buf(&get_priv_key_cmd_req(12), &heap).unwrap();
    let mut get_priv_key_cmd = GetPrivKeyCmd::<MockEnv>::new(get_priv_key_req, heap, app_session);

    assert_eq!(
        get_priv_key_cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Ok(())
    );

    assert!(get_priv_key_cmd.take_response().is_some());
    assert!(get_priv_key_cmd.session_id().is_some());
}

#[test]
fn test_get_privkey_no_key_found_in_get_key_kind() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut app_session = MockUserSession::new();
    app_session
        .expect_get_key_kind()
        .times(1)
        .returning(|_| Err(HsmErr::KeyNotFound));
    app_session.expect_id().times(1).returning(Default::default);

    // Get Private Key Command
    let get_priv_key_req = encode_buf(&get_priv_key_cmd_req(12), &heap).unwrap();
    let mut get_priv_key_cmd = GetPrivKeyCmd::<MockEnv>::new(get_priv_key_req, heap, app_session);

    assert_eq!(
        get_priv_key_cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::KeyNotFound)
    );

    assert!(get_priv_key_cmd.take_response().is_none());
    assert!(get_priv_key_cmd.session_id().is_some());
}

fn get_priv_key_cmd_req(key_id: u16) -> DdiGetPrivKeyCmdReq {
    DdiGetPrivKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetPrivKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiGetPrivKeyReq { key_id },
    }
}
