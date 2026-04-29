// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_crypto_pka::PkaEccCmd;
use mcr_crypto_pka::PkaEccCurve;
use mcr_crypto_pka::PkaEccPublicKey;
use mcr_types::*;

use super::*;
use crate::cmd_scheduler::*;
use crate::fsm::EccCurve;
use crate::fsm::HsmFsmResourceId;
use crate::fsm::OpenKeyPhase;
use crate::fsm::PublicKey;
use crate::partition::store::EntryAttributeFlags;
use crate::partition::*;
use crate::recorder::HsmFsmEventRecorder;
use crate::resource::PkaResource;

#[test]
fn test_invalid_event() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let part = MockPartition::new();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
    assert!(cmd.rollback(TagId::default()).is_ok());
}

#[test]
fn test_decode_req_err() {
    let heap = MockDmaHeap::new();
    let req = MockDmaAlloc::new(10);
    let partition = MockPartition::new();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, partition, SessionId::default());
    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DdiDecodeFailed)
    );
    assert!(cmd.take_response().is_none());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
    assert!(!cmd.retry());
}

#[test]
fn test_attest_key_no_session() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(1).return_const(None);
    part.expect_user_session()
        .times(1)
        .returning(move |_, _| Err(HsmErr::SessionNotFound));

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::SessionNotFound)
    );
}

#[test]
fn test_attest_ecc_key_on_start() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );

        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_attest_ecc_key_on_engine_ready() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(7)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .times(2)
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
        Ok(app_session)
    });

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_attest_ecc_key_on_engine_ready_with_key_import() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(7)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .times(2)
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
        Ok(app_session)
    });

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_attest_ecc_key_on_engine_ready_begin_sign_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(7)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .times(2)
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
        Ok(app_session)
    });

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::InvalidArgument),
        );
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_attest_ecc_key_on_engine_ready_but_no_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(7)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .times(2)
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
        Ok(app_session)
    });

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_tag, _key_in, _digest, _digest_hash_algo, _sig| Err(HsmErr::Pending));
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
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
fn test_encode_buf_err() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|_| None);

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_report_sign_error() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_begin_ecc_sign_zc()
            .once()
            .returning(move |_, _, _, _, _| Err(HsmErr::EccSignFailed));
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::EccSignFailed)
    );
}

#[test]
fn test_attest_ecc_key_done() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(4).return_const(None);

    // Open key
    for phase in [
        OpenKeyPhase::PendingUpkaEngine,
        OpenKeyPhase::PendingMontgomeryConstCalc,
        OpenKeyPhase::PendingPointMultiplication,
    ] {
        part.expect_user_session().times(1).returning(move |_, _| {
            let mut app_session = MockUserSession::new();
            app_session
                .expect_open_key_zc()
                .once()
                .returning(move |_, _, _, _, _, _, _, _| {
                    Ok(OpenKeyData {
                        phase,
                        id: 12,
                        kind: EntryKind::Ecc256Private,
                        flags: EntryAttributeFlags::default(),
                        pub_key: None,
                        bulk_key_id: None,
                    })
                });

            Ok(app_session)
        });
    }

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: 12,
                    kind: EntryKind::Ecc256Private,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    for event in [
        HsmFsmEvent::StartCmd,
        HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
        HsmFsmEvent::PkaDone(0),
    ] {
        assert_eq!(cmd.on_event(event, TagId::default()), Err(HsmErr::Pending));
        assert!(cmd.take_response().is_none());
        assert!(cmd.session_id().is_some());
    }
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
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_attest_unwrapping_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let key_id = 12;

    // Open key
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: key_id,
                    kind: EntryKind::Rsa2kPrivate,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_attest_unwrapping_key_with_key_import() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let key_id = 12;

    // Open key
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: key_id,
                    kind: EntryKind::Rsa2kPrivate,
                    flags: EntryAttributeFlags::default(),
                    pub_key: Some(pub_key()),
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
    assert!(cmd.take_response().is_none());
    assert!(cmd.rollback(TagId::default()).is_ok());
    assert_eq!(cmd.session_id(), Some(SessionId::default()));
}

#[test]
fn test_attest_aes_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(4)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_unwrapping_key_id().times(1).return_const(None);

    let key_id = 12;

    // Open key
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(move |_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Done,
                    id: key_id,
                    kind: EntryKind::Aes256,
                    flags: EntryAttributeFlags::default(),
                    pub_key: None,
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_app_id().once().returning(app_id);
        Ok(app_session)
    });

    part.expect_get_partition_id_private_key_blob()
        .once()
        .returning(move || Some([1; 48].as_slice()));

    // Sha
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_sha_single_block_zc()
            .once()
            .returning(|_, _, _| Ok(()));

        Ok(app_session)
    });

    // ECC sign
    let curve = EccCurve::P384;
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_begin_ecc_sign_zc().once().returning(
            move |_tag, _key_in, _digest, _digest_hash_algo, _sig| begin_ecc_sign(curve),
        );
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_end_ecc_sign_zc()
            .once()
            .returning(move |_tag, _op| Ok(()));
        Ok(app_session)
    });

    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session.expect_api_rev().once().returning(api_rev);
        Ok(app_session)
    });

    part.expect_vm_launch_guid().times(1).returning(|| [0; 16]);

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Ok(())
    );
    let resp = cmd.take_response();
    assert!(resp.is_some());
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
    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let part = MockPartition::new();
    let cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::Pka));
}

#[test]
fn test_acquire_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let part = MockPartition::new();
    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_open_key_key_not_found() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(0));
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(|_, _, _, _, _, _, _, _| Err(HsmErr::KeyNotFound));
        Ok(app_session)
    });

    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::KeyNotFound)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
    assert!(!cmd.retry());
}

#[test]
fn test_invalid_state_open_key() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .once()
        .returning(|s| Some(MockDmaAlloc::new(s)));

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let req = encode_buf::<DdiAttestKeyCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut part = MockPartition::new();
    part.expect_unwrapping_key_id()
        .times(1)
        .return_const(Some(0));
    part.expect_user_session().times(1).returning(move |_, _| {
        let mut app_session = MockUserSession::new();
        app_session
            .expect_open_key_zc()
            .once()
            .returning(|_, _, _, _, _, _, _, _| {
                Ok(OpenKeyData {
                    phase: OpenKeyPhase::Init,
                    id: 12,
                    kind: EntryKind::Aes128,
                    flags: EntryAttributeFlags::default(),
                    pub_key: None,
                    bulk_key_id: None,
                })
            });
        Ok(app_session)
    });

    let mut cmd = AttestKeyCmd::<MockEnv>::new(req, heap, part, SessionId::default());

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
    assert!(cmd.session_id().is_some());
}

fn pub_key() -> PublicKey {
    PublicKey::EccPubKey(PkaEccPublicKey {
        curve: PkaEccCurve::Ecc256,
        data: [0u8; PkaEccCurve::MAX_LEN * 2],
    })
}

fn api_rev() -> DdiApiRev {
    DdiApiRev { major: 1, minor: 0 }
}

fn app_id() -> AppId {
    [1u8; 16]
}

fn begin_ecc_sign(curve: EccCurve) -> HsmResult<EccSign<MockEnv>> {
    let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
    let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
    let engine = resource.acquire(TagId::default(), None);
    let pka_curve: PkaEccCurve = curve.into();
    Ok(EccSign {
        tag: TagId::default(),
        engine_ref: engine.unwrap(),
        curve,
        cmd_info: PkaEccCmd { curve: pka_curve },
    })
}

fn cmd_req() -> DdiAttestKeyCmdReq {
    DdiAttestKeyCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::AttestKey,
            sess_id: Some(SessionId::default()),
        },
        data: DdiAttestKeyReq {
            key_id: 1,
            report_data: MborByteArray::new_with_len([2u8; 128].as_ptr(), 128),
        },
    }
}
