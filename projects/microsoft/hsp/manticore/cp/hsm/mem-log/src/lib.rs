// Copyright (c) Microsoft Corporation. All rights reserved.

#![warn(missing_docs)]
#![no_std]

//! Memory logger suitable for `no_std` environments.
//!
//! You can use the crate to write log messages to the provided memory buffer.
//! The memory logger is useful for debugging purposes when you don't wish to print to UART
//! for performance reasons.
//!
//! You can use the standard `core::fmt::Write` trait to write to the memory logger so
//! something like write!() or writeln!() would both work.

use mcr_error::mcr_err_decl;
use mcr_error::McrResult;

/// Memory logger
pub struct MemLog {
    base: usize,
    length: usize,
    offset: usize,
    circular: bool,
}

impl MemLog {
    /// Create a new memory logger
    ///
    /// # Arguments
    ///
    /// * `base` - Base address of the memory buffer
    /// * `length` - Length of the memory buffer
    /// * `circular` - Whether to have it as circular buffer or not. Circular applies
    ///   only at call level, partial bytes will not be written to end of buffer and to continue
    ///   to the beginning.
    ///
    /// # Returns
    ///
    /// * `MemLog` - Memory logger
    pub fn new(base: usize, length: usize, circular: bool) -> Self {
        MemLog {
            base,
            length,
            offset: 0,
            circular,
        }
    }

    /// Write a byte array to memory
    ///
    /// # Arguments
    ///
    /// * `data` - Slice of bytes that we need to write
    ///
    /// # Returns
    ///
    /// * Ok on success, Err on error
    pub fn write_bytes(&mut self, data: &[u8]) -> McrResult<()> {
        let len = data.len();

        if len == 0 {
            return Ok(());
        }

        // Check if we are wanting to write data which is bigger than the whole buffer
        if len > self.length {
            Err(MemLogErr::NotEnoughMemory)?;
        }

        if self.offset + len > self.length {
            if self.circular {
                self.offset = 0;
            } else {
                Err(MemLogErr::NotEnoughMemory)?;
            }
        }

        // SAFETY: Buffer length check is done above to ensure that the copy operation is safe.
        unsafe {
            core::ptr::copy(
                data.as_ptr() as *mut u8,
                &mut *(self.base as *mut u8).add(self.offset),
                len,
            )
        };

        self.offset += len;
        Ok(())
    }
}

impl core::fmt::Write for MemLog {
    fn write_str(&mut self, s: &str) -> Result<(), core::fmt::Error> {
        let data = s.as_bytes();
        self.write_bytes(data).map_err(|_| core::fmt::Error {})
    }
}

mcr_err_decl! {
    MemLog,
    MemLogErr
    {
        // Not enough memory is left to write the input data
        NotEnoughMemory = 1,
    }
}

#[cfg(test)]
mod tests {
    use core::fmt::Write;

    use super::*;

    #[test]
    fn test_mem_log() {
        let log_buffer = [0u8; 0x100];
        let log_buffer_address = log_buffer.as_ptr() as usize;

        let mut log = MemLog::new(log_buffer_address, log_buffer.len(), false);
        let result = writeln!(&mut log, "Hello, world!");
        assert!(result.is_ok());

        assert_eq!(&log_buffer[0..14], b"Hello, world!\n");
    }

    #[test]
    fn test_mem_log_capacity() {
        let log_buffer = [0u8; 10];
        let log_buffer_address = log_buffer.as_ptr() as usize;

        let mut log = MemLog::new(log_buffer_address, log_buffer.len(), false);

        for _ in 0..log_buffer.len() {
            let result = write!(&mut log, "H");
            assert!(result.is_ok());
        }

        assert_eq!(&log_buffer[..], b"HHHHHHHHHH");

        let result = write!(&mut log, "H");
        assert!(result.is_err());
        assert_eq!(&log_buffer[..], b"HHHHHHHHHH");
    }

    #[test]
    fn test_mem_log_capacity_circular() {
        let log_buffer = [0u8; 10];
        let log_buffer_address = log_buffer.as_ptr() as usize;

        let mut log = MemLog::new(log_buffer_address, log_buffer.len(), true);

        for _ in 0..log_buffer.len() {
            let result = write!(&mut log, "H");
            assert!(result.is_ok());
        }

        assert_eq!(&log_buffer[..], b"HHHHHHHHHH");

        let result = write!(&mut log, "B");
        assert!(result.is_ok());
        assert_eq!(&log_buffer[..], b"BHHHHHHHHH");
    }

    #[test]
    fn test_mem_log_capacity_beyond_capacity() {
        let log_buffer = [0u8; 10];
        let log_buffer_address = log_buffer.as_ptr() as usize;

        let mut log = MemLog::new(log_buffer_address, log_buffer.len(), true);

        let result = write!(&mut log, "0123456789A");
        assert!(result.is_err());
        assert_eq!(&log_buffer[..], [0; 10]);
    }
}
