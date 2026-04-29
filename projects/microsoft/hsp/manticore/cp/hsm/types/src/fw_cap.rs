// Copyright (c) Microsoft Corporation. All rights reserved.

use bitfield_struct::bitfield;

/// Firmware Capabilities
#[bitfield(u32)]
pub struct FwCapabilities {
    /// Support Live Migration
    lm_support: bool,

    /// Firmware supports reporting SVN part of identify controller response
    svn_support: bool,

    /// Reserved
    #[bits(30)]
    rsvd: u32,
}

impl Default for FwCapabilities {
    /// Default capabilities supported by this firmware
    ///
    /// Notes:
    ///  - Bit0: Live Migration
    fn default() -> Self {
        Self::new().with_lm_support(true).with_svn_support(true)
    }
}

/// Runtime Firmware Capabilities
#[bitfield(u32)]
pub struct RtFwCapabilities {
    /// AES GCM WA
    aes_gcm_wa: bool,

    /// Reserved
    #[bits(31)]
    rsvd: u32,
}

impl Default for RtFwCapabilities {
    /// Default runtime capabilities supported by this firmware
    ///
    /// Notes:
    ///  - Bit0: AES GCM WA
    fn default() -> Self {
        Self::new().with_aes_gcm_wa(true)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lm_support() {
        let fw_cap = FwCapabilities::default();

        assert!(fw_cap.lm_support());
        assert!(fw_cap.svn_support());
        assert_eq!(fw_cap.rsvd(), 0);

        let raw_data: u32 = fw_cap.into();

        assert_eq!(raw_data, 0x00000003);
    }

    #[test]
    fn test_new() {
        let fw_cap = FwCapabilities::new();

        assert!(!fw_cap.lm_support());
        assert!(!fw_cap.svn_support());
        assert_eq!(fw_cap.rsvd(), 0);

        let raw_data: u32 = fw_cap.into();

        assert_eq!(raw_data, 0x00000000);
    }
}
