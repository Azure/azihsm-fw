// Copyright (c) Microsoft Corporation. All rights reserved.

#![cfg_attr(not(feature = "std"), no_std)]
#![cfg_attr(not(feature = "std"), no_main)]

cfg_if::cfg_if! {
    if #[cfg(not(feature = "std"))] {
        // extern crate alloc;

        #[allow(unused_imports)]
        use core::mem::MaybeUninit;
        use cortex_m_rt::entry;
        use embedded_alloc::TlsfHeap as Heap;
        use mcr_cpu::*;

        #[global_allocator]
        static HEAP: Heap = Heap::empty();

        use mcr_exception_handlers as _;

        /// CPU ID register address (DualCpM7 base + CP_ID offset)
        const CPU_ID_REG_ADDR: u32 = 0xB020_0000;
        /// Admin core initiator ID value
        const ADMIN_CORE_ID: u32 = 0x2;
        /// Admin application base address in DTCM
        const ADMIN_APP_BASE: u32 = 0x2002_0000;
        /// Vector Table Offset Register (VTOR) address in SCB
        const VTOR_REG_ADDR: u32 = 0xE000_ED08;
    }
}

mod cmd_scheduler;
mod crypto_env;
mod der;
mod env;
mod error;
mod event;
mod fsm;
mod handler;
mod heap;
mod key_attestation;
mod lm_key_derive;
mod masked_key;
mod nvic;
mod partition;
mod recorder;
mod resource;
mod self_test_handler;
mod x509;

#[cfg(test)]
mod mock;

#[macro_use]
mod hsm_logging_macros;

extern crate alloc;

pub(crate) use cmd_scheduler::*;
use env::*;
use error::HostStatusCode;
use error::HsmErr;
use event::*;
use handler::HsmEventHandler;
use log::trace;
use mcr_interrupt_controller::InterruptController;
use mcr_logging::*;
use mcr_registers::cortexm7::systemcontrol;
use mcr_types::DebugLogComponent;
use mcr_types::DebugLogEntryParameters;
use mcr_types::DebugLogSeverity;
use nvic::*;
use recorder::HsmFsmEventRecorder;
use resource::HsmFsmResourceId;

// The HSM subsystem is configured to receive I/O requests from a maximum of 65 I/O queues from
// the host NVMe interface. Each of these hardware queues is flow-controlled to bring in entry at a
// time. All the HSM I/Os from each I/O queue are transferred by the queue manager hardware into a
// single firmware-facing queue. This queue is called Inbound Completion Queue, which represents
// the number of outstanding work items for the firmware. This Inbound Completion Queue is sized at
// 32, which means the firmware can only parallelize 32 I/Os given we only have a maximum
// parallelism of 16 (using the 16-UPKA engines). The 32 outstanding I/Os will create enough
// pipeline for the I/O parallelism.
// In addition, we also need an extra slot to handle UPKA engine cleanup. This is done separately
// in a different state machine outside of the I/O lifecyle in order to reduce I/O response latency.
//
// With this data, the I/O Scheduler is sized to handle a maximum of 32 I/O FSMs and one resource cleanup FSM.
pub(crate) const NUM_HSM_IO_SCHEDULER_SLOT: usize = 34;

/// Main HSM loop. This function never returns.
pub fn run() -> ! {
    // Initialize debug log sender
    HsmHal::initialize_debug_log_sender();

    // Represents max ready queue count managed by the scheduler to hand over the resource
    // acquisition to the next command scheduler slot waiting inline for the given resource.
    const MAX_RESOURCE_WAKE_COUNT: usize = 1;

    let scheduler = CmdScheduler::new(
        NUM_HSM_IO_SCHEDULER_SLOT,
        MAX_RESOURCE_WAKE_COUNT,
        HsmFsmEventRecorder::default(),
    );
    let env = match HsmEnv::new(&scheduler) {
        Ok(env) => env,
        Err(err) => panic!("Failed to initialize HSM environment: {:#x}", err),
    };

    info!("Starting HSM event loop...");

    let mut handler = HsmEventHandler::new(env, scheduler);
    let intc = InterruptController::default();
    let mut pka_start_index = 0x0;
    loop {
        let reg = systemcontrol::RegisterBlock::block();

        handle_nvic_ipc_event(&reg, &mut handler, &intc);
        handle_nvic_ucd_event(&reg, &mut handler, &intc);
        handle_nvic_gdma_timer_event(&reg, &mut handler, &intc);
        handle_soft_interrupt(&mut handler);
        handle_nvic_crypto_done_event(&reg, &mut handler, pka_start_index);
        handle_nvic_crypto_error_event(&reg, &mut handler, pka_start_index);

        pka_start_index = (pka_start_index + 1) & 0xF;
    }
}

/// Pre-init function that redirects Admin core to admin-app without using the stack.
///
/// It is important to not use the stack for the Admin core, as it corrupts the text binary
/// loaded by the 1SP. The stack pointer at this point in time for the Admin core points to 0x20030000
/// and grows downwards. Any stack operations will update the memory in 0x2002xxxx region where the admin-app
/// binary is stored, which data for the admin-app to be corrupted. By using a naked function and register
/// operations, we can avoid using the stack and preserve the integrity of the admin-app binary in memory.
#[cfg(not(feature = "std"))]
#[unsafe(naked)]
#[export_name = "__pre_init"]
pub extern "C" fn admin_app_trampoline() {
    core::arch::naked_asm!(
        // Check CPU ID
        "ldr r0, ={cpu_id_reg}",      // Load CPU ID register address (0xB0200000)
        "ldr r0, [r0]",               // Read CPU ID register value
        "and r0, r0, #0x3F",          // Extract initiator_id (bits [5:0])
        "cmp r0, #{admin_id}",        // Compare with Admin core ID (0x2)
        "bne 1f",                     // If not Admin core, skip to label 1 (HSM path)

        // Admin core: Jump to admin-app
        "ldr r0, ={admin_base}",      // Load admin-app vector table base (0x20020000)
        "ldr r1, [r0]",               // r1 = Initial SP (msp) from vector_table[0]
        "ldr r2, [r0, #4]",           // r2 = Reset vector (rv) from vector_table[1]
        "orr r2, r2, #1",             // Ensure thumb mode bit is set on reset vector

        // Disable interrupts before modifying VTOR and MSP
        "cpsid i",                    // Disable interrupts (set PRIMASK)

        // Update VTOR to point to admin firmware's vector table
        "ldr r3, ={vtor_reg}",        // Load VTOR register address (0xE000ED08)
        "str r0, [r3]",               // Write admin-app base address to VTOR
        "dsb sy",                     // Data synchronization barrier
        "isb",                        // Instruction synchronization barrier

        // Switch to Main Stack Pointer (MSP)
        "mov r3, #0",                 // tmp = 0
        "mrs r3, CONTROL",            // Read CONTROL register into tmp
        "bics r3, r3, #2",            // Clear SPSEL bit (bit 1) to select MSP, set flags
        "msr CONTROL, r3",            // Write back to CONTROL
        "isb",                        // Instruction barrier to ensure CONTROL change takes effect

        // Set new stack pointer and jump to admin firmware
        "msr MSP, r1",                // Set Main Stack Pointer to admin-app's initial SP

        // Re-enable interrupts before jumping to admin-app
        "cpsie i",                    // Enable interrupts (clear PRIMASK)

        "bx r2",                      // Branch to admin-app reset vector (never returns)

        // HSM core: Return to continue normal mcr-app boot
        "1:",                         // Label for HSM core
        "bx lr",                      // Return to caller, Reset handler continues to execute mcr-app

        cpu_id_reg = const CPU_ID_REG_ADDR,
        admin_id = const ADMIN_CORE_ID,
        admin_base = const ADMIN_APP_BASE,
        vtor_reg = const VTOR_REG_ADDR,
    );
}

/// Entry function of demo application to handle IO
#[cfg(not(feature = "std"))]
#[entry]
fn app_entry() -> ! {
    #[cfg(feature = "log_level_info")]
    mcr_logger::Logger::init(log::LevelFilter::Info);
    #[cfg(not(feature = "log_level_info"))]
    mcr_logger::Logger::init(log::LevelFilter::Trace);

    match mcr_cpu::cpu_id() {
        CpuId::Admin => {
            panic!(
                "The control should be transferred to admin-app, and mcr-app should not be
                 running from Admin core"
            );
        }
        CpuId::Hsm => {
            use mcr_cpu::stack_guard::StackGuard;

            extern "C" {
                static __stack_limit__: u32; // lowest valid stack address
            }
            let stack_limit = unsafe { &__stack_limit__ as *const u32 as u32 };

            // Configure MPU region for stack guard.
            let stack_guard_configured = unsafe { StackGuard::configure(stack_limit) };

            // Initialize heap
            const HEAP_SIZE: usize = 118 * 1024;
            static mut HEAP_MEM: [MaybeUninit<u8>; HEAP_SIZE] = [MaybeUninit::uninit(); HEAP_SIZE];
            #[allow(static_mut_refs)]
            unsafe {
                HEAP.init(HEAP_MEM.as_ptr() as usize, HEAP_SIZE)
            }

            if !stack_guard_configured {
                error!("Failed to configure stack guard: invalid stack limit");
            }

            trace!("HSM core running");
            run()
        }
        CpuId::Unknown => {
            trace!("Unknown core!!!");
            loop {}
        }
    }
}

#[cfg(feature = "std")]
fn main() {}
