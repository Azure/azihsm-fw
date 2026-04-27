// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

extern crate alloc;

mod exception_handlers;

#[cfg(not(feature = "std"))]
pub use exception_handlers::*;
