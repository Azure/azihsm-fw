// Copyright (c) Microsoft Corporation. All rights reserved.
// This is an auto-generated file. Please do not modify manually.
// To regenerate use command: `cargo xtask telemetry-tokenize`

use hashbrown::HashMap;

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_MAP: HashMap<&'static str, u8> = {
        let mut m = HashMap::new();
        m.insert("Failed the self test {:?}", 0);
        m.insert("Preop-Self test failed for {:?} with error {}", 1);
        m.insert("[doe] on_rx_ready Failed to recv DOE message: {:?}", 2);
        m.insert("[doe] on_doe_go: Failed to recv DOE message: {:?}", 3);
        m.insert("Failed to end recv DOE message: {:?}", 4);
        m.insert("Failed to decode IPC message: {:?}", 5);
        m.insert("[doe] Invalid IPC response with status {}", 6);
        m.insert("[doe] Spurious Message", 7);
        m.insert("[doe] on_ipc_response: Failed to send DOE message: {:?}", 8);
        m.insert("[doe] on_tx_ready: Failed to send DOE message: {:?}", 9);
        m.insert("Failed to send IPC message to HSP: {:?}", 10);
        m.insert("[get_res] Invalid SQE: {:?}", 11);
        m.insert("[delete_cq] Invalid SQE: {:?}", 12);
        m.insert("Create Cq Failed {:?}", 13); // Deprecated message
        m.insert("[set_res] Invalid SQE: {:?}", 14);
        m.insert("Failed to set resource count: {:?}", 15);
        m.insert("[set_res] Invalid IPC response with status {}", 16);
        m.insert("[delete_sq] Invalid SQE: {:?}", 17);
        m.insert("Get device Sq Failed {:?}", 18);
        m.insert("Process HSM IPC failed {:?}", 19);
        m.insert("[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}", 20);
        m.insert("Process FP IPC failed {:?}", 21);
        m.insert("[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}", 22);
        m.insert("[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}", 23);
        m.insert("Failed to send FP IPC request {:?}", 24);
        m.insert("Failed to send HSM IPC request {:?}", 25);
        m.insert("[set_features] Invalid SQE: {:?}", 26);
        m.insert("Invalid state transition. Current state = {}", 27);
        m.insert("[admin-fsm] DMA begin_txn error. (0x{:08x})", 28); // Deprecated message
        m.insert("[admin-fsm] send_err_cqe IO channel send error. (0x{:08x})", 29); // Deprecated message
        m.insert("[admin-fsm] send_cqe IO channel send error. (0x{:08x})", 30); // Deprecated message
        m.insert("Telemetry FSM received unexpected event. Event: {:?}", 31);
        m.insert("Error running telemetry PCIe monitor, code {:#x}", 32);
        m.insert("Starting PCIe telemetry timer", 33); // Deprecated message
        m.insert("Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}", 34);
        m.insert("Unexpected PCIe Link. Link Speed: {}, Link Width: {}", 35);
        m.insert("Telemetry PCIe monitor not able to start, code {:#x}", 36);
        m.insert("Failed to receive IPC message from HSM", 37);
        m.insert("Failed to decode IPC message from HSM", 38);
        m.insert("Failed to receive IPC message from FP", 39);
        m.insert("Failed to decode IPC message from FP", 40);
        m.insert("Fail to receive queue_delete_notification", 41);
        m.insert("Mismatch tags: received ({:?}) vs expected ({:?})", 42);
        m.insert("Error in enabling controller {:?}", 43);
        m.insert("[cntrl] Failed to send IPC message to FP: {:?}", 44);
        m.insert("[cntrl] Failed to send IPC message to HSM: {:?}", 45);
        m.insert("[get_features] Invalid SQE: {:?}", 46);
        m.insert("Late IPC_HSM_SHUTDOWN_RSP after {}us", 47);
        m.insert("Late IPC_FP_SHUTDOWN_RSP after {}us", 48);
        m.insert("Late resource {} acquire {}us", 49);
        m.insert("Starting", 50);
        m.insert("Stopped after {}us", 51);
        m.insert("Admin abort with status: {:?}. State: ({:?})", 52);
        m.insert("HSM abort with status: {:?}. State: ({:?})", 53);
        m.insert("FP abort with status: {:?}. State: ({:?})", 54);
        m.insert("error_dfu: IPC_SP_SHUTDOWN_RSP {:?}", 55);
        m.insert("Timeout in state {:?} resources: {:#x}", 56);
        m.insert("handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}", 57);
        m.insert("Insufficient time to drain CP HSM/FP.", 58);
        m.insert("IPC_CP_HSM_SHUTDOWN_REQ {:#x}", 59);
        m.insert("IPC_FP_SHUTDOWN_REQ {:#x}", 60);
        m.insert("Ready in {} us", 61);
        m.insert("iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}", 62);
        m.insert("iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}", 63);
        m.insert("IPC_SP_SHUTDOWN_RSP {:#x}", 64);
        m.insert("No reset detected.", 65);
        m.insert("Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us", 66);
        m.insert("Spurious IPC_FP_SHUTDOWN_RSP after {}us", 67);
        m.insert("Received timer event while waiting for cast resource", 68);
        m.insert("Invalid event. state: {:?}", 69);
        m.insert("on_fp_to_admin_channel_ready: Invalid state {:?}", 70);
        m.insert("on_fp_to_admin_ipc_response: Invalid state {:?}", 71);
        m.insert("Failed to receive self test response for {:?}", 72); // Deprecated message
        m.insert("inspect_result: Self test {:?} FAILED with error code: {:?}", 73); // Deprecated message
        m.insert("check_for_err: Self test {:?} FAILED with error code: {:?}", 74); // Deprecated message
        m.insert("[vflr] Failed to send IPC message to FP: {:?}", 75);
        m.insert("[vflr] Failed to send IPC message to HSM: {:?}", 76);
        m.insert("PCIE_PERST_UP Error: 0x{:08x}", 77);
        m.insert("Link Speed={} Width=x{}", 78); // Deprecated message
        m.insert("FLR complete", 79); // Deprecated message
        m.insert("Perst Down complete", 80);
        m.insert("IPC response status: {:x}", 81);
        m.insert("Starting Admin event loop...", 82);
        m.insert("Invalid IPC message header", 83);
        m.insert("Cannot decode opcode. Invalid IPC message opcode", 84);
        m.insert("Opcode does not match. Invalid IPC message opcode", 85);
        m.insert("Invalid IPC message", 86);
        m.insert("Invalid IPC message for Negative self test.", 87);
        m.insert("Invalid test ID in negative self test IPC payload.", 88);
        m.insert("Invalid Header found in IPC message", 89);
        m.insert("Invalid Shutdown for reset request message", 90);
        m.insert("handle_invalid_message_opcode: Invalid IPC message header {:?}", 91);
        m.insert("Unhandled message with opcode {:#X}", 92);
        m.insert("Failed to send response to HSP core {:?}", 93);
        m.insert("send_hsm_ipc_response: Invalid IPC message header {:?}", 94);
        m.insert("Failed to send response to hsm core {:?}", 95);
        m.insert("DTCM error received. Fault code: {}", 96);
        m.insert("Delete Cq Failed {:?}", 97);
        m.insert("Complete VFLR for {:?}", 98);
        m.insert("VF Restore for PCIe function: {:?}", 99);
        m.insert("[Vf Restore] Unsupported event", 100);
        m.insert("VF Start for PCIe function: {:?}", 101);
        m.insert("[Vf_Start] Unsupported event, expected StartCmd", 102);
        m.insert("PF FLR complete", 103);
        m.insert("VF Stop for PCIe function: {:?}", 104);
        m.insert("[Vf_Stop] Unsupported event, expected StartCmd", 105);
        m.insert("VF Save for PCIe function: {:?}", 106);
        m.insert("[Vf Save] Unsupported event", 107);
        m.insert("VF Prepare for PCIe function: {:?}", 108);
        m.insert("[Vf Prepare] Unsupported event", 109);
        m.insert("[stop_interface] Timer elapsed, state: {:?}", 110);
        m.insert("[stop_interface] Failed to receive notification from queue", 111);
        m.insert("[stop_interface] Message tag {} is not expected {}", 112);
        m.insert("[stop_interface] Failed to send IPC message to FP: {:?}", 113);
        m.insert("[stop_interface] Failed to send IPC message to HSM: {:?}", 114);
        m.insert("[stop_interface] Failed to send response to HSP: {:?}", 115);
        m.insert("[tdisp_int] TdispIntFsm timer elapsed, state: {}", 116);
        m.insert("[tdisp_int] Invalid interrupt source, source_mask: {}", 117);
        m.insert("[tdisp_int] Failed to send IPC message to HSP: {:?}", 118);
        m.insert("[tdisp_int] Failed to decode IPC message: {:?}", 119);
        m.insert("[tdisp_int] Invalid IPC response with status {}", 120);
        m.insert("[tdisp_int] Spurious Message", 121);
        m.insert("Received deferred action for a different controller: {:?}", 122);
        m.insert("ITCM error received. Fault code: {}", 123);
        m.insert("Statemachine error {}", 124);
        m.insert("AES-GCM Decrypt Tag Mismatch", 125);
        m.insert("Failed to send AES GCM response entry to response queue", 126);
        m.insert("Invalid state transition in AesGcmExtFsm. Current state = {}", 127);
        m.insert("GDMA Data Structure Error. Fault code: {}", 128);
        m.insert("GDMA Data Access Error. Fault code: {}", 129);
        m.insert("GDMA Delivery Queue Error. Fault code: {}", 130);
        m.insert("GDMA Completion Queue Error. Fault code: {}", 131);
        m.insert("Failed to configure stack guard: invalid stack limit", 132);
        m.insert("Invalid IPC message for TriggerStackValidation", 133);
        m.insert("Invalid stack error type", 134);
        m.insert("Memory management fault received. CFSR={:#x}, MMFAR={:#x}", 135);
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "Failed the self test {:?}");
        m.insert(1, "Preop-Self test failed for {:?} with error {}");
        m.insert(2, "[doe] on_rx_ready Failed to recv DOE message: {:?}");
        m.insert(3, "[doe] on_doe_go: Failed to recv DOE message: {:?}");
        m.insert(4, "Failed to end recv DOE message: {:?}");
        m.insert(5, "Failed to decode IPC message: {:?}");
        m.insert(6, "[doe] Invalid IPC response with status {}");
        m.insert(7, "[doe] Spurious Message");
        m.insert(8, "[doe] on_ipc_response: Failed to send DOE message: {:?}");
        m.insert(9, "[doe] on_tx_ready: Failed to send DOE message: {:?}");
        m.insert(10, "Failed to send IPC message to HSP: {:?}");
        m.insert(11, "[get_res] Invalid SQE: {:?}");
        m.insert(12, "[delete_cq] Invalid SQE: {:?}");
        m.insert(13, "Create Cq Failed {:?}"); // Deprecated message
        m.insert(14, "[set_res] Invalid SQE: {:?}");
        m.insert(15, "Failed to set resource count: {:?}");
        m.insert(16, "[set_res] Invalid IPC response with status {}");
        m.insert(17, "[delete_sq] Invalid SQE: {:?}");
        m.insert(18, "Get device Sq Failed {:?}");
        m.insert(19, "Process HSM IPC failed {:?}");
        m.insert(20, "[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}");
        m.insert(21, "Process FP IPC failed {:?}");
        m.insert(22, "[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}");
        m.insert(23, "[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}");
        m.insert(24, "Failed to send FP IPC request {:?}");
        m.insert(25, "Failed to send HSM IPC request {:?}");
        m.insert(26, "[set_features] Invalid SQE: {:?}");
        m.insert(27, "Invalid state transition. Current state = {}");
        m.insert(28, "[admin-fsm] DMA begin_txn error. (0x{:08x})"); // Deprecated message
        m.insert(29, "[admin-fsm] send_err_cqe IO channel send error. (0x{:08x})"); // Deprecated message
        m.insert(30, "[admin-fsm] send_cqe IO channel send error. (0x{:08x})"); // Deprecated message
        m.insert(31, "Telemetry FSM received unexpected event. Event: {:?}");
        m.insert(32, "Error running telemetry PCIe monitor, code {:#x}");
        m.insert(33, "Starting PCIe telemetry timer"); // Deprecated message
        m.insert(34, "Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}");
        m.insert(35, "Unexpected PCIe Link. Link Speed: {}, Link Width: {}");
        m.insert(36, "Telemetry PCIe monitor not able to start, code {:#x}");
        m.insert(37, "Failed to receive IPC message from HSM");
        m.insert(38, "Failed to decode IPC message from HSM");
        m.insert(39, "Failed to receive IPC message from FP");
        m.insert(40, "Failed to decode IPC message from FP");
        m.insert(41, "Fail to receive queue_delete_notification");
        m.insert(42, "Mismatch tags: received ({:?}) vs expected ({:?})");
        m.insert(43, "Error in enabling controller {:?}");
        m.insert(44, "[cntrl] Failed to send IPC message to FP: {:?}");
        m.insert(45, "[cntrl] Failed to send IPC message to HSM: {:?}");
        m.insert(46, "[get_features] Invalid SQE: {:?}");
        m.insert(47, "Late IPC_HSM_SHUTDOWN_RSP after {}us");
        m.insert(48, "Late IPC_FP_SHUTDOWN_RSP after {}us");
        m.insert(49, "Late resource {} acquire {}us");
        m.insert(50, "Starting");
        m.insert(51, "Stopped after {}us");
        m.insert(52, "Admin abort with status: {:?}. State: ({:?})");
        m.insert(53, "HSM abort with status: {:?}. State: ({:?})");
        m.insert(54, "FP abort with status: {:?}. State: ({:?})");
        m.insert(55, "error_dfu: IPC_SP_SHUTDOWN_RSP {:?}");
        m.insert(56, "Timeout in state {:?} resources: {:#x}");
        m.insert(57, "handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}");
        m.insert(58, "Insufficient time to drain CP HSM/FP.");
        m.insert(59, "IPC_CP_HSM_SHUTDOWN_REQ {:#x}");
        m.insert(60, "IPC_FP_SHUTDOWN_REQ {:#x}");
        m.insert(61, "Ready in {} us");
        m.insert(62, "iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(63, "iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(64, "IPC_SP_SHUTDOWN_RSP {:#x}");
        m.insert(65, "No reset detected.");
        m.insert(66, "Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us");
        m.insert(67, "Spurious IPC_FP_SHUTDOWN_RSP after {}us");
        m.insert(68, "Received timer event while waiting for cast resource");
        m.insert(69, "Invalid event. state: {:?}");
        m.insert(70, "on_fp_to_admin_channel_ready: Invalid state {:?}");
        m.insert(71, "on_fp_to_admin_ipc_response: Invalid state {:?}");
        m.insert(72, "Failed to receive self test response for {:?}"); // Deprecated message
        m.insert(73, "inspect_result: Self test {:?} FAILED with error code: {:?}"); // Deprecated message
        m.insert(74, "check_for_err: Self test {:?} FAILED with error code: {:?}"); // Deprecated message
        m.insert(75, "[vflr] Failed to send IPC message to FP: {:?}");
        m.insert(76, "[vflr] Failed to send IPC message to HSM: {:?}");
        m.insert(77, "PCIE_PERST_UP Error: 0x{:08x}");
        m.insert(78, "Link Speed={} Width=x{}"); // Deprecated message
        m.insert(79, "FLR complete"); // Deprecated message
        m.insert(80, "Perst Down complete");
        m.insert(81, "IPC response status: {:x}");
        m.insert(82, "Starting Admin event loop...");
        m.insert(83, "Invalid IPC message header");
        m.insert(84, "Cannot decode opcode. Invalid IPC message opcode");
        m.insert(85, "Opcode does not match. Invalid IPC message opcode");
        m.insert(86, "Invalid IPC message");
        m.insert(87, "Invalid IPC message for Negative self test.");
        m.insert(88, "Invalid test ID in negative self test IPC payload.");
        m.insert(89, "Invalid Header found in IPC message");
        m.insert(90, "Invalid Shutdown for reset request message");
        m.insert(91, "handle_invalid_message_opcode: Invalid IPC message header {:?}");
        m.insert(92, "Unhandled message with opcode {:#X}");
        m.insert(93, "Failed to send response to HSP core {:?}");
        m.insert(94, "send_hsm_ipc_response: Invalid IPC message header {:?}");
        m.insert(95, "Failed to send response to hsm core {:?}");
        m.insert(96, "DTCM error received. Fault code: {}");
        m.insert(97, "Delete Cq Failed {:?}");
        m.insert(98, "Complete VFLR for {:?}");
        m.insert(99, "VF Restore for PCIe function: {:?}");
        m.insert(100, "[Vf Restore] Unsupported event");
        m.insert(101, "VF Start for PCIe function: {:?}");
        m.insert(102, "[Vf_Start] Unsupported event, expected StartCmd");
        m.insert(103, "PF FLR complete");
        m.insert(104, "VF Stop for PCIe function: {:?}");
        m.insert(105, "[Vf_Stop] Unsupported event, expected StartCmd");
        m.insert(106, "VF Save for PCIe function: {:?}");
        m.insert(107, "[Vf Save] Unsupported event");
        m.insert(108, "VF Prepare for PCIe function: {:?}");
        m.insert(109, "[Vf Prepare] Unsupported event");
        m.insert(110, "[stop_interface] Timer elapsed, state: {:?}");
        m.insert(111, "[stop_interface] Failed to receive notification from queue");
        m.insert(112, "[stop_interface] Message tag {} is not expected {}");
        m.insert(113, "[stop_interface] Failed to send IPC message to FP: {:?}");
        m.insert(114, "[stop_interface] Failed to send IPC message to HSM: {:?}");
        m.insert(115, "[stop_interface] Failed to send response to HSP: {:?}");
        m.insert(116, "[tdisp_int] TdispIntFsm timer elapsed, state: {}");
        m.insert(117, "[tdisp_int] Invalid interrupt source, source_mask: {}");
        m.insert(118, "[tdisp_int] Failed to send IPC message to HSP: {:?}");
        m.insert(119, "[tdisp_int] Failed to decode IPC message: {:?}");
        m.insert(120, "[tdisp_int] Invalid IPC response with status {}");
        m.insert(121, "[tdisp_int] Spurious Message");
        m.insert(122, "Received deferred action for a different controller: {:?}");
        m.insert(123, "ITCM error received. Fault code: {}");
        m.insert(124, "Statemachine error {}");
        m.insert(125, "AES-GCM Decrypt Tag Mismatch");
        m.insert(126, "Failed to send AES GCM response entry to response queue");
        m.insert(127, "Invalid state transition in AesGcmExtFsm. Current state = {}");
        m.insert(128, "GDMA Data Structure Error. Fault code: {}");
        m.insert(129, "GDMA Data Access Error. Fault code: {}");
        m.insert(130, "GDMA Delivery Queue Error. Fault code: {}");
        m.insert(131, "GDMA Completion Queue Error. Fault code: {}");
        m.insert(132, "Failed to configure stack guard: invalid stack limit");
        m.insert(133, "Invalid IPC message for TriggerStackValidation");
        m.insert(134, "Invalid stack error type");
        m.insert(135, "Memory management fault received. CFSR={:#x}, MMFAR={:#x}");
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_ADMIN_LOG_TOKENS_INDEX_TO_PREFIX_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "[mcr_admin::env]");
        m.insert(1, "[mcr_admin::env]");
        m.insert(2, "[mcr_admin::fsm::doe]");
        m.insert(3, "[mcr_admin::fsm::doe]");
        m.insert(4, "[mcr_admin::fsm::doe]");
        m.insert(5, "[mcr_admin::fsm::doe]");
        m.insert(6, "[mcr_admin::fsm::doe]");
        m.insert(7, "[mcr_admin::fsm::doe]");
        m.insert(8, "[mcr_admin::fsm::doe]");
        m.insert(9, "[mcr_admin::fsm::doe]");
        m.insert(10, "[mcr_admin::fsm::doe]");
        m.insert(11, "[mcr_admin::fsm::get_res]");
        m.insert(12, "[mcr_admin::fsm::delete_cq]");
        m.insert(13, "[mcr_admin::fsm::delete_cq]");
        m.insert(14, "[mcr_admin::fsm::set_res]");
        m.insert(15, "[mcr_admin::fsm::set_res]");
        m.insert(16, "[mcr_admin::fsm::set_res]");
        m.insert(17, "[mcr_admin::fsm::delete_sq]");
        m.insert(18, "[mcr_admin::fsm::delete_sq]");
        m.insert(19, "[mcr_admin::fsm::delete_sq]");
        m.insert(20, "[mcr_admin::fsm::delete_sq]");
        m.insert(21, "[mcr_admin::fsm::delete_sq]");
        m.insert(22, "[mcr_admin::fsm::delete_sq]");
        m.insert(23, "[mcr_admin::fsm::delete_sq]");
        m.insert(24, "[mcr_admin::fsm::delete_sq]");
        m.insert(25, "[mcr_admin::fsm::delete_sq]");
        m.insert(26, "[mcr_admin::fsm::set_features]");
        m.insert(27, "[mcr_admin::fsm::admin_fsm]");
        m.insert(28, "[mcr_admin::fsm::admin_fsm]");
        m.insert(29, "[mcr_admin::fsm::admin_fsm]");
        m.insert(30, "[mcr_admin::fsm::admin_fsm]");
        m.insert(31, "[mcr_admin::fsm::telemetry]");
        m.insert(32, "[mcr_admin::fsm::telemetry]");
        m.insert(33, "[mcr_admin::fsm::telemetry]");
        m.insert(34, "[mcr_admin::fsm::telemetry]");
        m.insert(35, "[mcr_admin::fsm::telemetry]");
        m.insert(36, "[mcr_admin::fsm::telemetry]");
        m.insert(37, "[mcr_admin::fsm::cntrl]");
        m.insert(38, "[mcr_admin::fsm::cntrl]");
        m.insert(39, "[mcr_admin::fsm::cntrl]");
        m.insert(40, "[mcr_admin::fsm::cntrl]");
        m.insert(41, "[mcr_admin::fsm::cntrl]");
        m.insert(42, "[mcr_admin::fsm::cntrl]");
        m.insert(43, "[mcr_admin::fsm::cntrl]");
        m.insert(44, "[mcr_admin::fsm::cntrl]");
        m.insert(45, "[mcr_admin::fsm::cntrl]");
        m.insert(46, "[mcr_admin::fsm::get_features]");
        m.insert(47, "[mcr_admin::fsm::idfu]");
        m.insert(48, "[mcr_admin::fsm::idfu]");
        m.insert(49, "[mcr_admin::fsm::idfu]");
        m.insert(50, "[mcr_admin::fsm::idfu]");
        m.insert(51, "[mcr_admin::fsm::idfu]");
        m.insert(52, "[mcr_admin::fsm::idfu]");
        m.insert(53, "[mcr_admin::fsm::idfu]");
        m.insert(54, "[mcr_admin::fsm::idfu]");
        m.insert(55, "[mcr_admin::fsm::idfu]");
        m.insert(56, "[mcr_admin::fsm::idfu]");
        m.insert(57, "[mcr_admin::fsm::idfu]");
        m.insert(58, "[mcr_admin::fsm::idfu]");
        m.insert(59, "[mcr_admin::fsm::idfu]");
        m.insert(60, "[mcr_admin::fsm::idfu]");
        m.insert(61, "[mcr_admin::fsm::idfu]");
        m.insert(62, "[mcr_admin::fsm::idfu]");
        m.insert(63, "[mcr_admin::fsm::idfu]");
        m.insert(64, "[mcr_admin::fsm::idfu]");
        m.insert(65, "[mcr_admin::fsm::idfu]");
        m.insert(66, "[mcr_admin::fsm::idfu]");
        m.insert(67, "[mcr_admin::fsm::idfu]");
        m.insert(68, "[mcr_admin::fsm::cast]");
        m.insert(69, "[mcr_admin::fsm::cast]");
        m.insert(70, "[mcr_admin::fsm::cast]");
        m.insert(71, "[mcr_admin::fsm::cast]");
        m.insert(72, "[mcr_admin::fsm::cast]");
        m.insert(73, "[mcr_admin::fsm::cast]");
        m.insert(74, "[mcr_admin::fsm::cast]");
        m.insert(75, "[mcr_admin::fsm::vflr]");
        m.insert(76, "[mcr_admin::fsm::vflr]");
        m.insert(77, "[mcr_admin::fsm::pcie]");
        m.insert(78, "[mcr_admin::fsm::pcie]");
        m.insert(79, "[mcr_admin::fsm::pcie]");
        m.insert(80, "[mcr_admin::fsm::pcie]");
        m.insert(81, "[mcr_admin::preop_cdma_io]");
        m.insert(82, "[mcr_admin]");
        m.insert(83, "[mcr_admin::handler]");
        m.insert(84, "[mcr_admin::handler]");
        m.insert(85, "[mcr_admin::handler]");
        m.insert(86, "[mcr_admin::handler]");
        m.insert(87, "[mcr_admin::handler]");
        m.insert(88, "[mcr_admin::handler]");
        m.insert(89, "[mcr_admin::handler]");
        m.insert(90, "[mcr_admin::handler]");
        m.insert(91, "[mcr_admin::handler]");
        m.insert(92, "[mcr_admin::handler]");
        m.insert(93, "[mcr_admin::handler]");
        m.insert(94, "[mcr_admin::handler]");
        m.insert(95, "[mcr_admin::handler]");
        m.insert(96, "[app::exception_handlers]");
        m.insert(97, "[mcr_admin::fsm::delete_cq]");
        m.insert(98, "[mcr_admin::fsm::vflr]");
        m.insert(99, "[mcr_admin::fsm::vf_restore]");
        m.insert(100, "[mcr_admin::fsm::vf_restore]");
        m.insert(101, "[mcr_admin::fsm::vf_start]");
        m.insert(102, "[mcr_admin::fsm::vf_start]");
        m.insert(103, "[mcr_admin::fsm::pcie]");
        m.insert(104, "[mcr_admin::fsm::vf_stop]");
        m.insert(105, "[mcr_admin::fsm::vf_stop]");
        m.insert(106, "[mcr_admin::fsm::vf_save]");
        m.insert(107, "[mcr_admin::fsm::vf_save]");
        m.insert(108, "[mcr_admin::fsm::vf_prep]");
        m.insert(109, "[mcr_admin::fsm::vf_prep]");
        m.insert(110, "[mcr_admin::fsm::stop_interface]");
        m.insert(111, "[mcr_admin::fsm::stop_interface]");
        m.insert(112, "[mcr_admin::fsm::stop_interface]");
        m.insert(113, "[mcr_admin::fsm::stop_interface]");
        m.insert(114, "[mcr_admin::fsm::stop_interface]");
        m.insert(115, "[mcr_admin::fsm::stop_interface]");
        m.insert(116, "[mcr_admin::fsm::tdisp_int]");
        m.insert(117, "[mcr_admin::fsm::tdisp_int]");
        m.insert(118, "[mcr_admin::fsm::tdisp_int]");
        m.insert(119, "[mcr_admin::fsm::tdisp_int]");
        m.insert(120, "[mcr_admin::fsm::tdisp_int]");
        m.insert(121, "[mcr_admin::fsm::tdisp_int]");
        m.insert(122, "[mcr_admin::fsm::cntrl]");
        m.insert(123, "[app::exception_handlers]");
        m.insert(124, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(125, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(126, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(127, "[mcr_admin::fsm::aes_gcm_ext]");
        m.insert(128, "[app::exception_handlers]");
        m.insert(129, "[app::exception_handlers]");
        m.insert(130, "[app::exception_handlers]");
        m.insert(131, "[app::exception_handlers]");
        m.insert(132, "[mcr_admin::main]");
        m.insert(133, "[mcr_admin::handler]");
        m.insert(134, "[mcr_admin::handler]");
        m.insert(135, "[app::exception_handlers]");
        m
    };
}

#[allow(dead_code)]
pub const MANTICORE_ADMIN_LOG_TOKENS: [&str; 136] = [
    "Failed the self test {:?}",
    "Preop-Self test failed for {:?} with error {}",
    "[doe] on_rx_ready Failed to recv DOE message: {:?}",
    "[doe] on_doe_go: Failed to recv DOE message: {:?}",
    "Failed to end recv DOE message: {:?}",
    "Failed to decode IPC message: {:?}",
    "[doe] Invalid IPC response with status {}",
    "[doe] Spurious Message",
    "[doe] on_ipc_response: Failed to send DOE message: {:?}",
    "[doe] on_tx_ready: Failed to send DOE message: {:?}",
    "Failed to send IPC message to HSP: {:?}",
    "[get_res] Invalid SQE: {:?}",
    "[delete_cq] Invalid SQE: {:?}",
    "Create Cq Failed {:?}",
    "[set_res] Invalid SQE: {:?}",
    "Failed to set resource count: {:?}",
    "[set_res] Invalid IPC response with status {}",
    "[delete_sq] Invalid SQE: {:?}",
    "Get device Sq Failed {:?}",
    "Process HSM IPC failed {:?}",
    "[delete_sq] Delete device queue on_hsm_ipc_response: Delete submission queue failed {:?}",
    "Process FP IPC failed {:?}",
    "[delete_sq] Delete device queue on_fp_ipc_response: Delete submission queue failed {:?}",
    "[delete_sq] on_deferred_queue_deletion: Delete submission queue failed {:?}",
    "Failed to send FP IPC request {:?}",
    "Failed to send HSM IPC request {:?}",
    "[set_features] Invalid SQE: {:?}",
    "Invalid state transition. Current state = {}",
    "[admin-fsm] DMA begin_txn error. (0x{:08x})",
    "[admin-fsm] send_err_cqe IO channel send error. (0x{:08x})",
    "[admin-fsm] send_cqe IO channel send error. (0x{:08x})",
    "Telemetry FSM received unexpected event. Event: {:?}",
    "Error running telemetry PCIe monitor, code {:#x}",
    "Starting PCIe telemetry timer",
    "Telemetry PCIe monitor is running normally. Link Speed: {}, Link Width: x{}",
    "Unexpected PCIe Link. Link Speed: {}, Link Width: {}",
    "Telemetry PCIe monitor not able to start, code {:#x}",
    "Failed to receive IPC message from HSM",
    "Failed to decode IPC message from HSM",
    "Failed to receive IPC message from FP",
    "Failed to decode IPC message from FP",
    "Fail to receive queue_delete_notification",
    "Mismatch tags: received ({:?}) vs expected ({:?})",
    "Error in enabling controller {:?}",
    "[cntrl] Failed to send IPC message to FP: {:?}",
    "[cntrl] Failed to send IPC message to HSM: {:?}",
    "[get_features] Invalid SQE: {:?}",
    "Late IPC_HSM_SHUTDOWN_RSP after {}us",
    "Late IPC_FP_SHUTDOWN_RSP after {}us",
    "Late resource {} acquire {}us",
    "Starting",
    "Stopped after {}us",
    "Admin abort with status: {:?}. State: ({:?})",
    "HSM abort with status: {:?}. State: ({:?})",
    "FP abort with status: {:?}. State: ({:?})",
    "error_dfu: IPC_SP_SHUTDOWN_RSP {:?}",
    "Timeout in state {:?} resources: {:#x}",
    "handle_dfu_started: IPC_SP_SHUTDOWN_RSP {:?}",
    "Insufficient time to drain CP HSM/FP.",
    "IPC_CP_HSM_SHUTDOWN_REQ {:#x}",
    "IPC_FP_SHUTDOWN_REQ {:#x}",
    "Ready in {} us",
    "iDFU Abort with OperationTimeout: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
    "iDFU Abort with OperationFailed: Failure sending IPC_SP_SHUTDOWN_RSP {:#x}",
    "IPC_SP_SHUTDOWN_RSP {:#x}",
    "No reset detected.",
    "Spurious IPC_CP_HSM_SHUTDOWN_RSP after {}us",
    "Spurious IPC_FP_SHUTDOWN_RSP after {}us",
    "Received timer event while waiting for cast resource",
    "Invalid event. state: {:?}",
    "on_fp_to_admin_channel_ready: Invalid state {:?}",
    "on_fp_to_admin_ipc_response: Invalid state {:?}",
    "Failed to receive self test response for {:?}",
    "inspect_result: Self test {:?} FAILED with error code: {:?}",
    "check_for_err: Self test {:?} FAILED with error code: {:?}",
    "[vflr] Failed to send IPC message to FP: {:?}",
    "[vflr] Failed to send IPC message to HSM: {:?}",
    "PCIE_PERST_UP Error: 0x{:08x}",
    "Link Speed={} Width=x{}",
    "FLR complete",
    "Perst Down complete",
    "IPC response status: {:x}",
    "Starting Admin event loop...",
    "Invalid IPC message header",
    "Cannot decode opcode. Invalid IPC message opcode",
    "Opcode does not match. Invalid IPC message opcode",
    "Invalid IPC message",
    "Invalid IPC message for Negative self test.",
    "Invalid test ID in negative self test IPC payload.",
    "Invalid Header found in IPC message",
    "Invalid Shutdown for reset request message",
    "handle_invalid_message_opcode: Invalid IPC message header {:?}",
    "Unhandled message with opcode {:#X}",
    "Failed to send response to HSP core {:?}",
    "send_hsm_ipc_response: Invalid IPC message header {:?}",
    "Failed to send response to hsm core {:?}",
    "DTCM error received. Fault code: {}",
    "Delete Cq Failed {:?}",
    "Complete VFLR for {:?}",
    "VF Restore for PCIe function: {:?}",
    "[Vf Restore] Unsupported event",
    "VF Start for PCIe function: {:?}",
    "[Vf_Start] Unsupported event, expected StartCmd",
    "PF FLR complete",
    "VF Stop for PCIe function: {:?}",
    "[Vf_Stop] Unsupported event, expected StartCmd",
    "VF Save for PCIe function: {:?}",
    "[Vf Save] Unsupported event",
    "VF Prepare for PCIe function: {:?}",
    "[Vf Prepare] Unsupported event",
    "[stop_interface] Timer elapsed, state: {:?}",
    "[stop_interface] Failed to receive notification from queue",
    "[stop_interface] Message tag {} is not expected {}",
    "[stop_interface] Failed to send IPC message to FP: {:?}",
    "[stop_interface] Failed to send IPC message to HSM: {:?}",
    "[stop_interface] Failed to send response to HSP: {:?}",
    "[tdisp_int] TdispIntFsm timer elapsed, state: {}",
    "[tdisp_int] Invalid interrupt source, source_mask: {}",
    "[tdisp_int] Failed to send IPC message to HSP: {:?}",
    "[tdisp_int] Failed to decode IPC message: {:?}",
    "[tdisp_int] Invalid IPC response with status {}",
    "[tdisp_int] Spurious Message",
    "Received deferred action for a different controller: {:?}",
    "ITCM error received. Fault code: {}",
    "Statemachine error {}",
    "AES-GCM Decrypt Tag Mismatch",
    "Failed to send AES GCM response entry to response queue",
    "Invalid state transition in AesGcmExtFsm. Current state = {}",
    "GDMA Data Structure Error. Fault code: {}",
    "GDMA Data Access Error. Fault code: {}",
    "GDMA Delivery Queue Error. Fault code: {}",
    "GDMA Completion Queue Error. Fault code: {}",
    "Failed to configure stack guard: invalid stack limit",
    "Invalid IPC message for TriggerStackValidation",
    "Invalid stack error type",
    "Memory management fault received. CFSR={:#x}, MMFAR={:#x}",
];
