// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

cfg_if::cfg_if! {
    if #[cfg(not(feature = "std"))] {
        use core::arch::global_asm;
        use cortex_m_rt::exception;
        use cortex_m_rt::ExceptionFrame;
        use crate::alloc::string::ToString;
        use log::*;
        use mcr_crashdump::crash_format::CpuRegisterContext;
        use mcr_crashdump::failure_code::FailureCode;
        use mcr_crashdump::failure_code::IRQn;
        use mcr_gdma_controller::*;
        use mcr_gsram_controller::*;
        use mcr_interrupt_controller::*;
        use mcr_io_controller::*;
        use mcr_logging::*;
        use mcr_mailbox_controller::*;
        use mcr_soc::SocInfo;
        use mcr_soc::SocInfoTrait;
        use mcr_tcon::*;
        use mcr_types::*;
    }
}

#[cfg(not(feature = "std"))]
#[panic_handler]
pub fn panic(info: &core::panic::PanicInfo) -> ! {
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    let mut context = CpuRegisterContext {
        ..Default::default()
    };

    error!("#### Panic: {}\n", info);
    trace!("Notifying Crash to to other cores");
    Tcon::fire_wakeup_timer1();

    // Location information
    if let Some(location) = info.location() {
        trace!(
            "Panic location: file_ptr={:#x}, line={}",
            location.file().as_ptr() as u32,
            location.line(),
        );

        context.lr = location.file().as_ptr() as u32;
        context.pc = location.line();
    }

    if let Some(message) = info.message().as_str() {
        trace!(
            "Panic message: {} message_ptr={:#x}",
            message,
            message.as_ptr() as u32
        );
        context.sp = message.as_ptr() as u32;
    }

    // Special case reuse of register fields to report a Rustlang panic:
    // - lr: offset of the panic filename within the .text section (mcr-app.text.bin)
    // - pc: line number within the filen
    // - sp: offset of the panic message within the .text section (mcr-app.text.bin)
    mcr_crashdump::crashdump_save(
        &context,
        FailureCode::Panic,
        Some(&info.message().to_string()),
    );

    loop {}
}

/// Hardfault handler
#[cfg(not(feature = "std"))]
#[exception]
unsafe fn HardFault(ef: &ExceptionFrame) -> ! {
    let scb = unsafe { &*cortex_m::peripheral::SCB::PTR };

    // HFSR bit 30 = FORCED: fault escalated from a configurable-priority fault
    // CFSR bit 4  = MSTKERR: MemManage stacking error (push of exception frame failed)
    let cfsr = scb.cfsr.read();
    let hfsr = scb.hfsr.read();
    let forced = (hfsr & (1 << 30)) != 0;
    let mstkerr = (cfsr & (1 << 4)) != 0;

    let msp = cortex_m::register::msp::read();

    let failure_code = if forced && mstkerr {
        // Stack overflow: MemManage fired but exception stacking also hit the
        // MPU guard region, escalating to HardFault. The exception frame is
        // unreliable (stacking failed — all fields are garbage/zero).
        //
        // We CANNOT recover PC/LR of the faulting function because:
        //   1. The CPU saves {r0,r1,r2,r3,r12,lr,pc,xpsr} onto the stack during exception entry
        //   2. That stacking write failed (hit the MPU guard)
        //   3. The registers are lost — no backup mechanism exists in ARMv7-M
        //   4. Only MSP tells us approximately where the stack was at fault time
        error!(
            "#### HardFault (stack overflow): MSP={:#010x}, CFSR={:#010x}, HFSR={:#010x}",
            msp, cfsr, hfsr
        );
        FailureCode::MemoryFault
    } else {
        error!("#### HardFault: {:#?}", ef);
        error!("CFSR={:#010x}, HFSR={:#010x}", cfsr, hfsr);
        FailureCode::HardFault
    };

    trace!("Disabling tcon_wakeup1_irq");
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    trace!("Notifying Crash to other cores");
    Tcon::fire_wakeup_timer1();

    let register_context = if forced && mstkerr {
        // Exception frame is unreliable; record MSP instead
        CpuRegisterContext {
            sp: msp,
            ..Default::default()
        }
    } else {
        let mut ctx = CpuRegisterContext::from_exception_frame(ef);
        ctx.sp = ef as *const _ as u32 + size_of::<ExceptionFrame>() as u32;
        ctx
    };
    mcr_crashdump::crashdump_save(&register_context, failure_code, None);

    loop {}
}

/// MemoryManagement fault handler
#[cfg(not(feature = "std"))]
#[exception]
unsafe fn MemoryManagement() -> ! {
    error!("#### MemoryManagement Fault");

    let scb = unsafe { &*cortex_m::peripheral::SCB::PTR };
    let cfsr = scb.cfsr.read();
    let mmfar = scb.mmfar.read();

    match mcr_cpu::cpu_id() {
        mcr_cpu::CpuId::Admin => {
            log_admin_error_message!(
                "Memory management fault received. CFSR={:#x}, MMFAR={:#x}",
                cfsr,
                mmfar
            );
        }
        _ => {
            log_hsm_error_message!(
                "Memory management fault received. CFSR={:#x}, MMFAR={:#x}",
                cfsr,
                mmfar
            );
        }
    }

    trace!("Disabling tcon_wakeup1_irq");
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    trace!("Notifying Crash to other cores");
    Tcon::fire_wakeup_timer1();

    // Record MSP for crash analysis. We cannot reliably read the exception
    // frame here because the compiler prologue has already modified MSP.
    // A proper fix requires an assembly trampoline (like tcon_wakeup1_irq).
    let register_context = CpuRegisterContext {
        sp: cortex_m::register::msp::read(),
        ..Default::default()
    };
    mcr_crashdump::crashdump_save(&register_context, FailureCode::MemoryFault, None);

    loop {}
}

/// Default handler
#[cfg(not(feature = "std"))]
#[exception]
unsafe fn DefaultHandler(irqn: i16) {
    error!("#### DefaultHandler: IRQn {}", irqn);

    trace!("Disabling tcon_wakeup1_irq");
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    trace!("Notifying Crash to to other cores");
    Tcon::fire_wakeup_timer1();

    mcr_crashdump::crashdump_save(
        &CpuRegisterContext::default(),
        FailureCode::from(<i16 as Into<IRQn>>::into(irqn)),
        None,
    );

    loop {}
}

/// RNG interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn rng_error_irq() {
    InterruptController::default().disable(Interrupt::rng_error_irq);
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    // Although the exception handler could be triggered on any core, RNG hardware error is expected
    // to get triggered as a result of an operation in the HSM core, hence logging in HSM core context.
    log_hsm_error_message!(
        "RNG Hardware error received. Fault code: {}",
        FailureCode::RngSelfTestFailure as u32
    );

    mcr_crashdump::crashdump_save(
        &CpuRegisterContext::default(),
        FailureCode::RngSelfTestFailure,
        None,
    );

    // Notify the HSP about the error using the mailbox
    let mbx_err = MailboxController::create(MailboxId::Mailbox0);
    mbx_err.trigger_mbx_err();

    loop {}
}

#[cfg(not(feature = "std"))]
fn tcm_err_handler(interrupt: Interrupt) -> ! {
    InterruptController::default().disable(interrupt);
    InterruptController::default().clear(interrupt);
    match interrupt {
        Interrupt::cp0_dtcm_err_irq => {
            log_admin_error_message!(
                "DTCM error received. Fault code: {}",
                FailureCode::DoubleBitErr as u32
            );
        }
        Interrupt::cp1_dtcm_err_irq => {
            log_hsm_error_message!(
                "DTCM error received. Fault code: {}",
                FailureCode::DoubleBitErr as u32
            );
        }
        Interrupt::cp0_itcm_err_irq => {
            log_admin_error_message!(
                "ITCM error received. Fault code: {}",
                FailureCode::DoubleBitErr as u32
            );
        }
        Interrupt::cp1_itcm_err_irq => {
            log_hsm_error_message!(
                "ITCM error received. Fault code: {}",
                FailureCode::DoubleBitErr as u32
            );
        }

        _ => (),
    }

    // Note: During DTCM error handling, both CP0 and CP1 fails to drop to timer interrupt handler to
    // disable the timer interrupt from keep firing. Currently FP cores cannot disable the timer interrupt
    // from its interrupt handler, so we are using the busy loop in the DTCM error handler to disable the
    // timer if it is fired by SPRT firmware to trigger crash dump.
    loop {
        if InterruptController::default().pending(Interrupt::tcon_wakeup1_irq) {
            Tcon::disable_wakeup_timer1();
            InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);
            InterruptController::default().clear(Interrupt::tcon_wakeup1_irq);
        }
        cortex_m::asm::nop(); // Minimal pause to reduce CPU usage
    }
}

/// CP0 DTCM ERR interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn cp0_dtcm_err_irq() {
    tcm_err_handler(Interrupt::cp0_dtcm_err_irq);
}

/// CP1 DTCM ERR interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn cp1_dtcm_err_irq() {
    tcm_err_handler(Interrupt::cp1_dtcm_err_irq);
}

/// GSRAM interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn gsram_irq() {
    InterruptController::default().disable(Interrupt::gsram_irq);
    InterruptController::default().clear(Interrupt::gsram_irq);

    match GsramController::handle_double_bit_error() {
        None => {
            error!("GSRAM double bit ECC error received.");
        }
        Some((errslog0_info, errslog1_info)) => {
            // log message will be helpful while there is any (if in-case) other GSRAM errors occur
            error!(
                "GSRAM error received. errslog0: {:#x}, errslog1: {:#x}",
                errslog0_info, errslog1_info
            );
        }
    }

    // Except Single/Double bit ECC errors if any unknown GSRAM errors occur
    // CP0 will hang after soft reset until SOC reset(POR)
    loop {}
}

/// CP0 ITCM ERR interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn cp0_itcm_err_irq() {
    tcm_err_handler(Interrupt::cp0_itcm_err_irq);
}

/// CP1 ITCM ERR interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn cp1_itcm_err_irq() {
    tcm_err_handler(Interrupt::cp1_itcm_err_irq);
}

// gdma error interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn gdma_err_irq() {
    InterruptController::default().disable(Interrupt::gdma_err_irq);
    InterruptController::default().clear(Interrupt::gdma_err_irq);

    // Read cause bits
    let raw_bits = GdmaController::read_gdma_interrupt_cause();

    // If multiple faults are set simultaneously, log all of them
    const GDMA_DATA_STRCTR_BIT: u32 = 1 << 23;
    const GDMA_DATA_ACCESS_BIT: u32 = 1 << 22;
    const GDMA_DQ_BIT: u32 = 1 << 16;
    const GDMA_CQ_BIT: u32 = 1 << 17;

    if (raw_bits & GDMA_DATA_STRCTR_BIT) != 0 {
        log_admin_error_message!(
            "GDMA Data Structure Error. Fault code: {}",
            FailureCode::GdmaDataStructureError as u32
        );

        // Clear the cause bits after logging
        GdmaController::clear_data_structure_error_cause();
        // Re-enable the IRQ so future GDMA error interrupts can be delivered.
        InterruptController::default().enable(Interrupt::gdma_err_irq);

        return;
    }

    // reset GDMA to avoid further interrupts.
    GdmaController::reset_gdma();

    if (raw_bits & GDMA_DATA_ACCESS_BIT) != 0 {
        log_admin_error_message!(
            "GDMA Data Access Error. Fault code: {}",
            FailureCode::GdmaDataAccessError as u32
        );
    }
    if (raw_bits & GDMA_DQ_BIT) != 0 {
        log_admin_error_message!(
            "GDMA Delivery Queue Error. Fault code: {}",
            FailureCode::GdmaDeliveryQueueError as u32
        );
    }
    if (raw_bits & GDMA_CQ_BIT) != 0 {
        log_admin_error_message!(
            "GDMA Completion Queue Error. Fault code: {}",
            FailureCode::GdmaCompletionQueueError as u32
        );
    }

    trace!("Disabling tcon_wakeup1_irq");
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    trace!("Notifying Crash to other cores");
    Tcon::fire_wakeup_timer1();

    loop {}
}

// Common UCD error handler.
#[cfg(not(feature = "std"))]
fn ucd_err_handler(interrupt: Interrupt) {
    InterruptController::default().disable(interrupt);
    InterruptController::default().clear(interrupt);

    // IB interrupt cause bit masks
    const IB_DATA_PATH_ERR: u32 = 1 << 31; // bit 31
    const DFL_OVRFLW_MASK: u32 = 0x3F << 23; // bits 23-28
    const DFL_EMPTY_MASK: u32 = 0x3F << 17; // bits 17-22
    const IQ_SOFT_ERR: u32 = 1 << 13; // bit 13
    const IB_CQ_FULL_MASK: u32 = 0x1F << 8; // bits 8-12
    const CQ_FULL_MASK: u32 = 0x1F << 17; // bits 17-21

    match interrupt {
        Interrupt::ucd_ib_err_irq => {
            // Clear all IB interrupt 1 enable bits to prevent level-sensitive
            // conditions from re-firing after warm reset.
            IoController::clear_ib_interrupt_1_enable();

            let (c0, c1) = IoController::read_ib_error_cause();
            let cause = c0 | c1;

            // Check data path error (bit 31) first — this is a halting
            // hardware error that corrupts IB data.
            if (cause & IB_DATA_PATH_ERR) != 0 {
                log_admin_error_message!(
                    "UCD IB Data Path Parity Error. c0=0x{:08x}, c1=0x{:08x}",
                    c0,
                    c1
                );
            } else if (cause & IQ_SOFT_ERR) != 0 {
                log_admin_error_message!(
                    "UCD IB Queue Overflow Error. Fault code: {}",
                    FailureCode::UcdIbQueueOverflowError as u32
                );
            } else if (cause & IB_CQ_FULL_MASK) != 0 {
                log_admin_error_message!(
                    "UCD IB Completion Queue Full Error. Fault code: {}",
                    FailureCode::UcdIbCqFullError as u32
                );
            } else if (cause & (DFL_OVRFLW_MASK | DFL_EMPTY_MASK)) != 0 {
                log_admin_error_message!(
                    "UCD IB DFL Overflow Error. Fault code: {}",
                    FailureCode::UcdIbDflOverflowError as u32
                );
            } else {
                log_admin_error_message!("UCD IB error. c0=0x{:08x}, c1=0x{:08x}", c0, c1);
            }
        }
        Interrupt::ucd_ob_err_irq => {
            // Clear all OB interrupt 1 enable bits to prevent level-sensitive
            // conditions from re-firing after warm reset.
            IoController::clear_ob_interrupt_1_enable();

            let (c0, c1) = IoController::read_ob_error_cause();
            let cause = c0 | c1;

            if (cause & CQ_FULL_MASK) != 0 {
                log_admin_error_message!(
                    "UCD OB Queue Full Error. Fault code: {}",
                    FailureCode::UcdObQueueFullError as u32
                );
            } else {
                log_admin_error_message!("UCD OB error. c0=0x{:08x}, c1=0x{:08x}", c0, c1);
            }
        }
        _ => (),
    }

    trace!("Disabling tcon_wakeup1_irq");
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);

    trace!("Notifying Crash to other cores");
    Tcon::fire_wakeup_timer1();

    loop {}
}

// UCD Inbound error interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn ucd_ib_err_irq() {
    ucd_err_handler(Interrupt::ucd_ib_err_irq);
}

// UCD Outbound error interrupt handler
#[cfg(not(feature = "std"))]
#[interrupt]
unsafe fn ucd_ob_err_irq() {
    ucd_err_handler(Interrupt::ucd_ob_err_irq);
}

// tcon wakeup1 ISR. This ISR is implemented as an assembly trampoline to collect
// stack frame of the code that was executing at the time the IRS was triggered.
// The exception frame is passed to the collect_crash_dump_tcon_irq function to
// collect the crash dump.
#[cfg(not(feature = "std"))]
global_asm!(
    ".global tcon_wakeup1_irq
     .type tcon_wakeup1_irq,%function
     .thumb_func
     .cfi_startproc
     tcon_wakeup1_irq:",
    "mov r0, lr
     movs r1, #4
     tst r0, r1
     bne 0f
     mrs r0, MSP
     b collect_crash_dump_tcon_irq
     0:
     mrs r0, PSP
     b collect_crash_dump_tcon_irq",
    ".cfi_endproc
     .size tcon_wakeup1_irq, . - tcon_wakeup1_irq",
);

/// Handler to collect crash dump triggered by tcon_wakeup1_irq
#[cfg(not(feature = "std"))]
#[allow(dead_code)]
#[no_mangle]
fn collect_crash_dump_tcon_irq(ef: &ExceptionFrame) -> ! {
    use cortex_m_rt::ExceptionFrame;
    use mcr_crashdump::crash_format::CpuRegisterContext;

    Tcon::disable_wakeup_timer1();
    InterruptController::default().disable(Interrupt::tcon_wakeup1_irq);
    InterruptController::default().clear(Interrupt::tcon_wakeup1_irq);

    match mcr_cpu::cpu_id() {
        mcr_cpu::CpuId::Admin => {
            // Reset GDMA to avoid any pending interrupts
            let soc_info = SocInfo::default();
            soc_info.reset_gdma();

            // Pause the HSM + Admin UCD controller instance
            let admin_io_cntrl = IoController::new(IoControllerId::Core0);
            admin_io_cntrl.pause_inbound();

            // Pause the Fast Path UCD controller instance
            let hsm_io_cntrl = IoController::new(IoControllerId::Core1);
            hsm_io_cntrl.pause_inbound();
        }
        _ => (),
    };

    let mut register_context = CpuRegisterContext::from_exception_frame(ef);
    register_context.sp = ef as *const _ as u32 + size_of::<ExceptionFrame>() as u32;
    mcr_crashdump::crashdump_save(&register_context, FailureCode::OtherCore, None);

    #[allow(clippy::empty_loop)]
    loop {}
}
