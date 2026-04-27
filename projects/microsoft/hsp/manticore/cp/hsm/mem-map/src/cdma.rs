// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_mem_map_derive::mem_map;

/// CDMA Memory Map
#[mem_map(address = 0xA0C0_1000, length = 0x4000)]
pub struct CdmaMemMap {
    #[field(cardinality = 4096, mutable = true)]
    key_vault: u32,
}

#[cfg(test)]
mod tests {
    use static_assertions as sa;

    use super::*;

    #[test]
    fn test_key_vault_address() {
        assert_eq!(CdmaMemMap::BASE_ADDRESS, 0xA0C0_1000);
    }

    #[test]
    fn test_key_vault() {
        assert_eq!(CdmaMemMap::KEY_VAULT_SIZE, 0x4000);
        assert_eq!(CdmaMemMap::KEY_VAULT_OFFSET, CdmaMemMap::BASE_ADDRESS,);
        assert_eq!(CdmaMemMap::KEY_VAULT_OFFSET % 4, 0);
        sa::const_assert!(
            CdmaMemMap::KEY_VAULT_OFFSET + CdmaMemMap::KEY_VAULT_SIZE
                <= CdmaMemMap::BASE_ADDRESS + CdmaMemMap::LENGTH
        );
    }
}
