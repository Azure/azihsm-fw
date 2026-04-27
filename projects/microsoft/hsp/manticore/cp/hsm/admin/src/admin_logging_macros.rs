// Copyright (c) Microsoft Corporation. All rights reserved.

//! These are macros used for Telemetry debug logging mechanism.

#[macro_export]
macro_rules! info {
    ($fmt:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Info,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: 0,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Info,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr, $arg2:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Info,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: $arg2,
            });
        }
    };
}

#[macro_export]
macro_rules! warn {
    ($fmt:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Warning,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: 0,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Warning,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr, $arg2:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Warning,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: $arg2,
            });
        }
    };
}

#[macro_export]
macro_rules! error {
    ($fmt:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Error,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: 0,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Error,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: 0,
            });
        }
    };
    ($fmt:expr, $arg1:expr, $arg2:expr) => {
        let component_id = mcr_logging_derive::get_component_id_from_directory!("admin");

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        if let Some(debug_log_sender) = mcr_logging::get_debug_log_sender() {
            debug_log_sender.send(DebugLogEntryParameters {
                severity: DebugLogSeverity::Error,
                component: component_id,
                msg_index: message_index,
                _reserved: 0,
                arg1: $arg1,
                arg2: $arg2,
            });
        }
    };
}
