// Copyright (c) Microsoft Corporation. All rights reserved.

/// DebugLogEntryParameters must match the debug_log_entry_parameters C structure
#[derive(Clone, Copy, PartialEq)]
#[repr(C)]
pub struct DebugLogEntryParameters {
    /// Debug log severity
    pub severity: DebugLogSeverity,

    /// Debug log component
    pub component: DebugLogComponent,

    /// Debug log message index
    pub msg_index: u8,

    /// Reserved field
    pub _reserved: u8,

    /// Debug log message argument 1
    pub arg1: u32,

    /// Debug log message argument 2
    pub arg2: u32,
}

/// DebugLogSeverity must match the Cerberus debug_log_severity C enumeration
#[repr(u8)]
#[derive(Clone, Copy, PartialEq)]
pub enum DebugLogSeverity {
    /// Error severity
    Error = 0,

    /// Warning severity
    Warning,

    /// Info severity
    Info,
}

/// DebugLogComponent must match the Cerberus debug_log_component C enumeration
/// Note: these device-speciffic component IDs are defined in the msft_debug_log.h header file.
#[repr(u8)]
#[derive(Clone, Copy, PartialEq)]
pub enum DebugLogComponent {
    // Device Specific Component IDs start at 0xf0,
    /// Log entry for HSP ROM, or ROM in general.
    MsftLoggingComponentHspRom = 0xf0,

    /// Log entry for HSP firmware messages.
    MsftLoggingComponentHsp,

    /// Log entry for MVDP message handling.
    MsftLoggingComponentMvdp,

    /// Log entry for Manticore SP firmware messages.
    MsftLoggingComponentManticoreSp,

    /// Log entry for Manticore CP Admin firmware messages.
    MsftLoggingComponentManticoreCp0,

    /// Log entry for Manticore CP HSM firmware messages.
    MsftLoggingComponentManticoreCp1,

    /// Log entry for Manticore FP0 firmware messages.
    MsftLoggingComponentManticoreFp0,

    /// Log entry for Manticore FP1 firmware messages.
    MsftLoggingComponentManticoreFp1,

    /// Log entry for Manticore FP2 firmware messages.
    MsftLoggingComponentManticoreFp2,

    /// Log entry for Overlake firmware messages.
    MsftLoggingComponentOverlake = 0xfe,

    /// Log entry for NXP LPC firmware messages.
    MsftLoggingComponentLpc,
}
