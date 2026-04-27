// Copyright (c) Microsoft Corporation. All rights reserved.

#![forbid(unsafe_code)]
#![warn(missing_docs)]
#![no_std]

//! Performance profiling library suitable for `no_std` environments.
//!
//! The crate contains two types of profilers:
//!
//! Method 1: Block style: detailed tree of call and timing summary. Not the best choice for super
//! high perf sensitive code paths since the library does look up existing sub tree nodes.
//! Method 2: Checkpoint style: measuring time from pointA to pointB to ... to pointN. It
//! is suitable for super high perf sensitive code paths.
//!
//! For either profiler you can fetch the profile log as a string so it can be sent
//! via the tracing/ telemetry system.

mod calltree;
mod checkpoint;

pub use calltree::*;
pub use checkpoint::*;
