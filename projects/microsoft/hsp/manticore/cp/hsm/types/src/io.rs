// Copyright (c) Microsoft Corporation. All rights reserved.

use open_enum::open_enum;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;

/// IO Controller Identifier
#[repr(u8)]
#[open_enum]
#[derive(Clone, Copy, PartialEq, Eq, IntoBytes, Immutable, FromBytes)]
pub enum IoControllerId {
    /// Core 0
    Core0 = 0,

    /// Core 1
    Core1 = 1,
}

// Io channel Id sequence macro
seq_macro::seq! {
    N in 0..5 {
        /// IO Channel Identifier
        #[repr(u8)]
        #[open_enum]
        #[derive(Clone, Copy, PartialEq, Eq, IntoBytes, Immutable, FromBytes)]
        pub enum IoChannelId {
            #(
                Channel~N = N,
            )*
        }
    }
}

impl From<IoChannelId> for usize {
    fn from(value: IoChannelId) -> Self {
        value.0 as usize
    }
}

impl From<IoChannelId> for u32 {
    fn from(value: IoChannelId) -> Self {
        value.0 as u32
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_iochannelid_from_iochannelid_for_usize() {
        assert_eq!(usize::from(IoChannelId::Channel0), 0usize);
        assert_eq!(usize::from(IoChannelId::Channel1), 1usize);
    }

    #[test]
    fn test_iochannelid_from_iochannelid_for_u32() {
        assert_eq!(u32::from(IoChannelId::Channel0), 0u32);
        assert_eq!(u32::from(IoChannelId::Channel1), 1u32);
    }
}
