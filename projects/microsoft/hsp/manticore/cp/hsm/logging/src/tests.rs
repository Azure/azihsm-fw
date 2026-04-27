// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_simplex::SimplexPipeConfig;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;
use mcr_types::VolatileCell;

use super::*;

fn mem_addr_to_slice<T>(addr: usize, len: usize) -> &'static mut [T] {
    unsafe { core::slice::from_raw_parts_mut(addr as *mut T, len) }
}

fn mem_add_to_volatile_ptr(addr: usize) -> &'static VolatileCell<u32> {
    unsafe {
        #[allow(clippy::transmute_ptr_to_ref)]
        core::mem::transmute(addr as *const u32)
    }
}

#[test]
fn test_debug_log_sender() {
    let log_default = DebugLogEntryParameters {
        severity: DebugLogSeverity::Error,
        component: mcr_types::DebugLogComponent::MsftLoggingComponentManticoreCp0,
        msg_index: 0,
        _reserved: 0,
        arg1: 0,
        arg2: 0,
    };
    let queue = [log_default; 3];
    let queue_ref: &'static mut [DebugLogEntryParameters] =
        mem_addr_to_slice(queue.as_ptr() as usize, queue.len());

    let ci = 0u32;
    let ci_ptr: *const u32 = &ci;
    let ci_ref = mem_add_to_volatile_ptr(ci_ptr as usize);

    let pi = 0u32;
    let pi_ptr: *const u32 = &pi;
    let pi_ref = mem_add_to_volatile_ptr(pi_ptr as usize);

    let buffer_size = 0u32;
    let buffer_size_ptr: *const u32 = &buffer_size;
    let buffer_size_ref = mem_add_to_volatile_ptr(buffer_size_ptr as usize);

    let sender_overflows = 0u32;
    let sender_overflows_ptr: *const u32 = &sender_overflows;
    let sender_overflows_ref = mem_add_to_volatile_ptr(sender_overflows_ptr as usize);

    let log_sender = DebugLogSender::new(
        SimplexPipeConfig {
            queue: queue_ref,
            ci: ci_ref,
            pi: pi_ref,
        },
        buffer_size_ref,
        sender_overflows_ref,
    );

    let log1 = DebugLogEntryParameters {
        severity: DebugLogSeverity::Error,
        component: mcr_types::DebugLogComponent::MsftLoggingComponentManticoreCp0,
        msg_index: 1,
        _reserved: 1,
        arg1: 1,
        arg2: 1,
    };

    log_sender.send(log1);
    assert!(queue[0] == log1);
    assert!(queue[1] == log_default);
    assert!(queue[2] == log_default);
    assert!(sender_overflows == 0);

    let log2 = DebugLogEntryParameters {
        severity: DebugLogSeverity::Error,
        component: mcr_types::DebugLogComponent::MsftLoggingComponentManticoreCp0,
        msg_index: 2,
        _reserved: 2,
        arg1: 2,
        arg2: 2,
    };
    log_sender.send(log2);
    assert!(queue[0] == log1);
    assert!(queue[1] == log2);
    assert!(queue[2] == log_default);
    assert!(sender_overflows == 0);

    let log3 = DebugLogEntryParameters {
        severity: DebugLogSeverity::Error,
        component: mcr_types::DebugLogComponent::MsftLoggingComponentManticoreCp0,
        msg_index: 3,
        _reserved: 3,
        arg1: 3,
        arg2: 3,
    };
    log_sender.send(log3);
    assert!(queue[0] == log1);
    assert!(queue[1] == log2);
    assert!(queue[2] == log_default);
    assert!(sender_overflows == 1);
}
