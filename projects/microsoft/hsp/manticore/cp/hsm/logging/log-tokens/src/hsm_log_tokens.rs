// Copyright (c) Microsoft Corporation. All rights reserved.
// This is an auto-generated file. Please do not modify manually.
// To regenerate use command: `cargo xtask telemetry-tokenize`

use hashbrown::HashMap;

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_MAP: HashMap<&'static str, u8> = {
        let mut m = HashMap::new();
        m.insert("Failed the self test {:?}", 0);
        m.insert("Preop-Self test failed for {:?} with error {}", 1);
        m.insert("[rsa_unwrap] Invalid State, state:{:?} event: {:?}", 2);
        m.insert("[rsa_unwrap] Invalid Event, state:{:?} event: {:?}", 3);
        m.insert("[rsa_unwrap] failed due to error: {:?}.", 4); // Deprecated message
        m.insert("Timeout!! State: {:?} Res Op State: {:?}", 5);
        m.insert("[open_key] Invalid State, state:{:?} event: {:?}", 6);
        m.insert("[open_key] Invalid Event, state:{:?} event: {:?}", 7);
        m.insert("[flush_session] Invalid Event, state:{:?}, event: {:?}", 8);
        m.insert("[flush_session] begin_close_user_session returned err: {:?}", 9);
        m.insert("[get_establish_cred_encryption_keys] begin_ecc_pct_validation failed due to {:?}.", 10); // Deprecated message
        m.insert("[get_establish_cred_encryption_key] on_engine_ready_begin_pct_validation failed due to {:?}.", 11); // Deprecated message
        m.insert("[get_establish_cred_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery.", 12); // Deprecated message
        m.insert("[get_establish_cred_encryption_key] continue_ecc_pct_validation failed due to {:?}.", 13); // Deprecated message
        m.insert("[aes_gen_key] Invalid Event, state:{:?}, event: {:?}", 14);
        m.insert("[aes_gen_key] begin_aesbulk256_gen_key returned err: {:?}", 15); // Deprecated message
        m.insert("[attest_key] Invalid State, state:{:?}, event: {:?}", 16);
        m.insert("[attest_key] Invalid Event, state:{:?}, event: {:?}", 17);
        m.insert("[close_session] Invalid Event, state:{:?}, event: {:?}", 18);
        m.insert("[close_session] begin_close_user_session err: {:?}", 19);
        m.insert("[hsm_fsm] IO Timed out while in {:?}", 20);
        m.insert("[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma", 21);
        m.insert("[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma", 22);
        m.insert("send_err_cqe: IO channel send error. (0x{:08x})", 23);
        m.insert("send_cqe: DMA buffer not found", 24);
        m.insert("send_cqe: IO channel send error. (0x{:08x})", 25);
        m.insert("IO channel send error. (0x{:08x})", 26);
        m.insert("[establish_credential] begin_establish_credential err: {:?}", 27); // Deprecated message
        m.insert("[establish_credential] on_engine_ready begin_establish_credential err: {:?}", 28);
        m.insert("[ecc_sign] Invalid State, state:{:?} event: {:?}", 29);
        m.insert("[ecc_sign] Invalid Event, state:{:?} event: {:?}", 30);
        m.insert("[test_action] Missing negative self test ID", 31);
        m.insert("[test_action] Missing pin policy config", 32);
        m.insert("Invalid rng failure test id received", 33);
        m.insert("[test_action] trigger_crashdump_local: Missing crash info", 34);
        m.insert("[test_action] send_crashdump_request: Missing crash info", 35);
        m.insert("[test_action] Invalid negative self test ID", 36);
        m.insert("[get_session_encryption_keys] on_engine_ready_begin_pct_validation failed due to {:?}.", 37); // Deprecated message
        m.insert("[get_session_encryption_keys] begin_ecc_pct_validation failed due to {:?}.", 38); // Deprecated message
        m.insert("[get_session_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery.", 39); // Deprecated message
        m.insert("[get_session_encryption_key] continue_ecc_pct_validation failed due to {:?}.", 40); // Deprecated message
        m.insert("[ecc_gen_key] Invalid State, state:{:?}, event: {:?}", 41); // Deprecated message
        m.insert("[ecc_gen_key] Invalid Event, state:{:?}, event: {:?}", 42); // Deprecated message
        m.insert("[ecc_gen_key] on_cmd_complete: No operation to continue. Error: {:?}", 43); // Deprecated message
        m.insert("[ecc_gen_key] begin_ecc_gen_key failed due to {:?}", 44); // Deprecated message
        m.insert("[ecc_gen_key] begin_ecc_pct_validation failed due to {:?}", 45); // Deprecated message
        m.insert("ECC PCT {:?} Verification failed! Forcing crash for recovery.", 46); // Deprecated message
        m.insert("[ecc_gen_key] on_cmd_complete end_ecc_pct_validation failed due to {:?}", 47); // Deprecated message
        m.insert("[ecc_gen_key] continue_ecc_pct_validation failed due to {:?}", 48); // Deprecated message
        m.insert("[der_key_import] Invalid State, state:{:?}, event: {:?}", 49);
        m.insert("[der_key_import] Invalid Event, state:{:?}, event: {:?}", 50);
        m.insert("[der_key_import] begin_import_der_crt_key returned err: {:?}", 51); // Deprecated message
        m.insert("[der_key_import] begin_import_der_aesbulk256_key returned err: {:?}", 52); // Deprecated message
        m.insert("[rsa_mod_exp] Invalid State, state:{:?} event: {:?}", 53);
        m.insert("[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}", 54);
        m.insert("Soft AES Invalid Response", 55); // Deprecated message
        m.insert("[get_unwrapping_key] RSA PCT Unwrap Verification failed! Forcing crash for recovery.", 56); // Deprecated message
        m.insert("[get_unwrapping_key] failed due to {:?}.", 57); // Deprecated message
        m.insert("GetUnwrappingKey command timed out. {:?}", 58);
        m.insert("[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}", 59);
        m.insert("[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}", 60);
        m.insert("[change_pin] on_start begin_change_pin err: {:?}", 61); // Deprecated message
        m.insert("[change_pin] on_engine_ready begin_change_pin err: {:?}", 62); // Deprecated message
        m.insert("[change_pin] continue_change_pin err: {:?}", 63); // Deprecated message
        m.insert("begin_delete_aesbulk256_user_keys {:?}", 64); // Deprecated message
        m.insert("Unexpected: Resource cleanup for an already cleaned up resource {}.", 65);
        m.insert("Unexpected: Resource cleanup completed for resource {}.", 66);
        m.insert("PKA instance {} cleanup error", 67);
        m.insert("[delete_key] Invalid event, state:{:?}, event: {:?}", 68);
        m.insert("[delete_key], begin_delete_aesbulk256_key returned err: {:?}", 69);
        m.insert("[open_session] on_start: begin_open_session err: {:?}", 70); // Deprecated message
        m.insert("[open_session] on_engine_ready: begin_open_session err: {:?}", 71);
        m.insert("[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}", 72); // Deprecated message
        m.insert("[part] Failed to send IPC message to FP: {:?}", 73); // Deprecated message
        m.insert("[part] Failed to decode IPC message header: {:?}", 74);
        m.insert("[part] Invalid IPC response with status {}", 75);
        m.insert("[part] Spurious Message", 76);
        m.insert("Establish cred key deletion failed {:?}. Ignoring error.", 77);
        m.insert("[ecc] ECC PCT Sign/Verify failed. Reporting failure to FSM.", 78); // Deprecated message
        m.insert("[ecc] ECC Verify operation failed: {:?}. Stopping validation.", 79); // Deprecated message
        m.insert("[ecc] Missing first shared secret for ECDH verification!", 80); // Deprecated message
        m.insert("[ecc] ECC PCT Key Agreement failed. Reporting failure to FSM.", 81); // Deprecated message
        m.insert("[ecc] ECC structural validation Public key length mismatch", 82);
        m.insert("[ecc] ECC structural validation Public key mismatch", 83);
        m.insert("[ecc] ECC structural validation Private key length mismatch", 84);
        m.insert("ECC structural validation Scalar d is not in the range 0 < d < n", 85);
        m.insert("send_crashdump_request: Failed to send IPC message to Admin: {:?}", 86);
        m.insert("[mod] Failed to send IPC message to FP: {:?}", 87);
        m.insert("Invalid core id", 88);
        m.insert("begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}", 89);
        m.insert("[mod] Failed to decode IPC message header: {:?}", 90);
        m.insert("Negative self test failed with status: {}", 91);
        m.insert("[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 92);
        m.insert("[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 93);
        m.insert("[aes] Import der key: Invalid IPC response with status {}", 94);
        m.insert("[aes] end_import_der_aesbulk256_key_inner: Spurious Message", 95);
        m.insert("[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 96);
        m.insert("[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 97);
        m.insert("[aes] Delete Key Operation: Invalid IPC response with status {}", 98);
        m.insert("[aes] end_delete_aesbulk256_key_inner: Spurious Message", 99);
        m.insert("[aes] begin_delete_aesbulk256_user_keys_inner: Failed to send IPC message to FP: {:?}", 100); // Deprecated message
        m.insert("[aes] end_delete_aesbulk256_user_keys_inner: Failed to decode IPC message header: {:?}", 101); // Deprecated message
        m.insert("[aes] Delete All keys: Invalid IPC response with status {}", 102); // Deprecated message
        m.insert("[aes] end_delete_aesbulk256_user_keys_inner: Spurious Message", 103); // Deprecated message
        m.insert("[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}", 104);
        m.insert("[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}", 105);
        m.insert("[aes] Delete key operation rollback: Invalid IPC response with status {}", 106);
        m.insert("[aes] end_rollback_aesbulk256_key_inner: Spurious Message", 107);
        m.insert("[rsa2k_mod] Failed to send self test response", 108);
        m.insert("[rsa2k_crt] Failed to send self test response", 109);
        m.insert("[ecdsa_engine] Failed to send self test response", 110);
        m.insert("[ecdh_engine] Failed to send self test response", 111);
        m.insert("[rng] Failed to send self test response", 112);
        m.insert("Failed to send self test response", 113);
        m.insert("Starting HSM event loop...", 114);
        m.insert("on_request_ready: spurious event", 115);
        m.insert("on_response_complete: spurious event", 116);
        m.insert("on_dma_complete: spurious event", 117);
        m.insert("Failed to send FLR complete event", 118);
        m.insert("on_flr: spurious event", 119);
        m.insert("Invalid Header found in IPC message", 120);
        m.insert("Invalid PcieFunction enable/disable message", 121);
        m.insert("Invalid Set Resource message {:?}", 122);
        m.insert("Invalid IPC Create Delete Submission Queue Message {:?}", 123);
        m.insert("{:?} not Enabled", 124);
        m.insert("Invalid Shutdown for reset request message", 125);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err", 126);
        m.insert("Failed to send prepare shutdown response to Admin core {:?}", 127);
        m.insert("Schedule is busy in draining IOs", 128);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err", 129);
        m.insert("iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout", 130);
        m.insert("Invalid IPC message header {:?}", 131);
        m.insert("Unhandled message with opcode {:#X}", 132);
        m.insert("Failed to send response to Admin core {:?}", 133);
        m.insert("on_pka: spurious event. Index: {}", 134);
        m.insert("on_fp_ipc_response: spurious event", 135);
        m.insert("on_hsp_ipc_response: spurious event", 136);
        m.insert("on_admin_ipc_response: spurious event", 137);
        m.insert("on_aes_unwrap_resp: spurious event", 138);
        m.insert("Failed to receive self test request", 139);
        m.insert("RNG Hardware error received. Fault code: {}", 140);
        m.insert("on_cmd_complete: No operation to continue failed due to HsmErr::InvalidState.", 141); // Deprecated message
        m.insert("begin_ecc_gen_key failed due to {:?}.", 142); // Deprecated message
        m.insert("begin_ecc_pct_validation failed due to {:?}.", 143); // Deprecated message
        m.insert("end_ecc_pct_validation failed due to {:?}.", 144); // Deprecated message
        m.insert("continue_ecc_pct_validation failed due to {:?}.", 145); // Deprecated message
        m.insert("[op_state: {:?}] failed due to {:?}.", 146); // Deprecated message
        m.insert("PCT Validation Failure: {:?}. System Reset Required! ", 147);
        m.insert("[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.", 148);
        m.insert("[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.", 149);
        m.insert("[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.", 150);
        m.insert("DTCM error received. Fault code: {}", 151);
        m.insert("[part_init] Unexpected event {} ", 152);
        m.insert("[part_init] Gen ID phase - expect to find UPKA engine on res ready", 153);
        m.insert("[part_init] Failed begin part id generation {}", 154);
        m.insert("[part_init] No context available for ending partition identifier generation.", 155);
        m.insert("[part_init] Failed end part id generation: {}", 156); // Deprecated message
        m.insert("Failed to allocate partition init FSM", 157);
        m.insert("[part] Failed to send IPC message to FP: {}", 158);
        m.insert("[unmask_key] Invalid Event, state:{:?}, event: {:?}", 159);
        m.insert("Invalid Header found for fp to hsm IPC message", 160);
        m.insert("Correctable ECC errors have exceeded threshold, Key vault memory reloaded", 161);
        m.insert("Failed to send response to FP core {:?}", 162);
        m.insert("Invalid op code found in IPC message", 163);
        m.insert("ITCM error received. Fault code: {}", 164);
        m.insert("[part_init] Failed Continue part id generation: {}", 165);
        m.insert("[part_init] Failed begin PCT validation: {}", 166);
        m.insert("[part_init] Failed PCT final validation", 167);
        m.insert("[part_init] Failed end PCT validation: {}", 168);
        m.insert("[part_init] Failed continue PCT validation: {}", 169);
        m.insert("Failed to get CDMA vault key entry", 170); // Deprecated message
        m.insert("Failed to configure stack guard: invalid stack limit", 171);
        m.insert("send_stack_validation_request: Failed to send IPC message to Admin: {:?}", 172);
        m.insert("Invalid core id for stack validation", 173);
        m.insert("Memory management fault received. CFSR={:#x}, MMFAR={:#x}", 174);
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_INDEX_TO_MESSAGE_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "Failed the self test {:?}");
        m.insert(1, "Preop-Self test failed for {:?} with error {}");
        m.insert(2, "[rsa_unwrap] Invalid State, state:{:?} event: {:?}");
        m.insert(3, "[rsa_unwrap] Invalid Event, state:{:?} event: {:?}");
        m.insert(4, "[rsa_unwrap] failed due to error: {:?}."); // Deprecated message
        m.insert(5, "Timeout!! State: {:?} Res Op State: {:?}");
        m.insert(6, "[open_key] Invalid State, state:{:?} event: {:?}");
        m.insert(7, "[open_key] Invalid Event, state:{:?} event: {:?}");
        m.insert(8, "[flush_session] Invalid Event, state:{:?}, event: {:?}");
        m.insert(9, "[flush_session] begin_close_user_session returned err: {:?}");
        m.insert(10, "[get_establish_cred_encryption_keys] begin_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(11, "[get_establish_cred_encryption_key] on_engine_ready_begin_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(12, "[get_establish_cred_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery."); // Deprecated message
        m.insert(13, "[get_establish_cred_encryption_key] continue_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(14, "[aes_gen_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(15, "[aes_gen_key] begin_aesbulk256_gen_key returned err: {:?}"); // Deprecated message
        m.insert(16, "[attest_key] Invalid State, state:{:?}, event: {:?}");
        m.insert(17, "[attest_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(18, "[close_session] Invalid Event, state:{:?}, event: {:?}");
        m.insert(19, "[close_session] begin_close_user_session err: {:?}");
        m.insert(20, "[hsm_fsm] IO Timed out while in {:?}");
        m.insert(21, "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma");
        m.insert(22, "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma");
        m.insert(23, "send_err_cqe: IO channel send error. (0x{:08x})");
        m.insert(24, "send_cqe: DMA buffer not found");
        m.insert(25, "send_cqe: IO channel send error. (0x{:08x})");
        m.insert(26, "IO channel send error. (0x{:08x})");
        m.insert(27, "[establish_credential] begin_establish_credential err: {:?}"); // Deprecated message
        m.insert(28, "[establish_credential] on_engine_ready begin_establish_credential err: {:?}");
        m.insert(29, "[ecc_sign] Invalid State, state:{:?} event: {:?}");
        m.insert(30, "[ecc_sign] Invalid Event, state:{:?} event: {:?}");
        m.insert(31, "[test_action] Missing negative self test ID");
        m.insert(32, "[test_action] Missing pin policy config");
        m.insert(33, "Invalid rng failure test id received");
        m.insert(34, "[test_action] trigger_crashdump_local: Missing crash info");
        m.insert(35, "[test_action] send_crashdump_request: Missing crash info");
        m.insert(36, "[test_action] Invalid negative self test ID");
        m.insert(37, "[get_session_encryption_keys] on_engine_ready_begin_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(38, "[get_session_encryption_keys] begin_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(39, "[get_session_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery."); // Deprecated message
        m.insert(40, "[get_session_encryption_key] continue_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(41, "[ecc_gen_key] Invalid State, state:{:?}, event: {:?}"); // Deprecated message
        m.insert(42, "[ecc_gen_key] Invalid Event, state:{:?}, event: {:?}"); // Deprecated message
        m.insert(43, "[ecc_gen_key] on_cmd_complete: No operation to continue. Error: {:?}"); // Deprecated message
        m.insert(44, "[ecc_gen_key] begin_ecc_gen_key failed due to {:?}"); // Deprecated message
        m.insert(45, "[ecc_gen_key] begin_ecc_pct_validation failed due to {:?}"); // Deprecated message
        m.insert(46, "ECC PCT {:?} Verification failed! Forcing crash for recovery."); // Deprecated message
        m.insert(47, "[ecc_gen_key] on_cmd_complete end_ecc_pct_validation failed due to {:?}"); // Deprecated message
        m.insert(48, "[ecc_gen_key] continue_ecc_pct_validation failed due to {:?}"); // Deprecated message
        m.insert(49, "[der_key_import] Invalid State, state:{:?}, event: {:?}");
        m.insert(50, "[der_key_import] Invalid Event, state:{:?}, event: {:?}");
        m.insert(51, "[der_key_import] begin_import_der_crt_key returned err: {:?}"); // Deprecated message
        m.insert(52, "[der_key_import] begin_import_der_aesbulk256_key returned err: {:?}"); // Deprecated message
        m.insert(53, "[rsa_mod_exp] Invalid State, state:{:?} event: {:?}");
        m.insert(54, "[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}");
        m.insert(55, "Soft AES Invalid Response"); // Deprecated message
        m.insert(56, "[get_unwrapping_key] RSA PCT Unwrap Verification failed! Forcing crash for recovery."); // Deprecated message
        m.insert(57, "[get_unwrapping_key] failed due to {:?}."); // Deprecated message
        m.insert(58, "GetUnwrappingKey command timed out. {:?}");
        m.insert(59, "[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}");
        m.insert(60, "[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}");
        m.insert(61, "[change_pin] on_start begin_change_pin err: {:?}"); // Deprecated message
        m.insert(62, "[change_pin] on_engine_ready begin_change_pin err: {:?}"); // Deprecated message
        m.insert(63, "[change_pin] continue_change_pin err: {:?}"); // Deprecated message
        m.insert(64, "begin_delete_aesbulk256_user_keys {:?}"); // Deprecated message
        m.insert(65, "Unexpected: Resource cleanup for an already cleaned up resource {}.");
        m.insert(66, "Unexpected: Resource cleanup completed for resource {}.");
        m.insert(67, "PKA instance {} cleanup error");
        m.insert(68, "[delete_key] Invalid event, state:{:?}, event: {:?}");
        m.insert(69, "[delete_key], begin_delete_aesbulk256_key returned err: {:?}");
        m.insert(70, "[open_session] on_start: begin_open_session err: {:?}"); // Deprecated message
        m.insert(71, "[open_session] on_engine_ready: begin_open_session err: {:?}");
        m.insert(72, "[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}"); // Deprecated message
        m.insert(73, "[part] Failed to send IPC message to FP: {:?}"); // Deprecated message
        m.insert(74, "[part] Failed to decode IPC message header: {:?}");
        m.insert(75, "[part] Invalid IPC response with status {}");
        m.insert(76, "[part] Spurious Message");
        m.insert(77, "Establish cred key deletion failed {:?}. Ignoring error.");
        m.insert(78, "[ecc] ECC PCT Sign/Verify failed. Reporting failure to FSM."); // Deprecated message
        m.insert(79, "[ecc] ECC Verify operation failed: {:?}. Stopping validation."); // Deprecated message
        m.insert(80, "[ecc] Missing first shared secret for ECDH verification!"); // Deprecated message
        m.insert(81, "[ecc] ECC PCT Key Agreement failed. Reporting failure to FSM."); // Deprecated message
        m.insert(82, "[ecc] ECC structural validation Public key length mismatch");
        m.insert(83, "[ecc] ECC structural validation Public key mismatch");
        m.insert(84, "[ecc] ECC structural validation Private key length mismatch");
        m.insert(85, "ECC structural validation Scalar d is not in the range 0 < d < n");
        m.insert(86, "send_crashdump_request: Failed to send IPC message to Admin: {:?}");
        m.insert(87, "[mod] Failed to send IPC message to FP: {:?}");
        m.insert(88, "Invalid core id");
        m.insert(89, "begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}");
        m.insert(90, "[mod] Failed to decode IPC message header: {:?}");
        m.insert(91, "Negative self test failed with status: {}");
        m.insert(92, "[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(93, "[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(94, "[aes] Import der key: Invalid IPC response with status {}");
        m.insert(95, "[aes] end_import_der_aesbulk256_key_inner: Spurious Message");
        m.insert(96, "[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(97, "[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(98, "[aes] Delete Key Operation: Invalid IPC response with status {}");
        m.insert(99, "[aes] end_delete_aesbulk256_key_inner: Spurious Message");
        m.insert(100, "[aes] begin_delete_aesbulk256_user_keys_inner: Failed to send IPC message to FP: {:?}"); // Deprecated message
        m.insert(101, "[aes] end_delete_aesbulk256_user_keys_inner: Failed to decode IPC message header: {:?}"); // Deprecated message
        m.insert(102, "[aes] Delete All keys: Invalid IPC response with status {}"); // Deprecated message
        m.insert(103, "[aes] end_delete_aesbulk256_user_keys_inner: Spurious Message"); // Deprecated message
        m.insert(104, "[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}");
        m.insert(105, "[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}");
        m.insert(106, "[aes] Delete key operation rollback: Invalid IPC response with status {}");
        m.insert(107, "[aes] end_rollback_aesbulk256_key_inner: Spurious Message");
        m.insert(108, "[rsa2k_mod] Failed to send self test response");
        m.insert(109, "[rsa2k_crt] Failed to send self test response");
        m.insert(110, "[ecdsa_engine] Failed to send self test response");
        m.insert(111, "[ecdh_engine] Failed to send self test response");
        m.insert(112, "[rng] Failed to send self test response");
        m.insert(113, "Failed to send self test response");
        m.insert(114, "Starting HSM event loop...");
        m.insert(115, "on_request_ready: spurious event");
        m.insert(116, "on_response_complete: spurious event");
        m.insert(117, "on_dma_complete: spurious event");
        m.insert(118, "Failed to send FLR complete event");
        m.insert(119, "on_flr: spurious event");
        m.insert(120, "Invalid Header found in IPC message");
        m.insert(121, "Invalid PcieFunction enable/disable message");
        m.insert(122, "Invalid Set Resource message {:?}");
        m.insert(123, "Invalid IPC Create Delete Submission Queue Message {:?}");
        m.insert(124, "{:?} not Enabled");
        m.insert(125, "Invalid Shutdown for reset request message");
        m.insert(126, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err");
        m.insert(127, "Failed to send prepare shutdown response to Admin core {:?}");
        m.insert(128, "Schedule is busy in draining IOs");
        m.insert(129, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err");
        m.insert(130, "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout");
        m.insert(131, "Invalid IPC message header {:?}");
        m.insert(132, "Unhandled message with opcode {:#X}");
        m.insert(133, "Failed to send response to Admin core {:?}");
        m.insert(134, "on_pka: spurious event. Index: {}");
        m.insert(135, "on_fp_ipc_response: spurious event");
        m.insert(136, "on_hsp_ipc_response: spurious event");
        m.insert(137, "on_admin_ipc_response: spurious event");
        m.insert(138, "on_aes_unwrap_resp: spurious event");
        m.insert(139, "Failed to receive self test request");
        m.insert(140, "RNG Hardware error received. Fault code: {}");
        m.insert(141, "on_cmd_complete: No operation to continue failed due to HsmErr::InvalidState."); // Deprecated message
        m.insert(142, "begin_ecc_gen_key failed due to {:?}."); // Deprecated message
        m.insert(143, "begin_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(144, "end_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(145, "continue_ecc_pct_validation failed due to {:?}."); // Deprecated message
        m.insert(146, "[op_state: {:?}] failed due to {:?}."); // Deprecated message
        m.insert(147, "PCT Validation Failure: {:?}. System Reset Required! ");
        m.insert(148, "[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(149, "[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(150, "[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.");
        m.insert(151, "DTCM error received. Fault code: {}");
        m.insert(152, "[part_init] Unexpected event {} ");
        m.insert(153, "[part_init] Gen ID phase - expect to find UPKA engine on res ready");
        m.insert(154, "[part_init] Failed begin part id generation {}");
        m.insert(155, "[part_init] No context available for ending partition identifier generation.");
        m.insert(156, "[part_init] Failed end part id generation: {}"); // Deprecated message
        m.insert(157, "Failed to allocate partition init FSM");
        m.insert(158, "[part] Failed to send IPC message to FP: {}");
        m.insert(159, "[unmask_key] Invalid Event, state:{:?}, event: {:?}");
        m.insert(160, "Invalid Header found for fp to hsm IPC message");
        m.insert(161, "Correctable ECC errors have exceeded threshold, Key vault memory reloaded");
        m.insert(162, "Failed to send response to FP core {:?}");
        m.insert(163, "Invalid op code found in IPC message");
        m.insert(164, "ITCM error received. Fault code: {}");
        m.insert(165, "[part_init] Failed Continue part id generation: {}");
        m.insert(166, "[part_init] Failed begin PCT validation: {}");
        m.insert(167, "[part_init] Failed PCT final validation");
        m.insert(168, "[part_init] Failed end PCT validation: {}");
        m.insert(169, "[part_init] Failed continue PCT validation: {}");
        m.insert(170, "Failed to get CDMA vault key entry"); // Deprecated message
        m.insert(171, "Failed to configure stack guard: invalid stack limit");
        m.insert(172, "send_stack_validation_request: Failed to send IPC message to Admin: {:?}");
        m.insert(173, "Invalid core id for stack validation");
        m.insert(174, "Memory management fault received. CFSR={:#x}, MMFAR={:#x}");
        m
    };
}

lazy_static::lazy_static! {
    pub static ref MANTICORE_HSM_LOG_TOKENS_INDEX_TO_PREFIX_MAP: HashMap<u8, &'static str> = {
        let mut m = HashMap::new();
        m.insert(0, "[mcr_hsm::env]");
        m.insert(1, "[mcr_hsm::env]");
        m.insert(2, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(3, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(4, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(5, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(6, "[mcr_hsm::fsm::open_key]");
        m.insert(7, "[mcr_hsm::fsm::open_key]");
        m.insert(8, "[mcr_hsm::fsm::flush_session]");
        m.insert(9, "[mcr_hsm::fsm::flush_session]");
        m.insert(10, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(11, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(12, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(13, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(14, "[mcr_hsm::fsm::aes_gen_key]");
        m.insert(15, "[mcr_hsm::fsm::aes_gen_key]");
        m.insert(16, "[mcr_hsm::fsm::attest_key]");
        m.insert(17, "[mcr_hsm::fsm::attest_key]");
        m.insert(18, "[mcr_hsm::fsm::close_session]");
        m.insert(19, "[mcr_hsm::fsm::close_session]");
        m.insert(20, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(21, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(22, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(23, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(24, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(25, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(26, "[mcr_hsm::fsm::hsm_fsm]");
        m.insert(27, "[mcr_hsm::fsm::establish_credential]");
        m.insert(28, "[mcr_hsm::fsm::establish_credential]");
        m.insert(29, "[mcr_hsm::fsm::ecc_sign]");
        m.insert(30, "[mcr_hsm::fsm::ecc_sign]");
        m.insert(31, "[mcr_hsm::fsm::test_action]");
        m.insert(32, "[mcr_hsm::fsm::test_action]");
        m.insert(33, "[mcr_hsm::fsm::test_action]");
        m.insert(34, "[mcr_hsm::fsm::test_action]");
        m.insert(35, "[mcr_hsm::fsm::test_action]");
        m.insert(36, "[mcr_hsm::fsm::test_action]");
        m.insert(37, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(38, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(39, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(40, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(41, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(42, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(43, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(44, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(45, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(46, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(47, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(48, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(49, "[mcr_hsm::fsm::der_key_import]");
        m.insert(50, "[mcr_hsm::fsm::der_key_import]");
        m.insert(51, "[mcr_hsm::fsm::der_key_import]");
        m.insert(52, "[mcr_hsm::fsm::der_key_import]");
        m.insert(53, "[mcr_hsm::fsm::rsa_mod_exp]");
        m.insert(54, "[mcr_hsm::fsm::rsa_mod_exp]");
        m.insert(55, "[mcr_hsm::fsm::soft_aes]");
        m.insert(56, "[mcr_hsm::fsm::get_unwrapping_key]");
        m.insert(57, "[mcr_hsm::fsm::get_unwrapping_key]");
        m.insert(58, "[mcr_hsm::fsm::get_unwrapping_key]");
        m.insert(59, "[mcr_hsm::fsm::ecdh_key_exchange]");
        m.insert(60, "[mcr_hsm::fsm::ecdh_key_exchange]");
        m.insert(61, "[mcr_hsm::fsm::change_pin]");
        m.insert(62, "[mcr_hsm::fsm::change_pin]");
        m.insert(63, "[mcr_hsm::fsm::change_pin]");
        m.insert(64, "[mcr_hsm::fsm::reset_part]");
        m.insert(65, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(66, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(67, "[mcr_hsm::fsm::res_cleanup_fsm]");
        m.insert(68, "[mcr_hsm::fsm::delete_key]");
        m.insert(69, "[mcr_hsm::fsm::delete_key]");
        m.insert(70, "[mcr_hsm::fsm::open_session]");
        m.insert(71, "[mcr_hsm::fsm::open_session]");
        m.insert(72, "[mcr_hsm::partition::part]");
        m.insert(73, "[mcr_hsm::partition::part]");
        m.insert(74, "[mcr_hsm::partition::part]");
        m.insert(75, "[mcr_hsm::partition::part]");
        m.insert(76, "[mcr_hsm::partition::part]");
        m.insert(77, "[mcr_hsm::partition::part]");
        m.insert(78, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(79, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(80, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(81, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(82, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(83, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(84, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(85, "[mcr_hsm::partition::session::app_sess::ecc]");
        m.insert(86, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(87, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(88, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(89, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(90, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(91, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(92, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(93, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(94, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(95, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(96, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(97, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(98, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(99, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(100, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(101, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(102, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(103, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(104, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(105, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(106, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(107, "[mcr_hsm::partition::session::app_sess::aes]");
        m.insert(108, "[mcr_hsm::self_test_handler]");
        m.insert(109, "[mcr_hsm::self_test_handler]");
        m.insert(110, "[mcr_hsm::self_test_handler]");
        m.insert(111, "[mcr_hsm::self_test_handler]");
        m.insert(112, "[mcr_hsm::self_test_handler]");
        m.insert(113, "[mcr_hsm::self_test_handler]");
        m.insert(114, "[mcr_hsm]");
        m.insert(115, "[mcr_hsm::handler]");
        m.insert(116, "[mcr_hsm::handler]");
        m.insert(117, "[mcr_hsm::handler]");
        m.insert(118, "[mcr_hsm::handler]");
        m.insert(119, "[mcr_hsm::handler]");
        m.insert(120, "[mcr_hsm::handler]");
        m.insert(121, "[mcr_hsm::handler]");
        m.insert(122, "[mcr_hsm::handler]");
        m.insert(123, "[mcr_hsm::handler]");
        m.insert(124, "[mcr_hsm::handler]");
        m.insert(125, "[mcr_hsm::handler]");
        m.insert(126, "[mcr_hsm::handler]");
        m.insert(127, "[mcr_hsm::handler]");
        m.insert(128, "[mcr_hsm::handler]");
        m.insert(129, "[mcr_hsm::handler]");
        m.insert(130, "[mcr_hsm::handler]");
        m.insert(131, "[mcr_hsm::handler]");
        m.insert(132, "[mcr_hsm::handler]");
        m.insert(133, "[mcr_hsm::handler]");
        m.insert(134, "[mcr_hsm::handler]");
        m.insert(135, "[mcr_hsm::handler]");
        m.insert(136, "[mcr_hsm::handler]");
        m.insert(137, "[mcr_hsm::handler]");
        m.insert(138, "[mcr_hsm::handler]");
        m.insert(139, "[mcr_hsm::handler]");
        m.insert(140, "[app::exception_handlers]");
        m.insert(141, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(142, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(143, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(144, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(145, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(146, "[mcr_hsm::fsm::rsa_unwrap]");
        m.insert(147, "[mcr_hsm::env]");
        m.insert(148, "[mcr_hsm::fsm::ecc_gen_key]");
        m.insert(149, "[mcr_hsm::fsm::get_establish_cred_encryption_key]");
        m.insert(150, "[mcr_hsm::fsm::get_session_encryption_key]");
        m.insert(151, "[app::exception_handlers]");
        m.insert(152, "[mcr_hsm::fsm::part_init]");
        m.insert(153, "[mcr_hsm::fsm::part_init]");
        m.insert(154, "[mcr_hsm::fsm::part_init]");
        m.insert(155, "[mcr_hsm::fsm::part_init]");
        m.insert(156, "[mcr_hsm::fsm::part_init]");
        m.insert(157, "[mcr_hsm::handler]");
        m.insert(158, "[mcr_hsm::partition::part]");
        m.insert(159, "[mcr_hsm::fsm::unmask_key]");
        m.insert(160, "[mcr_hsm::handler]");
        m.insert(161, "[mcr_hsm::handler]");
        m.insert(162, "[mcr_hsm::handler]");
        m.insert(163, "[mcr_hsm::handler]");
        m.insert(164, "[app::exception_handlers]");
        m.insert(165, "[mcr_hsm::fsm::part_init]");
        m.insert(166, "[mcr_hsm::fsm::part_init]");
        m.insert(167, "[mcr_hsm::fsm::part_init]");
        m.insert(168, "[mcr_hsm::fsm::part_init]");
        m.insert(169, "[mcr_hsm::fsm::part_init]");
        m.insert(170, "[mcr_hsm::handler]");
        m.insert(171, "[mcr_hsm::main]");
        m.insert(172, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(173, "[mcr_hsm::partition::session::app_sess::mod]");
        m.insert(174, "[app::exception_handlers]");
        m
    };
}

#[allow(dead_code)]
pub const MANTICORE_HSM_LOG_TOKENS: [&str; 175] = [
    "Failed the self test {:?}",
    "Preop-Self test failed for {:?} with error {}",
    "[rsa_unwrap] Invalid State, state:{:?} event: {:?}",
    "[rsa_unwrap] Invalid Event, state:{:?} event: {:?}",
    "[rsa_unwrap] failed due to error: {:?}.",
    "Timeout!! State: {:?} Res Op State: {:?}",
    "[open_key] Invalid State, state:{:?} event: {:?}",
    "[open_key] Invalid Event, state:{:?} event: {:?}",
    "[flush_session] Invalid Event, state:{:?}, event: {:?}",
    "[flush_session] begin_close_user_session returned err: {:?}",
    "[get_establish_cred_encryption_keys] begin_ecc_pct_validation failed due to {:?}.",
    "[get_establish_cred_encryption_key] on_engine_ready_begin_pct_validation failed due to {:?}.",
    "[get_establish_cred_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery.",
    "[get_establish_cred_encryption_key] continue_ecc_pct_validation failed due to {:?}.",
    "[aes_gen_key] Invalid Event, state:{:?}, event: {:?}",
    "[aes_gen_key] begin_aesbulk256_gen_key returned err: {:?}",
    "[attest_key] Invalid State, state:{:?}, event: {:?}",
    "[attest_key] Invalid Event, state:{:?}, event: {:?}",
    "[close_session] Invalid Event, state:{:?}, event: {:?}",
    "[close_session] begin_close_user_session err: {:?}",
    "[hsm_fsm] IO Timed out while in {:?}",
    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_in_dma",
    "[hsm_fsm] DMA begin_txn error. (0x{:08x}) in begin_out_dma",
    "send_err_cqe: IO channel send error. (0x{:08x})",
    "send_cqe: DMA buffer not found",
    "send_cqe: IO channel send error. (0x{:08x})",
    "IO channel send error. (0x{:08x})",
    "[establish_credential] begin_establish_credential err: {:?}",
    "[establish_credential] on_engine_ready begin_establish_credential err: {:?}",
    "[ecc_sign] Invalid State, state:{:?} event: {:?}",
    "[ecc_sign] Invalid Event, state:{:?} event: {:?}",
    "[test_action] Missing negative self test ID",
    "[test_action] Missing pin policy config",
    "Invalid rng failure test id received",
    "[test_action] trigger_crashdump_local: Missing crash info",
    "[test_action] send_crashdump_request: Missing crash info",
    "[test_action] Invalid negative self test ID",
    "[get_session_encryption_keys] on_engine_ready_begin_pct_validation failed due to {:?}.",
    "[get_session_encryption_keys] begin_ecc_pct_validation failed due to {:?}.",
    "[get_session_encryption_key] ECC PCT KeyAgreement Verification failed! Forcing crash for recovery.",
    "[get_session_encryption_key] continue_ecc_pct_validation failed due to {:?}.",
    "[ecc_gen_key] Invalid State, state:{:?}, event: {:?}",
    "[ecc_gen_key] Invalid Event, state:{:?}, event: {:?}",
    "[ecc_gen_key] on_cmd_complete: No operation to continue. Error: {:?}",
    "[ecc_gen_key] begin_ecc_gen_key failed due to {:?}",
    "[ecc_gen_key] begin_ecc_pct_validation failed due to {:?}",
    "ECC PCT {:?} Verification failed! Forcing crash for recovery.",
    "[ecc_gen_key] on_cmd_complete end_ecc_pct_validation failed due to {:?}",
    "[ecc_gen_key] continue_ecc_pct_validation failed due to {:?}",
    "[der_key_import] Invalid State, state:{:?}, event: {:?}",
    "[der_key_import] Invalid Event, state:{:?}, event: {:?}",
    "[der_key_import] begin_import_der_crt_key returned err: {:?}",
    "[der_key_import] begin_import_der_aesbulk256_key returned err: {:?}",
    "[rsa_mod_exp] Invalid State, state:{:?} event: {:?}",
    "[rsa_mod_exp] Invalid Event, state:{:?} event: {:?}",
    "Soft AES Invalid Response",
    "[get_unwrapping_key] RSA PCT Unwrap Verification failed! Forcing crash for recovery.",
    "[get_unwrapping_key] failed due to {:?}.",
    "GetUnwrappingKey command timed out. {:?}",
    "[ecdh_key_exchange] Invalid State, state:{:?} event: {:?}",
    "[ecdh_key_exchange] Invalid Event, state:{:?} event: {:?}",
    "[change_pin] on_start begin_change_pin err: {:?}",
    "[change_pin] on_engine_ready begin_change_pin err: {:?}",
    "[change_pin] continue_change_pin err: {:?}",
    "begin_delete_aesbulk256_user_keys {:?}",
    "Unexpected: Resource cleanup for an already cleaned up resource {}.",
    "Unexpected: Resource cleanup completed for resource {}.",
    "PKA instance {} cleanup error",
    "[delete_key] Invalid event, state:{:?}, event: {:?}",
    "[delete_key], begin_delete_aesbulk256_key returned err: {:?}",
    "[open_session] on_start: begin_open_session err: {:?}",
    "[open_session] on_engine_ready: begin_open_session err: {:?}",
    "[part] delete_session_keys: failed to delete key key_id={:?}, Error: {:?}",
    "[part] Failed to send IPC message to FP: {:?}",
    "[part] Failed to decode IPC message header: {:?}",
    "[part] Invalid IPC response with status {}",
    "[part] Spurious Message",
    "Establish cred key deletion failed {:?}. Ignoring error.",
    "[ecc] ECC PCT Sign/Verify failed. Reporting failure to FSM.",
    "[ecc] ECC Verify operation failed: {:?}. Stopping validation.",
    "[ecc] Missing first shared secret for ECDH verification!",
    "[ecc] ECC PCT Key Agreement failed. Reporting failure to FSM.",
    "[ecc] ECC structural validation Public key length mismatch",
    "[ecc] ECC structural validation Public key mismatch",
    "[ecc] ECC structural validation Private key length mismatch",
    "ECC structural validation Scalar d is not in the range 0 < d < n",
    "send_crashdump_request: Failed to send IPC message to Admin: {:?}",
    "[mod] Failed to send IPC message to FP: {:?}",
    "Invalid core id",
    "begin_neg_self_test_req: Failed to send IPC message to Admin: {:?}",
    "[mod] Failed to decode IPC message header: {:?}",
    "Negative self test failed with status: {}",
    "[aes] begin_import_der_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_import_der_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Import der key: Invalid IPC response with status {}",
    "[aes] end_import_der_aesbulk256_key_inner: Spurious Message",
    "[aes] begin_delete_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_delete_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Delete Key Operation: Invalid IPC response with status {}",
    "[aes] end_delete_aesbulk256_key_inner: Spurious Message",
    "[aes] begin_delete_aesbulk256_user_keys_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_delete_aesbulk256_user_keys_inner: Failed to decode IPC message header: {:?}",
    "[aes] Delete All keys: Invalid IPC response with status {}",
    "[aes] end_delete_aesbulk256_user_keys_inner: Spurious Message",
    "[aes] begin_rollback_aesbulk256_key_inner: Failed to send IPC message to FP: {:?}",
    "[aes] end_rollback_aesbulk256_key_inner: Failed to decode IPC message header: {:?}",
    "[aes] Delete key operation rollback: Invalid IPC response with status {}",
    "[aes] end_rollback_aesbulk256_key_inner: Spurious Message",
    "[rsa2k_mod] Failed to send self test response",
    "[rsa2k_crt] Failed to send self test response",
    "[ecdsa_engine] Failed to send self test response",
    "[ecdh_engine] Failed to send self test response",
    "[rng] Failed to send self test response",
    "Failed to send self test response",
    "Starting HSM event loop...",
    "on_request_ready: spurious event",
    "on_response_complete: spurious event",
    "on_dma_complete: spurious event",
    "Failed to send FLR complete event",
    "on_flr: spurious event",
    "Invalid Header found in IPC message",
    "Invalid PcieFunction enable/disable message",
    "Invalid Set Resource message {:?}",
    "Invalid IPC Create Delete Submission Queue Message {:?}",
    "{:?} not Enabled",
    "Invalid Shutdown for reset request message",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_no_resp_err",
    "Failed to send prepare shutdown response to Admin core {:?}",
    "Schedule is busy in draining IOs",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_shutdown_ipc_resp_err",
    "iDFU Fault Injected: idfu_fault_pre_reset_hsm_drain_timeout",
    "Invalid IPC message header {:?}",
    "Unhandled message with opcode {:#X}",
    "Failed to send response to Admin core {:?}",
    "on_pka: spurious event. Index: {}",
    "on_fp_ipc_response: spurious event",
    "on_hsp_ipc_response: spurious event",
    "on_admin_ipc_response: spurious event",
    "on_aes_unwrap_resp: spurious event",
    "Failed to receive self test request",
    "RNG Hardware error received. Fault code: {}",
    "on_cmd_complete: No operation to continue failed due to HsmErr::InvalidState.",
    "begin_ecc_gen_key failed due to {:?}.",
    "begin_ecc_pct_validation failed due to {:?}.",
    "end_ecc_pct_validation failed due to {:?}.",
    "continue_ecc_pct_validation failed due to {:?}.",
    "[op_state: {:?}] failed due to {:?}.",
    "PCT Validation Failure: {:?}. System Reset Required! ",
    "[ecc_gen_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "[get_establish_cred_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "[get_session_encryption_key] [op state: {:?}] PCT Validation Error: {:?}.",
    "DTCM error received. Fault code: {}",
    "[part_init] Unexpected event {} ",
    "[part_init] Gen ID phase - expect to find UPKA engine on res ready",
    "[part_init] Failed begin part id generation {}",
    "[part_init] No context available for ending partition identifier generation.",
    "[part_init] Failed end part id generation: {}",
    "Failed to allocate partition init FSM",
    "[part] Failed to send IPC message to FP: {}",
    "[unmask_key] Invalid Event, state:{:?}, event: {:?}",
    "Invalid Header found for fp to hsm IPC message",
    "Correctable ECC errors have exceeded threshold, Key vault memory reloaded",
    "Failed to send response to FP core {:?}",
    "Invalid op code found in IPC message",
    "ITCM error received. Fault code: {}",
    "[part_init] Failed Continue part id generation: {}",
    "[part_init] Failed begin PCT validation: {}",
    "[part_init] Failed PCT final validation",
    "[part_init] Failed end PCT validation: {}",
    "[part_init] Failed continue PCT validation: {}",
    "Failed to get CDMA vault key entry",
    "Failed to configure stack guard: invalid stack limit",
    "send_stack_validation_request: Failed to send IPC message to Admin: {:?}",
    "Invalid core id for stack validation",
    "Memory management fault received. CFSR={:#x}, MMFAR={:#x}",
];
