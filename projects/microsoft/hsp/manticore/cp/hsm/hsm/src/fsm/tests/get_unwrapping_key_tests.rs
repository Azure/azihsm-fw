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
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| true);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    part.expect_mark_unwrapping_key_pct_verified()
        .once()
        .return_const(());

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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

    // First tick: synchronous import succeeds, FSM kicks PCT off, returns Pending.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // PKA fires for the encrypt half — not final yet → continue, return Pending.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // PKA fires for the decrypt half — final state → end_rsa_pct → Ok.
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
fn test_get_unwrapping_key_pct_failure_clears_slot() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    part.expect_clear_unwrapping_key()
        .once()
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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

    // PCT chain: begin -> continue -> end returning Ok(false) (PCT failed).
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
        .expect_notify_pct_validation_failure()
        .once()
        .return_const(());

    // FSM still calls prepare_response, so masking mocks match the success test.
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
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
}

// A hard error during PCT must terminate the command and rollback-zeroize the staged key.
#[test]
fn test_get_unwrapping_key_error_in_verify_clears_slot() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    // clear_unwrapping_key is driven by rollback(), not by the error path itself.
    part.expect_clear_unwrapping_key()
        .once()
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });

    // PCT chain: begin -> continue -> end returning Err (hard engine fault).
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
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // The hard error surfaces as a terminal failure (not Pending).
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::RsaModExpFailed)
    );

    // Dispatcher rollback after the terminal error must zeroize the staged key.
    assert_eq!(cmd.rollback(TagId::default()), Ok(()));
}

// A hard error while *beginning* PCT (before the engine accepts the op) must terminate
// the command and leave the staged key for rollback to zeroize.
#[test]
fn test_get_unwrapping_key_error_in_begin_pct() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    // Rollback after the terminal error must zeroize the staged key.
    part.expect_clear_unwrapping_key()
        .once()
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    // begin_rsa_pct_validation fails hard (non-pending) on the very first attempt.
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::RsaModExpFailed));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    // The hard error surfaces on the very first tick as a terminal failure (not Pending).
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::RsaModExpFailed)
    );
    assert_eq!(cmd.rollback(TagId::default()), Ok(()));
}

// begin PCT reports `Pending` (engine busy) so the FSM parks in `WaitForEngineToBeginPct`
// and re-issues begin on the next `ResourceReady`. That retry then fails hard and must
// terminate the command.
#[test]
fn test_get_unwrapping_key_error_on_begin_pct_retry() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    // Rollback after the terminal error must zeroize the staged key.
    part.expect_clear_unwrapping_key()
        .once()
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
                output: Some(GetUnwrappingKeyOut {
                    id: 4,
                    data: RsaPubKey::from_priv_pka_slice(&[1; 516], RsaSize::Rsa2k).unwrap(),
                }),
            })
        });
    // First begin attempt: engine busy -> Pending (FIFO expectation #1).
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::Pending));
    // Retry on ResourceReady: hard failure (FIFO expectation #2).
    app_session
        .expect_begin_rsa_pct_validation()
        .once()
        .returning(move |_tag, _key_id, _usage, _rsa_type, _, _| Err(HsmErr::RsaModExpFailed));

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    // First tick: begin PCT is pending -> FSM parks waiting for the engine.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Engine ready: begin PCT is retried and fails hard -> terminal failure.
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::RsaModExpFailed)
    );
    assert_eq!(cmd.rollback(TagId::default()), Ok(()));
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
    // Vault key that is already `PctPassed` → PCT is skipped.
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| true);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(2)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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
fn test_get_unwrapping_key_cache_hit_skips_pct() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(4));
    // Vault key that is already `PctPassed` → PCT is skipped.
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| true);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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

    // No PCT expectations: a stray PCT call panics with "no matching expectation".

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    // Cache-hit path must complete in a single tick — no Pending, no PCT.
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
}

#[test]
fn test_get_unwrapping_key_none_key_id_pct_verified_skips_pct() {
    // key_id == None but slot already `PctPassed` (post-NSSR re-import): import and respond, no PCT.
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| true);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(1)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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

    // No PCT expectations: a stray PCT call panics with "no matching expectation".

    let mut cmd =
        GetUnwrappingKeyCmd::<MockEnv>::new(req, heap, app_session, part, PcieFunction::Pf);

    // Skip-PCT path completes in one tick — no Pending, no PCT.
    assert!(cmd
        .on_event(HsmFsmEvent::StartCmd, TagId::default())
        .is_ok());
    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
}

// Regression: a key imported via `unmask_unwrapping_key_and_import` is in the vault
// (`key_id.is_some()`) but only `PendingPct`.  `GetUnwrappingKey` must still run the deferred RSA
// PCT before returning it, and promote the slot to `PctPassed` on success.  This closes the FIPS
// PCT-coverage gap where an imported unwrapping key was previously returned without any PCT.
#[test]
fn test_get_unwrapping_key_some_key_id_pending_pct_runs_pct() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    // Imported key present in the vault ...
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(4));
    // ... but not yet PCT-verified (`PendingPct`) → PCT MUST run.
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    part.expect_mark_unwrapping_key_pct_verified()
        .once()
        .return_const(());

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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

    // First tick: vault hit, but PendingPct → FSM kicks off PCT, returns Pending.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    // Encrypt half — not final → continue, Pending.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Decrypt half — final → end_rsa_pct Ok(true) → mark verified → respond.
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    let resp = decode_buf::<DdiGetUnwrappingKeyCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.key_id, 4);
}

// Regression companion: imported key (`key_id.is_some()`) in `PendingPct`, PCT FAILS
// (e.g. TriggerNegativePctFailure injected a bad modulus).  The FSM must clear the staged slot and
// notify the PCT-validation failure — it must NOT silently return the un-validated key.
#[test]
fn test_get_unwrapping_key_some_key_id_pending_pct_failure_clears_slot() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(4));
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| false);
    part.expect_clear_unwrapping_key()
        .once()
        .returning(|| Ok(()));

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session.expect_id().returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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
        .expect_notify_pct_validation_failure()
        .once()
        .return_const(());
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
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
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
    part.expect_is_unwrapping_key_pct_verified()
        .once()
        .returning(|| true);

    let req = encode_buf::<DdiGetUnwrappingKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut app_session = MockUserSession::new();
    app_session
        .expect_id()
        .times(3)
        .returning(SessionId::default);
    app_session
        .expect_get_unwrapping_key()
        .once()
        .returning(|_, _, _| {
            Ok(GetUnwrappingKeyCtx {
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
        .expect_get_unwrapping_key()
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
