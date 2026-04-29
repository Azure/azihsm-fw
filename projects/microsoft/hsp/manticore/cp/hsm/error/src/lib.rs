// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

/// Manticore Component
#[repr(u16)]
#[derive(Debug, Clone, Copy, Eq, PartialEq)]
pub enum McrComponent {
    /// Interrupt Controller
    InterruptController = 0x01,

    /// IO Controller
    IoController = 0x02,

    /// Queue Controller
    QueueController = 0x03,

    /// IPC Controller
    IpcController = 0x04,

    /// Gdma Controller
    GdmaController = 0x05,

    /// Pcie Controller
    PcieController = 0x6,

    /// Sha Engine
    Sha = 0x07,

    /// PKA Engine
    Pka = 0x08,

    /// IPC message
    IpcMessage = 0x09,

    /// UART Controller
    Uart = 0x0A,

    /// Admin processor firmware
    Admin = 0x0B,

    /// AES Engine
    Aes = 0x0C,

    /// Key Vault
    KeyVault = 0x0D,

    /// HSM core firmware
    Hsm = 0x0E,

    /// RNG engine
    Rng = 0x0F,

    /// PCIe Doe
    Doe = 0x10,

    /// Tcon
    Tcon = 0x11,

    /// MemLog
    MemLog = 0x12,

    /// Simplex Pipe
    SimplexPipe = 0x13,

    /// SoftAes
    SoftAes = 0x14,

    /// CdmaIo
    CdmaIo = 0x15,
}

#[macro_export]
macro_rules! mcr_err_decl {
    ($comp_name:ident, $enum_name: ident { $($field_name: ident = $field_val: literal,)* }) => {
        /// Component specific error
        #[derive(Debug, Copy, Clone, Eq, PartialEq)]
        #[allow(clippy::enum_variant_names)]
        #[repr(u32)]
        pub(crate) enum $enum_name {
            $($field_name =((($crate::McrComponent::$comp_name) as u32) << 16) | ($field_val ),)*
        }

        impl From<$enum_name> for u32 {
            fn from(val: $enum_name) -> Self {
                ((($crate::McrComponent::$comp_name) as Self) << 16) | (val as Self)
            }
        }
    }
}

pub type McrResult<T> = Result<T, u32>;
