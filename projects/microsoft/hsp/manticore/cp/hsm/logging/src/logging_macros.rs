// Copyright (c) Microsoft Corporation. All rights reserved.

//! These are macros used for Telemetry debug logging mechanism.

#[macro_export]
macro_rules! log_admin_error_message {
    ($fmt:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp0;

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: 0,
                    arg2: 0,
                });
            }
        }
    };
    ($fmt:expr, $arg1:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp0;

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: $arg1,
                    arg2: 0,
                });
            }
        }
    };
    ($fmt:expr, $arg1:expr, $arg2:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp0;

        let message_index = mcr_logging_derive::get_admin_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: $arg1,
                    arg2: $arg2,
                });
            }
        }
    };
}

#[macro_export]
macro_rules! log_hsm_error_message {
    ($fmt:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp1;

        let message_index = mcr_logging_derive::get_hsm_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: 0,
                    arg2: 0,
                });
            }
        }
    };
    ($fmt:expr, $arg1:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp1;

        let message_index = mcr_logging_derive::get_hsm_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: $arg1,
                    arg2: 0,
                });
            }
        }
    };
    ($fmt:expr, $arg1:expr, $arg2:expr) => {
        let component_id = DebugLogComponent::MsftLoggingComponentManticoreCp1;

        let message_index = mcr_logging_derive::get_hsm_message_index!($fmt);

        unsafe {
            if let Some(debug_log_sender) = mcr_logging::DEBUG_LOG_SENDER.as_ref() {
                debug_log_sender.send(DebugLogEntryParameters {
                    severity: DebugLogSeverity::Error,
                    component: component_id,
                    msg_index: message_index,
                    _reserved: 0,
                    arg1: $arg1,
                    arg2: $arg2,
                });
            }
        }
    };
}
