// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::TagId;
use crate::fsm::ComboFsm;
use crate::fsm::HsmFsmEventRecorder;
use crate::partition::CertSignContext;
use crate::partition::GetCertChainLengthsInfo;
use crate::partition::GetCertLengthsContext;
use crate::partition::MAX_CERTS;
use crate::resource::HspIpcChannelResource;
use crate::x509::AzihsmLeafCertTbs;
use crate::x509::Ecdsa384Signature;
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
    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();

    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);
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

    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);
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

    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, cert_len_ctx| {
            cert_len_ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 3,
                cert_lengths: [1024; MAX_CERTS],
            });

            Ok(())
        });

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::DmaAllocFailure)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_invalid_slot_id() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();

    let cmd_req = DdiGetCertChainInfoCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetCertChainInfo,
            sess_id: None,
        },
        data: DdiGetCertChainInfoReq { slot_id: 1 },
    };

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req, &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::InvalidArgument)
    );
}

#[test]
fn test_requires_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let part = MockPartition::new();
    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.requires_resource(TagId::default(), HsmFsmResourceId::HspIpcChannel));
}

#[test]
fn test_acquire_hsp_ipc_resource() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    let part = MockPartition::new();
    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::HspIpcChannel)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel)
    );
}

#[test]
fn test_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, _| Ok(()));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::InvalidState));
}

#[test]
fn test_get_cert_sizes_cached() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, cert_len_ctx| {
            cert_len_ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 3,
                cert_lengths: [1024; MAX_CERTS],
            });

            Ok(())
        });

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Ok(()));

    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetCertChainInfoCmdResp, MockEnv>(&resp.unwrap()).unwrap();

    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetCertChainInfo);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.thumbprint.as_slice(), &[0; 32]);
    assert_eq!(resp.data.num_certs, 3);
}

#[test]
fn test_get_cert_sizes() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| true);
    part.expect_update_cert_chain_lengths_info()
        .times(1)
        .returning(move |cert_info| {
            cert_info.hash = [1; 32];

            Ok(())
        });

    part.expect_set_cert_chain_lengths_info()
        .times(1)
        .returning(move |_| ());

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert_eq!(
        cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()),
        Err(HsmErr::Pending)
    );
    assert_eq!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()),
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetCertChainInfoCmdResp, MockEnv>(&resp.unwrap()).unwrap();

    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetCertChainInfo);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.thumbprint.as_slice(), &[1; 32]);
    assert_eq!(resp.data.num_certs, 5);
}

#[test]
fn test_get_cert_sizes_ipc_send_failure() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();

    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, _| Err(HsmErr::IpcSendFailure));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::IpcSendFailure));
}

#[test]
fn test_get_cert_sizes_with_resource_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);

    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));
    part.expect_begin_get_dev_id_cert_chain_info()
        .once()
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });

    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 6,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });

    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| true);
    part.expect_update_cert_chain_lengths_info()
        .times(1)
        .returning(move |cert_info| {
            cert_info.hash = [1; 32];

            Ok(())
        });

    part.expect_set_cert_chain_lengths_info()
        .times(1)
        .returning(move |_| ());

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            TagId::default()
        ) == Err(HsmErr::Pending)
    );
    assert!(cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Ok(()));

    let resp = cmd.take_response();
    assert!(resp.is_some());

    let resp = decode_buf::<DdiGetCertChainInfoCmdResp, MockEnv>(&resp.unwrap()).unwrap();

    assert_eq!(resp.hdr.rev.unwrap(), DdiApiRev { major: 1, minor: 0 });
    assert_eq!(resp.hdr.op, DdiOp::GetCertChainInfo);
    assert_eq!(resp.hdr.sess_id, None);
    assert_eq!(resp.hdr.status, DdiStatus::Success);
    assert_eq!(resp.data.thumbprint.as_slice(), &[1; 32]);
    assert_eq!(resp.data.num_certs, 6);
}

#[test]
fn test_invalid_state_for_cert_sizes() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::HspIpcChannel),
            TagId::default()
        ) == Err(HsmErr::InvalidState)
    );
    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_cert_chain_info_generate_partition_id_cert() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(|tag, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();

            let signature_buf = MockDmaAlloc::new(96);
            let tbs_digest_buf = MockDmaAlloc::new(48);
            Ok(CertSignContext {
                tbs: AzihsmLeafCertTbs::default(),
                _tbs_digest_buf: tbs_digest_buf,
                signature_buf,
                engine_ref: engine,
            })
        });

    part.expect_get_ecdsa384_signature_from_buffer()
        .once()
        .returning(|_| {
            Ok(Ecdsa384Signature {
                r: [0; 48],
                s: [0; 48],
            })
        });

    part.expect_end_generate_pid_cert()
        .times(1)
        .returning(move |_, _| Ok(()));

    part.expect_set_partition_cert_length()
        .once()
        .returning(|_| Ok(()));

    part.expect_partition_cert().once().returning(|| {
        let slice = [0u8; 800];
        IoMemRange::from(&slice[..])
    });

    part.expect_set_partition_cert_valid()
        .once()
        .returning(|_| ());

    part.expect_update_cert_chain_lengths_info()
        .times(1)
        .returning(move |cert_info| {
            cert_info.hash = [1; 32];

            Ok(())
        });

    part.expect_set_cert_chain_lengths_info()
        .times(1)
        .returning(move |_| ());

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

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
        Ok(())
    );

    let resp = cmd.take_response();
    assert!(resp.is_some());
}

#[test]
fn test_get_cert_chain_info_generate_partition_id_cert_resource_pending() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(2)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_is_fips_approved().times(1).returning(|| false);
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(|_, _| Err(HsmErr::Pending));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(|tag, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();

            let signature_buf = MockDmaAlloc::new(96);
            let tbs_digest_buf = MockDmaAlloc::new(48);
            Ok(CertSignContext {
                tbs: AzihsmLeafCertTbs::default(),
                _tbs_digest_buf: tbs_digest_buf,
                signature_buf,
                engine_ref: engine,
            })
        });

    part.expect_end_generate_pid_cert()
        .times(1)
        .returning(move |_, _| Ok(()));

    part.expect_set_partition_cert_length()
        .once()
        .returning(|_| Ok(()));

    part.expect_get_ecdsa384_signature_from_buffer()
        .once()
        .returning(|_| {
            Ok(Ecdsa384Signature {
                r: [0; 48],
                s: [0; 48],
            })
        });

    part.expect_partition_cert().once().returning(|| {
        let slice = [0u8; 800];
        IoMemRange::from(&slice[..])
    });

    part.expect_set_partition_cert_valid()
        .once()
        .returning(|_| ());

    part.expect_update_cert_chain_lengths_info()
        .times(1)
        .returning(move |cert_info| {
            cert_info.hash = [1; 32];

            Ok(())
        });

    part.expect_set_cert_chain_lengths_info()
        .times(1)
        .returning(move |_| ());

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Err(HsmErr::Pending)
    );
    assert!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ) == Err(HsmErr::Pending)
    );
    assert!(cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()) == Ok(()));

    let resp = cmd.take_response();
    assert!(resp.is_some());
}

#[test]
fn test_get_cert_chain_info_generate_partition_id_cert_invalid_state() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(2)
        .returning(|_, _| Err(HsmErr::Pending));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Err(HsmErr::Pending)
    );
    assert!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ) == Err(HsmErr::InvalidState)
    );

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_cert_chain_info_begin_cert_generation_failed() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(|_, _| Err(HsmErr::PartitionCertInvalidTypeConversion));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default())
            == Err(HsmErr::PartitionCertInvalidTypeConversion)
    );

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_cert_chain_info_cert_ecc_sign_failed() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(move |tag, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();

            let signature_buf = MockDmaAlloc::new(96);
            let tbs_digest_buf = MockDmaAlloc::new(48);
            Ok(CertSignContext {
                tbs: AzihsmLeafCertTbs::default(),
                _tbs_digest_buf: tbs_digest_buf,
                signature_buf,
                engine_ref: engine,
            })
        });

    part.expect_end_generate_pid_cert()
        .times(1)
        .returning(move |_, _| Err(HsmErr::EccSignFailed));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Err(HsmErr::Pending)
    );
    assert!(cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()) == Err(HsmErr::EccSignFailed));

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_cert_chain_info_cert_build_failed() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_get_ecdsa384_signature_from_buffer()
        .once()
        .returning(|_| {
            Ok(Ecdsa384Signature {
                r: [0; 48],
                s: [0; 48],
            })
        });

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(|tag, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();

            let signature_buf = MockDmaAlloc::new(96);
            let tbs_digest_buf = MockDmaAlloc::new(48);
            Ok(CertSignContext {
                tbs: AzihsmLeafCertTbs::default(),
                _tbs_digest_buf: tbs_digest_buf,
                signature_buf,
                engine_ref: engine,
            })
        });

    part.expect_end_generate_pid_cert()
        .times(1)
        .returning(move |_, _| Ok(()));

    part.expect_set_partition_cert_length()
        .once()
        .returning(|_| Ok(()));

    part.expect_partition_cert().once().returning(|| {
        let slice = [0u8; 10];
        IoMemRange::from(&slice[..])
    });

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Err(HsmErr::Pending)
    );
    assert!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default())
            == Err(HsmErr::PartitionCertGenerationFailed)
    );

    assert!(cmd.take_response().is_none());
}

#[test]
fn test_get_cert_chain_info_cert_size_too_large() {
    let mut heap = MockDmaHeap::new();
    heap.expect_allocate_from_pool()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));
    heap.expect_allocate()
        .times(1)
        .returning(|s| Some(MockDmaAlloc::new(s)));

    let mut part = MockPartition::new();
    part.expect_begin_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |_, cert_len_ctx| {
            cert_len_ctx.cert_info = None;
            cert_len_ctx.channel_ref = acquire_ipc_channnel();

            Ok(())
        });
    part.expect_end_get_dev_id_cert_chain_info()
        .times(1)
        .returning(move |ctx: &mut GetCertLengthsContext<MockEnv>| {
            ctx.cert_info = Some(GetCertChainLengthsInfo {
                hash: [0; 32],
                num_certs: 5,
                cert_lengths: [1024; MAX_CERTS],
            });
            Ok(())
        });
    part.expect_is_partition_cert_valid()
        .times(1)
        .returning(|| false);

    part.expect_get_raw_alias_key()
        .once()
        .returning(move || Ok(vec![1; 48].into()));

    part.expect_begin_generate_pid_cert()
        .times(1)
        .returning(move |tag, _| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let mock_pka = MockPka::new();
            let resource = CmdResource::new(PkaResource::new(vec![mock_pka]), scheduler, 1);

            let engine = resource.acquire(tag, None).unwrap();

            let signature_buf = MockDmaAlloc::new(96);
            let tbs_digest_buf = MockDmaAlloc::new(48);
            Ok(CertSignContext {
                tbs: AzihsmLeafCertTbs::default(),
                _tbs_digest_buf: tbs_digest_buf,
                signature_buf,
                engine_ref: engine,
            })
        });

    part.expect_end_generate_pid_cert()
        .once()
        .returning(move |_, _| Err(HsmErr::PartitionCertTooLarge));

    let req = encode_buf::<DdiGetCertChainInfoCmdReq, _>(&cmd_req(), &heap).unwrap();
    let mut cmd = GetCertChainInfoCmd::<MockEnv>::new(req, heap, part);

    assert!(cmd.on_event(HsmFsmEvent::StartCmd, TagId::default()) == Err(HsmErr::Pending));
    assert!(
        cmd.on_event(HsmFsmEvent::HspToHsmIpcResponse, TagId::default()) == Err(HsmErr::Pending)
    );
    assert!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default())
            == Err(HsmErr::PartitionCertTooLarge)
    );

    assert!(cmd.take_response().is_none());
}

fn cmd_req() -> DdiGetCertChainInfoCmdReq {
    DdiGetCertChainInfoCmdReq {
        hdr: DdiReqHdr {
            rev: Some(DdiApiRev { major: 1, minor: 0 }),
            op: DdiOp::GetCertChainInfo,
            sess_id: None,
        },
        data: DdiGetCertChainInfoReq { slot_id: 0 },
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
