// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod debug_log_sender;

extern crate alloc;
#[cfg(test)]
mod tests;

pub use debug_log_sender::*;

#[macro_use]
pub mod logging_macros;
