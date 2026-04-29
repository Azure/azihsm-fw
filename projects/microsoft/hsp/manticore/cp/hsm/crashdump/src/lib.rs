// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]
pub mod crash_format;
pub mod crash_mgr;
pub mod failure_code;

use core::panic;
use crash_format::*;
use crash_mgr::*;
use failure_code::FailureCode;
use log::*;
use mcr_ddi_types::DdiTestActionCrashType;
use mcr_ddi_types::DdiTestStackErrorType;
use mcr_interrupt_controller::*;
use mcr_mem_map::AdminDtcmMemMap;
use mcr_mem_map::HsmDtcmMemMap;
use mcr_tcon::Tcon;
use mcr_types::CoreId;

/// Save Crashdump into dtcm memory
///
/// # Arguments
/// * `cpu_register_context` - CPU register context
/// * `failure_code` - Failure code
/// * `additional_info` - Additional information
pub fn crashdump_save(
    cpu_register_context: &CpuRegisterContext,
    failure_code: FailureCode,
    additional_info: Option<&str>,
) {
    let mut mgr = match mcr_cpu::cpu_id() {
        mcr_cpu::CpuId::Admin => CrashDumpManager::new(AdminDtcmMemMap::crashdump_base()),
        mcr_cpu::CpuId::Hsm => CrashDumpManager::new(HsmDtcmMemMap::crashdump_base()),
        mcr_cpu::CpuId::Unknown => {
            trace!("Unknown core!!!");
            return;
        }
    };

    mgr.create_dump(
        failure_code as u32,
        cpu_register_context,
        &RegisterBlockContext::from_register_block(unsafe { &*cortex_m::peripheral::SCB::PTR }),
        CoreId::from(mcr_cpu::cpu_id()) as u8,
        additional_info.unwrap_or(""),
    );
}

/// Trigger crash based on the crash type.
/// This function is called when the firmware encounters an unrecoverable failure.
///
/// # Arguments
/// * `crash_type` - Crash type
pub fn crashdump_trigger(crash_type: DdiTestActionCrashType) {
    match crash_type {
        DdiTestActionCrashType::Hang => {
            trace!("Inducing core hang");
            #[allow(clippy::empty_loop)]
            loop {}
        }
        DdiTestActionCrashType::Panic => {
            panic!("Triggering panic by calling BKPT instruction");
        }
        DdiTestActionCrashType::ExplicitCrash => {
            trace!("Triggering Explicit Crash by calling SVC instruction");
            explicit_crash(Some("Triggered by DDI Action::ExplicitCrash"));
        }
        _ => {
            trace!("Triggering HardFault by calling UDF instruction");
            #[cfg(target_arch = "arm")]
            unsafe {
                core::arch::asm!("udf #0");
            }
        }
    }
}

/// Trigger a stack validation test based on the error type.
///
/// # Arguments
/// * `error_type` - The type of stack error to trigger
pub fn trigger_stack_validation(error_type: DdiTestStackErrorType) {
    match error_type {
        DdiTestStackErrorType::StackOverflow => {
            trace!("Triggering Stack Overflow to invoke MemManage fault");
            #[cfg(target_arch = "arm")]
            stack_overflow(0);
        }
        DdiTestStackErrorType::StackGuardViolation => {
            trace!("Triggering Stack Guard Violation to invoke MemManage fault");
            #[cfg(target_arch = "arm")]
            trigger_memmanage_fault();
        }
        _ => {
            trace!("Unknown stack error type");
        }
    }
}

/// Stack overflow to intentionally overflow the stack and trigger MemManage fault.
/// Each frame consumes 128 bytes; recurses until the MPU stack guard region is hit.
#[cfg(target_arch = "arm")]
#[inline(never)]
#[allow(unconditional_recursion)]
fn stack_overflow(depth: u32) {
    let mut buf = [0u8; 16];
    buf[0] = depth as u8;
    core::hint::black_box(&mut buf);
    stack_overflow(depth + 1);
}

/// Trigger a MemManage fault by performing an invalid memory access to the stack guard region.
#[cfg(target_arch = "arm")]
fn trigger_memmanage_fault() {
    {
        use mcr_cpu::stack_guard::GUARD_SIZE_BYTES;

        extern "C" {
            static __stack_limit__: u32;
        }
        let guard_addr = unsafe { &__stack_limit__ as *const u32 as u32 } - (GUARD_SIZE_BYTES / 2);
        unsafe {
            core::ptr::write_volatile(guard_addr as *mut u8, 0xDE);
        }
    }
}

/// Trigger explicit crash
/// This function is called when the firmware encounters an unrecoverable failure.
///
/// # Arguments
/// * `additional_info` - String containing the description of the unrecoverable failure
#[allow(unused_variables)]
pub fn explicit_crash(additional_info: Option<&str>) {
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    error!("[failed] ");
    error!("Explicit Crash");
    trace!("Notifying Crash to other cores");
    Tcon::fire_wakeup_timer1();

    #[cfg(target_arch = "arm")]
    crashdump_save(
        &CpuRegisterContext {
            lr: cortex_m::register::lr::read(),
            sp: cortex_m::register::msp::read(),
            pc: cortex_m::register::pc::read(),
            ..Default::default()
        },
        FailureCode::ExplicitFailure,
        additional_info,
    );

    #[allow(clippy::empty_loop)]
    loop {}
}
