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
    trace!("Notifying Crash to to other cores");
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
