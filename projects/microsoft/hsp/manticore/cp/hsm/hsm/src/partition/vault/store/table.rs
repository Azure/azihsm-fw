// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::partition::{VarLenHmacShaKey, VarLenHmacShaKeyKind};

use super::*;
use zeroize::Zeroize;

const METADATA_LIST_OFFSET: usize = 0;
const METADATA_LIST_SIZE_BYTES: usize = 2048;
const USED_BLOCK_TRACKING_OFFSET: usize = METADATA_LIST_OFFSET + METADATA_LIST_SIZE_BYTES;
const USED_BLOCK_TRACKING_SIZE_BYTES: usize = 256;
pub(super) const USED_BLOCK_TRACKING_SIZE_WORDS: usize = USED_BLOCK_TRACKING_SIZE_BYTES / 4;
const BLOB_MEMORY_OFFSET: usize = USED_BLOCK_TRACKING_OFFSET + USED_BLOCK_TRACKING_SIZE_BYTES;

/// Size of the table memory available for storing key blobs in bytes.
pub(crate) const BLOB_MEMORY_SIZE_BYTES: usize = TOTAL_TABLE_LEN - BLOB_MEMORY_OFFSET;

/// `Table` is for managing a single table of entries/ keys.
#[repr(C)]
pub(crate) struct PhysicalTable {
    entry_list: [PhysicalEntry; MAX_TABLE_ENTRY_COUNT],
    used_block_tracking_memory: [u32; USED_BLOCK_TRACKING_SIZE_WORDS],
    key_data_memory: [u8; BLOB_MEMORY_SIZE_BYTES],
}

impl PhysicalTable {
    /// Add a new entry to the table.
    ///
    /// # Arguments
    /// * `flags` - Entry flags.
    /// * `session_id_or_key_tag` - Session ID (for session only key) or Key Tag (for app key).
    /// * `kind` - Kind of entry.
    /// * `app_id` - App ID.
    /// * `blob` - Blob to be stored.
    ///
    /// # Returns
    /// Entry index within the table.
    ///
    /// # Errors
    /// * `HsmErr::InvalidArgument` - If the input blob is not of expected size or if the kind is `Free`.
    /// * `HsmErr::NotEnoughSpace` - If there is not enough space in the table to store the blob.
    /// * `HsmErr::DefragNeeded` - If there is not enough contiguous space in the table to store the blob.
    pub(crate) fn add_entry(
        &mut self,
        attributes: &EntryAttributes,
        session_id_or_key_tag: u16,
        kind: EntryKind,
        app_id: u8,
        blob: &[u8],
    ) -> Result<u8, HsmErr> {
        let flags = attributes.common.flags.into();
        let is_var_hmac_key = kind.is_var_hmac_key(); // If this is a var hmac key, the blob size can vary.

        // Check if the input blob is of expected size.
        let expected_key_len = if is_var_hmac_key {
            let key_len = attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] as usize;
            let var_hmac_key_kind: VarLenHmacShaKeyKind = kind.try_into()?;
            if key_len < var_hmac_key_kind.min_length() || key_len > var_hmac_key_kind.max_length()
            {
                Err(HsmErr::InvalidArgument)?
            }

            key_len
        } else {
            kind.raw_key_blob_size()
        };

        if blob.len() != expected_key_len {
            Err(HsmErr::InvalidArgument)?
        }

        if kind == EntryKind::Free {
            Err(HsmErr::InvalidArgument)?
        }

        let size_needed = if is_var_hmac_key {
            kind.storage_size(Some(blob.len()))
        } else {
            kind.storage_size(None)
        };
        let blocks_needed = size_needed / ENTRY_BLOB_BLOCK_ALIGNMENT;

        // Find the first empty slot in the table.
        let (entry_number_to_use, _entry) = self
            .entry_list
            .iter()
            .enumerate()
            .find(|(_, e)| e.is_empty())
            .ok_or(HsmErr::NotEnoughSpace)?;

        // Search for suitable length space if we have it.
        let mut blocks_used = self.used_block_tracker();
        let mut slot_found = false;
        let mut total_free_slots_count = 0usize;
        let mut last_consecutive_free_slots_count = 0usize;
        let mut last_consecutive_free_slots_started_at = 0usize;
        let mut last_consecutive_free_slots_ended_at = 0usize;
        for index in 0..(BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT) {
            let used = blocks_used.get(index).unwrap_or(true);
            if used {
                last_consecutive_free_slots_count = 0;
                last_consecutive_free_slots_started_at = 0;
            } else {
                if last_consecutive_free_slots_count == 0 {
                    last_consecutive_free_slots_started_at = index;
                }

                last_consecutive_free_slots_count += 1;
                total_free_slots_count += 1;
            }

            if blocks_needed == last_consecutive_free_slots_count {
                slot_found = true;
                last_consecutive_free_slots_ended_at = index;
                break;
            }
        }

        if !slot_found {
            // Check if there is possibly enough space in the table.
            // NOTE: This space may be fragmented and unusable.
            if blocks_needed > total_free_slots_count {
                Err(HsmErr::NotEnoughSpace)?
            }

            // There were enough blocks free but we couldn't find them consecutive
            // so we need to defrag the table.
            Err(HsmErr::DefragNeeded)?
        }

        // Set the slot bits to used
        for i in last_consecutive_free_slots_started_at..=last_consecutive_free_slots_ended_at {
            blocks_used.set(i, true);
        }

        let offset = last_consecutive_free_slots_started_at as u16;
        let entry_index = entry_number_to_use as u8;

        let attributes_offset = offset as usize * ENTRY_BLOB_BLOCK_ALIGNMENT;
        let key_blob_offset = attributes_offset + ATTRIBUTES_BLOB_SIZE;

        // Copy the attributes into the table.
        self.key_data_memory_slice_mut(attributes_offset, ATTRIBUTES_BLOB_SIZE)
            .copy_from_slice(attributes.as_bytes());

        // Copy the key blob into the table.
        self.key_data_memory_slice_mut(key_blob_offset, blob.len())
            .copy_from_slice(blob);

        // Save the metadata into entry list.
        self.entry_list[entry_number_to_use] =
            PhysicalEntry::new(offset, flags, session_id_or_key_tag, kind, app_id);

        Ok(entry_index)
    }

    /// Enable an entry from the table.
    ///
    /// # Arguments
    /// * `entry_index` - Index of the entry to be enabled.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key index is invalid.
    pub(crate) fn enable_entry(&mut self, entry_index: u8) -> Result<(), HsmErr> {
        // Check if the key exists.
        let entry = self.get_entry_mut(entry_index, true)?;

        entry.enable();

        Ok(())
    }

    /// Disable an entry from the table.
    ///
    /// # Arguments
    /// * `entry_index` - Index of the entry to be disabled.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key index is invalid.
    pub(crate) fn disable_entry(&mut self, entry_index: u8) -> Result<(), HsmErr> {
        // Check if the key exists.
        let entry = self.get_entry_mut(entry_index, true)?;

        entry.disable();

        Ok(())
    }

    /// Remove an entry from the table.
    ///
    /// # Arguments
    /// * `entry_index` - Index of the entry to be removed.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key index is invalid.
    pub(crate) fn remove_entry<F>(
        &mut self,
        key_in_use_cb: F,
        table_index: u8,
        entry_index: u8,
    ) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        // Check if the key exists.
        let key_length_in_attributes = self.get_entry_attributes(entry_index, true)?.entry_specific
            [VarLenHmacShaKey::KEY_LENGTH_INDEX] as usize;
        let entry = self.get_entry_mut(entry_index, true)?;

        if key_in_use_cb(KeyNumber::new(table_index, entry_index).0) {
            entry.disable();
            Err(HsmErr::CannotDeleteKeyInUse)?
        }

        let entry_storage_offset = entry.attributes_bytes_offset();
        let kind = entry.kind();
        let storage_size = if kind.is_var_hmac_key() {
            kind.storage_size(Some(key_length_in_attributes))
        } else {
            kind.storage_size(None)
        };
        let entry_raw_offset = entry.raw_block_offset() as usize;

        // Clear the entry metadata.
        self.entry_list[entry_index as usize] =
            PhysicalEntry::new(0, EntryFlags::default(), 0, EntryKind::Free, 0);

        // Clear the entry blob.
        let entry_blob = self.key_data_memory_slice_mut(entry_storage_offset, storage_size);
        entry_blob.zeroize();

        // Clear the used block tracker.
        let mut blocks_used = self.used_block_tracker();
        for i in 0..(storage_size / ENTRY_BLOB_BLOCK_ALIGNMENT) {
            blocks_used.set(entry_raw_offset + i, false);
        }

        Ok(())
    }

    /// Remove all session only entries for a given session id.
    ///
    /// # Arguments
    /// * `session_id` - Session ID whose session only entries will be removed.
    ///
    /// # Returns
    /// Number of entries removed.
    ///
    /// # Errors
    /// * `HsmErr::CannotDeleteKeyInUse` - If one of the keys are in use.
    /// * `HsmErr::CannotDeleteSomeKeysInUse` - If some of the keys are in use.
    pub(crate) fn remove_all_session_only_entries<F>(
        &mut self,
        key_in_use_cb: F,
        table_index: u8,
        session_id: u16,
    ) -> Result<u8, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        let mut delete_count = 0;
        let mut failed_delete_count: u8 = 0;

        for i in 0..MAX_TABLE_ENTRY_COUNT {
            let entry = &self.entry_list[i];
            if entry.is_empty() {
                continue;
            }

            if entry.flags().session() && entry.session_id() == Some(session_id) {
                match self.remove_entry(&key_in_use_cb, table_index, i as u8) {
                    Ok(_) => delete_count += 1,
                    Err(_) => failed_delete_count += 1,
                }
            }
        }

        match failed_delete_count {
            0 => (),
            1 => Err(HsmErr::CannotDeleteKeyInUse)?,
            _ => Err(HsmErr::CannotDeleteSomeKeysInUse)?,
        }

        Ok(delete_count)
    }

    /// Remove all entries for an app.
    ///
    /// # Arguments
    /// * `app_id` - App ID of the app.
    ///
    /// # Returns
    /// Number of entries removed.
    ///
    /// # Errors
    /// * `HsmErr::CannotDeleteKeyInUse` - If one of the keys are in use.
    /// * `HsmErr::CannotDeleteSomeKeysInUse` - If some of the keys are in use.
    pub(crate) fn remove_all_entries_for_app<F>(
        &mut self,
        key_in_use_cb: F,
        table_index: u8,
        app_id: u8,
    ) -> Result<u8, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        let mut delete_count = 0;
        let mut failed_delete_count: u8 = 0;

        for i in 0..MAX_TABLE_ENTRY_COUNT {
            let entry = &self.entry_list[i];
            if entry.is_empty() {
                continue;
            }

            if entry.app_id() == app_id {
                match self.remove_entry(&key_in_use_cb, table_index, i as u8) {
                    Ok(_) => delete_count += 1,
                    Err(_) => failed_delete_count += 1,
                }
            }
        }

        match failed_delete_count {
            0 => (),
            1 => Err(HsmErr::CannotDeleteKeyInUse)?,
            _ => Err(HsmErr::CannotDeleteSomeKeysInUse)?,
        }

        Ok(delete_count)
    }

    /// Get the entry index by name.
    ///
    /// # Arguments
    /// * `app_id` - App ID of the app.
    /// * `key_tag` - Key tag of the key.
    ///
    /// # Returns
    /// Index of the entry.
    ///
    /// # Errors
    /// * `HsmErr::InvalidArgument` - If the key tag is invalid.
    /// * `HsmErr::KeyNotFound` - If the key is not found.
    pub(crate) fn get_entry_index_by_tag(&self, app_id: u8, key_tag: u16) -> Result<u8, HsmErr> {
        if key_tag == KEY_TAG_UNASSIGNED {
            Err(HsmErr::InvalidArgument)?
        }

        self.entry_list
            .iter()
            .position(|e| {
                !e.is_empty()
                    && !e.disabled()
                    && e.app_id() == app_id
                    && e.key_tag() == Some(key_tag)
            })
            .ok_or(HsmErr::KeyNotFound)
            .map(|i| i as u8)
    }

    /// Get the entry metadata object.
    ///
    /// # Arguments
    /// * `entry_index` - Index of the entry to be removed.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key index is invalid.
    pub(crate) fn get_entry(
        &self,
        entry_index: u8,
        allow_disabled: bool,
    ) -> Result<&PhysicalEntry, HsmErr> {
        let entry = &self.entry_list[entry_index as usize];
        if entry.is_empty() {
            Err(HsmErr::InvalidKeyIndex)?
        }

        if !allow_disabled && entry.disabled() {
            Err(HsmErr::InvalidKeyIndex)?
        }

        Ok(entry)
    }

    pub(crate) fn get_entry_attributes(
        &self,
        entry_index: u8,
        allow_disabled: bool,
    ) -> Result<&EntryAttributes, HsmErr> {
        let entry = self.get_entry(entry_index, allow_disabled)?;

        self.get_attributes_from_entry(entry)
    }

    /// Get the entry blob.
    ///
    /// # Arguments
    /// * `entry_index` - Index of the entry to be removed.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key index is invalid.
    pub(crate) fn get_entry_blob(&self, entry_index: u8) -> Result<&[u8], HsmErr> {
        let entry = self.get_entry(entry_index, true)?;
        let kind = entry.kind();
        let key_blob_size = if kind.is_var_hmac_key() {
            self.get_attributes_from_entry(entry)?.entry_specific
                [VarLenHmacShaKey::KEY_LENGTH_INDEX] as usize
        } else {
            kind.raw_key_blob_size()
        };

        let key_blob_offset = entry.key_blob_bytes_offset();

        let blob = self.key_data_memory_slice(key_blob_offset, key_blob_size);
        Ok(blob)
    }

    fn used_block_tracker(&mut self) -> UsedBlocksTracker {
        UsedBlocksTracker::new(self.used_block_tracking_memory.as_mut_ptr() as usize)
    }

    fn key_data_memory_slice(&self, offset: usize, len: usize) -> &[u8] {
        &self.key_data_memory[offset..offset + len]
    }

    fn key_data_memory_slice_mut(&mut self, offset: usize, len: usize) -> &mut [u8] {
        &mut self.key_data_memory[offset..offset + len]
    }

    pub(crate) fn nuke(&mut self) {
        for entry in self.entry_list.iter_mut() {
            *entry = PhysicalEntry::new(0, EntryFlags::default(), 0, EntryKind::Free, 0);
        }

        self.used_block_tracking_memory.zeroize();
        self.key_data_memory.zeroize();
    }

    fn get_entry_mut(
        &mut self,
        entry_index: u8,
        allow_disabled: bool,
    ) -> Result<&mut PhysicalEntry, HsmErr> {
        let entry = &mut self.entry_list[entry_index as usize];
        if entry.is_empty() {
            Err(HsmErr::InvalidKeyIndex)?
        }

        if !allow_disabled && entry.disabled() {
            Err(HsmErr::InvalidKeyIndex)?
        }

        Ok(entry)
    }

    fn get_attributes_from_entry(&self, entry: &PhysicalEntry) -> Result<&EntryAttributes, HsmErr> {
        let attributes_offset = entry.attributes_bytes_offset();
        let attributes_slice =
            &self.key_data_memory[attributes_offset..attributes_offset + ATTRIBUTES_BLOB_SIZE];

        EntryAttributes::ref_from_bytes(attributes_slice).map_err(|_| HsmErr::InvalidKeyIndex)
    }
}

#[cfg(test)]
mod tests {

    use core::cmp::min;

    use rand::Rng;

    use super::*;

    #[test]
    fn test_new() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &*(table_memory.as_ptr() as usize as *const PhysicalTable) };

        assert!(table.entry_list[0].is_empty());
        assert!(table.entry_list[2].is_empty());
        assert!(table.entry_list[20].is_empty());
        assert!(table.entry_list[150].is_empty());
        assert!(table.entry_list[MAX_TABLE_ENTRY_COUNT - 1].is_empty());
    }

    #[test]
    fn test_add_entry_incorrect_size() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let result = table.add_entry(
            &EntryAttributes::default(),
            0,
            EntryKind::Rsa2kPublic,
            0,
            &[0x1; 16],
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidArgument)));
    }

    #[test]
    fn test_add_entry_kind_free() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let result = table.add_entry(
            &EntryAttributes::default(),
            0,
            EntryKind::Free,
            0,
            &[0x0; 0],
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidArgument)));
    }

    #[test]
    fn test_add_entry() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_public_key_blob = [i + 1; 260];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        assert!(!table.entry_list[0].is_empty());
        assert_eq!(table.entry_list[0].app_id(), app_id);
        assert!(table.entry_list[0].kind() == kind);
        assert_eq!(
            table.key_data_memory_slice(
                table.entry_list[0].key_blob_bytes_offset(),
                kind.raw_key_blob_size()
            ),
            [0x1; 260]
        );

        assert!(!table.entry_list[1].is_empty());
        assert_eq!(table.entry_list[1].app_id(), app_id);
        assert!(table.entry_list[1].kind() == kind);
        assert_eq!(
            table.key_data_memory_slice(
                table.entry_list[1].key_blob_bytes_offset(),
                kind.raw_key_blob_size()
            ),
            [0x2; 260]
        );

        assert!(table.entry_list[2].is_empty());
        assert!(table.entry_list[20].is_empty());
        assert!(table.entry_list[150].is_empty());
        assert!(table.entry_list[MAX_TABLE_ENTRY_COUNT - 1].is_empty());
    }

    #[test]
    fn test_add_max_keys() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Aes128;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        );

        for i in 0..allowed_entry_count {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index as usize, i);
        }

        assert!(!table.entry_list[0].is_empty());
        assert!(!table.entry_list[3].is_empty());
        let last_entry_index = allowed_entry_count - 1;
        assert!(!table.entry_list[last_entry_index].is_empty());

        let aes_key_blob = [(allowed_entry_count + 1) as u8; 16];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &aes_key_blob,
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::NotEnoughSpace)));
    }

    #[test]
    fn test_add_max_bytes() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        );

        for i in 0..allowed_entry_count {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index as usize, i);
        }

        assert!(!table.entry_list[0].is_empty());
        assert!(!table.entry_list[3].is_empty());
        let last_entry_index = allowed_entry_count - 1;
        assert!(!table.entry_list[last_entry_index].is_empty());

        let rsa_public_key_blob = [(allowed_entry_count + 1) as u8; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::NotEnoughSpace)));
    }

    #[test]
    fn test_add_defrag_needed() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        );

        for i in 0..allowed_entry_count {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index as usize, i);
        }

        assert!(!table.entry_list[0].is_empty());
        assert!(!table.entry_list[3].is_empty());
        let last_entry_index = allowed_entry_count - 1;
        assert!(!table.entry_list[last_entry_index].is_empty());

        assert!(table.remove_entry(|_| false, 0, 0).is_ok());
        assert!(table.remove_entry(|_| false, 0, 1).is_ok());
        assert!(table.remove_entry(|_| false, 0, 3).is_ok());
        assert!(table.remove_entry(|_| false, 0, 4).is_ok());

        let rsa_private_key_blob = [(allowed_entry_count + 1) as u8; 1028];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa4kPrivate,
            app_id,
            &rsa_private_key_blob,
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::DefragNeeded)));
    }

    #[test]
    fn test_get() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let session_id_or_key_tag = 5;
        let app_id = 10;

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);

        let rsa_private_key_blob = [2; 516];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPrivate,
            app_id,
            &rsa_private_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 1);

        let entry = table.get_entry(0, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::Rsa2kPublic);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), None);
        assert_eq!(entry.session_id(), Some(session_id_or_key_tag));
        assert_eq!(table.get_entry_blob(0).unwrap(), rsa_public_key_blob);

        let entry = table.get_entry(1, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::Rsa2kPrivate);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), None);
        assert_eq!(entry.session_id(), Some(session_id_or_key_tag));
        assert_eq!(table.get_entry_blob(1).unwrap(), rsa_private_key_blob);
    }

    #[test]
    fn test_get_by_name() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(false);

        let session_id_or_key_tag = 0x5453;
        let app_id = 10;

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);

        let rsa_private_key_blob = [2; 516];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag + 1,
            EntryKind::Rsa2kPrivate,
            app_id,
            &rsa_private_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 1);

        let result = table.get_entry_index_by_tag(app_id, 0);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidArgument)));

        let entry_index = table
            .get_entry_index_by_tag(app_id, session_id_or_key_tag)
            .unwrap();
        assert_eq!(entry_index, 0);
        let entry = table.get_entry(entry_index, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::Rsa2kPublic);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), Some(session_id_or_key_tag));
        assert_eq!(entry.session_id(), None);
        assert_eq!(table.get_entry_blob(0).unwrap(), rsa_public_key_blob);

        let entry_index = table
            .get_entry_index_by_tag(app_id, session_id_or_key_tag + 1)
            .unwrap();
        assert_eq!(entry_index, 1);
        let entry = table.get_entry(entry_index, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::Rsa2kPrivate);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), Some(session_id_or_key_tag + 1));
        assert_eq!(entry.session_id(), None);
        assert_eq!(table.get_entry_blob(1).unwrap(), rsa_private_key_blob);
    }

    #[test]
    fn test_remove_basic() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert!(table.remove_entry(|_| false, 0, 0).is_ok());
        assert!(table.entry_list[0].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);

        assert!(table.remove_entry(|_| true, 0, 0).is_err());
        let result = table.get_entry(0, false);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidKeyIndex)));
        assert!(table.entry_list[0].disabled());

        assert!(table.remove_entry(|_| false, 0, 0).is_ok());
    }

    #[test]
    fn test_remove_all_session_only_entries() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag + i as u16,
                kind,
                app_id + i,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert!(table
            .remove_all_session_only_entries(|_| false, 0, session_id_or_key_tag)
            .is_ok());
        assert!(table.entry_list[0].is_empty());
        assert!(!table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_all_session_only_entries_single_key_in_use() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag + i as u16,
                kind,
                app_id + i,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        let result =
            table.remove_all_session_only_entries(|key_id| key_id == 0, 0, session_id_or_key_tag);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::CannotDeleteKeyInUse)));
        assert!(table.entry_list[0].disabled());

        let result = table.remove_all_session_only_entries(|_| false, 0, session_id_or_key_tag);
        assert!(result.is_ok());

        assert!(table.entry_list[0].is_empty());
        assert!(!table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_all_session_only_entries_multiple_key_in_use() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let mut keys_in_use = vec![];

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id + i,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
            keys_in_use.push(index as u16);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        let result = table.remove_all_session_only_entries(
            |key_id| keys_in_use.contains(&key_id),
            0,
            session_id_or_key_tag,
        );
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::CannotDeleteSomeKeysInUse)));
        assert!(table.entry_list[0].disabled());
        assert!(table.entry_list[1].disabled());

        keys_in_use.clear();
        let result = table.remove_all_session_only_entries(
            |key_id| keys_in_use.contains(&key_id),
            0,
            session_id_or_key_tag,
        );
        assert!(result.is_ok());

        assert!(table.entry_list[0].is_empty());
        assert!(table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_all_entries_for_app() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag + i as u16,
                kind,
                app_id + i,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert!(table
            .remove_all_entries_for_app(|_| false, 0, app_id)
            .is_ok());
        assert!(table.entry_list[0].is_empty());
        assert!(!table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_all_entries_for_app_session_single_key_in_use() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag + i as u16,
                kind,
                app_id + i,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        let result = table.remove_all_entries_for_app(|key_id| key_id == 0, 0, app_id);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::CannotDeleteKeyInUse)));
        assert!(table.entry_list[0].disabled());

        let result = table.remove_all_entries_for_app(|_| false, 0, app_id);
        assert!(result.is_ok());

        assert!(table.entry_list[0].is_empty());
        assert!(!table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_all_entries_for_app_session_multiple_key_in_use() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let mut keys_in_use = vec![];

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag + i as u16,
                kind,
                app_id,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
            keys_in_use.push(index as u16);
        }

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0].kind().aligned_key_blob_size(None);

        assert_ne!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        let result =
            table.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), 0, app_id);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::CannotDeleteSomeKeysInUse)));
        assert!(table.entry_list[0].disabled());
        assert!(table.entry_list[1].disabled());

        keys_in_use.clear();
        let result =
            table.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), 0, app_id);
        assert!(result.is_ok());

        assert!(table.entry_list[0].is_empty());
        assert!(table.entry_list[1].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 1032]
        );

        assert_eq!(
            table.used_block_tracker().used_blocks()[..1032 / 8 / 32],
            [0u32; 4]
        );

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 260];
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa2kPublic,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);
    }

    #[test]
    fn test_remove_invalid_key_index() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_public_key_blob = [i + 1; 260];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        assert!(matches!(
            table.remove_entry(|_| false, 0, 20),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            table.remove_entry(|_| false, 0, 255),
            Err(HsmErr::InvalidKeyIndex)
        ));
    }

    #[test]
    fn test_nuke() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Aes128;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        );

        for i in 0..allowed_entry_count {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index as usize, i);
        }

        assert!(!table.entry_list[0].is_empty());
        assert!(!table.entry_list[3].is_empty());
        let last_entry_index = allowed_entry_count - 1;
        assert!(!table.entry_list[last_entry_index].is_empty());

        table.nuke();

        assert!(table.entry_list[0].is_empty());
        assert!(table.entry_list[3].is_empty());
        let last_entry_index = allowed_entry_count - 1;
        assert!(table.entry_list[last_entry_index].is_empty());
    }

    #[test]
    fn test_entry_enable_disable() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::AesGcmBulk256Unapproved;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        );

        for i in 0..allowed_entry_count {
            let aes_key_blob = [(i + 1) as u8; 2];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index as usize, i);
        }

        assert!(table.disable_entry(0).is_ok());
        assert!(table.entry_list[0].disabled());
        assert!(table.enable_entry(0).is_ok());
        assert!(!table.entry_list[0].disabled());
    }

    #[test]
    fn test_entry_enable_disable_failed() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        assert!(table.enable_entry(0).is_err());
        assert!(table.disable_entry(0).is_err());
    }

    #[test]
    fn test_add_var_hmac_keys() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let session_id_or_key_tag = 5;
        let app_id = 10;

        let var_hmac_key_blob1 = [1; 39];
        attributes.entry_specific[0] = var_hmac_key_blob1.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &var_hmac_key_blob1,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);

        let var_hmac_key_blob2 = [2; 55];
        attributes.entry_specific[0] = var_hmac_key_blob2.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha384,
            app_id,
            &var_hmac_key_blob2,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 1);

        let var_hmac_key_blob3 = [3; 101];
        attributes.entry_specific[0] = var_hmac_key_blob3.len() as u8;

        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha512,
            app_id,
            &var_hmac_key_blob3,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 2);

        let entry = table.get_entry(0, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::VarLenHmacSha256);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), None);
        assert_eq!(entry.session_id(), Some(session_id_or_key_tag));
        assert_eq!(table.get_entry_blob(0).unwrap(), var_hmac_key_blob1);

        let entry = table.get_entry(1, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::VarLenHmacSha384);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), None);
        assert_eq!(entry.session_id(), Some(session_id_or_key_tag));
        assert_eq!(table.get_entry_blob(1).unwrap(), var_hmac_key_blob2);

        let entry = table.get_entry(2, false).unwrap();
        assert_eq!(entry.app_id(), app_id);
        assert!(entry.kind() == EntryKind::VarLenHmacSha512);
        assert_eq!(entry.flags(), attributes.common.flags.into());
        assert_eq!(entry.key_tag(), None);
        assert_eq!(entry.session_id(), Some(session_id_or_key_tag));
        assert_eq!(table.get_entry_blob(2).unwrap(), var_hmac_key_blob3);
    }

    #[test]
    fn test_add_invalid_length_var_hmac_keys() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let session_id_or_key_tag = 5;
        let app_id = 10;

        let var_hmac_key_blob1 = [1; 39];
        attributes.entry_specific[0] = (var_hmac_key_blob1.len() * 2) as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &var_hmac_key_blob1,
        );
        assert!(result.is_err());

        let var_hmac_key_blob2 = [1; 31];
        attributes.entry_specific[0] = var_hmac_key_blob2.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &var_hmac_key_blob2,
        );
        assert!(result.is_err());

        let var_hmac_key_blob3 = [1; 65];
        attributes.entry_specific[0] = var_hmac_key_blob3.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &var_hmac_key_blob3,
        );
        assert!(result.is_err());

        let var_hmac_key_blob4 = [1; 47];
        attributes.entry_specific[0] = var_hmac_key_blob4.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha384,
            app_id,
            &var_hmac_key_blob4,
        );
        assert!(result.is_err());

        let var_hmac_key_blob5 = [1; 129];
        attributes.entry_specific[0] = var_hmac_key_blob5.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha384,
            app_id,
            &var_hmac_key_blob5,
        );
        assert!(result.is_err());

        let var_hmac_key_blob6 = [1; 63];
        attributes.entry_specific[0] = var_hmac_key_blob6.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha512,
            app_id,
            &var_hmac_key_blob6,
        );
        assert!(result.is_err());

        let var_hmac_key_blob7 = [1; 129];
        attributes.entry_specific[0] = var_hmac_key_blob7.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha512,
            app_id,
            &var_hmac_key_blob7,
        );
        assert!(result.is_err());
    }

    #[test]
    fn test_remove_var_hmac_keys() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let session_id_or_key_tag = 5;
        let app_id = 10;

        let var_hmac_key_blob1 = [1; 39];
        attributes.entry_specific[0] = var_hmac_key_blob1.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            &var_hmac_key_blob1,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);

        let var_hmac_key_blob2 = [2; 55];
        attributes.entry_specific[0] = var_hmac_key_blob2.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha384,
            app_id,
            &var_hmac_key_blob2,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 1);

        let var_hmac_key_blob3 = [3; 101];
        attributes.entry_specific[0] = var_hmac_key_blob3.len() as u8;

        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha512,
            app_id,
            &var_hmac_key_blob3,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 2);

        let entry_0_offset = table.entry_list[0].key_blob_bytes_offset();
        let entry_0_size = table.entry_list[0]
            .kind()
            .aligned_key_blob_size(Some(var_hmac_key_blob1.len()));

        assert_eq!(table.used_block_tracker().used_blocks()[0], 0xffffffffu32);
        assert_eq!(table.used_block_tracker().used_blocks()[1], 0x1fu32);
        assert_ne!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 40]
        );

        assert!(table.remove_entry(|_| false, 0, 0).is_ok());
        assert!(table.entry_list[0].is_empty());

        assert_eq!(
            table.key_data_memory_slice(entry_0_offset, entry_0_size),
            [0; 40]
        );
        assert_eq!(table.used_block_tracker().used_blocks()[0], 0xfffffe00u32);
        assert_eq!(table.used_block_tracker().used_blocks()[1], 0x1fu32);

        let entry_0_raw_mem = unsafe {
            &*((table_memory.as_ptr() as usize + METADATA_LIST_OFFSET) as *const [u8; 8])
        };
        assert_eq!(entry_0_raw_mem, &[0u8; 8]);

        let rsa_public_key_blob = [1; 16];
        attributes.entry_specific[0] = 0;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Aes128,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0);
        assert_eq!(table.entry_list[0].key_blob_bytes_offset(), entry_0_offset);

        assert!(table.remove_entry(|_| true, 0, 0).is_err());
        let result = table.get_entry(0, false);
        assert!(result.is_err());
        assert!(matches!(result, Err(HsmErr::InvalidKeyIndex)));
        assert!(table.entry_list[0].disabled());

        assert!(table.remove_entry(|_| false, 0, 0).is_ok());
    }

    #[test]
    fn test_add_and_remove_var_hmac_keys_from_existing_table() {
        let table_memory = [0u32; TOTAL_TABLE_LEN / 4];
        let table = unsafe { &mut *(table_memory.as_ptr() as usize as *mut PhysicalTable) };

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa4kPrivate;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2u8 {
            let rsa_private_key_blob = [i + 1; 1028];
            let result = table.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, i);
        }

        let table_memory_before_var_hmac = table_memory;

        let mut rng = rand::thread_rng();

        let var_hmac_key_blob1 = vec![1u8; rng.gen_range(32..=64)];
        attributes.entry_specific[0] = var_hmac_key_blob1.len() as u8;

        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha256,
            app_id,
            var_hmac_key_blob1.as_slice(),
        );
        assert!(
            result.is_ok(),
            "Failed to add VarLenHmacSha256 key: {:?}",
            result
        );

        let index = result.unwrap();
        assert_eq!(index, 2);

        let var_hmac_key_blob2 = vec![2u8; rng.gen_range(48..=128)];
        attributes.entry_specific[0] = var_hmac_key_blob2.len() as u8;
        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha384,
            app_id,
            var_hmac_key_blob2.as_slice(),
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 3);

        let var_hmac_key_blob3 = vec![3u8; rng.gen_range(64..=128)];
        attributes.entry_specific[0] = var_hmac_key_blob3.len() as u8;

        let result = table.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::VarLenHmacSha512,
            app_id,
            var_hmac_key_blob3.as_slice(),
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 4);

        assert!(table.remove_entry(|_| false, 0, 2).is_ok());
        assert!(table.remove_entry(|_| false, 0, 3).is_ok());
        assert!(table.remove_entry(|_| false, 0, 4).is_ok());

        assert_eq!(table_memory, table_memory_before_var_hmac);
    }
}
