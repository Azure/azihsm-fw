// Copyright (c) Microsoft Corporation. All rights reserved.

#![allow(unexpected_cfgs)]
#![cfg_attr(not(feature = "std"), no_std)]

mod uart;

use mcr_error::mcr_err_decl;
use mcr_error::McrResult;
pub use uart::*;

/// Interface to read and write bytes.
///
/// Note: There are similar existing traits outside of the std library (namely core::fmt::Write for
/// write functionality, or bare-io for read functionality), but the trait below is a simplified
/// version combining read and write functionality. This trait only includes read_exact() and
/// write_all(), making the trait more straightforward for firmware to implement. If broader use
/// cases arise for the read and write functionality typically found in pre-existing traits such as
/// core::fmt::Write and bare-io, then this trait can be expanded upon or deleted in favor of the
/// pre-existing traits.
pub trait ByteIo {
    /// Read the exact number of bytes needed to fill buffer.
    ///
    /// # Arguments
    ///
    /// * `buf` - Buffer to fill
    ///
    /// # Returns
    ///
    /// * McrResult<()> - Ok or an appropriate Err
    fn read_bytes(&mut self, buf: &mut [u8]) -> McrResult<()>;

    /// Write entire buffer into the writer.
    ///
    /// # Arguments
    ///
    /// * `buf` - Buffer to write
    ///
    /// # Returns
    ///
    /// * McrResult<()> - Ok or an appropriate Err
    fn write_bytes(&mut self, buf: &[u8]) -> McrResult<()>;
}

mcr_err_decl! {
    Uart,
    UartErr
    {
        // Read failure
        ReadFail = 0x1,

        // Write failure
        WriteFail = 0x2,
    }
}
