// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;
use crate::cmd_scheduler::*;
use crate::partition::pct_engine::PctEngine;
use crate::partition::pct_engine_impl::PctEngineImpl;
use crate::partition::GetPartitionIdCtx;
use crate::partition::PartitionIdGenResult;
use core::cell::RefCell;
use std::rc::Rc;

use crate::fsm::part_init::HsmPartInitFsm;
use crate::resource::HsmFsmResourceId;
use mcr_ipc_message::IpcMessageSetRes;
use mcr_ipc_message::SetResInfo;

#[test]
fn test_invalid_event() {
    let part = MockPartition::new();
    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(MockEnv::new())), part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::Unknown, TagId::default()),
        Err(HsmErr::InvalidEvent)
    );
}

#[test]
fn test_part_init_check_alive() {
    let part = MockPartition::new();
    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(MockEnv::new())), part);
    assert_eq!(
        cmd.on_event(HsmFsmEvent::CheckAlive, TagId::default()),
        Err(HsmErr::Pending)
    );
}

#[test]
fn test_part_init_id_present() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());
    part.expect_begin_generate_partition_identifiers()
        .once()
        .returning(|tag| {
            Ok(GetPartitionIdCtx {
                tag,
                cmd_info: None,
                identifiers_present: true,
                engine: None,
            })
        });

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
        Ok(())
    );
}

#[test]
fn test_part_init_acquire_resources() {
    let part = MockPartition::new();
    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(MockEnv::new())), part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::Pka)
            == HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka)
    );
}

#[test]
fn test_part_init_acquire_resources_failure() {
    let part = MockPartition::new();
    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(MockEnv::new())), part);

    assert!(
        cmd.acquire_resource(TagId::default(), HsmFsmResourceId::HsmToAdminIpcChannel)
            == HsmFsmEvent::Unknown
    );
}

#[test]
fn test_part_init_id_not_present_waiting_for_engine_on_pid_key() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());
    part.expect_begin_generate_partition_identifiers()
        .once()
        .returning(|_| Err(HsmErr::Pending));
    part.expect_begin_generate_partition_identifiers()
        .once()
        .returning(|tag| {
            Ok(GetPartitionIdCtx {
                tag,
                cmd_info: None,
                identifiers_present: false,
                engine: None,
            })
        });

    part.expect_continue_generate_partition_identifiers()
        .once()
        .returning(|_, _| Ok(dummy_km_ecc384()));

    part.expect_end_generate_partition_identifiers()
        .once()
        .returning(|_| Ok(()));

    set_ecc_pct_key_raw_agreement_expectations(&mut part);

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
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

    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
}

// Tiny helper to build staged key material for ECC-384
fn dummy_km_ecc384() -> PartitionIdGenResult {
    // 384-bit ECC: d = 48 bytes, X||Y = 96 bytes
    PartitionIdGenResult {
        curve: PkaEccCurve::Ecc384,
        priv_d: [0x11; 48].to_vec().into(),
        pub_xy: [0x22; 96].to_vec().into(),
    }
}

fn set_ecc_pct_key_raw_agreement_expectations(part: &mut MockPartition) {
    part.expect_begin_ecc_pct_validation_raw().once().returning(
        move |_tag, _usage, _pubk, _privd| {
            let scheduler = CmdScheduler::new(128, 1, HsmFsmEventRecorder::default());
            let resource = CmdResource::new(PkaResource::new(vec![MockPka::new()]), scheduler, 1);
            let engine = resource.acquire(TagId::default(), None);

            let key_agreement_buffer_size =
                (PkaEccCurve::MAX_LEN * 2) + PkaEccCurve::MAX_LEN + (PkaEccCurve::MAX_LEN * 2);
            let op_dma_buf = MockDmaAlloc::new(key_agreement_buffer_size);

            let priv_key_blob = &[0u8; 96];
            let pub_key_blob = &[0u8; 136];

            let priv_key_size = PkaEccPrivateKey::data_len(PkaEccCurve::Ecc384);
            let pub_key_data_len = PkaEccCurve::MAX_LEN * 2;

            let mut key_blob_dma_buf = MockDmaAlloc::new(pub_key_data_len + priv_key_size);

            key_blob_dma_buf.as_ref_mut()[pub_key_data_len..pub_key_data_len + priv_key_size]
                .copy_from_slice(&priv_key_blob[..priv_key_size]);
            let priv_key_blob = IoMemRange::from(
                &key_blob_dma_buf.as_ref()[pub_key_data_len..pub_key_data_len + priv_key_size],
            );

            key_blob_dma_buf.as_ref_mut()[..pub_key_data_len].copy_from_slice(pub_key_blob);
            let pub_key_blob = IoMemRange::from(&key_blob_dma_buf.as_ref()[..pub_key_data_len]);

            let sha = MockSha::new();
            let engine: Box<dyn PctEngine> =
                Box::new(PctEngineImpl::<MockEnv>::new(engine.unwrap(), sha));

            let ecc_key_pct = EccKeyPct::new(
                priv_key_blob,
                pub_key_blob,
                key_blob_dma_buf,
                PkaEccCurve::Ecc384,
                op_dma_buf,
                engine,
            );

            Ok(ecc_key_pct)
        },
    );
    part.expect_continue_ecc_pct_validation()
        .times(3)
        .returning(move |_tag, _op| Ok(()));
    part.expect_end_ecc_pct_validation()
        .once()
        .returning(|_tag, _op| Ok(true));
    part.expect_is_pct_final_state()
        .times(3)
        .returning(|_| false);
    part.expect_is_pct_final_state().once().returning(|_| true);
}

#[test]
fn test_part_init_success() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());

    part.expect_begin_generate_partition_identifiers()
        .once()
        .returning(|tag| {
            Ok(GetPartitionIdCtx {
                tag,
                cmd_info: None,
                identifiers_present: false,
                engine: None,
            })
        });

    part.expect_continue_generate_partition_identifiers()
        .once()
        .returning(|_, _| Ok(dummy_km_ecc384()));

    part.expect_end_generate_partition_identifiers()
        .once()
        .returning(|_| Ok(()));

    part.expect_begin_ecc_pct_validation_raw()
        .once()
        .returning(|_tag, _usage, _pubk, _privd| Err(HsmErr::Pending));
    set_ecc_pct_key_raw_agreement_expectations(&mut part);

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
        Err(HsmErr::Pending)
    );

    // complete and PCT Validation starts
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Got resource for PCT Validation
    assert_eq!(
        cmd.on_event(
            HsmFsmEvent::ResourceReady(HsmFsmResourceId::Pka),
            TagId::default()
        ),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (Init -> EcdhMontgomeryConstCalculationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhMontgomeryConstCalculationFirst -> EcdhPointMultiplicationFirst)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Continue PCT Validation (EcdhPointMultiplicationFirst -> EcdhMontgomeryConstCalculationSecond)
    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::Pending)
    );
    // Finish PCT validation
    assert!(cmd
        .on_event(HsmFsmEvent::PkaDone(0), TagId::default())
        .is_ok());
}

#[test]
fn test_part_init_begin_id_generation_pka_pending_twice() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());

    part.expect_begin_generate_partition_identifiers()
        .times(2)
        .returning(|_| Err(HsmErr::Pending));

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
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
fn test_part_init_begin_id_generation_error() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());

    part.expect_begin_generate_partition_identifiers()
        .times(1)
        .returning(|_| Err(HsmErr::EccGenKeyFailed));

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
        Err(HsmErr::PartitionIdGenerationFailed)
    );
}

#[test]
fn test_part_init_part_id_gen_end_failure() {
    let mut part = MockPartition::new();
    part.expect_set_resource_mask().once().returning(|_| ());
    part.expect_set_vm_launch_guid().once().returning(|_| ());

    part.expect_begin_generate_partition_identifiers()
        .once()
        .returning(|tag| {
            Ok(GetPartitionIdCtx {
                tag,
                cmd_info: None,
                identifiers_present: false,
                engine: None,
            })
        });

    part.expect_continue_generate_partition_identifiers()
        .once()
        .returning(|_, _| Err(HsmErr::EccGenKeyFailed));

    let mut env = MockEnv::new();
    let mut hal = MockHal::new();

    let mut ipc_channel = MockIpcMessageChannel::new();
    ipc_channel
        .expect_send_response()
        .once()
        .return_const(Ok(()));

    hal.expect_admin_ipc_channel()
        .times(..)
        .return_const(ipc_channel);

    env.expect_hal().times(1).return_const(hal);

    let mut cmd = HsmPartInitFsm::new(Rc::new(RefCell::new(env)), part);
    let message = prepare_ipc_message();
    assert_eq!(
        cmd.on_event(HsmFsmEvent::InitPartition(message), TagId::default()),
        Err(HsmErr::Pending)
    );

    assert_eq!(
        cmd.on_event(HsmFsmEvent::PkaDone(0), TagId::default()),
        Err(HsmErr::EccGenKeyFailed)
    );
}

fn prepare_ipc_message() -> IpcMessageSetRes {
    IpcMessageSetRes {
        info: SetResInfo {
            mask: Default::default(),
            pfn: PcieFunction::Pf,
            vm_launch_guid: [0u8; 16],
        },
        ..Default::default()
    }
}
