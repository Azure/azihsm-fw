// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;

use super::*;

use crate::cmd_scheduler::TagId;
use crate::fsm::establish_credential::EstablishCredentialCmd;
use crate::fsm::EstablishCredentialCtx;
use crate::partition::EstablishCredentialCmdState;
use crate::partition::ShaType;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::HsmFsmResourceId;
use crate::resource::PkaResource;
use crate::CmdResource;
use crate::CmdScheduler;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();

    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let part = MockPartition::new();
    let req = MockDmaAlloc::new(10);

    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);
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
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InsufficientBuffer));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_establish_credential() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiEstablishCredentialCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(
        resp.hdr.rev,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false)
            .hdr
            .rev
    );
    assert_eq!(
        resp.hdr.op,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false)
            .hdr
            .op
    );
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_partition_already_provisioned() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| true);

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::PartitionAlreadyProvisioned)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_unmask_bk3_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);

    part.expect_unmask_bk3()
        .times(1)
        .returning(|_, _| Err(HsmErr::UnmaskingBk3Failed));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::UnmaskingBk3Failed)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_new_mk_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Err(HsmErr::KbkdfError));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::KbkdfError)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_bmk_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InvalidState));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_bmk_failure_2() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::KbkdfError));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::KbkdfError)
    );

    assert!(cmd.take_response().is_some());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_generate_bmk_from_bmk_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));

    part.expect_import_mk_from_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::MaskedKeyDecodeFailed));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), true),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::MaskedKeyDecodeFailed)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());
}

#[test]
fn test_establish_credential_and_generate_bmk_from_bmk_success() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));

    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_import_mk_from_bmk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), true),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiEstablishCredentialCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(
        resp.hdr.rev,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), true)
            .hdr
            .rev
    );
    assert_eq!(
        resp.hdr.op,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), true).hdr.op
    );
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_establish_credential_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(..)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::Pending));

    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Ok(()));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    part.expect_is_partition_provisioned()
        .times(1)
        .returning(|| false);
    part.expect_unmask_bk3().times(1).returning(|_, _| Ok(()));
    part.expect_generate_and_store_bk3_session()
        .times(1)
        .returning(|_| Ok(()));
    part.expect_generate_bk()
        .times(1)
        .returning(|_, _, _| Ok(()));
    part.expect_generate_new_mk_and_import()
        .times(1)
        .returning(|| Ok(()));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InsufficientBuffer));
    part.expect_generate_bmk()
        .times(1)
        .returning(|_, _, _| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
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
        Err(HsmErr::Pending)
    );
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());

    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(!cmd.retry());

    let resp = decode_buf::<DdiEstablishCredentialCmdResp, MockEnv>(&resp.unwrap()).unwrap();
    assert_eq!(
        resp.hdr.rev,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false)
            .hdr
            .rev
    );
    assert_eq!(
        resp.hdr.op,
        cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false)
            .hdr
            .op
    );
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
}

#[test]
fn test_establish_credential_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Err(HsmErr::InvalidMgrCredential));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidMgrCredential)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::InvalidArgument));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();

    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_resource_pending_twice_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::Pending));
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| Err(HsmErr::Pending));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
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
fn test_establish_credential_null_pin() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_verify_nonce().times(1).returning(|_| Ok(()));

    let masked_bk3 = [0u8; 300];
    let bmk = [0u8; 300];
    let rev = DdiApiRev { major: 1, minor: 0 };
    let bmk_present = true;
    let req = DdiEstablishCredentialCmdReq {
        hdr: DdiReqHdr {
            rev: Some(rev),
            op: DdiOp::EstablishCredential,
            sess_id: None,
        },
        data: DdiEstablishCredentialReq {
            encrypted_credential: DdiEncryptedEstablishCredential {
                encrypted_id: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 0),
                iv: MborByteArray::new_with_len(core::ptr::null(), 16),
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                tag: [0u8; 48],
            },
            pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 96),
                key_kind: DdiKeyType::Ecc384Public,
            },
            masked_bk3: MborByteArray::new_with_len(
                &masked_bk3 as *const _ as *const u8,
                masked_bk3.len(),
            ),
            bmk: if bmk_present {
                MborByteArray::new_with_len(&bmk as *const _ as *const u8, bmk.len())
            } else {
                MborByteArray::new_with_len(core::ptr::null(), 0)
            },
            masked_unwrapping_key: MborByteArray::new_with_len(core::ptr::null(), 0),
            pota_sig: MborByteArray::new_with_len(core::ptr::null(), 0),
            pota_pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 0),
                key_kind: DdiKeyType::Ecc384Public,
            },
        },
    };

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(&req, &heap).unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_requires_and_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);
    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_establish_credential_empty_rev() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(&cmd_req(None, false), &heap).unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::UnsupportedRevision)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_bad_nonce() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_verify_nonce()
        .times(1)
        .returning(|_| Err(HsmErr::NonceMismatch));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::NonceMismatch)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_cred_already_set() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Err(HsmErr::AppLimitReached));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::AppLimitReached)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_on_continue_err_non_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::InvalidArgument));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_on_continue_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Err(HsmErr::Pending));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_on_err_non_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Err(HsmErr::InvalidArgument));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidArgument)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

#[test]
fn test_establish_credential_on_err_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_establish_credential()
        .times(1)
        .returning(|_, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::MontgomeryConstCalc,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| {
            Ok(establish_cred_ctx(
                EstablishCredentialCmdState::PublicKeyValidation,
            ))
        });

    part.expect_continue_establish_credential()
        .times(1)
        .returning(|_, _, _, _| Ok(establish_cred_ctx(EstablishCredentialCmdState::EcdhCompute)));

    part.expect_end_establish_credential()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    part.expect_verify_nonce().times(1).returning(|_| Ok(()));
    part.expect_verify_cred_is_not_set()
        .times(1)
        .returning(|| Ok(()));

    let req = encode_buf::<DdiEstablishCredentialCmdReq, _>(
        &cmd_req(Some(DdiApiRev { major: 1, minor: 0 }), false),
        &heap,
    )
    .unwrap();
    let mut cmd = EstablishCredentialCmd::<MockEnv>::new(req, heap, part);

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
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_none());
    assert!(cmd.retry());
}

fn cmd_req(rev: Option<DdiApiRev>, bmk_present: bool) -> DdiEstablishCredentialCmdReq {
    let masked_bk3 = [0u8; 300];
    let bmk = [0u8; 300];
    DdiEstablishCredentialCmdReq {
        hdr: DdiReqHdr {
            rev,
            op: DdiOp::EstablishCredential,
            sess_id: None,
        },
        data: DdiEstablishCredentialReq {
            encrypted_credential: DdiEncryptedEstablishCredential {
                encrypted_id: MborByteArray::new_with_len(core::ptr::null(), 16),
                encrypted_pin: MborByteArray::new_with_len(core::ptr::null(), 16),
                iv: MborByteArray::new_with_len(core::ptr::null(), 16),
                nonce: (1..33u8).collect::<Vec<_>>().try_into().unwrap(),
                tag: [0u8; 48],
            },
            pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 96),
                key_kind: DdiKeyType::Ecc384Public,
            },
            masked_bk3: MborByteArray::new_with_len(
                &masked_bk3 as *const _ as *const u8,
                masked_bk3.len(),
            ),
            bmk: if bmk_present {
                MborByteArray::new_with_len(&bmk as *const _ as *const u8, bmk.len())
            } else {
                MborByteArray::new_with_len(core::ptr::null(), 0)
            },
            masked_unwrapping_key: MborByteArray::new_with_len(core::ptr::null(), 0),
            pota_sig: MborByteArray::new_with_len(core::ptr::null(), 0),
            pota_pub_key: DdiDerPublicKey {
                der: MborByteArray::new_with_len(core::ptr::null(), 0),
                key_kind: DdiKeyType::Ecc384Public,
            },
        },
    }
}

fn establish_cred_ctx(state: EstablishCredentialCmdState) -> EstablishCredentialCtx<MockEnv> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), Some(key_id()));
    let pka_curve = PkaEccCurve::Ecc384;
    EstablishCredentialCtx {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        cmd_info: PkaEccCmd { curve: pka_curve },
        state,
        digest_buf: MockDmaAlloc::new(ShaType::Sha384.get_digest_size_hw()),
    }
}

fn key_id() -> u16 {
    1
}
