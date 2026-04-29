// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaRsaCmd;
use mcr_crypto_pka::PkaRsaSize;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::HsmFsmResourceId;
use crate::partition::*;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;
use crate::HsmFsmEventRecorder;

fn cmd_req() -> DdiGetUnwrappingKeyCmdReq {
    DdiGetUnwrappingKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetUnwrappingKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiGetUnwrappingKeyReq {},
    }
}

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);
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
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);
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
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(0));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: Some(GetUnwrappingKeyOut {
                    id: 0,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_get_unwrapping_key_new_unwrapping_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let encoded_resp = resp.unwrap();

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&encoded_resp).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetUnwrappingKey);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
    assert_eq!(resp.data.pub_key.der.len(), 260);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
}

#[test]
fn test_get_unwrapping_key_existing_unwrapping_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(4));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let encoded_resp = resp.unwrap();

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&encoded_resp).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetUnwrappingKey);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
    assert_eq!(resp.data.pub_key.der.len(), 260);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(0));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(3)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_acquire_hsp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let app_session = MockUserSession::new();
    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::HspIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel)
    );
}

#[test]
fn test_acquire_pka_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let app_session = MockUserSession::new();
    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_get_unwrapping_key_on_resource_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(2).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        });

    app_session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |op| {
            let mut op = op;
            op.state = RsaPctValidationState::WaitForDecrypt;
            Ok(op)
        });
    app_session
        .expect_end_rsa_pct_validation()
        .once()
        .returning(|_op| Ok(true));
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| true);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let encoded_resp = resp.unwrap();

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&encoded_resp).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetUnwrappingKey);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
    assert_eq!(resp.data.pub_key.der.len(), 260);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
}

#[test]
fn test_get_unwrapping_key_failure_in_verify() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_clear_unwrapping_key()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();

    app_session
        .expect_notify_pct_validation_failure()
        .once()
        .returning(|err| println!("Simulating PCT Validation Failure: {:?}", err));

    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        });

    app_session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |op| {
            let mut op = op;
            op.state = RsaPctValidationState::WaitForDecrypt;
            Ok(op)
        });
    app_session
        .expect_end_rsa_pct_validation()
        .once()
        .returning(|_op| Ok(false));
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| true);
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // PCT Validation completes failure

    let _ = cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default());
}

#[test]
fn test_get_unwrapping_key_error_in_verify() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        });

    app_session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |op| {
            let mut op = op;
            op.state = RsaPctValidationState::WaitForDecrypt;
            Ok(op)
        });
    app_session
        .expect_end_rsa_pct_validation()
        .once()
        .returning(|_op| Err(HsmErr::RsaModExpFailed));
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| true);

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::RsaModExpFailed)
    );
}

#[test]
fn test_get_unwrapping_key_invalid_state_on_resource_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidState)
    );
}

#[test]
fn test_get_unwrapping_key_error_in_begin_pct() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::RsaModExpFailed));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::RsaModExpFailed)
    );
}

#[test]
fn test_get_unwrapping_key_error_on_begin_pct_retry() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::RsaModExpFailed));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::RsaModExpFailed)
    );
}

#[test]
fn test_get_unwrapping_key_error_in_continue_pct() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| {
            Ok(GetUnwrappingKeyOut {
                id: 4,
                data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
            })
        });
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |tag, key_id, usage, rsa_type, _, _| {
            begin_rsa_pct_validation(tag, key_id, usage, rsa_type)
        });

    app_session
        .expect_continue_rsa_pct_validation()
        .times(1)
        .returning(move |_op| Err(HsmErr::RsaModExpFailed));
    app_session
        .expect_is_rsa_pct_final_state()
        .once()
        .returning(|_| false);

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::RsaModExpFailed)
    );
}

#[test]
fn test_get_unwrapping_key_on_resource_ready_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(2).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

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

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_get_unwrapping_key_on_ipc_response_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(2).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });

    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| Err(HsmErr::IpcResponseError));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

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
        Err(HsmErr::IpcResponseError)
    );

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_get_unwrapping_key_err_on_ipc_response() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(2).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: None,
            })
        });
    app_session
        .expect_end_get_unwrapping_key()
        .once()
        .returning(|_| Err(HsmErr::IpcResponseError));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

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
        Err(HsmErr::IpcResponseError)
    );

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_get_unwrapping_key_key_not_found_on_start() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::KeyNotFound));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::KeyNotFound)
    );

    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_get_unwrapping_key_key_available_after_ipc_resource_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(4));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| Err(HsmErr::Pending));

    app_session
        .expect_begin_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                channel_ref: None,
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    app_session
        .expect_api_rev()
        .once()
        .returning(|| DdiApiRev { major: 1, minor: 0 });
    app_session
        .expect_get_masked_key_len_from_vault()
        .once()
        .returning(|_, _, _| Ok(32));
    app_session
        .expect_mask_key_from_vault()
        .once()
        .returning(|_, _, _, _| Ok(()));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            TagId::default()
        )
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetUnwrappingKey);
    assert_eq!(resp.hdr.sess_id, Some(SessionId::default()));
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
    assert_eq!(resp.data.pub_key.der.len(), 260);
    assert_eq!(
        resp.data.pub_key.der.as_slice(),
        vec![1; resp.data.pub_key.der.len()]
    );
}

fn begin_rsa_pct_validation(
    tag: TagId,
    key_id: u16,
    usage: RsaKeyUsage,
    rsa_type: PkaRsaSize,
) -> Result<RsaPctValidationCmd<MockEnv>, HsmErr> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id));
    let cmd_info = PkaRsaCmd { rsa_type };
    let rsa_op = RsaModExp {
        tag,
        engine_ref: engine.unwrap(),
        is_crt: false,
        cmd_info,
    };
    Ok(RsaPctValidationCmd {
        tag,
        key_id,
        rsa_type,
        usage,
        dma_buf: MockDmaAlloc::new(400),
        state: RsaPctValidationState::WaitForEncrypt,
        rsa_op,
    })
}
