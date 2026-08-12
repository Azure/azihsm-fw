// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield::BitMut;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::AES_GCM_256_AAD_ALIGNED_DATA_ONLY_TEST_VECTORS;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::AES_GCM_256_TEST_VECTORS;
use mcr_crypto_cdma_io::aes_fp_self_test_constants::AES_XTS_256_TEST_VECTORS;
use mcr_crypto_cdma_io::AesFpCipher;
use mcr_crypto_cdma_io::AesFpOp;
use mcr_crypto_softaes::*;
use mcr_gdma_controller::*;
use mcr_io_controller::*;
use mcr_ipc_controller::*;
use mcr_pcie_controller::PcieLinkStatus;
use mcr_queue_controller::QueueCntrlId;
use mcr_self_test::SelfTestRespPacket;
use mcr_types::*;
use zerocopy::FromBytes;
use zerocopy::FromZeros;
use zerocopy::IntoBytes;

use super::aes_gcm_ext_tests::AesGcmExtTestConfigs;
use super::cast_tests::AdminFsmSelfTestTestConfigs;
use super::cntrl_tests::ControllerFsmTestConfigs;
use super::create_cq_tests::AdminFsmCreateCqTestConfigs;
use super::create_sq_tests::AdminFsmCreateSqTestConfigs;
use super::delete_sq_tests::AdminFsmDeleteSqTestConfigs;
use super::doe_tests::AdminFsmDoeTestConfigs;
use super::harness::*;
use super::identify_tests::AdminFsmIdentifyTestConfigs;
use super::pcie_tests::PcieFsmTestConfigs;
use super::set_res_tests::AdminFsmSetResTestConfigs;
use super::telemetry_tests::TelemetryFsmTestConfigs;
use super::vflr_tests::AdminFsmVFlrTestConfigs;
use crate::cmd_scheduler::CmdScheduler;
use crate::context::AdminFsmContext;
use crate::error::AdminErr;
use crate::fsm::tests::cntrl_tests::ControllerAction;
use crate::fsm::tests::stop_interface_tests::AdminFsmStopInterfaceTestConfigs;
use crate::fsm::tests::tdisp_int_tests::AdminFsmTdispIntTestConfigs;
use crate::fsm::tests::vf_prep_tests::VfPrepareCmdTestConfigs;
use crate::fsm::tests::vf_restore_tests::VfRestoreCmdTestConfigs;
use crate::fsm::tests::vf_save_tests::VfSaveCmdTestConfigs;
use crate::fsm::tests::vf_start_tests::VfStartCmdTestConfigs;
use crate::fsm::tests::vf_stop_tests::VfStopCmdTestConfigs;
use crate::fsm::types::AdminSqe;
use crate::fsm::types::*;
use crate::fsm::PcieResetReason;
use crate::function::AdminQueue;
use crate::function::CntrlStateChangeAction;
use crate::mock::*;
use crate::preop_cdma_io::CdmaKeyIndex;
use crate::recorder::AdminFsmEventRecorder;

/// Set default expectations on the resources held in handler
pub(crate) fn default_resource_expectations(test: &mut AdminFsmTest) {
    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
    test.hsm_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
}

pub(crate) fn get_context_from_environment(
    env: MockAdminEnvTrait,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let scheduler = CmdScheduler::new(65 + 5, 1, AdminFsmEventRecorder::default());

    AdminFsmContext::new(env, scheduler)
}

pub(crate) fn get_context(mut test: AdminFsmTest) -> AdminFsmContext<MockAdminEnvTrait> {
    let env = test.env();
    get_context_from_environment(env)
}

/// Create a mock admin PCIe FSM expectations set to handle PERST UP and PCIE PF FLR.
pub(crate) fn make_pcie_fsm(config: PcieFsmTestConfigs) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    default_resource_expectations(&mut test);

    test.pcie_cntrl()
        .expect_link_status()
        .times(1)
        .returning(move || {
            if config.warm_reset {
                Ok(mcr_pcie_controller::PcieLinkStatus { speed: 5, width: 4 })
            } else {
                Err(u32::MAX)
            }
        });

    if !config.warm_reset && !config.unknown_event {
        test.pcie_cntrl()
            .expect_perst_up()
            .times(1)
            .returning(|| Ok(()));
    }

    if config.reset && !config.unknown_event {
        test.hsm_ipc_event_channel()
            .expect_begin_event()
            .times(config.ipc_call_count)
            .returning(move |_, _, _| {
                if config.send_hsm_event_succeed {
                    Ok(())
                } else {
                    Err(0)
                }
            });

        if config.send_hsm_event_succeed {
            test.hsm_ipc_event_channel()
                .expect_receive_event()
                .times(config.ipc_call_count)
                .returning(|_| Some(PcieResetReason::Flr as u32));

            test.fp_ipc_event_channel()
                .expect_begin_event()
                .times(config.ipc_call_count)
                .returning(move |_, _, _| {
                    if config.send_fp_event_succeed {
                        Ok(())
                    } else {
                        Err(0)
                    }
                });

            if config.send_fp_event_succeed {
                test.fp_ipc_event_channel()
                    .expect_receive_event()
                    .times(config.ipc_call_count)
                    .returning(|_| Some(PcieResetReason::Flr as u32));
            }
        }

        test.function_mgr().expect_reset().times(1).returning(|| ());
        if config.flr {
            if config.perst_down_while_flr_pending {
                test.pcie_cntrl()
                    .expect_perst_down()
                    .times(1)
                    .returning(|| ());
            } else {
                test.pcie_cntrl()
                    .expect_complete_flr()
                    .times(1)
                    .return_const(());
            }
        } else {
            test.pcie_cntrl()
                .expect_perst_down()
                .times(1)
                .returning(|| ());
        }
    }

    get_context(test)
}

/// Create a mock admin Telemetry FSM expectations set to handle PERST UP and Timer event.
pub(crate) fn make_telemetry_fsm(
    config: TelemetryFsmTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    default_resource_expectations(&mut test);

    test.pcie_cntrl()
        .expect_link_status()
        .times(1)
        .returning(move || {
            if config.link_status_unexpected {
                Ok(mcr_pcie_controller::PcieLinkStatus { speed: 4, width: 4 })
            } else if config.warm_reset {
                Ok(mcr_pcie_controller::PcieLinkStatus { speed: 5, width: 4 })
            } else {
                Err(u32::MAX)
            }
        });

    if !config.unknown_event {
        if config.timer_event {
            test.pcie_cntrl()
                .expect_link_status()
                .times(1)
                .returning(move || {
                    if config.link_status_unexpected || config.abnormal_to_abnormal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 3, width: 4 })
                    } else if config.warm_reset
                        & (!config.faulted_to_normal & !config.faulted_to_abnormal)
                    {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 5, width: 4 })
                    } else {
                        Err(u32::MAX)
                    }
                });
        }
        if config.test_service_fsm {
            test.pcie_cntrl()
                .expect_link_status()
                .times(1)
                .returning(move || {
                    if config.normal_to_abnormal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 4, width: 4 })
                    } else if config.abnormal_to_normal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 5, width: 4 })
                    } else if config.abnormal_to_abnormal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 2, width: 4 })
                    } else if config.faulted_to_normal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 5, width: 4 })
                    } else if config.faulted_to_abnormal {
                        Ok(mcr_pcie_controller::PcieLinkStatus { speed: 4, width: 4 })
                    } else {
                        Err(u32::MAX)
                    }
                });
        }
    }

    get_context(test)
}

/// Create a mock admin FSM with DOE FSM expectations set.
pub(crate) fn make_doe_fsm(config: AdminFsmDoeTestConfigs) -> AdminFsmContext<MockAdminEnvTrait> {
    let test = make_doe_fsm_test(config);

    get_context(test)
}

pub(crate) fn make_doe_fsm_test(config: AdminFsmDoeTestConfigs) -> AdminFsmTest {
    let mut test = AdminFsmTest::default();

    let error_state = config.doe_error
        || config.rx_ready_err
        || config.doe_go_recv_err
        || config.doe_go_end_recv_err
        || config.hsp_response_send_err
        || config.hsp_response_spurious_msg
        || config.hsp_response_header_err
        || config.tx_ready_err
        || config.send_request_err;

    let doe_active = config.rx_ready;

    if config.link_up {
        test.pcie_cntrl()
            .expect_link_status()
            .once()
            .return_const(Ok(PcieLinkStatus::default()));
        test.pcie_doe().expect_set_busy().once().return_const(());
    } else {
        test.pcie_cntrl()
            .expect_link_status()
            .once()
            .return_const(Err(u32::MAX));
        if config.perst_up {
            test.pcie_doe().expect_set_busy().once().return_const(());
        }
    }

    if config.wait_doe_idle {
        test.pcie_doe().expect_set_busy().once().return_const(());
    }

    if config.doe_abort {
        test.pcie_doe().expect_set_busy().once().return_const(());

        if doe_active {
            test.pcie_doe().expect_abort().once().return_const(());
            test.pcie_doe().expect_reset().once().return_const(());
        }
    }

    if config.perst_down && doe_active {
        test.pcie_doe().expect_abort().once().return_const(());
    }

    if error_state {
        test.pcie_doe().expect_set_err().once().return_const(());
    }

    if config.rx_ready {
        test.pcie_doe().expect_recv().once().returning(move || {
            if config.rx_ready_err {
                Err(u32::MAX)
            } else {
                Ok(())
            }
        });
    }

    if config.rx_ready_timeout {
        test.pcie_doe().expect_set_busy().once().return_const(());
    }

    if config.doe_go {
        test.pcie_doe().expect_recv().once().returning(move || {
            if config.doe_go_recv_err {
                Err(u32::MAX)
            } else {
                Ok(())
            }
        });

        if !config.doe_go_recv_err {
            test.pcie_doe().expect_end_recv().once().returning(move || {
                if config.doe_go_end_recv_err {
                    Err(u32::MAX)
                } else {
                    Ok(())
                }
            });
        }

        if !config.doe_go_recv_err && !config.doe_go_end_recv_err {
            test.pcie_doe()
                .expect_buffer_addr()
                .once()
                .return_const(MemoryAddr { lo: 0, hi: 0 });
        }
    }

    if config.hsp_response && !config.hsp_response_header_err {
        test.pcie_doe().expect_send().once().returning(move || {
            if config.hsp_response_send_err {
                Err(u32::MAX)
            } else {
                Ok(())
            }
        });
    }

    if config.tx_ready {
        test.pcie_doe().expect_send().once().returning(move || {
            if config.tx_ready_err {
                Err(u32::MAX)
            } else {
                Ok(())
            }
        });
    }

    if config.tx_done {
        test.pcie_doe().expect_end_send().once().return_const(());
        test.pcie_doe().expect_set_busy().once().return_const(());
    }

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    test.hsm_ipc_channel()
        .expect_clone()
        .once()
        .returning(MockIpcMessageChannel::new);

    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .once()
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if config.doe_go && !config.doe_go_recv_err && !config.doe_go_end_recv_err {
                channel.expect_send_request().once().returning(move |_, _| {
                    if config.send_request_err {
                        Err(u32::MAX)
                    } else {
                        Ok(())
                    }
                });
            }

            if config.hsp_response || config.hsp_response_after_abort {
                channel.expect_receive_message().once().returning(move || {
                    let mut msg = IpcMessage::new_zeroed();

                    if config.hsp_response_spurious_msg {
                        None
                    } else {
                        if config.hsp_response_header_err {
                            msg.data[0] = 0x0401FF40;
                        } else {
                            msg.data[0] = 0x0400FF40;
                        }

                        Some(msg)
                    }
                });
            }

            channel
        });

    test
}

/// Create a mock admin FSM with TDISP interrupts expectations set.
pub(crate) fn make_tdisp_interrupt_fsm(
    config: AdminFsmTdispIntTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let test = make_tdisp_interrupt_fsm_test(config);

    get_context(test)
}

pub(crate) fn make_tdisp_interrupt_fsm_test(config: AdminFsmTdispIntTestConfigs) -> AdminFsmTest {
    let mut test = AdminFsmTest::default();

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
    test.hsm_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if config.receive_tdisp_interrupt
                | config.receive_ide_interrupt
                | config.receive_flr_interrupt
                | config.receive_perst_down_interrupt
                | config.receive_perst_up_interrupt
            {
                channel.expect_send_request().once().returning(move |_, _| {
                    if config.err_when_sending_ipc {
                        Err(AdminErr::IpcSendRequestError.into())
                    } else {
                        Ok(())
                    }
                });
            }

            if config.receive_response {
                channel.expect_receive_message().once().returning(move || {
                    if config.err_when_receiving_response {
                        return None;
                    }

                    let mut data = [0; IPC_MESSAGE_LENGTH];

                    if config.err_when_invalid_header_response {
                        data[0] = 0x24000044;
                    } else {
                        data[0] = 0x24000049; // Header for IpcMessageStopInterface
                    }

                    if config.receive_tdisp_interrupt {
                        data[1] = 0; // Source mask for TDISP interrupt
                    } else if config.receive_ide_interrupt {
                        data[1] = 1; // Source mask for IDE interrupt
                    } else if config.receive_flr_interrupt {
                        data[1] = 2; // Source mask for FLR interrupt
                    } else if config.receive_perst_up_interrupt {
                        data[1] = 3; // Source mask for PERST_UP interrupt
                    } else if config.receive_perst_down_interrupt {
                        data[1] = 4; // Source mask for PERST_DOWN interrupt
                    }

                    if config.err_when_invalid_payload_response {
                        data[1] = 0xFF; // Invalid payload
                    }

                    Some(IpcMessage { data })
                });
            }

            if config.receive_response && config.another_interrupt_during_response {
                channel
                    .expect_send_request()
                    .once()
                    .returning(move |_, _| Ok(()));
            }

            channel
        });

    test
}

/// Create a mock admin FSM with Stop Interface FSM expectations set
pub(crate) fn make_stop_interface_fsm(
    config: AdminFsmStopInterfaceTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    let mut count_of_full_cycle = 0;
    if let (Some(fp_response), Some(hsm_response)) =
        (config.fp_response_count, config.hsm_response_count)
    {
        count_of_full_cycle = std::cmp::min(fp_response, hsm_response);
    }

    let pending_bits = (config.stop_interface_info.vf_mask as u128)
        | ((config.stop_interface_info.pf_mask as u128) << 64);

    let count_send_follow_up_request = std::cmp::min(
        count_of_full_cycle,
        pending_bits.count_ones().saturating_sub(1),
    );

    assert!(count_of_full_cycle <= pending_bits.count_ones());

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut ipc_channel = MockIpcMessageChannel::new();

            if config.receive_stop_interface_request {
                ipc_channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| {
                        if config.err_fp_request {
                            Err(AdminErr::IpcSendRequestError.into())
                        } else {
                            Ok(())
                        }
                    });
            }

            if let Some(count) = config.fp_response_count {
                ipc_channel
                    .expect_receive_message()
                    .times(count as usize)
                    .returning(move || {
                        if config.err_fp_response {
                            None
                        } else {
                            let mut data = [0; IPC_MESSAGE_LENGTH];
                            data[0] = 0x0200ff05;

                            Some(IpcMessage { data })
                        }
                    });
            }

            if count_of_full_cycle > 0 {
                ipc_channel
                    .expect_send_request()
                    .times((count_send_follow_up_request) as usize)
                    .returning(|_, _| Ok(()));
            }

            ipc_channel
        });

    test.hsm_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut ipc_channel = MockIpcMessageChannel::new();

            if config.receive_stop_interface_request {
                ipc_channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| {
                        if config.err_hsm_request {
                            Err(AdminErr::IpcSendRequestError.into())
                        } else {
                            Ok(())
                        }
                    });
            }

            if let Some(count) = config.hsm_response_count {
                ipc_channel
                    .expect_receive_message()
                    .times(count as usize)
                    .returning(move || {
                        if config.err_hsm_response {
                            None
                        } else {
                            let mut data = [0; IPC_MESSAGE_LENGTH];

                            if config.deferred_io {
                                data[0] = 0x0206ff05; // Status -> Pending
                            } else {
                                data[0] = 0x0200ff05;
                            }

                            Some(IpcMessage { data })
                        }
                    });
            }

            if count_of_full_cycle > 0 {
                ipc_channel
                    .expect_send_request()
                    .times((count_send_follow_up_request) as usize)
                    .returning(|_, _| Ok(()));
            }

            ipc_channel
        });

    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    if count_of_full_cycle > 0
        && count_of_full_cycle == pending_bits.count_ones()
        && (!config.deferred_io || config.deferred_io_completed)
    {
        test.hsp_to_admin_stop_interface_ipc_channel()
            .expect_send_response()
            .times(1)
            .returning(|_| Ok(()));
    }

    if config.deferred_io && config.deferred_io_completed {
        let mut bit = pending_bits;
        while bit > 0 {
            let id = bit.trailing_zeros();
            test.deferred_queue_delete_pipe()
                .expect_recv()
                .times(1)
                .returning(move || {
                    Some(QueueDeleteResponse {
                        tag: 0xff,
                        pfn: PcieFunction::try_from(id as u8).unwrap(),
                        _rsvd: 0,
                    })
                });
            bit.set_bit(id as usize, false);
        }
    }

    get_context(test)
}

/// Create a mock admin FSM with vFLR FSM expectations set.
pub(crate) fn make_vflr_fsm_test(config: AdminFsmVFlrTestConfigs) -> AdminFsmTest {
    let mut test = AdminFsmTest::default();

    if config.error {
        default_resource_expectations(&mut test);

        if !config.unknown_event {
            test.function_mgr().expect_function().returning(move |_| {
                let mut function = MockFunction::new();

                function
                    .expect_ready()
                    .times(config.call_count)
                    .returning(move || true);

                function
            });
        }
    } else {
        test.admin_to_fp_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();
                let mut ipc_message = IpcMessage { data: [0; 16] };
                ipc_message.data[0] |= config.fp_receive_message_status << 16u32;
                if config.ready && !config.unknown_event {
                    channel
                        .expect_send_request()
                        .times(config.call_count)
                        .returning(move |_, _| {
                            if config.fp_send_request_fail {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                    if !config.fp_send_request_fail && !config.hsm_send_request_fail {
                        channel
                            .expect_receive_message()
                            .times(config.call_count)
                            .returning(move || Some(ipc_message));
                    }
                }
                channel
            });

        test.hsm_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();
                let mut ipc_message = IpcMessage { data: [0; 16] };
                ipc_message.data[0] |= config.hsm_receive_message_status << 16u32;
                if !config.fp_send_request_fail && config.ready && !config.unknown_event {
                    channel
                        .expect_send_request()
                        .times(config.call_count)
                        .returning(move |_, _| {
                            if config.hsm_send_request_fail {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                }
                if !config.hsm_send_request_fail
                    && !config.fp_send_request_fail
                    && config.ready
                    && !config.unknown_event
                {
                    channel
                        .expect_receive_message()
                        .times(config.call_count)
                        .returning(move || Some(ipc_message));
                }
                channel
            });

        test.admin_to_hsp_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);

        let mut mock_repeat_cnt = config.function_count;
        loop {
            if !config.unknown_event {
                test.function_mgr()
                    .expect_function()
                    .once()
                    .returning(move |_| {
                        let mut function = MockFunction::new();

                        function
                            .expect_ready()
                            .times(1)
                            .return_once(move || config.ready);

                        function
                    });
            }
            if !config.fp_send_request_fail
                && !config.hsm_send_request_fail
                && config.ready
                && !config.unknown_event
            {
                test.function_mgr()
                    .expect_function()
                    .once()
                    .returning(move |_| {
                        let mut function = MockFunction::new();
                        function.expect_disable().return_const(());

                        function
                    });
            }
            if !config.fp_send_request_fail
                && !config.hsm_send_request_fail
                && !config.unknown_event
            {
                test.pcie_cntrl()
                    .expect_complete_flr()
                    .once()
                    .return_const(());
            }

            mock_repeat_cnt -= 1;
            if mock_repeat_cnt == 0 {
                break;
            }
        }
    }

    test
}

pub(crate) fn make_vflr_fsm(config: AdminFsmVFlrTestConfigs) -> AdminFsmContext<MockAdminEnvTrait> {
    let test = make_vflr_fsm_test(config);

    get_context(test)
}

/// Create a mock admin FSM with Controller FSM expectations set.
pub(crate) fn make_cntrl_fsm_test(config: ControllerFsmTestConfigs) -> AdminFsmTest {
    let mut test = AdminFsmTest::default();

    if config.error {
        default_resource_expectations(&mut test);

        if !config.unknown_event {
            test.function_mgr().expect_function().returning(move |_| {
                let mut function = MockFunction::new();

                function
                    .expect_query_state_change()
                    .times(config.call_count)
                    .returning(move || CntrlStateChangeAction::Invalid);

                function
            });
        }
    } else {
        test.admin_to_fp_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();
                let mut ipc_message = IpcMessage { data: [0; 16] };
                if !config.fp_invalid_response {
                    ipc_message.data[0] = 0x0201ff05;
                }

                if !config.unknown_event {
                    channel
                        .expect_send_request()
                        .times(config.call_count)
                        .returning(move |_, _| {
                            if config.fp_send_request_fail {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                    if !config.fp_send_request_fail && !config.hsm_send_request_fail {
                        channel
                            .expect_receive_message()
                            .times(config.call_count)
                            .returning(move || Some(ipc_message));
                    }
                }
                channel
            });

        if config.defer_action {
            test.deferred_queue_delete_pipe()
                .expect_recv()
                .returning(move || config.deferred_action_response);
        }

        test.hsm_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();
                let mut ipc_message = IpcMessage { data: [0; 16] };
                if !config.hsm_invalid_response {
                    ipc_message.data[0] = 0x0201ff05;
                    if config.defer_action {
                        // pending response from HSM core
                        ipc_message.data[0] = 0x0206ff05;
                    }
                }

                if !config.fp_send_request_fail && !config.unknown_event {
                    channel
                        .expect_send_request()
                        .times(config.call_count)
                        .returning(move |_, _| {
                            if config.hsm_send_request_fail {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                }
                if !config.hsm_send_request_fail
                    && !config.fp_send_request_fail
                    && !config.unknown_event
                {
                    channel
                        .expect_receive_message()
                        .times(config.call_count)
                        .returning(move || Some(ipc_message));
                }
                channel
            });

        test.admin_to_hsp_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);

        let mut mock_repeat_cnt = config.call_count;
        loop {
            if !config.unknown_event && config.action != ControllerAction::Migrate {
                test.function_mgr()
                    .expect_function()
                    .once()
                    .returning(move |_| {
                        let mut function = MockFunction::new();

                        function
                            .expect_query_state_change()
                            .times(1)
                            .return_once(move || {
                                if config.action == ControllerAction::Enable {
                                    CntrlStateChangeAction::Enable
                                } else if config.action == ControllerAction::Disable {
                                    CntrlStateChangeAction::Disable
                                } else if config.action == ControllerAction::Migrate {
                                    CntrlStateChangeAction::Migrate
                                } else {
                                    CntrlStateChangeAction::Invalid
                                }
                            });

                        function
                    });
            }
            if !config.fp_send_request_fail
                && !config.hsm_send_request_fail
                && !config.unknown_event
                && !config.defer_action_error
                && !config.hsm_invalid_response
                && !config.fp_invalid_response
                && config.io_cancellation_complete
            {
                test.function_mgr()
                    .expect_function()
                    .once()
                    .returning(move |_| {
                        let mut function = MockFunction::new();
                        if config.action == ControllerAction::Enable {
                            function.expect_enable().return_const(Ok(()));
                        } else {
                            function.expect_disable().return_const(());
                        }

                        function
                    });

                if config.action == ControllerAction::Enable {
                    test.msix_cntrl()
                        .expect_enable_pcie_fn()
                        .once()
                        .return_const(());
                }
            }
            mock_repeat_cnt -= 1;
            if mock_repeat_cnt == 0 {
                break;
            }
        }
    }

    test
}

pub(crate) fn make_cntrl_fsm(
    config: ControllerFsmTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let test = make_cntrl_fsm_test(config);

    get_context(test)
}

/// Create Io receive descriptor.
pub(crate) fn make_rx_desc(pfn: PcieFunction, sqe: AdminSqe, status: bool) -> Option<IoRxDesc> {
    let mut entry = [0u8; 64];
    entry.copy_from_slice(sqe.as_bytes());
    Some(IoRxDesc {
        sq_id: DevSqId(0),
        addr: 0,
        entry,
        pfn,
        status,
    })
}

/// Create a Io Submission Queue Entry with the given opcode.
pub(crate) fn make_sqe_with_opcode(opcode: u8) -> AdminSqe {
    let sqe = AdminSqe::default();
    let mut entry = [0u8; 64];
    entry.copy_from_slice(sqe.as_bytes());
    entry[0] = opcode;
    let sqe = AdminSqe::read_from_bytes(entry.as_bytes()).unwrap();

    sqe
}

/// Create a mock admin FSM with invalid opcode.
pub(crate) fn make_fsm_for_invalid_opcode() -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x88);
    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: 0xFF,
        status: IoTxCompleteStatus::Success,
    };

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(|_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                Some(AdminQueue::new(
                    QueueCntrlId::Pf.into(),
                    QueueCntrlId::Pf.into(),
                ))
            });

            function
        });
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Ok(()));
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(send_complete_desc));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    get_context(test)
}

/// Create a mock stand alone admin FSM for identify controller command FSM
pub(crate) fn make_stand_alone_fsm_for_identify(
    unknown_event: bool,
    dma_alloc_succeeds: bool,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    default_resource_expectations(&mut test);

    if !unknown_event {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(|_| {
                let mut function = MockFunction::new();
                function
                    .expect_cntrl_id()
                    .times(1)
                    .return_const(QueueCntrlId::Pf);

                function
            });
        test.dma_heap()
            .expect_allocate()
            .times(1)
            .return_once(move |s| {
                if dma_alloc_succeeds {
                    Some(MockDmaAlloc::new(s))
                } else {
                    None
                }
            });
    }

    let mut env = test.env();
    if dma_alloc_succeeds && !unknown_event {
        let mut soc_info = MockSocInfo::new();
        soc_info
            .expect_fw_version()
            .times(1)
            .returning(|| [0u8; 32]);
        soc_info.expect_id().times(1).returning(|| [0u8; 32]);
        soc_info.expect_svn().times(1).returning(|| [0u8; 8]);

        env.expect_soc_info().once().return_const(soc_info);
    }

    let ctx = get_context_from_environment(env);
    let sqe = make_sqe_with_opcode(0x6);

    (ctx, sqe)
}

/// Create a mock admin FSM for identify controller command FSM
pub(crate) fn make_fsm_for_identify(
    config: AdminFsmIdentifyTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x6);
    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: if config.invalid_io_tag { 0xFFFF } else { 0xFF },
        status: if config.io_success_status {
            IoTxCompleteStatus::Success
        } else {
            IoTxCompleteStatus::Undefined
        },
    };
    let desc = Some(DmaTxnCompletionDesc {
        success: config.dma_success_status,
        tag: if config.invalid_dma_tag { 0xFFFF } else { 0xFF },
    });

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(move || {
                if config.none_admin_queue {
                    None
                } else {
                    let queue = AdminQueue::new(QueueCntrlId::Pf.into(), QueueCntrlId::Pf.into());
                    if config.invalid_admin_queue {
                        queue.invalidate();
                    }

                    Some(queue)
                }
            });

            function
        });
    if !config.none_admin_queue {
        if config.dma_alloc_succeeds && !config.invalid_admin_queue {
            test.dma_channel()
                .expect_begin_txn()
                .times(1)
                .return_const(Ok(()));
            test.dma_channel()
                .expect_end_txn()
                .times(1)
                .return_once(move || if config.optional_dma_desc { desc } else { None });
        }
        if !config.invalid_admin_queue {
            test.io_channel()
                .expect_begin_send()
                .times(1)
                .return_once(|_| Ok(()));
            test.io_channel()
                .expect_end_send()
                .times(1)
                .return_once(move || {
                    if config.io_end_send_returns_none {
                        None
                    } else {
                        Some(send_complete_desc)
                    }
                });
        }
    }

    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    let mut env = test.env();
    if !config.none_admin_queue {
        env.expect_clone().times(1).returning(move || {
            let mut env = MockAdminEnvTrait::new();

            let mut function_mgr = MockFunctionMgr::new();
            function_mgr.expect_function().times(1).returning(|_| {
                let mut function = MockFunction::new();

                function
                    .expect_cntrl_id()
                    .times(1)
                    .return_const(QueueCntrlId::Pf);

                function
            });

            let mut dma_heap = MockDmaHeap::new();
            if !config.none_admin_queue {
                dma_heap.expect_allocate().times(1).return_once(move |s| {
                    if config.dma_alloc_succeeds {
                        Some(MockDmaAlloc::new(s))
                    } else {
                        None
                    }
                });
            }

            let mut soc_info = MockSocInfo::new();
            soc_info
                .expect_fw_version()
                .times(1)
                .returning(|| [0u8; 32]);
            soc_info.expect_id().times(1).returning(|| [0u8; 32]);
            soc_info.expect_svn().times(1).returning(|| [0u8; 8]);

            env.expect_function_mgr()
                .times(1)
                .return_const(function_mgr);
            env.expect_dma_heap().times(1).return_const(dma_heap);
            env.expect_soc_info().once().return_const(soc_info);

            env
        });
    }

    get_context_from_environment(env)
}

/// Create a mock admin FSM to get failures in DMA out.
pub(crate) fn make_fsm_for_dma_out_fail() -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x6);
    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: 0xFF,
        status: IoTxCompleteStatus::Success,
    };

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(|_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                Some(AdminQueue::new(
                    QueueCntrlId::Pf.into(),
                    QueueCntrlId::Pf.into(),
                ))
            });
            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Err(u32::MAX));
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Ok(()));
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(|| Some(send_complete_desc));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    let mut env = test.env();
    env.expect_clone().times(1).returning(|| {
        let mut env = MockAdminEnvTrait::new();

        let mut function_mgr = MockFunctionMgr::new();
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();

            function
                .expect_cntrl_id()
                .times(1)
                .return_const(QueueCntrlId::Pf);

            function
        });

        let mut dma_heap = MockDmaHeap::new();
        dma_heap
            .expect_allocate()
            .times(1)
            .return_once(|s| Some(MockDmaAlloc::new(s)));

        let mut soc_info = MockSocInfo::new();
        soc_info
            .expect_fw_version()
            .times(1)
            .returning(|| [0u8; 32]);
        soc_info.expect_id().times(1).returning(|| [0u8; 32]);
        soc_info.expect_svn().times(1).returning(|| [0u8; 8]);

        env.expect_function_mgr()
            .times(1)
            .return_const(function_mgr);
        env.expect_dma_heap().times(1).return_const(dma_heap);
        env.expect_soc_info().once().return_const(soc_info);

        env
    });

    get_context_from_environment(env)
}

/// Create a mock admin FSM to get failures in DMA out and subsequent send_err_cqe
pub(crate) fn make_fsm_for_dma_out_and_send_err_cqe_fail() -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x6);

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(|_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                Some(AdminQueue::new(
                    QueueCntrlId::Pf.into(),
                    QueueCntrlId::Pf.into(),
                ))
            });

            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Err(u32::MAX));
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Err(u32::MAX));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .returning(|_, _| ());

    let mut env = test.env();
    env.expect_clone().times(1).returning(|| {
        let mut env = MockAdminEnvTrait::new();
        let mut function_mgr = MockFunctionMgr::new();
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();

            function
                .expect_cntrl_id()
                .times(1)
                .return_const(QueueCntrlId::Pf);

            function
        });

        let mut dma_heap = MockDmaHeap::new();
        dma_heap
            .expect_allocate()
            .times(1)
            .return_once(|s| Some(MockDmaAlloc::new(s)));

        let mut soc_info = MockSocInfo::new();
        soc_info
            .expect_fw_version()
            .times(1)
            .returning(|| [0u8; 32]);
        soc_info.expect_id().times(1).returning(|| [0u8; 32]);
        soc_info.expect_svn().times(1).returning(|| [0u8; 8]);

        env.expect_function_mgr()
            .times(1)
            .return_const(function_mgr);
        env.expect_dma_heap().once().return_const(dma_heap);
        env.expect_soc_info().once().return_const(soc_info);

        env
    });

    get_context_from_environment(env)
}

/// Create a mock admin FSM for identify controller command with send_cqe failure
pub(crate) fn make_fsm_for_send_cqe_fails_after_dma_complete() -> AdminFsmContext<MockAdminEnvTrait>
{
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x6);
    let desc = Some(DmaTxnCompletionDesc {
        success: true,
        tag: 0xFF,
    });

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(|_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                Some(AdminQueue::new(
                    QueueCntrlId::Pf.into(),
                    QueueCntrlId::Pf.into(),
                ))
            });

            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(|| desc);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Err(u32::MAX));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .returning(|_, _| ());

    let mut env = test.env();
    env.expect_clone().times(1).returning(|| {
        let mut env = MockAdminEnvTrait::new();
        let mut function_mgr = MockFunctionMgr::new();
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();

            function
                .expect_cntrl_id()
                .times(1)
                .return_const(QueueCntrlId::Pf);

            function
        });

        let mut soc_info = MockSocInfo::new();
        soc_info
            .expect_fw_version()
            .times(1)
            .returning(|| [0u8; 32]);
        soc_info.expect_id().times(1).returning(|| [0u8; 32]);
        soc_info.expect_svn().times(1).returning(|| [0u8; 8]);

        let mut dma_heap = MockDmaHeap::new();
        dma_heap
            .expect_allocate()
            .times(1)
            .return_once(|s| Some(MockDmaAlloc::new(s)));

        env.expect_function_mgr()
            .times(1)
            .return_const(function_mgr);
        env.expect_dma_heap().times(1).return_const(dma_heap);
        env.expect_soc_info().times(1).return_const(soc_info);

        env
    });

    get_context_from_environment(env)
}

pub(crate) fn make_fsm_for_send_cqe_fails_after_cmd_completes() -> AdminFsmContext<MockAdminEnvTrait>
{
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x88);

    default_resource_expectations(&mut test);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, sqe, true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(|_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                Some(AdminQueue::new(
                    QueueCntrlId::Pf.into(),
                    QueueCntrlId::Pf.into(),
                ))
            });

            function
        });
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Err(u32::MAX));
    test.io_channel()
        .expect_end_recv()
        .times(1)
        .returning(|_, _| ());

    get_context(test)
}

/// Create a mock admin FSM for SetRes command FSM
pub(crate) fn make_fsm_for_set_res(
    config: AdminFsmSetResTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0xC3);
    let mut set_res_sqe: GetSetResourceSqe = sqe.into();
    set_res_sqe.num_resource = config.num_resource;
    set_res_sqe.cntrl_id = config.cntrl_id;

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    if config.num_resource < 66
        && config.cntrl_id < 65
        && !config.unknown_event
        && config.request_from_pf
    {
        test.function_mgr()
            .expect_set_res_cnt()
            .times(1)
            .returning(move |_, _| {
                if config.num_resource > 0 {
                    Ok(config.prev_resource)
                } else {
                    Err(AdminErr::SetResCountLimitExceeded)
                }
            });
        if config.num_resource > 0 {
            if config.prev_resource != config.num_resource {
                test.function_mgr()
                    .expect_function()
                    .times(1)
                    .returning(move |_| {
                        let mut function = MockFunction::new();
                        function
                            .expect_res_mask()
                            .times(1)
                            .return_const(0x100000000u128.to_be_bytes());

                        function
                    });
                test.hsm_ipc_channel()
                    .expect_clone()
                    .once()
                    .returning(move || {
                        let mut channel = MockIpcMessageChannel::new();
                        let mut ipc_message = IpcMessage { data: [0; 16] };
                        if config.ipc_resp_err_status {
                            ipc_message.data[0] = 0x000F0000
                        }
                        channel
                            .expect_send_request()
                            .times(1)
                            .returning(move |_, _| {
                                if config.ipc_send_req_err_status {
                                    Err(u32::MAX)
                                } else {
                                    Ok(())
                                }
                            });
                        if !config.ipc_send_req_err_status {
                            channel
                                .expect_receive_message()
                                .times(1)
                                .returning(move || {
                                    if config.receive_none_ipc_message {
                                        None
                                    } else {
                                        Some(ipc_message)
                                    }
                                });
                        }

                        channel
                    });
            } else {
                test.hsm_ipc_channel()
                    .expect_clone()
                    .times(1)
                    .returning(MockIpcMessageChannel::new);
            }
        } else {
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
        }
    } else {
        test.hsm_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);
    }

    let ctx = get_context(test);

    (ctx, set_res_sqe.into())
}

/// Create a mock admin FSM for GetRes command FSM
pub(crate) fn make_fsm_for_get_res(
    cntrl_id: u32,
    unknown_event: bool,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0xC4);
    let mut get_res_sqe: GetSetResourceSqe = sqe.into();
    get_res_sqe.cntrl_id = cntrl_id;

    default_resource_expectations(&mut test);

    if cntrl_id < 65 && !unknown_event {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(|_| {
                let mut function = MockFunction::new();
                function.expect_res_cnt().times(1).return_const(1u32);

                function
            });
    }

    let ctx = get_context(test);

    (ctx, get_res_sqe.into())
}

/// Create a mock admin FSM for Create Completion Queue command FSM
pub(crate) fn make_fsm_for_create_cq(
    config: AdminFsmCreateCqTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let invalid_iv = match HostQueueType::from(config.host_cq) {
        HostQueueType::Admin => true,
        HostQueueType::Hsm => config.ien && !(1..=0xF).contains(&config.iv),
        HostQueueType::Fp => config.ien && !(0x10..=0x1F).contains(&config.iv),
    };

    let sqe = make_sqe_with_opcode(0x5);
    let mut create_cq: CreateCqSqe = sqe.into();
    create_cq.queue_id = config.host_cq;
    create_cq.queue_len = config.queue_len;
    create_cq.prp1.lo = 0x11223344;
    create_cq.prp2.lo = 0x55667788;
    create_cq.attr.set_pc(config.pc);
    create_cq.attr.set_ien(config.ien);
    create_cq.attr.set_iv(config.iv);

    default_resource_expectations(&mut test);

    if !invalid_iv
        && config.pc
        && config.queue_len > 0
        && config.host_cq > HostCqId(0)
        && !config.unknown_event
    {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_create_cq()
                    .times(1)
                    .returning(move |_, _, _| {
                        if config.create_cq_fails {
                            Err(AdminErr::InvalidQueueId)
                        } else {
                            Ok(())
                        }
                    });
                function
            });
    }

    let ctx = get_context(test);

    (ctx, create_cq.into())
}

/// Create a mock admin FSM for Delete Completion Queue command FSM
pub(crate) fn make_fsm_for_delete_cq(
    host_cq: HostCqId,
    delete_cq_err_code: Option<AdminErr>,
    unknown_event: bool,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x4);
    let mut delete_cq: DeleteCqSqe = sqe.into();
    delete_cq.queue_id = host_cq;

    default_resource_expectations(&mut test);

    if host_cq > HostCqId(0) && !unknown_event {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_delete_cq()
                    .times(1)
                    .returning(move |_| match delete_cq_err_code {
                        Some(err) => Err(err),
                        None => Ok(()),
                    });
                function
            });
    }

    let ctx = get_context(test);

    (ctx, delete_cq.into())
}

/// Create a mock admin FSM for Create Submission Queue command FSM
pub(crate) fn make_fsm_for_create_sq(
    config: AdminFsmCreateSqTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x1);
    let mut create_sq: CreateSqSqe = sqe.into();
    create_sq.queue_id = config.host_sq;
    create_sq.queue_len = config.queue_len;
    create_sq.host_cq_id = config.host_cq;
    create_sq.attr.set_priority(config.queue_priority.into());
    create_sq.attr.set_pc(config.pc);

    if config.host_sq > HostSqId(0)
        && config.pc
        && config.queue_len > 0
        && config.host_cq > HostCqId(0)
        && !config.unknown_event
    {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_create_sq()
                    .times(1)
                    .returning(move |_, _, _| match config.create_sq_err_code {
                        Some(err) => Err(err),
                        None => Ok((DevSqId::Id1, DevCqId::Id1)),
                    });
                function
            });
    }

    if config.ipc_response_err_status || config.ipc_response_none {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_delete_sq()
                    .times(1)
                    .returning(|_| Ok((DevSqId::Id1, DevCqId::Id1)));
                function
            });
    }

    match config.host_sq.into() {
        HostQueueType::Admin => {
            default_resource_expectations(&mut test);
        }
        HostQueueType::Hsm => {
            if config.pc
                && config.queue_len > 0
                && config.host_cq > HostCqId(0)
                && config.create_sq_err_code.is_none()
                && !config.unknown_event
            {
                test.hsm_ipc_channel()
                    .expect_clone()
                    .once()
                    .returning(move || {
                        let mut channel = MockIpcMessageChannel::new();
                        let mut ipc_message = IpcMessage { data: [0; 16] };
                        if config.ipc_response_err_status {
                            ipc_message.data[0] = 0x000F0000
                        }
                        channel
                            .expect_send_request()
                            .times(1)
                            .returning(move |_, _| {
                                if config.ipc_send_failure {
                                    Err(u32::MAX)
                                } else {
                                    Ok(())
                                }
                            });
                        if !config.ipc_send_failure {
                            channel
                                .expect_receive_message()
                                .times(1)
                                .returning(move || {
                                    if config.ipc_response_none {
                                        None
                                    } else {
                                        Some(ipc_message)
                                    }
                                });
                        }

                        channel
                    });
            } else {
                test.hsm_ipc_channel()
                    .expect_clone()
                    .times(1)
                    .returning(MockIpcMessageChannel::new);
            }
            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
            test.admin_to_hsp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
        }
        HostQueueType::Fp => {
            if config.pc
                && config.queue_len > 0
                && config.host_cq > HostCqId(0)
                && config.create_sq_err_code.is_none()
                && !config.unknown_event
            {
                test.admin_to_fp_ipc_channel()
                    .expect_clone()
                    .once()
                    .returning(move || {
                        let mut channel = MockIpcMessageChannel::new();
                        let mut ipc_message = IpcMessage { data: [0; 16] };
                        if config.ipc_response_err_status {
                            ipc_message.data[0] = 0x000F0000
                        }
                        channel
                            .expect_send_request()
                            .times(1)
                            .returning(move |_, _| {
                                if config.ipc_send_failure {
                                    Err(u32::MAX)
                                } else {
                                    Ok(())
                                }
                            });
                        if !config.ipc_send_failure {
                            channel
                                .expect_receive_message()
                                .times(1)
                                .returning(move || {
                                    if config.ipc_response_none {
                                        None
                                    } else {
                                        Some(ipc_message)
                                    }
                                });
                        }

                        channel
                    });
            } else {
                test.admin_to_fp_ipc_channel()
                    .expect_clone()
                    .times(1)
                    .returning(MockIpcMessageChannel::new);
            }
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
            test.admin_to_hsp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
        }
    }

    let ctx = get_context(test);

    (ctx, create_sq.into())
}

/// Create a mock admin FSM for Delete Submission Queue command FSM
pub(crate) fn make_fsm_for_delete_sq(
    config: AdminFsmDeleteSqTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x0);
    let mut delete_sq: DeleteSqSqe = sqe.into();
    delete_sq.queue_id = config.host_sq;

    if config.host_sq > HostSqId(0) && !config.unknown_event {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function.expect_dev_sq().times(1).returning(move |_| {
                    match config.dev_sq_dev_cq_err_code {
                        Some(err) => Err(err),
                        None => Ok(DevSqId::Id1),
                    }
                });
                function
            });
    }

    if config.host_sq > HostSqId(0)
        && config.dev_sq_dev_cq_err_code.is_none()
        && !config.ipc_send_failure
        && !config.ipc_response_err_status
        && !config.ipc_response_none
        && !config.unknown_event
        && !config.deferred_delete_response_error
    {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();

                function.expect_delete_sq().times(1).returning(move |_| {
                    match config.delete_sq_err_code {
                        Some(err) => Err(err),
                        None => Ok((DevSqId::Id1, DevCqId::Id1)),
                    }
                });

                function
            });
    }

    match config.host_sq.into() {
        HostQueueType::Admin => {
            default_resource_expectations(&mut test);
        }
        HostQueueType::Hsm => {
            if config.dev_sq_dev_cq_err_code.is_none() {
                test.hsm_ipc_channel()
                    .expect_clone()
                    .once()
                    .returning(move || {
                        let mut channel = MockIpcMessageChannel::new();
                        let mut ipc_message = IpcMessage { data: [0; 16] };
                        if config.ipc_response_err_status {
                            ipc_message.data[0] = 0x000F0000
                        } else if config.deferred_delete_response {
                            // Set error code as pending
                            ipc_message.data[0] = 0x00060000
                        }
                        channel
                            .expect_send_request()
                            .times(1)
                            .returning(move |_, _| {
                                if config.ipc_send_failure {
                                    Err(u32::MAX)
                                } else {
                                    Ok(())
                                }
                            });
                        if !config.ipc_send_failure {
                            channel
                                .expect_receive_message()
                                .times(1)
                                .returning(move || {
                                    if config.ipc_response_none {
                                        None
                                    } else {
                                        Some(ipc_message)
                                    }
                                });
                        }

                        channel
                    });
                if config.deferred_delete_response {
                    test.deferred_queue_delete_pipe()
                        .expect_recv()
                        .once()
                        .returning(move || config.deferred_queue_delete_response);
                }
            } else {
                test.hsm_ipc_channel()
                    .expect_clone()
                    .times(1)
                    .returning(MockIpcMessageChannel::new);
            }
            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
            test.admin_to_hsp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
        }
        HostQueueType::Fp => {
            if config.dev_sq_dev_cq_err_code.is_none() {
                test.admin_to_fp_ipc_channel()
                    .expect_clone()
                    .once()
                    .returning(move || {
                        let mut channel = MockIpcMessageChannel::new();
                        let mut ipc_message = IpcMessage { data: [0; 16] };
                        if config.ipc_response_err_status {
                            ipc_message.data[0] = 0x000F0000
                        }
                        channel
                            .expect_send_request()
                            .times(1)
                            .returning(move |_, _| {
                                if config.ipc_send_failure {
                                    Err(u32::MAX)
                                } else {
                                    Ok(())
                                }
                            });
                        if !config.ipc_send_failure {
                            channel
                                .expect_receive_message()
                                .times(1)
                                .returning(move || {
                                    if config.ipc_response_none {
                                        None
                                    } else {
                                        Some(ipc_message)
                                    }
                                });
                        }

                        channel
                    });
            } else {
                test.admin_to_fp_ipc_channel()
                    .expect_clone()
                    .times(1)
                    .returning(MockIpcMessageChannel::new);
            }
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
            test.admin_to_hsp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);
        }
    }

    let ctx = get_context(test);

    (ctx, delete_sq.into())
}

/// Create a mock admin FSM for Set Features command FSM
pub(crate) fn make_fsm_for_get_set_features(
    op_code: AdminCommandOpCodes,
    id: AdminFeatureId,
    num_res: u32,
    unknown_event: bool,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(op_code.0);
    let mut get_set_features_sqe: SetFeaturesSqe = sqe.into();
    get_set_features_sqe.id = id;

    default_resource_expectations(&mut test);

    if !unknown_event {
        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function.expect_res_cnt().times(1).return_const(num_res);

                function
            });
    }

    let ctx = get_context(test);

    (ctx, get_set_features_sqe.into())
}

pub(crate) fn make_fsm_test_for_self_test(config: AdminFsmSelfTestTestConfigs) -> AdminFsmTest {
    let mut test = AdminFsmTest::default();

    if config.execute_aes_unwrap {
        test.soft_aes()
            .expect_aes_256_key_unwrap_self_test()
            .times(1)
            .returning(|| {
                let soft_aes = SoftAes::new();

                soft_aes.aes_256_key_unwrap_self_test()
            });
    }

    if config.execute_aes_ecb {
        test.soft_aes()
            .expect_aes_ecb_256_decrypt_self_test()
            .times(1)
            .returning(|| {
                let soft_aes = SoftAes::new();

                soft_aes.aes_ecb_256_decrypt_self_test()
            });
    }

    if config.execute_aes_xts_neg_enc {
        make_fsm_test_for_aes_xts_gcm_self_tests(&mut test, &config, AesSelfTestType::XtsNegEnc);
    }

    if config.execute_aes_xts_neg_dec {
        make_fsm_test_for_aes_xts_gcm_self_tests(&mut test, &config, AesSelfTestType::XtsNegDec);
    }

    if config.execute_aes_gcm_aligned_and_unaligned_data {
        make_fsm_test_for_aes_xts_gcm_self_tests(
            &mut test,
            &config,
            AesSelfTestType::GcmAlignedAndUnalignedData,
        );
    }

    if config.execute_aes_gcm_aligned_data {
        make_fsm_test_for_aes_xts_gcm_self_tests(
            &mut test,
            &config,
            AesSelfTestType::GcmAlignedData,
        );
    }

    if config.execute_aes_gcm_no_aligned_data {
        make_fsm_test_for_aes_xts_gcm_self_tests(
            &mut test,
            &config,
            AesSelfTestType::GcmNoAlignedData,
        );
    }

    let ipc_send_failure = config.ipc_send_failure;
    let num_xts_send_req = config.num_xts_send_requests;
    let num_xts_rec_resp = config.num_xts_receive_responses;
    let num_gcm_send_req = config.num_gcm_send_requests;
    let fp_msg_status = config.fp_receive_message_status;
    let aes_xts = config.execute_aes_xts_neg_enc | config.execute_aes_xts_neg_dec;
    let aes_gcm_aligned_and_unaligned = config.execute_aes_gcm_aligned_and_unaligned_data;
    let aes_gcm_aligned = config.execute_aes_gcm_aligned_data;
    let aes_gcm_no_aligned = config.execute_aes_gcm_no_aligned_data;

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            let mut ipc_message = IpcMessage { data: [0; 16] };
            ipc_message.data[0] |= fp_msg_status << 16u32;

            if aes_xts {
                channel
                    .expect_send_request()
                    .times(num_xts_send_req)
                    .returning(move |_, _| {
                        if ipc_send_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                channel
                    .expect_receive_message()
                    .times(num_xts_rec_resp)
                    .returning(move || Some(ipc_message));
            }

            if aes_gcm_aligned_and_unaligned {
                channel
                    .expect_send_request()
                    .times(num_gcm_send_req)
                    .returning(move |_, _| {
                        if ipc_send_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                // If AES GCM, then the 2nd response message we expect a non zero status
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || Some(ipc_message));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || {
                        let ipc_message = IpcMessage {
                            data: {
                                let mut data = [0; 16];
                                data[0] |= 0x00060000; // Pending status
                                data
                            },
                        };
                        Some(ipc_message)
                    });
            }

            if aes_gcm_aligned {
                channel
                    .expect_send_request()
                    .times(num_gcm_send_req)
                    .returning(move |_, _| {
                        if ipc_send_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                // For AES GCM aligned only data, we expect all messages to be success
                channel
                    .expect_receive_message()
                    .times(num_gcm_send_req)
                    .returning(move || Some(ipc_message));
            }

            if aes_gcm_no_aligned {
                channel
                    .expect_send_request()
                    .times(num_gcm_send_req)
                    .returning(move |_, _| {
                        if ipc_send_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || Some(ipc_message));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || {
                        let ipc_message = IpcMessage {
                            data: {
                                let mut data = [0; 16];
                                data[0] |= 0x00060000; // Pending status
                                data
                            },
                        };
                        Some(ipc_message)
                    });
            }

            channel
        });

    test.hsm_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    if config.hsm_executed_self_test {
        if config.send_failure {
            test.self_test_req()
                .expect_send()
                .times(config.num_self_tests)
                .return_once(|_| Err(u32::MAX));

            return test;
        }

        test.self_test_req()
            .expect_send()
            .times(config.num_self_tests)
            .return_const(Ok(()));

        if !config.notify_self_test_failure && !config.test_timeout {
            test.self_test_resp()
                .expect_recv()
                .times(config.num_self_tests)
                .returning(move || {
                    if config.recv_failure {
                        None
                    } else {
                        Some(SelfTestRespPacket {
                            result: if config.test_success {
                                Ok(())
                            } else {
                                Err(u32::MAX)
                            },
                        })
                    }
                });
        }

        if config.rng_self_test {
            test.io_controller()
                .expect_pause_inbound()
                .times(0..=1)
                .returning(|| ());
            test.io_controller()
                .expect_resume_inbound()
                .times(0..=1)
                .returning(|| ());
        }
    }

    if config.notify_self_test_failure {
        test.env()
            .expect_notify_self_test_failure()
            .times(1)
            .return_const(());
    }

    test
}

#[derive(PartialEq, Copy, Clone)]
pub enum AesSelfTestType {
    XtsNegEnc,
    XtsNegDec,
    GcmAlignedAndUnalignedData,
    GcmNoAlignedData,
    GcmAlignedData,
}

fn make_fsm_test_for_aes_xts_gcm_self_tests(
    test: &mut AdminFsmTest,
    config: &AdminFsmSelfTestTestConfigs,
    aes_self_test_type: AesSelfTestType,
) {
    let end_enc_dec_failure = config.end_enc_dec_failure;

    // Initialize the self_test_key_table so the expectation is set up in env()
    let self_test_key_table = test.self_test_key_table();

    // Populate the key table
    self_test_key_table[CdmaKeyIndex::XtsNegEncEnc as usize] =
        Some(AesBulk256KeyId::from(CdmaKeyIndex::XtsNegEncEnc as u16));
    self_test_key_table[CdmaKeyIndex::XtsNegEncTweak as usize] =
        Some(AesBulk256KeyId::from(CdmaKeyIndex::XtsNegEncTweak as u16));
    self_test_key_table[CdmaKeyIndex::XtsNegDecEnc as usize] =
        Some(AesBulk256KeyId::from(CdmaKeyIndex::XtsNegDecEnc as u16));
    self_test_key_table[CdmaKeyIndex::XtsNegDecTweak as usize] =
        Some(AesBulk256KeyId::from(CdmaKeyIndex::XtsNegDecTweak as u16));
    self_test_key_table[CdmaKeyIndex::GcmAlignedAndUnalignedData as usize] = Some(
        AesBulk256KeyId::from(CdmaKeyIndex::GcmAlignedAndUnalignedData as u16),
    );
    self_test_key_table[CdmaKeyIndex::GcmAlignedDataOnly as usize] = Some(AesBulk256KeyId::from(
        CdmaKeyIndex::GcmAlignedDataOnly as u16,
    ));
    self_test_key_table[CdmaKeyIndex::GcmAadNoAlignedData as usize] = Some(AesBulk256KeyId::from(
        CdmaKeyIndex::GcmAadNoAlignedData as u16,
    ));

    test.cdma_io()
        .expect_begin_enc_dec()
        .times(config.num_begin_enc_dec)
        .return_const({
            if config.begin_enc_dec_failure {
                // CdmaIoErr::InvalidState
                Err(0x150005)
            } else {
                Ok(())
            }
        });

    test.cdma_io()
        .expect_end_enc_dec()
        .withf(move |_, cdma_io_config, _| {
            match aes_self_test_type {
                AesSelfTestType::XtsNegEnc | AesSelfTestType::XtsNegDec => {
                    matches!(cdma_io_config.mode, AesFpCipher::Xts)
                }
                AesSelfTestType::GcmAlignedAndUnalignedData => {
                    let expected_src_len = 32u32
                        + unsafe {
                            #[allow(static_mut_refs)]
                            AES_GCM_256_TEST_VECTORS.aligned_data_len
                        };
                    cdma_io_config.mode == AesFpCipher::Gcm
                        && cdma_io_config.src_len == expected_src_len
                }
                AesSelfTestType::GcmNoAlignedData => {
                    let expected_src_len = 32u32
                        + unsafe {
                            #[allow(static_mut_refs)]
                            AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS.aligned_data_len
                        };
                    cdma_io_config.mode == AesFpCipher::Gcm
                        && cdma_io_config.src_len == expected_src_len
                }
                AesSelfTestType::GcmAlignedData => {
                    let expected_src_len = 32u32
                        + unsafe {
                            #[allow(static_mut_refs)]
                            AES_GCM_256_AAD_ALIGNED_DATA_ONLY_TEST_VECTORS.aligned_data_len
                        };
                    cdma_io_config.mode == AesFpCipher::Gcm
                        && cdma_io_config.src_len == expected_src_len
                }
            }
        })
        .times(config.num_end_enc_dec)
        .returning(move |_, cdma_io_config, res_buf| {
            if end_enc_dec_failure {
                // CdmaIoErr::CdmaIoEncDecFailed
                Err(0x150001)
            } else {
                match cdma_io_config.mode {
                    AesFpCipher::Gcm => {
                        let test_vector = match aes_self_test_type {
                            AesSelfTestType::GcmAlignedAndUnalignedData => unsafe {
                                #[allow(static_mut_refs)]
                                &AES_GCM_256_TEST_VECTORS
                            },
                            AesSelfTestType::GcmNoAlignedData => unsafe {
                                #[allow(static_mut_refs)]
                                &AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS
                            },
                            AesSelfTestType::GcmAlignedData => unsafe {
                                #[allow(static_mut_refs)]
                                &AES_GCM_256_AAD_ALIGNED_DATA_ONLY_TEST_VECTORS
                            },
                            _ => unreachable!(),
                        };

                        // Use the runtime config AAD if present so our mock matches how the
                        // production code sizes/uses the result buffer.
                        let aad = cdma_io_config
                            .padded_aad
                            .or(test_vector.padded_aad)
                            .unwrap();
                        let expected_res_len = aad.len() + test_vector.aligned_data_len as usize;
                        assert_eq!(
                            res_buf.len(),
                            expected_res_len,
                            "Unexpected AES-GCM res_buf length: got {}, expected {} (aad_len={}, aligned_data_len={})",
                            res_buf.len(),
                            expected_res_len,
                            aad.len(),
                            test_vector.aligned_data_len
                        );
                        let aad_len = aad.len();
                        let aligned_data_len = test_vector.aligned_data_len as usize;
                        match cdma_io_config.op {
                            AesFpOp::Encrypt => {
                                // Simulate encryption: return ciphertext and tag
                                res_buf[0..aad_len].copy_from_slice(aad.as_slice());
                                res_buf[aad_len..aad_len + aligned_data_len]
                                    .copy_from_slice(&test_vector.ciphertext[0..aligned_data_len]);

                                Ok(Some(test_vector.tag))
                            }
                            AesFpOp::Decrypt => {
                                // Simulate decryption: return plaintext
                                res_buf[0..aad_len].copy_from_slice(aad.as_slice());
                                res_buf[aad_len..aad_len + aligned_data_len]
                                    .copy_from_slice(&test_vector.plaintext[0..aligned_data_len]);

                                Ok(Some(test_vector.tag))
                            }
                        }
                    }
                    AesFpCipher::Xts => {
                        match cdma_io_config.op {
                            AesFpOp::Encrypt => {
                                res_buf.copy_from_slice(unsafe {
                                    #[allow(static_mut_refs)]
                                    &AES_XTS_256_TEST_VECTORS.ciphertext
                                });
                            }
                            AesFpOp::Decrypt => {
                                res_buf.copy_from_slice(unsafe {
                                    #[allow(static_mut_refs)]
                                    &AES_XTS_256_TEST_VECTORS.plaintext
                                });
                            }
                        }

                        Ok(None)
                    }
                }
            }
        });

    if aes_self_test_type == AesSelfTestType::GcmAlignedAndUnalignedData
        || aes_self_test_type == AesSelfTestType::GcmNoAlignedData
    {
        let is_aligned_gcm = aes_self_test_type == AesSelfTestType::GcmAlignedAndUnalignedData;

        test.soft_aes()
            .expect_aes_gcm_tag_correction()
            .withf(
                move |_, _, _, _, text_len, _, _, _, aligned_data_len, tag_ext_out_buf| {
                    let expected = if is_aligned_gcm {
                        unsafe {
                            #[allow(static_mut_refs)]
                            &AES_GCM_256_TEST_VECTORS
                        }
                    } else {
                        unsafe {
                            #[allow(static_mut_refs)]
                            &AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS
                        }
                    };

                    // Match this expectation only for the intended vector.
                    // `text_len` is the full input length; `aligned_data_len` is how much CDMA handled.
                    ((*text_len as usize) == expected.text_len)
                        && ((*aligned_data_len as u32) == expected.aligned_data_len)
                        && (tag_ext_out_buf.len()
                            == expected.text_len - expected.aligned_data_len as usize)
                },
            )
            .times(config.num_tag_correction)
            .returning(
                move |encrypt, _, _, _, text_len, _, _, _, aligned_data_len, tag_ext_out_buf| {
                    let test_vector = if is_aligned_gcm {
                        unsafe {
                            #[allow(static_mut_refs)]
                            &AES_GCM_256_TEST_VECTORS
                        }
                    } else {
                        unsafe {
                            #[allow(static_mut_refs)]
                            &AES_GCM_256_AAD_NO_ALIGNED_DATA_TEST_VECTORS
                        }
                    };

                    let start = std::cmp::min(aligned_data_len, test_vector.text_len) as usize;
                    let end = std::cmp::min(text_len, test_vector.text_len as u64) as usize;

                    let src = if encrypt {
                        &test_vector.ciphertext
                    } else {
                        &test_vector.plaintext
                    };

                    let start = std::cmp::min(start, src.len());
                    let end = std::cmp::min(end, src.len());
                    let slice = if end >= start { &src[start..end] } else { &[] };
                    let copy_len = std::cmp::min(tag_ext_out_buf.len(), slice.len());
                    if copy_len > 0 {
                        tag_ext_out_buf[0..copy_len].copy_from_slice(&slice[0..copy_len]);
                    }

                    Ok(test_vector.tag)
                },
            );
    }

    test.cdma_io()
        .expect_zeroize_buffers()
        .times(config.num_zeroize_buffers)
        .return_const(());
}

/// Create a mock admin FSM for Self Test command FSM
pub(crate) fn make_fsm_for_self_test(
    config: AdminFsmSelfTestTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let test = make_fsm_test_for_self_test(config);

    get_context(test)
}

/// Create a mock admin FSM for set resource FSM being called through AdminFsm
pub(crate) fn make_env_for_admin_fsm_to_perform_set_res() -> MockAdminEnvTrait {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0xC3);
    let mut set_res_sqe: GetSetResourceSqe = sqe.into();
    set_res_sqe.cntrl_id = QueueCntrlId::Pf as u32;
    set_res_sqe.num_resource = 1;
    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: 0xF,
        status: IoTxCompleteStatus::Success,
    };
    let desc = Some(DmaTxnCompletionDesc {
        success: true,
        tag: 0xFF,
    });

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);
    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, set_res_sqe.into(), true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                let queue = AdminQueue::new(QueueCntrlId::Pf.into(), QueueCntrlId::Pf.into());

                Some(queue)
            });

            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(move || desc);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Ok(()));
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(move || Some(send_complete_desc));

    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    test.hsm_ipc_channel()
        .expect_clone()
        .times(..)
        .returning(|| {
            let mut channel = MockIpcMessageChannel::new();

            channel
                .expect_send_request()
                .times(1)
                .returning(move |_, _| Ok(()));
            channel
                .expect_receive_message()
                .times(1)
                .returning(|| Some(IpcMessage { data: [0; 16] }));

            channel
        });

    let mut env = test.env();

    env.expect_clone().times(..).returning(move || {
        let mut env = MockAdminEnvTrait::new();

        let mut function_mgr = MockFunctionMgr::new();
        function_mgr
            .expect_set_res_cnt()
            .times(1)
            .returning(move |_, _| Ok(0));

        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();

            function
                .expect_res_mask()
                .times(1)
                .return_const(0x100000000u128.to_be_bytes());

            function
        });

        env.expect_function_mgr()
            .times(..)
            .return_const(function_mgr);

        env
    });

    env
}

/// Create a mock admin FSM for create submission queue FSM being called through AdminFsm
pub(crate) fn make_env_for_admin_fsm_to_perform_create_sq(
    config: AdminFsmCreateSqTestConfigs,
) -> MockAdminEnvTrait {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x1);
    let mut create_sq: CreateSqSqe = sqe.into();
    create_sq.queue_id = config.host_sq;
    create_sq.queue_len = config.queue_len;
    create_sq.host_cq_id = config.host_cq;
    create_sq.attr.set_priority(config.queue_priority.into());
    create_sq.attr.set_pc(config.pc);

    let hsm_msg = matches!(HostQueueType::from(create_sq.queue_id), HostQueueType::Hsm);

    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: 0xF,
        status: IoTxCompleteStatus::Success,
    };
    let desc = Some(DmaTxnCompletionDesc {
        success: true,
        tag: 0xFF,
    });

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if !hsm_msg {
                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| Ok(()));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(|| Some(IpcMessage { data: [0; 16] }));
            }

            channel
        });
    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, create_sq.into(), true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                let queue = AdminQueue::new(QueueCntrlId::Pf.into(), QueueCntrlId::Pf.into());

                Some(queue)
            });

            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(move || desc);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Ok(()));
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(move || Some(send_complete_desc));

    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    test.hsm_ipc_channel()
        .expect_clone()
        .times(..)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if hsm_msg {
                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| Ok(()));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(|| Some(IpcMessage { data: [0; 16] }));
            }

            channel
        });

    let mut env = test.env();

    env.expect_clone().times(..).returning(move || {
        let mut env = MockAdminEnvTrait::new();

        let mut function_mgr = MockFunctionMgr::new();
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();

            function
                .expect_create_sq()
                .times(1)
                .returning(move |_, _, _| Ok((DevSqId::Id1, DevCqId::Id1)));

            function
        });

        env.expect_function_mgr()
            .times(..)
            .return_const(function_mgr);

        env
    });

    env
}

/// Create a mock admin FSM for delete submission queue FSM being called through AdminFsm
pub(crate) fn make_env_for_admin_fsm_to_perform_delete_sq(
    config: AdminFsmDeleteSqTestConfigs,
) -> MockAdminEnvTrait {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(0x0);
    let mut delete_sq: DeleteSqSqe = sqe.into();
    delete_sq.queue_id = config.host_sq;

    let hsm_msg = matches!(HostQueueType::from(delete_sq.queue_id), HostQueueType::Hsm);

    let send_complete_desc = IoTxCompleteDesc {
        queue_id: Default::default(),
        queue_index: Default::default(),
        tag: 0xF,
        status: IoTxCompleteStatus::Success,
    };
    let desc = Some(DmaTxnCompletionDesc {
        success: true,
        tag: 0xFF,
    });

    test.admin_to_fp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if !hsm_msg {
                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| Ok(()));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(|| Some(IpcMessage { data: [0; 16] }));
            }

            channel
        });
    test.admin_to_hsp_ipc_channel()
        .expect_clone()
        .times(1)
        .returning(MockIpcMessageChannel::new);

    test.io_channel()
        .expect_begin_recv()
        .times(1)
        .return_once(move || make_rx_desc(PcieFunction::Pf, delete_sq.into(), true));
    test.function_mgr()
        .expect_function()
        .times(1)
        .returning(move |_| {
            let mut function = MockFunction::new();

            function.expect_admin_queue().times(1).returning(|| {
                let queue = AdminQueue::new(QueueCntrlId::Pf.into(), QueueCntrlId::Pf.into());

                Some(queue)
            });

            function
        });
    test.dma_channel()
        .expect_begin_txn()
        .times(1)
        .return_const(Ok(()));
    test.dma_channel()
        .expect_end_txn()
        .times(1)
        .return_once(move || desc);
    test.io_channel()
        .expect_begin_send()
        .times(1)
        .return_once(|_| Ok(()));
    test.io_channel()
        .expect_end_send()
        .times(1)
        .return_once(move || Some(send_complete_desc));

    test.io_channel()
        .expect_end_recv()
        .times(1)
        .return_once(|_, _| ());

    test.hsm_ipc_channel()
        .expect_clone()
        .times(..)
        .returning(move || {
            let mut channel = MockIpcMessageChannel::new();

            if hsm_msg {
                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| Ok(()));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(|| Some(IpcMessage { data: [0; 16] }));
            }

            channel
        });

    let mut env = test.env();

    env.expect_clone().times(..).returning(move || {
        let mut env = MockAdminEnvTrait::new();

        let mut function_mgr = MockFunctionMgr::new();
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();
            function
                .expect_dev_sq()
                .times(1)
                .return_const(Ok(DevSqId::Id1));

            function
        });
        function_mgr.expect_function().times(1).returning(|_| {
            let mut function = MockFunction::new();
            function
                .expect_delete_sq()
                .times(1)
                .return_const(Ok((DevSqId::Id1, DevCqId::Id1)));

            function
        });

        env.expect_function_mgr()
            .times(..)
            .return_const(function_mgr);

        env
    });

    env
}

pub(crate) fn make_fsm_for_vf_prepare(
    test_configs: VfPrepareCmdTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    default_resource_expectations(&mut test);

    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_prep: VfPrepSqe = sqe.into();
    vf_prep.cntrl_id = test_configs.cntrl_id;

    let ctx = get_context(test);

    (ctx, vf_prep.into())
}

pub(crate) fn make_fsm_for_vf_restore(
    test_configs: VfRestoreCmdTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_restore: VfRestoreSqe = sqe.into();
    vf_restore.cntrl_id = test_configs.cntrl_id;

    let (env, sqe) = make_env_for_vf_restore_fsm(test_configs);
    let ctx = get_context_from_environment(env);

    (ctx, sqe)
}

pub(crate) fn make_env_for_vf_restore_fsm(
    test_configs: VfRestoreCmdTestConfigs,
) -> (MockAdminEnvTrait, AdminSqe) {
    let mut test = AdminFsmTest::default();
    let mut valid_cntrl_id = test_configs.cntrl_id > 0 && test_configs.cntrl_id <= 64;
    if test_configs.unknown_event {
        valid_cntrl_id = false;
    }
    let mut persistent_store_expectations = false;

    if valid_cntrl_id {
        test.admin_to_hsp_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);
    } else {
        default_resource_expectations(&mut test);
    }

    test.dma_heap()
        .expect_allocate()
        .times(1)
        .return_once(move |s| Some(MockDmaAlloc::new(s)));

    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_restore: VfRestoreSqe = sqe.into();
    vf_restore.cntrl_id = test_configs.cntrl_id;

    if test_configs.fp_ipc_send_req_failure
        || test_configs.none_fp_ipc_resp
        || test_configs.invalid_fp_ipc_resp
    {
        test.hsm_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);
        test.admin_to_fp_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();

                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| {
                        if test_configs.fp_ipc_send_req_failure {
                            Err(u32::MAX)
                        } else {
                            Ok(())
                        }
                    });

                if !test_configs.fp_ipc_send_req_failure {
                    channel
                        .expect_receive_message()
                        .times(1)
                        .returning(move || {
                            if test_configs.none_fp_ipc_resp {
                                None
                            } else {
                                let mut ipc_message = IpcMessage { data: [0; 16] };
                                if test_configs.invalid_fp_ipc_resp {
                                    ipc_message.data[0] = 0x000F0000; // Set error status
                                }

                                Some(ipc_message)
                            }
                        });
                }

                channel
            });
    } else if valid_cntrl_id {
        test.admin_to_fp_ipc_channel()
            .expect_clone()
            .once()
            .returning(move || {
                let mut channel = MockIpcMessageChannel::new();
                let ipc_message = IpcMessage { data: [0; 16] };

                channel
                    .expect_send_request()
                    .times(1)
                    .returning(move |_, _| Ok(()));
                channel
                    .expect_receive_message()
                    .times(1)
                    .returning(move || Some(ipc_message));

                channel
            });

        if test_configs.hsm_ipc_send_req_failure
            || test_configs.none_hsm_ipc_resp
            || test_configs.invalid_hsm_ipc_resp
        {
            test.hsm_ipc_channel()
                .expect_clone()
                .once()
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(1)
                        .returning(move |_, _| {
                            if test_configs.hsm_ipc_send_req_failure {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });

                    if !test_configs.hsm_ipc_send_req_failure {
                        channel
                            .expect_receive_message()
                            .times(1)
                            .returning(move || {
                                if test_configs.none_hsm_ipc_resp {
                                    None
                                } else {
                                    let mut ipc_message = IpcMessage { data: [0; 16] };
                                    if test_configs.invalid_hsm_ipc_resp {
                                        ipc_message.data[0] = 0x000F0000; // Set error status
                                    }
                                    Some(ipc_message)
                                }
                            });
                    }

                    channel
                });
        } else {
            test.hsm_ipc_channel()
                .expect_clone()
                .once()
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();
                    let ipc_message = IpcMessage { data: [0; 16] };

                    channel
                        .expect_send_request()
                        .times(1)
                        .returning(move |_, _| Ok(()));
                    channel
                        .expect_receive_message()
                        .times(1)
                        .returning(move || Some(ipc_message));

                    channel
                });

            test.function_mgr()
                .expect_function()
                .times(1)
                .returning(move |_| {
                    let mut function = MockFunction::new();

                    function
                        .expect_complete_live_migration()
                        .times(1)
                        .returning(|| ());

                    function
                });

            test.msix_cntrl()
                .expect_enable_pcie_fn()
                .times(1)
                .returning(|_| ());

            persistent_store_expectations = true;
            test.function_mgr()
                .expect_function()
                .times(1)
                .returning(move |_| {
                    let mut function = MockFunction::new();
                    function
                        .expect_restore_lm_context()
                        .times(1)
                        .returning(move |_| Ok(()));

                    function
                });
        }
    }

    let mut env = test.env();

    if persistent_store_expectations {
        assert!(test_configs.persistent_store_addr.is_some());

        env.expect_hsm_part_persistent_store_addr()
            .times(1)
            .returning(move || test_configs.persistent_store_addr.unwrap());
    }

    (env, vf_restore.into())
}

pub(crate) fn make_fsm_for_vf_save(
    test_configs: VfSaveCmdTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    default_resource_expectations(&mut test);

    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_prep: VfSaveSqe = sqe.into();
    vf_prep.cntrl_id = test_configs.cntrl_id;

    let mut valid_cntrl_id = test_configs.cntrl_id > 0 && test_configs.cntrl_id <= 64;
    if test_configs.unknown_event {
        valid_cntrl_id = false;
    }

    if test_configs.dma_alloc_fail {
        test.dma_heap()
            .expect_allocate()
            .times(1)
            .return_once(move |_| None);
    } else if valid_cntrl_id {
        test.dma_heap()
            .expect_allocate()
            .times(1)
            .return_once(move |s| Some(MockDmaAlloc::new(s)));

        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_save_lm_context()
                    .times(1)
                    .returning(move |_, _, _, _| ());

                function
            });

        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();
                function
                    .expect_clear_enable()
                    .times(1)
                    .returning(move || ());

                function
            });
    }

    let mut env = test.env();

    if !test_configs.dma_alloc_fail && valid_cntrl_id {
        let persistent_store = [0u8; 2048];
        env.expect_hsm_part_persistent_store_addr()
            .times(1)
            .returning(move || persistent_store.as_ptr() as usize);
    }

    let ctx = get_context_from_environment(env);

    (ctx, vf_prep.into())
}

pub(crate) fn make_fsm_for_vf_stop(
    test_configs: VfStopCmdTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_prep: VfSaveSqe = sqe.into();
    vf_prep.cntrl_id = test_configs.cntrl_id;

    let mut valid_cntrl_id = test_configs.cntrl_id > 0 && test_configs.cntrl_id <= 64;
    if test_configs.unknown_event {
        valid_cntrl_id = false;
    }

    if valid_cntrl_id {
        test.admin_to_hsp_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);

        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();

                function
                    .expect_get_enabled_sq_info()
                    .times(1)
                    .returning(move || {
                        let mut sq_info_list = Vec::new();

                        for i in 0..test_configs.num_res_grps {
                            let hsm_host_sq_start = 1;
                            let fp_host_sq_start = 256;
                            let hsm_dev_start = 65;
                            let fp_dev_start = 0;
                            // HSM SQ info
                            sq_info_list.push((
                                HostSqId(hsm_host_sq_start as u16 + i as u16),
                                DevSqId(hsm_dev_start as u8 + i as u8),
                                DevCqId(hsm_dev_start as u8 + i as u8),
                            ));

                            // FP SQ info
                            sq_info_list.push((
                                HostSqId(fp_host_sq_start as u16 + i as u16),
                                DevSqId(fp_dev_start as u8 + i as u8),
                                DevCqId(fp_dev_start as u8 + i as u8),
                            ));
                            sq_info_list.push((
                                HostSqId(fp_host_sq_start as u16 + 1 + i as u16),
                                DevSqId(fp_dev_start as u8 + 65 + i as u8),
                                DevCqId(fp_dev_start as u8 + 65 + i as u8),
                            ));
                        }

                        sq_info_list
                    });

                function
            });

        if test_configs.fp_ipc_send_req_failure
            || test_configs.none_fp_ipc_resp
            || test_configs.invalid_fp_ipc_resp
        {
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);

            test.function_mgr()
                .expect_function()
                .times(1)
                .returning(move |_| {
                    let mut function = MockFunction::new();

                    function
                        .expect_disable_sq()
                        .times(1)
                        .returning(move |_, _| ());

                    function
                });

            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(1)
                        .returning(move |_, _| {
                            if test_configs.none_fp_ipc_resp || test_configs.invalid_fp_ipc_resp {
                                Ok(())
                            } else {
                                Err(u32::MAX)
                            }
                        });

                    if test_configs.none_fp_ipc_resp {
                        channel.expect_receive_message().times(1).returning(|| None);
                    } else if test_configs.invalid_fp_ipc_resp {
                        let mut message = [0; 16];
                        message[0] = 0x000F0000;

                        channel
                            .expect_receive_message()
                            .times(1)
                            .returning(move || Some(IpcMessage { data: message }));
                    }

                    channel
                });

            if test_configs.fp_ipc_send_req_failure {
                test.function_mgr()
                    .expect_function()
                    .times(1)
                    .returning(move |_| {
                        let mut function = MockFunction::new();

                        function
                            .expect_enable_sq()
                            .times(1)
                            .returning(move |_, _| ());

                        function
                    });
            }
        } else {
            test.function_mgr()
                .expect_function()
                .times(test_configs.num_res_grps * 3)
                .returning(move |_| {
                    let mut function = MockFunction::new();

                    function
                        .expect_disable_sq()
                        .times(1)
                        .returning(move |_, _| ());

                    function
                });

            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(test_configs.num_res_grps * 2)
                        .returning(move |_, _| Ok(()));
                    channel
                        .expect_receive_message()
                        .times(test_configs.num_res_grps * 2)
                        .returning(|| Some(IpcMessage { data: [0; 16] }));

                    channel
                });

            if test_configs.hsm_ipc_send_req_failure {
                test.function_mgr()
                    .expect_function()
                    .times(1)
                    .returning(move |_| {
                        let mut function = MockFunction::new();

                        function
                            .expect_enable_sq()
                            .times(1)
                            .returning(move |_, _| ());

                        function
                    });
            }
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(test_configs.num_res_grps)
                        .returning(move |_, _| {
                            if test_configs.hsm_ipc_send_req_failure {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                    if !test_configs.hsm_ipc_send_req_failure {
                        let num_times_recv = if test_configs.invalid_hsm_ipc_resp
                            || test_configs.none_hsm_ipc_resp
                        {
                            1
                        } else {
                            test_configs.num_res_grps
                        };
                        channel
                            .expect_receive_message()
                            .times(num_times_recv)
                            .returning(move || {
                                let mut message = [0; 16];

                                if test_configs.deferred_completion {
                                    message[0] = 0x00060000;

                                    Some(IpcMessage { data: message })
                                } else if test_configs.none_hsm_ipc_resp {
                                    None
                                } else if test_configs.invalid_hsm_ipc_resp {
                                    message[0] = 0x000F0000;

                                    Some(IpcMessage { data: message })
                                } else {
                                    Some(IpcMessage { data: message })
                                }
                            });
                    }
                    channel
                });

            if test_configs.deferred_completion {
                test.deferred_queue_delete_pipe()
                    .expect_recv()
                    .times(test_configs.num_res_grps)
                    .returning(move || test_configs.deferred_completion_message);
            }
        }
    } else {
        default_resource_expectations(&mut test);
    }

    let ctx = get_context(test);

    (ctx, vf_prep.into())
}

pub(crate) fn make_fsm_for_vf_start(
    test_configs: VfStartCmdTestConfigs,
) -> (AdminFsmContext<MockAdminEnvTrait>, AdminSqe) {
    let mut test = AdminFsmTest::default();

    let sqe = make_sqe_with_opcode(test_configs.opcode);
    let mut vf_prep: VfStartSqe = sqe.into();
    vf_prep.cntrl_id = test_configs.cntrl_id;

    let mut valid_cntrl_id = test_configs.cntrl_id > 0 && test_configs.cntrl_id <= 64;
    if test_configs.unknown_event {
        valid_cntrl_id = false;
    }

    if valid_cntrl_id {
        test.admin_to_hsp_ipc_channel()
            .expect_clone()
            .times(1)
            .returning(MockIpcMessageChannel::new);

        test.function_mgr()
            .expect_function()
            .times(1)
            .returning(move |_| {
                let mut function = MockFunction::new();

                function
                    .expect_get_enabled_sq_info()
                    .times(1)
                    .returning(move || {
                        let mut sq_info_list = Vec::new();

                        for i in 0..test_configs.num_res_grps {
                            let hsm_host_sq_start = 1;
                            let fp_host_sq_start = 256;
                            let hsm_dev_start = 65;
                            let fp_dev_start = 0;
                            // HSM SQ info
                            sq_info_list.push((
                                HostSqId(hsm_host_sq_start as u16 + i as u16),
                                DevSqId(hsm_dev_start as u8 + i as u8),
                                DevCqId(hsm_dev_start as u8 + i as u8),
                            ));

                            // FP SQ info
                            sq_info_list.push((
                                HostSqId(fp_host_sq_start as u16 + i as u16),
                                DevSqId(fp_dev_start as u8 + i as u8),
                                DevCqId(fp_dev_start as u8 + i as u8),
                            ));
                            sq_info_list.push((
                                HostSqId(fp_host_sq_start as u16 + 1 + i as u16),
                                DevSqId(fp_dev_start as u8 + 65 + i as u8),
                                DevCqId(fp_dev_start as u8 + 65 + i as u8),
                            ));
                        }

                        sq_info_list
                    });

                function
            });

        if test_configs.fp_ipc_send_req_failure
            || test_configs.none_fp_ipc_resp
            || test_configs.invalid_fp_ipc_resp
        {
            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(MockIpcMessageChannel::new);

            test.function_mgr()
                .expect_function()
                .times(1)
                .returning(move |_| {
                    let mut function = MockFunction::new();

                    function
                        .expect_enable_sq()
                        .times(1)
                        .returning(move |_, _| ());

                    function
                });

            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(1)
                        .returning(move |_, _| {
                            if test_configs.none_fp_ipc_resp || test_configs.invalid_fp_ipc_resp {
                                Ok(())
                            } else {
                                Err(u32::MAX)
                            }
                        });

                    if test_configs.none_fp_ipc_resp {
                        channel.expect_receive_message().times(1).returning(|| None);
                    } else if test_configs.invalid_fp_ipc_resp {
                        let mut message = [0; 16];
                        message[0] = 0x000F0000;

                        channel
                            .expect_receive_message()
                            .times(1)
                            .returning(move || Some(IpcMessage { data: message }));
                    }

                    channel
                });
        } else {
            test.function_mgr()
                .expect_function()
                .times(test_configs.num_res_grps * 3)
                .returning(move |_| {
                    let mut function = MockFunction::new();

                    function
                        .expect_enable_sq()
                        .times(1)
                        .returning(move |_, _| ());

                    function
                });

            test.admin_to_fp_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(test_configs.num_res_grps * 2)
                        .returning(move |_, _| Ok(()));
                    channel
                        .expect_receive_message()
                        .times(test_configs.num_res_grps * 2)
                        .returning(|| Some(IpcMessage { data: [0; 16] }));

                    channel
                });

            test.hsm_ipc_channel()
                .expect_clone()
                .times(1)
                .returning(move || {
                    let mut channel = MockIpcMessageChannel::new();

                    channel
                        .expect_send_request()
                        .times(test_configs.num_res_grps)
                        .returning(move |_, _| {
                            if test_configs.hsm_ipc_send_req_failure {
                                Err(u32::MAX)
                            } else {
                                Ok(())
                            }
                        });
                    if !test_configs.hsm_ipc_send_req_failure {
                        let num_times_recv = if test_configs.invalid_hsm_ipc_resp
                            || test_configs.none_hsm_ipc_resp
                        {
                            1
                        } else {
                            test_configs.num_res_grps
                        };
                        channel
                            .expect_receive_message()
                            .times(num_times_recv)
                            .returning(move || {
                                let mut message = [0; 16];

                                if test_configs.none_hsm_ipc_resp {
                                    None
                                } else if test_configs.invalid_hsm_ipc_resp {
                                    message[0] = 0x000F0000;

                                    Some(IpcMessage { data: message })
                                } else {
                                    Some(IpcMessage { data: message })
                                }
                            });
                    }
                    channel
                });

            if !test_configs.hsm_ipc_send_req_failure
                && !test_configs.none_hsm_ipc_resp
                && !test_configs.invalid_hsm_ipc_resp
            {
                test.msix_cntrl()
                    .expect_enable_pcie_fn()
                    .times(1)
                    .returning(|_| ());
            }
        }
    } else {
        default_resource_expectations(&mut test);
    }

    let ctx = get_context(test);

    (ctx, vf_prep.into())
}

/// Create a mock AES GCM EXT FSM context for invalid request entry
pub(crate) fn make_fsm_for_aes_gcm_ext_invalid_request_config(
    cfg: AesGcmExtTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let req_pipe = test.aes_gcm_req_queue();
    req_pipe.expect_recv().once().returning(move || {
        (!cfg.missing_request).then(|| {
            AesGcmReqEntry::new()
                .with_pfn(cfg.pfn)
                .with_sqe_addr(cfg.sqe_addr as u64)
                .with_sqe_idx(cfg.sqe_idx)
        })
    });

    if cfg.response_error {
        let resp_pipe = test.aes_gcm_resp_queue();
        resp_pipe.expect_send().once().returning(|_| Err(0x130001));
    } else if !cfg.missing_request {
        let resp_pipe = test.aes_gcm_resp_queue();
        resp_pipe.expect_send().once().returning(|_| Ok(()));
    }

    let env = test.env();
    get_context_from_environment(env)
}

/// Create a mock AES GCM EXT FSM context for invalid unaligned data length
pub(crate) fn make_fsm_for_aes_gcm_ext_invalid_unaligned_data_length(
    cfg: AesGcmExtTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let req_pipe = test.aes_gcm_req_queue();
    req_pipe.expect_recv().once().returning(move || {
        Some(
            AesGcmReqEntry::new()
                .with_pfn(cfg.pfn)
                .with_sqe_addr(cfg.sqe_addr as u64)
                .with_sqe_idx(cfg.sqe_idx),
        )
    });

    let resp_pipe = test.aes_gcm_resp_queue();
    resp_pipe.expect_send().once().returning(|_| Ok(()));

    get_context(test)
}

/// Create a mock AES GCM EXT FSM context for valid request entry with unaligned data length zero
pub(crate) fn make_fsm_for_aes_gcm_ext_unaligned_data_len_zero(
    cfg: AesGcmExtTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let req_pipe = test.aes_gcm_req_queue();
    req_pipe.expect_recv().once().returning(move || {
        Some(
            AesGcmReqEntry::new()
                .with_pfn(cfg.pfn)
                .with_sqe_addr(cfg.sqe_addr as u64)
                .with_sqe_idx(cfg.sqe_idx),
        )
    });

    let resp_pipe = test.aes_gcm_resp_queue();
    resp_pipe.expect_send().once().returning(|_| Ok(()));

    let dma_heap = test.dma_heap();
    dma_heap
        .expect_allocate()
        .once()
        .returning(|size| Some(MockDmaAlloc::new(size)));

    let soft = test.soft_aes();
    soft.expect_aes_gcm_tag_correction()
        .once()
        .returning(move |_, _, _, _, _, _, _, _, _, _| {
            if cfg.tag_correction_fail {
                // SoftAesErr::AesGcmTagCorrectionFailed
                Err(0x140014)
            } else if cfg.tag_mismatch_error {
                // Unexpected tag for mismatch
                Ok([2u8; 16])
            } else {
                // Expected tag
                Ok([0u8; 16])
            }
        });

    get_context(test)
}

/// Create a mock AES GCM EXT FSM context for valid request entry with unaligned data length nonzero
pub(crate) fn make_fsm_for_aes_gcm_ext_unaligned_data_len_nonzero(
    cfg: AesGcmExtTestConfigs,
) -> AdminFsmContext<MockAdminEnvTrait> {
    let mut test = AdminFsmTest::default();
    default_resource_expectations(&mut test);

    let req_pipe = test.aes_gcm_req_queue();
    req_pipe.expect_recv().once().returning(move || {
        Some(
            AesGcmReqEntry::new()
                .with_pfn(cfg.pfn)
                .with_sqe_addr(cfg.sqe_addr as u64)
                .with_sqe_idx(cfg.sqe_idx),
        )
    });

    let resp_pipe = test.aes_gcm_resp_queue();
    resp_pipe.expect_send().once().returning(|_| Ok(()));

    let dma_heap = test.dma_heap();
    dma_heap
        .expect_allocate()
        .once()
        .returning(|size| Some(MockDmaAlloc::new(size)));

    if !cfg.unaligned_src_data_ptr_null {
        setup_dma_channel_expectations(&mut test, &cfg);

        let is_dma_in_success =
            !cfg.dma_in_begin_txn_fail && !cfg.dma_in_end_txn_fail && !cfg.dma_in_end_txn_empty;

        if is_dma_in_success {
            let dma_heap = test.dma_heap();
            dma_heap
                .expect_allocate()
                .once()
                .returning(|size| Some(MockDmaAlloc::new(size)));

            let soft = test.soft_aes();
            soft.expect_aes_gcm_tag_correction().once().returning(
                move |_, _, _, _, _, _, _, _, _, _| {
                    if cfg.tag_correction_fail {
                        // SoftAesErr::AesGcmTagCorrectionFailed
                        Err(0x140014)
                    } else {
                        // Expected tag
                        Ok([0u8; 16])
                    }
                },
            );

            return get_context(test);
        }
    }

    get_context(test)
}

/// Helper function for AES GCM EXT FSM DMA channel expectation configuration
fn setup_dma_channel_expectations(test: &mut AdminFsmTest, cfg: &AesGcmExtTestConfigs) {
    let dma_channel = test.dma_channel();
    let cfg = *cfg;

    let mut begin_call_count = 0;
    dma_channel
        .expect_begin_txn()
        .times(cfg.dma_begin_txn_count)
        .returning(move |_| {
            begin_call_count += 1;
            let is_fail = (begin_call_count == 1 && cfg.dma_in_begin_txn_fail)
                || (begin_call_count == 2 && cfg.dma_out_begin_txn_fail);

            if is_fail {
                Err(0xB0038) // AdminErr::ExpectedDmaBuf
            } else {
                Ok(())
            }
        });

    let mut end_call_count = 0;
    dma_channel
        .expect_end_txn()
        .times(cfg.dma_end_txn_count)
        .returning(move || {
            end_call_count += 1;
            if end_call_count == 1 {
                if cfg.dma_in_end_txn_empty {
                    None
                } else {
                    Some(DmaTxnCompletionDesc {
                        success: !cfg.dma_in_end_txn_fail,
                        tag: cfg.tag,
                    })
                }
            } else if cfg.dma_out_end_txn_empty {
                None
            } else {
                Some(DmaTxnCompletionDesc {
                    success: !cfg.dma_out_end_txn_fail,
                    tag: cfg.tag,
                })
            }
        });
}
