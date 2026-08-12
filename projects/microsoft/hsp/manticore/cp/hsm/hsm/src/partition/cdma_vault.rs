// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;
use mcr_types::IoMemRange;

use bitfield::Bit;
use bitfield::BitMut;
use mcr_ddi_types::DdiAesKeySize;

use mcr_crypto_cdma_io::MAX_KEYS_PER_TABLE;
use mcr_types::AesBulk256KeyId;

use crate::error::*;

///
/// CDMA vault memory layout: Total vault memory on FP for all 65 tables is 16KB
/// Each AES bulk 256 key is of size 32 bytes
/// No of AES entries that can be stored in 16KB is 16KB/32 = 512 entries
/// Each of the 65 tables will get 512/65 = ~7 key entries
/// CDMA vault meta data management: Each table has 7 entries so it takes at most
/// 1 byte where each bit will represent the availability of an entry
///
/// Maximum number of tables possible are 65
const MAX_TABLE_COUNT: usize = 65;

/// AES Bulk 256 key size is 32 bytes
const KEY_SIZE: usize = 32;

/// Each table can accommodate 7 AES Bulk 256 key(32 bytes each)
const TOTAL_TABLE_SIZE: usize = MAX_KEYS_PER_TABLE * KEY_SIZE;

#[allow(unused)]
#[derive(Clone)]
pub(crate) struct CdmaKeyVault {
    rimpl: Rc<RefCell<CdmaKeyVaultImpl>>,
}

#[allow(unused)]
impl CdmaKeyVault {
    pub(crate) fn new(vault_addr: usize, mask: u128, meta_data_base: usize) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(CdmaKeyVaultImpl::new(
                vault_addr,
                mask,
                meta_data_base,
            ))),
        }
    }

    /// Import AES key into the key store
    ///
    /// # Arguments
    ///
    /// * `key_blob` - AES 256 Bulk raw key blob to import
    ///
    /// # Return
    ///
    /// * Returns `Ok(AesBulk256KeyId)` if successful else `Err(HsmError)` if failed
    pub fn import_key(&mut self, key_blob: &[u8]) -> HsmResult<AesBulk256KeyId> {
        self.rimpl.borrow_mut().import_key(key_blob)
    }

    /// Delete key
    ///
    /// # Arguments
    ///
    /// * `key_id` - Key ID
    ///
    /// # Return
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    pub fn delete_key(&self, key_id: AesBulk256KeyId) -> HsmResult<()> {
        self.rimpl.borrow_mut().delete_key(key_id)
    }

    /// Delete all keys belong to this vault
    pub fn clear(&self) {
        self.rimpl.borrow_mut().clear()
    }

    /// Retrieves a AES Bulk key entry from the CDMA vault.
    ///
    /// This method fetches a key entry based on its table and key index.
    /// It is primarily used for FIPS validation testing.
    ///
    /// # Arguments
    ///
    /// * `AesBulk256KeyId` - Key ID
    ///
    /// # Returns
    ///
    /// * `Ok([u8; 32])` - The key entry if successful.
    /// * `Err(HsmError)` - An error if retrieval fails.
    pub fn get_key_entry(&mut self, key_id: AesBulk256KeyId) -> HsmResult<IoMemRange> {
        self.rimpl.borrow_mut().get_key_entry(key_id)
    }
}

#[allow(unused)]
#[repr(C)]
#[derive(PartialEq)]
struct CdmaTableEntry {
    pub(crate) entry: [u8; 32],
}

struct CdmaKeyVaultImpl {
    base: usize,
    mask: u128,
    meta_data_base: usize,
}

impl CdmaKeyVaultImpl {
    pub fn new(base: usize, mask: u128, meta_data_base: usize) -> Self {
        Self {
            base,
            mask,
            meta_data_base,
        }
    }

    fn base(&self) -> usize {
        self.base
    }

    fn mask(&self) -> u128 {
        self.mask
    }

    fn is_valid_table(&self, table_index: u8) -> bool {
        (table_index < MAX_TABLE_COUNT as u8) && ((self.mask() & (1 << table_index)) != 0)
    }

    /// Import AES key into the key store
    fn import_key(&mut self, key_blob: &[u8]) -> HsmResult<AesBulk256KeyId> {
        let key_size: usize = DdiAesKeySize::AesGcmBulk256
            .try_into()
            .map_err(|_| HsmErr::InvalidArgument)?;
        if key_blob.len() != key_size {
            Err(HsmErr::InvalidArgument)?
        }

        let table_meta_data = CdmaKeyVaultImpl::get_cdma_meta_data_mut(self.meta_data_base);
        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if !self.is_valid_table(table_index) {
                continue;
            }

            let table_index = table_index as usize;
            let mut availability = table_meta_data[table_index];
            for key_index in 0..MAX_KEYS_PER_TABLE {
                if !availability.bit(key_index) {
                    availability.set_bit(key_index, true);

                    table_meta_data[table_index] = availability;

                    return Ok(AesBulk256KeyId::new()
                        .with_key_index(key_index as u8)
                        .with_vault_id(table_index as u8)
                        .with_rsvd(0));
                }
            }
        }

        Err(HsmErr::ReachedMaxAesBulkKeys)
    }

    /// Delete key
    fn delete_key(&mut self, key_id: AesBulk256KeyId) -> HsmResult<()> {
        let table_index = key_id.vault_id();
        let key_index = key_id.key_index();

        if !self.is_valid_table(table_index) {
            Err(HsmErr::InvalidKeyTableIndex)?
        }

        let table_meta_data = CdmaKeyVaultImpl::get_cdma_meta_data_mut(self.meta_data_base);

        if !table_meta_data[table_index as usize].bit(key_index as usize) {
            Err(HsmErr::InvalidKeyIndex)?
        }

        table_meta_data[table_index as usize].set_bit(key_index as usize, false);

        Ok(())
    }

    /// Delete all keys
    fn clear(&mut self) {
        let table_meta_data = CdmaKeyVaultImpl::get_cdma_meta_data_mut(self.meta_data_base);
        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if !self.is_valid_table(table_index) {
                continue;
            }

            let table_index = table_index as usize;
            let mut availability = table_meta_data[table_index];

            for key_index in 0..MAX_KEYS_PER_TABLE {
                availability.set_bit(key_index, false);
            }

            table_meta_data[table_index] = availability;
        }
    }

    fn get_table_entry_mut(&mut self, table_index: u8, table_offset: u8) -> &mut CdmaTableEntry {
        let table_index = table_index as usize;
        let table_offset = table_offset as usize;

        unsafe {
            &mut *((self.base() + table_index * TOTAL_TABLE_SIZE + table_offset * KEY_SIZE)
                as *mut CdmaTableEntry)
        }
    }

    fn get_cdma_meta_data_mut(meta_data_base: usize) -> &'static mut [u8; MAX_TABLE_COUNT] {
        unsafe { &mut *((meta_data_base) as *mut [u8; MAX_TABLE_COUNT]) }
    }

    /// Internal function to retrieve a key entry.
    fn get_key_entry(&mut self, key_id: AesBulk256KeyId) -> HsmResult<IoMemRange> {
        let table_index = key_id.vault_id();
        let key_index = key_id.key_index();

        // Validate table index
        if !self.is_valid_table(table_index) {
            Err(HsmErr::InvalidKeyIndex)?
        }

        // Access metadata
        let table_meta_data = CdmaKeyVaultImpl::get_cdma_meta_data_mut(self.meta_data_base);

        // Validate key index
        if !table_meta_data[table_index as usize].bit(key_index as usize) {
            Err(HsmErr::InvalidKeyIndex)?
        }

        // Retrieve the key entry
        let entry = self.get_table_entry_mut(table_index, key_index);

        Ok(IoMemRange::from(&entry.entry[..]))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use alloc::vec::Vec;
    use rand::Rng;

    /// Generate a new AES256 key using random number generator
    fn aes256_key() -> Vec<u8> {
        let mut rng = rand::thread_rng();
        let mut rand_num = Vec::new();

        for _ in 0..KEY_SIZE {
            rand_num.push(rng.gen());
        }

        rand_num
    }

    #[test]
    fn test_cdma_vault_import_key_delete_key() {
        let cdma_vault_memory: [u8; 16384] = [0; 16384];
        let cdma_vault_meta_data: [u8; 65] = [0; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = [0u8; 32];
        let result = cdma_vault.import_key(&key_blob[..]);

        let cdma_key_id = result.unwrap();
        assert_eq!(cdma_key_id.vault_id(), 0);
        assert_eq!(cdma_key_id.key_index(), 0);

        let result = cdma_vault.import_key(&key_blob[..]);

        let cdma_key_id = result.unwrap();
        assert_eq!(cdma_key_id.vault_id(), 0);
        assert_eq!(cdma_key_id.key_index(), 1);

        let result = cdma_vault.delete_key(cdma_key_id);

        assert_eq!(result, Ok(()));
    }

    #[test]
    fn test_cdma_vault_import_key_with_2nd_table() {
        let cdma_vault_memory: [u8; 16384] = [0; 16384];
        let cdma_vault_meta_data: [u8; 65] = [0; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x2;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = [0u8; 32];
        let result = cdma_vault.import_key(&key_blob[..]);

        let cdma_key_id = result.unwrap();
        assert_eq!(cdma_key_id.vault_id(), 1);
        assert_eq!(cdma_key_id.key_index(), 0);
    }

    #[test]
    fn test_cdma_vault_import_key_incorrect_key_size() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = [0u8; 64]; // incorrect key size
        let result = cdma_vault.import_key(&key_blob[..]);

        assert_eq!(result, Err(HsmErr::InvalidArgument));
    }

    #[test]
    fn test_cdma_vault_import_key_table_full() {
        let cdma_vault_memory: [u8; 16384] = [0; 16384];
        let mut cdma_vault_meta_data: [u8; 65] = [0; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;
        cdma_vault_meta_data[0] = 0xff;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = [0u8; 32];
        let result = cdma_vault.import_key(&key_blob[..]);

        assert_eq!(result, Err(HsmErr::ReachedMaxAesBulkKeys));
    }

    #[test]
    fn test_cdma_vault_delete_key_incorrect_table() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let cdma_key_id = AesBulk256KeyId::new()
            .with_key_index(0)
            .with_vault_id(66)
            .with_rsvd(0);
        let result = cdma_vault.delete_key(cdma_key_id);
        assert_eq!(result, Err(HsmErr::InvalidKeyTableIndex));
    }

    #[test]
    fn test_cdma_vault_delete_key_incorrect_key() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        // below key_id will cause table_index check to pass but cause
        // key_index to fail as it is not already set in meta data for delete
        // to succeed.
        let cdma_key_id = AesBulk256KeyId::new()
            .with_key_index(0)
            .with_vault_id(0)
            .with_rsvd(0);
        let result = cdma_vault.delete_key(cdma_key_id);
        assert_eq!(result, Err(HsmErr::InvalidKeyIndex));
    }

    #[test]
    fn test_cdma_vault_delete_all_with_one_table() {
        let cdma_vault_memory: [u8; 16384] = [0; 16384];
        let cdma_vault_meta_data: [u8; 65] = [0; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let mut key_ids = Vec::new();

        loop {
            let key_blob = aes256_key();
            let result = cdma_vault.import_key(key_blob.as_slice());

            if let Err(e) = result {
                if matches!(e, HsmErr::ReachedMaxAesBulkKeys) {
                    break;
                }
            } else {
                let aes_bulk_key = result.unwrap();
                key_ids.push((aes_bulk_key, key_blob));
            }
        }

        for (index, (cdme_key_id, _key)) in key_ids.iter().enumerate() {
            assert_eq!(cdme_key_id.key_index() as usize, index);
            assert_eq!(cdme_key_id.vault_id() as usize, 0);
        }

        cdma_vault.clear();

        for (key_id, _) in key_ids.iter() {
            assert_eq!(cdma_vault.delete_key(*key_id), Err(HsmErr::InvalidKeyIndex));
        }
    }

    #[test]
    fn test_cdma_vault_delete_all_with_multiple_tables() {
        let cdma_vault_memory: [u8; 16384] = [0; 16384];
        let cdma_vault_meta_data: [u8; 65] = [0; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x3;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let mut key_ids = Vec::new();

        loop {
            let key_blob = aes256_key();
            let result = cdma_vault.import_key(key_blob.as_slice());

            if let Err(e) = result {
                if matches!(e, HsmErr::ReachedMaxAesBulkKeys) {
                    break;
                }
            } else {
                let aes_bulk_key = result.unwrap();
                key_ids.push(aes_bulk_key);
            }
        }

        for (index, key) in key_ids.iter().enumerate() {
            assert_eq!(key.key_index() as usize, index % 7);
            assert_eq!(key.vault_id() as usize, index / 7);
        }

        cdma_vault.clear();

        for key_id in key_ids.iter() {
            assert_eq!(cdma_vault.delete_key(*key_id), Err(HsmErr::InvalidKeyIndex));
        }
    }

    #[test]
    fn test_get_key_entry_success() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = aes256_key();
        let result = cdma_vault.import_key(key_blob.as_slice());
        assert!(result.is_ok(), "Key import failed: {:?}", result);

        let key_id = result.unwrap();

        // Retrieve the key entry
        let result = cdma_vault.get_key_entry(key_id);
        assert!(result.is_ok());

        let _key_entry = result.unwrap();
    }

    #[test]
    fn test_get_key_entry_invalid_table() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        // Create an invalid key ID with a non-existent table index
        let invalid_key_id = AesBulk256KeyId::new()
            .with_key_index(0)
            .with_vault_id(1)
            .with_rsvd(0);

        let result = cdma_vault.get_key_entry(invalid_key_id);
        assert!(matches!(result, Err(HsmErr::InvalidKeyIndex)));
    }

    #[test]
    fn test_get_key_entry_invalid_key_index() {
        let cdma_vault_memory = [0u8; 16384];
        let cdma_vault_meta_data = [0u8; 65];

        let vault_addr = cdma_vault_memory.as_ptr() as usize;
        let meta_data_base = cdma_vault_meta_data.as_ptr() as usize;
        let mask = 0x1;

        let mut cdma_vault = CdmaKeyVault::new(vault_addr, mask, meta_data_base);

        let key_blob = aes256_key();
        let result = cdma_vault.import_key(key_blob.as_slice());
        assert!(result.is_ok(), "Key import failed: {:?}", result);

        let valid_key_id = result.unwrap();

        // Create an invalid key ID with a non-existent key index
        let invalid_key_id = AesBulk256KeyId::new()
            .with_key_index(valid_key_id.key_index() + 1)
            .with_vault_id(valid_key_id.vault_id())
            .with_rsvd(0);

        let result = cdma_vault.get_key_entry(invalid_key_id);
        assert!(matches!(result, Err(HsmErr::InvalidKeyIndex)));
    }
}
