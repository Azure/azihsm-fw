// Copyright (c) Microsoft Corporation. All rights reserved.

#![no_std]

mod soc_info;

#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::NegKind;
#[cfg(feature = "fips_validation_hooks")]
use mcr_self_test::SelfTest;
pub use soc_info::SocInfo;

/// Manticore SoC Reset Type as populated by the boot processor
#[derive(Default, PartialEq, Eq)]
pub enum SocResetType {
    /// Power On Reset
    #[default]
    Por = 0,

    /// Warm Reset, issued during any fault recovery
    WarmReset = 1,

    /// Firmware Update Warm Reset
    FwUpdateWarmReset = 2,
}

impl From<u32> for SocResetType {
    fn from(value: u32) -> Self {
        match value {
            1 => Self::WarmReset,
            2 => Self::FwUpdateWarmReset,
            _ => Self::default(),
        }
    }
}

/// SoC information trait
pub trait SocInfoTrait {
    /// Get SoC firmware package version
    ///
    /// # Returns
    ///
    /// * `[u8; 32]` containing ASCII-encoded firmware package version. Any indices in the array
    ///   following the string are filled with the ASCII space character ' '.
    fn fw_version(&self) -> [u8; 32];

    /// Get SoC ID
    ///
    /// # Returns
    ///
    /// * `[u8; 32]` containing ASCII-encoded SoC ID. Any indices in the array following the string
    ///   are filled with the ASCII space character ' '.
    fn id(&self) -> [u8; 32];

    /// Get Firmware SVN
    ///
    /// # Returns
    ///
    /// * `[u8; 8]` contains a 64-bit firmware SVN in little endian byte order.
    fn svn(&self) -> [u8; 8];

    /// Get Reset type
    ///
    /// # Returns
    ///
    /// * `SocResetType` returns any one of SoCReset Type
    fn reset_type(&self) -> SocResetType;

    /// Reset GDMA engine
    fn reset_gdma(&self);

    /// Reset NVMe interface
    fn reset_nvme(&self);

    /// Set negative cast indicator
    #[cfg(feature = "fips_validation_hooks")]
    fn set_negative_cast_hooks(&self, id: SelfTest);
}
