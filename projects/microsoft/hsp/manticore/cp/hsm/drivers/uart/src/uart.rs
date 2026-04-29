// Copyright (c) Microsoft Corporation. All rights reserved.

use core::fmt;

use mcr_error::McrResult;
use mcr_registers::uart;

use crate::*;

/// Manticore UART
#[derive(Default)]
pub struct Uart {}

impl Uart {
    /// Read bytes from UART
    ///
    /// # Arguments
    ///
    /// `buf` - Output buffer to read into
    ///
    /// # Returns
    ///
    /// `Option<u8>` - Number of bytes read.
    pub fn read(&mut self, buf: &mut [u8]) -> Option<u8> {
        let mut i = 0;

        loop {
            let byte = self.read_byte()?;
            if byte == b'\0' || i >= buf.len() {
                return Some(i as u8);
            }

            buf[i] = byte;
            i += 1;
        }
    }

    /// Read a byte from UART
    ///
    /// # Arguments
    ///
    /// `byte` - Output for byte read from UART
    ///
    /// # Returns
    ///
    /// `Option<u8>` - The byte read, or None if timeout occurred.
    fn read_byte(&mut self) -> Option<u8> {
        let uart = uart::RegisterBlock::block();

        while !uart.status().read().rx_ready_uart() {}

        let val = u32::from(uart.receiver_buffer().read());

        Some((val & 0xff) as u8)
    }

    /// Write the string to UART
    ///
    /// # Arguments
    ///
    /// `str` - String to write to UART
    pub fn write(&mut self, str: &str) {
        for byte in str.bytes() {
            match byte {
                0x20..=0x7e | b'\n' | b'\t' => self.write_byte(byte),
                _ => self.write_byte(0xfe),
            }
        }
    }

    /// Write the byte to UART
    ///
    /// # Arguments
    ///
    /// `byte` - Byte to write to UART
    fn write_byte(&mut self, byte: u8) {
        let uart = uart::RegisterBlock::block();

        while !uart.status().read().tx_ready_uart() {}

        uart.transmitter_holding()
            .write(|w| w.uart_trans_hld_uart(byte as u32));
    }
}

impl fmt::Write for Uart {
    /// Writes a [`char`] into this writer, returning whether the write succeeded.
    fn write_str(&mut self, s: &str) -> fmt::Result {
        self.write(s);
        Ok(())
    }
}

impl ByteIo for Uart {
    fn read_bytes(&mut self, buf: &mut [u8]) -> McrResult<()> {
        for item in buf.iter_mut() {
            *item = match self.read_byte() {
                Some(c) => c,
                None => Err(UartErr::ReadFail)?,
            };
        }

        Ok(())
    }

    fn write_bytes(&mut self, buf: &[u8]) -> McrResult<()> {
        for item in buf.iter() {
            self.write_byte(*item);
        }

        Ok(())
    }
}
