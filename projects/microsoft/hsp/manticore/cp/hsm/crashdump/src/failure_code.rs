// Copyright (c) Microsoft Corporation. All rights reserved.

#[derive(Debug, PartialEq)]
/// IRQ number
pub enum IRQn {
    /// Non-maskable interrupt IRQn
    IrqnNonMaskableInterrupt = -14,

    /// Hard fault IRQn
    IrqnHardFault = -13,

    /// Memory fault IRQn
    IrqnMemoryFault = -12,

    /// Bus fault IRQn
    IrqnBusFault = -11,

    /// Usage fault IRQn
    IrqnUsageFault = -10,

    /// Secure fault IRQn
    IrqnSecureFault = -9,

    /// SVCall IRQn
    IrqnSVCall = -5,

    /// Debug monitor IRQn
    IrqnDebugMonitor = -4,

    /// PendSV IRQn
    IrqnPendSV = -3,

    /// SysTick IRQn
    IrqnSysTick = -1,

    /// Invalid IRQn
    IrqnInvalid = 0,
}

impl From<i16> for IRQn {
    fn from(value: i16) -> Self {
        match value {
            x if x == IRQn::IrqnNonMaskableInterrupt as i16 => IRQn::IrqnNonMaskableInterrupt,
            x if x == IRQn::IrqnHardFault as i16 => IRQn::IrqnHardFault,
            x if x == IRQn::IrqnMemoryFault as i16 => IRQn::IrqnMemoryFault,
            x if x == IRQn::IrqnBusFault as i16 => IRQn::IrqnBusFault,
            x if x == IRQn::IrqnUsageFault as i16 => IRQn::IrqnUsageFault,
            x if x == IRQn::IrqnSecureFault as i16 => IRQn::IrqnSecureFault,
            x if x == IRQn::IrqnSVCall as i16 => IRQn::IrqnSVCall,
            x if x == IRQn::IrqnDebugMonitor as i16 => IRQn::IrqnDebugMonitor,
            x if x == IRQn::IrqnPendSV as i16 => IRQn::IrqnPendSV,
            x if x == IRQn::IrqnSysTick as i16 => IRQn::IrqnSysTick,
            _ => IRQn::IrqnInvalid,
        }
    }
}

/// Failure code for the crash dump
#[derive(Clone, Debug, PartialEq)]
pub enum FailureCode {
    // Unknown failure code
    Unknown = 0,

    // Non-maskable interrupt
    NonMaskableInterrupt = 1,

    // Hard fault
    HardFault = 2,

    // Memory fault
    MemoryFault = 3,

    // Bus fault
    BusFault = 4,

    // Usage fault
    UsageFault = 5,

    // Secure fault
    SecureFault = 6,

    // SVCall
    SVCall = 7,

    // Debug monitor
    DebugMonitor = 8,

    // PendSV
    PendSV = 9,

    // SysTick
    SysTick = 10,

    // Panic
    Panic = 11,

    // Watchdog reset as sent by HSP
    Watchdog = 12,

    // Stack overflow detected
    StackOverflow = 13,

    // Double fault
    DoubleFault = 14,

    // Triggered by other other core
    OtherCore = 15,

    // Explicitly triggered on unrecoverable failure
    ExplicitFailure = 16,

    // RNG self test failure
    RngSelfTestFailure = 20,

    // Double bit error
    DoubleBitErr = 21,

    // GDMA Data Structure Error
    GdmaDataStructureError = 22,

    // GDMA Data Access Error
    GdmaDataAccessError = 23,

    // GDMA Completion Queue Error
    GdmaCompletionQueueError = 24,

    // GDMA Delivery Queue Error
    GdmaDeliveryQueueError = 25,

    // UCD IB DFL Overflow Error
    UcdIbDflOverflowError = 30,

    // UCD IB Queue Overflow Error
    UcdIbQueueOverflowError = 31,

    // UCD OB Queue Full Error
    UcdObQueueFullError = 32,

    // UCD IB Completion Queue Full Error
    UcdIbCqFullError = 33,
}

impl From<IRQn> for FailureCode {
    fn from(value: IRQn) -> Self {
        match value {
            IRQn::IrqnNonMaskableInterrupt => FailureCode::NonMaskableInterrupt,
            IRQn::IrqnHardFault => FailureCode::HardFault,
            IRQn::IrqnMemoryFault => FailureCode::MemoryFault,
            IRQn::IrqnBusFault => FailureCode::BusFault,
            IRQn::IrqnUsageFault => FailureCode::UsageFault,
            IRQn::IrqnSecureFault => FailureCode::SecureFault,
            IRQn::IrqnSVCall => FailureCode::SVCall,
            IRQn::IrqnDebugMonitor => FailureCode::DebugMonitor,
            IRQn::IrqnPendSV => FailureCode::PendSV,
            IRQn::IrqnSysTick => FailureCode::SysTick,
            IRQn::IrqnInvalid => FailureCode::Unknown,
        }
    }
}

#[test]
fn test_failure_code() {
    assert_eq!(
        FailureCode::from(IRQn::IrqnNonMaskableInterrupt),
        FailureCode::NonMaskableInterrupt
    );
    assert_eq!(
        FailureCode::from(IRQn::IrqnHardFault),
        FailureCode::HardFault
    );
    assert_eq!(
        FailureCode::from(IRQn::IrqnMemoryFault),
        FailureCode::MemoryFault
    );
    assert_eq!(FailureCode::from(IRQn::IrqnBusFault), FailureCode::BusFault);
    assert_eq!(
        FailureCode::from(IRQn::IrqnUsageFault),
        FailureCode::UsageFault
    );
    assert_eq!(
        FailureCode::from(IRQn::IrqnSecureFault),
        FailureCode::SecureFault
    );
    assert_eq!(FailureCode::from(IRQn::IrqnSVCall), FailureCode::SVCall);
    assert_eq!(
        FailureCode::from(IRQn::IrqnDebugMonitor),
        FailureCode::DebugMonitor
    );
    assert_eq!(FailureCode::from(IRQn::IrqnPendSV), FailureCode::PendSV);
    assert_eq!(FailureCode::from(IRQn::IrqnSysTick), FailureCode::SysTick);
    assert_eq!(FailureCode::from(IRQn::IrqnInvalid), FailureCode::Unknown);
}

#[test]
fn test_irqn() {
    assert_eq!(
        IRQn::from(IRQn::IrqnNonMaskableInterrupt as i16),
        IRQn::IrqnNonMaskableInterrupt
    );
    assert_eq!(IRQn::from(IRQn::IrqnHardFault as i16), IRQn::IrqnHardFault);
    assert_eq!(
        IRQn::from(IRQn::IrqnMemoryFault as i16),
        IRQn::IrqnMemoryFault
    );
    assert_eq!(IRQn::from(IRQn::IrqnBusFault as i16), IRQn::IrqnBusFault);
    assert_eq!(
        IRQn::from(IRQn::IrqnUsageFault as i16),
        IRQn::IrqnUsageFault
    );
    assert_eq!(
        IRQn::from(IRQn::IrqnSecureFault as i16),
        IRQn::IrqnSecureFault
    );
    assert_eq!(IRQn::from(IRQn::IrqnSVCall as i16), IRQn::IrqnSVCall);
    assert_eq!(
        IRQn::from(IRQn::IrqnDebugMonitor as i16),
        IRQn::IrqnDebugMonitor
    );
    assert_eq!(IRQn::from(IRQn::IrqnPendSV as i16), IRQn::IrqnPendSV);
    assert_eq!(IRQn::from(IRQn::IrqnSysTick as i16), IRQn::IrqnSysTick);
    assert_eq!(IRQn::from(0), IRQn::IrqnInvalid);
}
