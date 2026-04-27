// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use super::*;

use crate::cmd_scheduler::TagId;
use crate::mock::*;
use crate::partition::tests::cmd_scheduler;
use crate::partition::tests::set_fp_ipc_expectations;
use crate::partition::tests::set_hsm_to_admin_ipc_expectations;
use crate::partition::tests::set_hsp_ipc_send_failed_expectations;
use crate::partition::tests::set_hsp_ipc_send_recv_expectations_with_recv_data;
use crate::partition::GetCertChainLengthsInfo;
use crate::partition::GetCertContext;
use crate::partition::GetCertLengthsContext;
use crate::partition::HsmPartition;
use crate::partition::PartEnv;
use crate::partition::MAX_CERTS;
use crate::HsmErr;

#[test]
#[allow(clippy::unusual_byte_groupings)]
pub fn test_get_cert_chain_lengths() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_alias_cert_len().return_const(512usize);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [
        0x03B000045,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x35_1234_05,
        0x37_1236_12,
        0x00_1238_12,
    ];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut context = GetCertLengthsContext::<MockEnv> {
        cert_info: None,
        channel_ref: None,
    };
    let result = part.begin_get_dev_id_cert_chain_info(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_dev_id_cert_chain_info(&mut context);
    assert_eq!(result, Ok(()));
    assert!(context.cert_info.is_some());
    let cert_info = context.cert_info.unwrap();
    assert_eq!(cert_info.num_certs, 7);
    for i in 0..5 {
        assert_eq!(cert_info.cert_lengths[i], (0x1234 + i) as u16);
    }
}

#[test]
fn test_get_cert_lengths_failed_ipc() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    set_hsp_ipc_send_failed_expectations(&mut hal);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut context = GetCertLengthsContext::<MockEnv> {
        cert_info: None,
        channel_ref: None,
    };
    let result = part.begin_get_dev_id_cert_chain_info(TagId::default(), &mut context);
    assert_eq!(result, Err(HsmErr::IpcSendFailure));

    let result = part.end_get_dev_id_cert_chain_info(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
fn test_get_cert_lengths_failed_response() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [0x03B010045];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut context = GetCertLengthsContext::<MockEnv> {
        cert_info: None,
        channel_ref: None,
    };
    let result = part.begin_get_dev_id_cert_chain_info(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_dev_id_cert_chain_info(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert_lengths_invalid_num_certs() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_alias_cert_len().return_const(512usize);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [
        0x03B000045,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x00000000,
        0x35_1234_1E,
        0x37_1236_12,
        0x00_1238_12,
    ];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut context = GetCertLengthsContext::<MockEnv> {
        cert_info: None,
        channel_ref: None,
    };
    let result = part.begin_get_dev_id_cert_chain_info(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());
    let result = part.end_get_dev_id_cert_chain_info(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_alias_cert_len().times(1).return_const(2048usize);
    hal.expect_alias_cert()
        .times(1)
        .return_const([100; 2048].into());

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;
    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 1,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_none());

    assert_eq!(context.cert_id, 0x1);
    assert_eq!(context.cert_len, Some(info.cert_lengths[1]));
    assert!(context.cert_buf == Some(buff.as_slice().into()));
}

#[test]
fn test_get_cert_with_failed_response() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);
    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [0x07010046];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;

    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 0,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_cert(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert_mismatched_cert_id() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [0x07000046, 0xCD_0800_00, 0x00_5678AB];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;
    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 0,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_cert(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert_mismatched_cert_len() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [0x07000046, 0xCD_0801_01, 0x00_5678AB];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;
    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 0,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_cert(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert_mismatched_cert_buf() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);
    let ipc_recv_data = [0x07000046, 0xCF_0800_01, 0x00_5678AB];
    set_hsp_ipc_send_recv_expectations_with_recv_data(&mut hal, &ipc_recv_data);

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;
    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 0,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Ok(()));
    assert!(context.channel_ref.is_some());

    let result = part.end_get_cert(&mut context);
    assert_eq!(result, Err(HsmErr::IpcResponseError));
}

#[test]
#[allow(clippy::unusual_byte_groupings)]
fn test_get_cert_unexpected_cert_id() {
    let mut pka = MockPka::new();
    pka.expect_clone().once().returning(MockPka::new);

    let mut hal = MockHal::new();
    let mut rng_nonce = MockRng::new();
    rng_nonce.expect_bytes().once().returning(|buf| {
        let nonce_to_return: [u8; 32] = (1..33u8).collect::<Vec<_>>().try_into().unwrap();
        buf.copy_from_slice(&nonce_to_return);
    });

    hal.expect_rng().once().return_const(rng_nonce);
    let part_persistent_store_memory = [0u8; 2048 * 65];
    hal.expect_part_persistent_store_addr()
        .return_const(part_persistent_store_memory.as_ptr() as usize);

    hal.expect_pka().once().return_const(vec![pka]);

    set_fp_ipc_expectations(&mut hal);
    set_hsm_to_admin_ipc_expectations(&mut hal);

    let mut mock_ipc_message_channel = MockIpcMessageChannel::new();

    mock_ipc_message_channel
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    hal.expect_hsp_ipc_channel()
        .once()
        .return_const(mock_ipc_message_channel);

    hal.expect_clone().once().returning(move || {
        let mut hal = MockHal::new();

        let part_persistent_store_memory = [0u8; 2048 * 65];
        hal.expect_part_persistent_store_addr()
            .return_const(part_persistent_store_memory.as_ptr() as usize);

        hal
    });

    let part = Partition::<MockEnv>::new(
        PcieFunction(0),
        PartEnv::<MockEnv>::new(hal, cmd_scheduler()),
    );

    let mut info = GetCertChainLengthsInfo {
        hash: [0; 32],
        num_certs: 3,
        cert_lengths: [0; MAX_CERTS],
    };
    info.cert_lengths[0] = 1024;
    info.cert_lengths[1] = 2048;
    info.cert_lengths[2] = 1024;
    part.state.set_cert_chain_lengths_info(Some(info));

    let buff = [100; 2048];
    let mut context = GetCertContext::<MockEnv> {
        cert_id: 3,
        cert_len: Some(info.cert_lengths[1]),
        cert_buf: Some(buff.as_slice().into()),
        channel_ref: None,
    };
    let result = part.begin_get_cert(TagId::default(), &mut context);
    assert_eq!(result, Err(HsmErr::InvalidCertificate));
}
