// Copyright (c) Microsoft Corporation. All rights reserved.

use crate::partition::VarLenHmacShaKey;

use super::*;

#[derive(Clone)]
pub(crate) struct KeyStore {
    rimpl: Rc<RefCell<KeyStoreImpl>>,
}

impl KeyStore {
    /// Creates a new `KeyStore` instance.
    ///
    /// # Arguments
    /// * `base` - The base address of the table memory containing MAX_TABLE_COUNT tables. Each
    ///   table should be TOTAL_TABLE_LEN in size.
    /// * `mask` - A bit mask indicating which tables belong to this specific store. The LSB
    ///   corresponds to table 0.
    ///
    /// # Returns
    /// A new `KeyStore` instance.
    pub(crate) fn new(base: usize, mask: u128) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(KeyStoreImpl::new(base, mask))),
        }
    }

    /// Add a new entry to the store.
    ///
    /// # Arguments
    /// * `flags` - The flags for the new entry.
    /// * `session_id_or_key_tag` - The session ID (for session only key) or key tag (for
    ///   app key) for the new entry.
    /// * `kind` - The kind of the new entry.
    /// * `app_id` - The app ID for the new entry.
    /// * `blob` - The blob for the new entry.
    ///
    /// # Returns
    /// The key number of the new entry.
    ///
    /// # Errors
    /// * `HsmErr::InvalidArgument` - If the kind is `EntryKind::Free`.
    /// * `HsmErr::NotEnoughSpace` - If there is not enough space in the store.
    /// * `HsmErr::DefragNeeded` - If a defrag is needed to make space for the new entry.
    /// * `HsmErr::KeyTagAlreadyExists` - If the key tag already exists.
    pub(crate) fn add_entry(
        &mut self,
        attributes: &EntryAttributes,
        session_id_or_key_tag: u16,
        kind: EntryKind,
        app_id: u8,
        blob: &[u8],
    ) -> Result<u16, HsmErr> {
        self.rimpl
            .borrow_mut()
            .add_entry(attributes, session_id_or_key_tag, kind, app_id, blob)
    }

    /// Enable the entry with the given key number from the store.
    ///
    /// # Arguments
    /// * `entry` - The key number of the entry to enable.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key number is invalid.
    pub(crate) fn enable_entry(&mut self, entry: u16) -> Result<(), HsmErr> {
        self.rimpl.borrow_mut().enable_entry(entry)
    }

    /// Disable the entry with the given key number from the store.
    ///
    /// # Arguments
    /// * `entry` - The key number of the entry to disable.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key number is invalid.
    pub(crate) fn disable_entry(&mut self, entry: u16) -> Result<(), HsmErr> {
        self.rimpl.borrow_mut().disable_entry(entry)
    }

    /// Remove the entry with the given key number from the store.
    ///
    /// # Arguments
    /// * `entry` - The key number of the entry to remove.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key number is invalid.
    pub(crate) fn remove_entry<F>(&mut self, entry: u16, key_in_use_cb: F) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.rimpl.borrow_mut().remove_entry(key_in_use_cb, entry)
    }

    /// Remove all session only entries for the given session ID from the store.
    ///
    /// # Arguments
    /// * `session_id` - The session ID.
    ///
    /// # Returns
    /// The number of entries removed.
    ///
    /// # Errors
    /// * `HsmErr::CannotDeleteKeyInUse` - If any of the entries cannot be deleted because
    ///   they are in use.
    /// * `HsmErr::CannotDeleteSomeKeysInUse` - If some of the entries cannot be deleted
    ///   because they are in use.
    pub(crate) fn remove_all_session_only_entries<F>(
        &mut self,
        key_in_use_cb: F,
        session_id: u16,
    ) -> Result<u16, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.rimpl
            .borrow_mut()
            .remove_all_session_only_entries(key_in_use_cb, session_id)
    }

    /// Remove all entries for the given app ID from the store.
    ///
    /// # Arguments
    /// * `app_id` - The app ID.
    ///
    /// # Returns
    /// The number of entries removed.
    ///
    /// # Errors
    /// * `HsmErr::CannotDeleteKeyInUse` - If any of the entries cannot be deleted because
    ///   they are in use.
    /// * `HsmErr::CannotDeleteSomeKeysInUse` - If some of the entries cannot be deleted
    ///   because they are in use.
    pub(crate) fn remove_all_entries_for_app<F>(
        &mut self,
        key_in_use_cb: F,
        app_id: u8,
    ) -> Result<u16, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        self.rimpl
            .borrow_mut()
            .remove_all_entries_for_app(key_in_use_cb, app_id)
    }

    /// Fetch the key number of the entry with the given key tag for the specified application
    /// from the store.
    ///
    /// # Arguments
    /// * `app_id` - The app ID.
    /// * `key_tag` - The key tag.
    ///
    /// # Returns
    /// The key number of the entry with the given key tag for the specified application.
    ///
    /// # Errors
    /// * `HsmErr::KeyNotFound` - If the key tag is not found.
    pub(crate) fn get_entry_index_by_tag(&self, app_id: u8, key_tag: u16) -> Result<u16, HsmErr> {
        self.rimpl.borrow().get_entry_index_by_tag(app_id, key_tag)
    }

    /// Fetch the entry with the given key number from the store.
    ///
    /// # Arguments
    /// * `entry` - The key number of the entry to fetch.
    ///
    /// # Returns
    /// The entry with the given key number.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key number is invalid.
    pub(crate) fn get_entry(
        &self,
        entry: u16,
        allow_disabled: bool,
    ) -> Result<Ref<'_, PhysicalEntry>, HsmErr> {
        let mut err = None;
        Ref::filter_map(self.rimpl.borrow(), |rimpl| {
            match rimpl.get_entry(entry, allow_disabled) {
                Ok(value) => Some(value),
                Err(error) => {
                    err = Some(error);
                    None
                }
            }
        })
        .map_err(|_| err.unwrap_or(HsmErr::InvalidArgument))
    }

    pub(crate) fn get_entry_attributes(
        &self,
        entry: u16,
        allow_disabled: bool,
    ) -> Result<Ref<'_, EntryAttributes>, HsmErr> {
        let mut err = None;
        Ref::filter_map(self.rimpl.borrow(), |rimpl| {
            match rimpl.get_entry_attributes(entry, allow_disabled) {
                Ok(value) => Some(value),
                Err(error) => {
                    err = Some(error);
                    None
                }
            }
        })
        .map_err(|_| err.unwrap_or(HsmErr::InvalidArgument))
    }

    /// Fetch the blob of the entry with the given key number from the vault.
    ///
    /// # Arguments
    /// * `entry` - The key number of the entry to fetch.
    ///
    /// # Returns
    /// The blob of the entry with the given key number.
    ///
    /// # Errors
    /// * `HsmErr::InvalidKeyIndex` - If the key number is invalid.
    pub(crate) fn get_entry_blob(&self, entry: u16) -> Result<Ref<'_, [u8]>, HsmErr> {
        let mut err = None;
        Ref::filter_map(self.rimpl.borrow(), |rimpl| {
            match rimpl.get_entry_blob(entry) {
                Ok(value) => Some(value),
                Err(error) => {
                    err = Some(error);
                    None
                }
            }
        })
        .map_err(|_| err.unwrap_or(HsmErr::InvalidArgument))
    }

    /// CAUTION: This function deletes everything in this store.
    pub(crate) fn nuke(&mut self) {
        self.rimpl.borrow_mut().nuke()
    }
}

/// `KeyStore` is for managing a single key store.
struct KeyStoreImpl {
    base: usize,
    mask: u128,
}

impl KeyStoreImpl {
    fn new(base: usize, mask: u128) -> Self {
        Self { base, mask }
    }

    fn is_valid_table(&self, table_index: u8) -> bool {
        (table_index < MAX_TABLE_COUNT as u8) && ((self.mask & (1 << table_index)) != 0)
    }

    fn add_entry(
        &mut self,
        attributes: &EntryAttributes,
        session_id_or_key_tag: u16,
        kind: EntryKind,
        app_id: u8,
        blob: &[u8],
    ) -> Result<u16, HsmErr> {
        if kind == EntryKind::Free {
            Err(HsmErr::InvalidArgument)?
        }

        // Check if the input blob is of expected size.
        let expected_key_len = if kind.is_var_hmac_key() {
            let key_len = attributes.entry_specific[VarLenHmacShaKey::KEY_LENGTH_INDEX] as usize;
            if key_len == 0 {
                Err(HsmErr::InvalidArgument)?
            }
            key_len
        } else {
            kind.raw_key_blob_size()
        };

        if blob.len() != expected_key_len {
            Err(HsmErr::InvalidArgument)?
        }

        if !attributes.common.flags.session() && session_id_or_key_tag != KEY_TAG_UNASSIGNED {
            let key_tag_exists = self.get_entry_index_by_tag(app_id, session_id_or_key_tag);
            if key_tag_exists.is_ok() {
                Err(HsmErr::KeyTagAlreadyExists)?
            }
        }

        let mut defrag_needed = false;

        let key_num = (0..MAX_TABLE_COUNT)
            .find_map(|table_index| {
                let table = self.get_table_mut(table_index as u8).ok()?;
                let result = table
                    .add_entry(attributes, session_id_or_key_tag, kind, app_id, blob)
                    .map(|entry_index| KeyNumber::new(table_index as u8, entry_index));
                if let Err(err) = result {
                    if err == HsmErr::DefragNeeded {
                        defrag_needed = true;
                    }
                }
                result.ok()
            })
            .ok_or({
                if defrag_needed {
                    HsmErr::DefragNeeded
                } else {
                    HsmErr::NotEnoughSpace
                }
            })?;

        Ok(key_num.0)
    }

    fn enable_entry(&mut self, entry: u16) -> Result<(), HsmErr> {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table();
        let entry_index = key_num.entry();

        let table = self.get_table_mut(table_index)?;

        table.enable_entry(entry_index)
    }

    fn disable_entry(&mut self, entry: u16) -> Result<(), HsmErr> {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table();
        let entry_index = key_num.entry();

        let table = self.get_table_mut(table_index)?;

        table.disable_entry(entry_index)
    }

    fn remove_entry<F>(&mut self, key_in_use_cb: F, entry: u16) -> Result<(), HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table();
        let entry_index = key_num.entry();

        let table = self.get_table_mut(table_index)?;

        table.remove_entry(key_in_use_cb, table_index, entry_index)
    }

    fn remove_all_session_only_entries<F>(
        &mut self,
        key_in_use_cb: F,
        session_id: u16,
    ) -> Result<u16, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        let mut delete_count = 0;
        let mut failed_delete_count: u16 = 0;

        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if !self.is_valid_table(table_index) {
                continue;
            }

            let table = self.get_table_mut(table_index)?;
            match table.remove_all_session_only_entries(&key_in_use_cb, table_index, session_id) {
                Ok(count) => {
                    delete_count += count as u16;
                }
                Err(err) => {
                    if err == HsmErr::CannotDeleteKeyInUse {
                        failed_delete_count += 1;
                    } else {
                        // Safe to add 2 even though we don't know the exact count since that the
                        // only other error can be HsmErr::CannotDeleteSomeKeysInUse which is
                        // when we have more than 1 key we could not delete.
                        // And all we care about is if more than 1 key failed to delete so we can
                        // return the correct error code.
                        failed_delete_count += 2;
                    }
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

    fn remove_all_entries_for_app<F>(&mut self, key_in_use_cb: F, app_id: u8) -> Result<u16, HsmErr>
    where
        F: Fn(u16) -> bool,
    {
        let mut delete_count = 0;
        let mut failed_delete_count: u16 = 0;

        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if !self.is_valid_table(table_index) {
                continue;
            }

            let table = self.get_table_mut(table_index)?;
            match table.remove_all_entries_for_app(&key_in_use_cb, table_index, app_id) {
                Ok(count) => {
                    delete_count += count as u16;
                }
                Err(err) => {
                    if err == HsmErr::CannotDeleteKeyInUse {
                        failed_delete_count += 1;
                    } else {
                        // Safe to add 2 even though we don't know the exact count since that the
                        // only other error can be HsmErr::CannotDeleteSomeKeysInUse which is
                        // when we have more than 1 key we could not delete.
                        // And all we care about is if more than 1 key failed to delete so we can
                        // return the correct error code.
                        failed_delete_count += 2;
                    }
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

    fn get_entry_index_by_tag(&self, app_id: u8, key_tag: u16) -> Result<u16, HsmErr> {
        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if !self.is_valid_table(table_index) {
                continue;
            }

            let table = self.get_table(table_index)?;
            if let Ok(entry_index) = table.get_entry_index_by_tag(app_id, key_tag) {
                let key_num = KeyNumber::new(table_index, entry_index);
                return Ok(key_num.0);
            }
        }

        Err(HsmErr::KeyNotFound)?
    }

    fn get_entry(&self, entry: u16, allow_disabled: bool) -> Result<&PhysicalEntry, HsmErr> {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table() as usize;
        let entry_index = key_num.entry();

        let table = self.get_table(table_index as u8)?;

        table.get_entry(entry_index, allow_disabled)
    }

    fn get_entry_attributes(
        &self,
        entry: u16,
        allow_disabled: bool,
    ) -> Result<&EntryAttributes, HsmErr> {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table() as usize;
        let entry_index = key_num.entry();

        let table = self.get_table(table_index as u8)?;

        table.get_entry_attributes(entry_index, allow_disabled)
    }

    fn get_entry_blob(&self, entry: u16) -> Result<&[u8], HsmErr> {
        let key_num = KeyNumber(entry);
        let table_index = key_num.table() as usize;
        let entry_index = key_num.entry();

        let table = self.get_table(table_index as u8)?;

        table.get_entry_blob(entry_index)
    }

    fn get_table(&self, table_index: u8) -> Result<&PhysicalTable, HsmErr> {
        if !self.is_valid_table(table_index) {
            Err(HsmErr::InvalidKeyIndex)?
        }

        let table_index = table_index as usize;

        let table =
            unsafe { &*((self.base + table_index * TOTAL_TABLE_LEN) as *const PhysicalTable) };
        Ok(table)
    }

    fn get_table_mut(&mut self, table_index: u8) -> Result<&mut PhysicalTable, HsmErr> {
        if !self.is_valid_table(table_index) {
            Err(HsmErr::InvalidKeyIndex)?
        }

        let table_index = table_index as usize;

        let table =
            unsafe { &mut *((self.base + table_index * TOTAL_TABLE_LEN) as *mut PhysicalTable) };
        Ok(table)
    }

    fn nuke(&mut self) {
        for table_index in 0..MAX_TABLE_COUNT as u8 {
            if let Ok(table) = self.get_table_mut(table_index) {
                table.nuke();
            }
        }
    }
}

#[cfg(test)]
mod tests {
    use core::cmp::min;

    use super::*;

    #[test]
    fn test_new() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);
        assert_eq!(store.base, store_memory.as_ptr() as usize);
        assert_eq!(store.mask, 0b1010110);
    }

    #[test]
    fn test_add_entry_basic() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2 {
            let rsa_public_key_blob = [i + 1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i as u16);
        }

        assert!(store.remove_entry(|_| false, 0x0000).is_err());
        assert!(store.remove_entry(|_| false, 0x0001).is_err());
        assert!(store.remove_entry(|_| false, 0x0200).is_err());
        assert!(store.remove_entry(|_| false, 0x0201).is_err());
        assert!(store.remove_entry(|_| false, 0x0100).is_ok());
        assert!(store.remove_entry(|_| false, 0x0101).is_ok());
    }

    #[test]
    fn test_add_entry_duplicate_name() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(false);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 0x5453;
        let app_id = 10;

        {
            let i = 0;
            let rsa_public_key_blob = [i + 1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i as u16);
        }

        {
            let i = 1;
            let rsa_public_key_blob = [i + 1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_err());

            assert!(matches!(result, Err(HsmErr::KeyTagAlreadyExists)));
        }

        assert!(store.remove_entry(|_| false, 0x0100).is_ok());
        assert!(store.remove_entry(|_| false, 0x0101).is_err());
    }

    #[test]
    fn test_add_entry_kind_free() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Free;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2 {
            let rsa_public_key_blob = [i + 1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_err());

            assert!(matches!(result, Err(HsmErr::InvalidArgument)));
        }

        assert!(store.remove_entry(|_| false, 0x0000).is_err());
        assert!(store.remove_entry(|_| false, 0x0001).is_err());
        assert!(store.remove_entry(|_| false, 0x0200).is_err());
        assert!(store.remove_entry(|_| false, 0x0201).is_err());
        assert!(store.remove_entry(|_| false, 0x0100).is_err());
        assert!(store.remove_entry(|_| false, 0x0101).is_err());
    }

    #[test]
    fn test_add_entry_max_keys() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Aes128;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0400 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0600 + i);
        }

        let aes_key_blob = [(allowed_entry_count_per_table + 1) as u8; 16];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &aes_key_blob,
        );
        assert!(result.is_err());

        let err = result.unwrap_err();
        assert_eq!(err, HsmErr::NotEnoughSpace);

        assert!(store.remove_entry(|_| false, 0x0403).is_ok());

        let aes_key_blob = [(allowed_entry_count_per_table + 1) as u8; 16];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &aes_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0x0403);
    }

    #[test]
    fn test_add_entry_max_bytes() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0400 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0600 + i);
        }

        let rsa_public_key_blob = [(allowed_entry_count_per_table + 1) as u8; 260];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_err());

        let err = result.unwrap_err();
        assert_eq!(err, HsmErr::NotEnoughSpace);

        assert!(store.remove_entry(|_| false, 0x0403).is_ok());

        let rsa_public_key_blob = [(allowed_entry_count_per_table + 1) as u8; 260];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &rsa_public_key_blob,
        );
        assert!(result.is_ok());

        let index = result.unwrap();
        assert_eq!(index, 0x0403);
    }

    #[test]
    fn test_add_entry_defrag_needed() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0400 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0600 + i);
        }

        assert!(store.remove_entry(|_| false, 0x0200).is_ok());
        assert!(store.remove_entry(|_| false, 0x0201).is_ok());
        assert!(store.remove_entry(|_| false, 0x0203).is_ok());
        assert!(store.remove_entry(|_| false, 0x0204).is_ok());

        let rsa_private_key_blob = [(allowed_entry_count_per_table + 1) as u8; 1028];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            EntryKind::Rsa4kPrivate,
            app_id,
            &rsa_private_key_blob,
        );
        assert!(result.is_err());

        let err = result.unwrap_err();
        assert_eq!(err, HsmErr::DefragNeeded);

        assert!(store.remove_entry(|_| false, 0x0403).is_ok());
    }

    #[test]
    fn test_remove_all_session_only_entries() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag + 1,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        assert!(store
            .remove_all_session_only_entries(|_| false, session_id_or_key_tag)
            .is_ok());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_ok());
        }
    }

    #[test]
    fn test_remove_all_session_only_entries_single_key_in_use() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        let entry_to_use = store.get_entry(0x0201, false).unwrap();
        assert!(!entry_to_use.disabled());

        let result =
            store.remove_all_session_only_entries(|key_id| key_id == 0x0201, session_id_or_key_tag);
        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::CannotDeleteKeyInUse);
        assert!(store.get_entry(0x0201, false).is_err());

        let entry0201 =
            unsafe { &*((store.base + 2 * TOTAL_TABLE_LEN + 8) as *const PhysicalEntry) };
        assert!(entry0201.disabled());

        let result = store.remove_all_session_only_entries(|_| false, session_id_or_key_tag);
        assert!(result.is_ok());
        assert!(entry0201.is_empty());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_err());
        }
    }

    #[test]
    fn test_remove_all_session_only_entries_multiple_key_in_use() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);
        let mut keys_in_use = vec![];

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        let entry_to_use = store.get_entry(0x0201, false).unwrap();
        keys_in_use.push(0x0201);
        keys_in_use.push(0x0102);
        keys_in_use.push(0x0103);
        assert!(!entry_to_use.disabled());

        let result = store.remove_all_session_only_entries(
            |key_id| keys_in_use.contains(&key_id),
            session_id_or_key_tag,
        );
        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::CannotDeleteSomeKeysInUse);
        assert!(store.get_entry(0x0201, false).is_err());
        assert!(store.get_entry(0x0102, false).is_err());

        let entry0201 =
            unsafe { &*((store.base + 2 * TOTAL_TABLE_LEN + 8) as *const PhysicalEntry) };
        let entry0102 =
            unsafe { &*((store.base + TOTAL_TABLE_LEN + 8 * 2) as *const PhysicalEntry) };
        assert!(entry0201.disabled());
        assert!(entry0102.disabled());
        assert!(!entry0201.is_empty());
        assert!(!entry0102.is_empty());

        keys_in_use.clear();
        let result = store.remove_all_session_only_entries(
            |key_id| keys_in_use.contains(&key_id),
            session_id_or_key_tag,
        );
        assert!(result.is_ok());
        assert!(entry0201.is_empty());
        assert!(entry0102.is_empty());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_err());
        }
    }

    #[test]
    fn test_remove_entry_basic() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);
        let mut keys_in_use = vec![];

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        for i in 0..2 {
            let rsa_public_key_blob = [i + 1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i as u16);
        }

        keys_in_use.push(0x0100);
        keys_in_use.push(0x0100);

        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0000)
            .is_err());
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0001)
            .is_err());
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0200)
            .is_err());
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0201)
            .is_err());
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0100)
            .is_err());
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0101)
            .is_ok());

        keys_in_use.remove(0);
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0100)
            .is_err());

        keys_in_use.clear();
        assert!(store
            .remove_entry(|key_id| keys_in_use.contains(&key_id), 0x0100)
            .is_ok());
    }

    #[test]
    fn test_remove_all_entries_for_app() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id + 1,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        assert!(store.remove_all_entries_for_app(|_| false, app_id).is_ok());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_ok());
        }
    }

    #[test]
    fn test_remove_all_entries_for_app_single_key_in_use() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);
        let mut keys_in_use = vec![];

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        let entry_to_use = store.get_entry(0x0201, false).unwrap();
        keys_in_use.push(0x0201);
        assert!(!entry_to_use.disabled());

        let result =
            store.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), app_id);
        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::CannotDeleteKeyInUse);
        assert!(store.get_entry(0x0201, false).is_err());

        let entry0201 =
            unsafe { &*((store.base + 2 * TOTAL_TABLE_LEN + 8) as *const PhysicalEntry) };
        assert!(entry0201.disabled());

        keys_in_use.clear();
        let result =
            store.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), app_id);
        assert!(result.is_ok());
        assert!(entry0201.is_empty());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_err());
        }
    }

    #[test]
    fn test_remove_all_entries_for_app_multiple_key_in_use() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);
        let mut keys_in_use = vec![];

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Rsa2kPublic;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let rsa_public_key_blob = [(i + 1) as u8; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        let entry_to_use = store.get_entry(0x0201, false).unwrap();
        keys_in_use.push(0x0201);
        keys_in_use.push(0x0102);
        keys_in_use.push(0x0103);
        assert!(!entry_to_use.disabled());

        let result =
            store.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), app_id);
        assert!(result.is_err());
        assert_eq!(result.unwrap_err(), HsmErr::CannotDeleteSomeKeysInUse);
        assert!(store.get_entry(0x0201, false).is_err());
        assert!(store.get_entry(0x0102, false).is_err());

        let entry0201 =
            unsafe { &*((store.base + 2 * TOTAL_TABLE_LEN + 8) as *const PhysicalEntry) };
        let entry0102 =
            unsafe { &*((store.base + TOTAL_TABLE_LEN + 8 * 2) as *const PhysicalEntry) };
        assert!(entry0201.disabled());
        assert!(entry0102.disabled());
        assert!(!entry0201.is_empty());
        assert!(!entry0102.is_empty());

        keys_in_use.clear();
        let result =
            store.remove_all_entries_for_app(|key_id| keys_in_use.contains(&key_id), app_id);
        assert!(result.is_ok());
        assert!(entry0201.is_empty());
        assert!(entry0102.is_empty());

        for i in 0..allowed_entry_count_per_table {
            let table1_result = store.get_entry(0x0100 + i, false);
            assert!(table1_result.is_err());
            assert!(matches!(table1_result, Err(HsmErr::InvalidKeyIndex)));

            let table2_result = store.get_entry(0x0200 + i, false);
            assert!(table2_result.is_err());
        }
    }

    #[test]
    fn test_nuke() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::Aes128;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let allowed_entry_count_per_table = min(
            BLOB_MEMORY_SIZE_BYTES / kind.storage_size(None),
            MAX_TABLE_ENTRY_COUNT,
        ) as u16;

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0200 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0400 + i);
        }

        for i in 0..allowed_entry_count_per_table {
            let aes_key_blob = [(i + 1) as u8; 16];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &aes_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0600 + i);
        }

        assert!(store.get_entry(0x0100, false).is_ok());
        assert!(store.get_entry(0x0202, false).is_ok());
        assert!(store.get_entry(0x0404, false).is_ok());
        assert!(store
            .get_entry(0x0600 + allowed_entry_count_per_table - 1, false)
            .is_ok());

        store.nuke();

        assert!(store.get_entry(0x0100, false).is_err());
        assert!(store.get_entry(0x0202, false).is_err());
        assert!(store.get_entry(0x0404, false).is_err());
        assert!(store
            .get_entry(0x0600 + allowed_entry_count_per_table - 1, false)
            .is_err());

        assert!(matches!(
            store.get_entry(0x0100, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry(0x0202, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry(0x0404, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry(0x0600 + allowed_entry_count_per_table - 1, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
    }

    #[test]
    fn test_get_entry_and_blob() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);

        {
            let mut attributes = EntryAttributes::default();
            attributes.common.flags.set_session(true);

            let kind = EntryKind::Rsa2kPublic;
            let session_id_or_key_tag = 5;
            let app_id = 10;

            let rsa_public_key_blob = [1; 260];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_public_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0100);
        }
        {
            let mut attributes = EntryAttributes::default();
            attributes.common.flags.set_local(true);

            let kind = EntryKind::Rsa4kPrivate;
            let session_id_or_key_tag = 6;
            let app_id = 11;

            let rsa_private_key_blob = [2; 1028];
            let result = store.add_entry(
                &attributes,
                session_id_or_key_tag,
                kind,
                app_id,
                &rsa_private_key_blob,
            );
            assert!(result.is_ok());

            let index = result.unwrap();
            assert_eq!(index, 0x0101);
        }

        assert!(store.get_entry(0x0100, false).unwrap().kind() == EntryKind::Rsa2kPublic);
        assert_eq!(
            store.get_entry(0x0100, false).unwrap().session_id(),
            Some(5)
        );
        assert_eq!(store.get_entry(0x0100, false).unwrap().key_tag(), None);
        assert_eq!(store.get_entry(0x0100, false).unwrap().app_id(), 10);
        assert_eq!(
            store
                .get_entry(0x0100, false)
                .unwrap()
                .kind()
                .raw_key_blob_size(),
            260
        );
        assert!(store
            .get_entry_attributes(0x0100, false)
            .unwrap()
            .common
            .flags
            .session());
        assert!(!store.get_entry(0x0100, false).unwrap().disabled());
        assert!(!store
            .get_entry_attributes(0x0100, false)
            .unwrap()
            .common
            .flags
            .local());
        assert_eq!(store.get_entry_blob(0x0100).unwrap(), [0x1; 260]);

        assert!(store.get_entry(0x0101, false).unwrap().kind() == EntryKind::Rsa4kPrivate);
        assert_eq!(store.get_entry(0x0101, false).unwrap().session_id(), None);
        assert_eq!(store.get_entry(0x0101, false).unwrap().key_tag(), Some(6));
        assert_eq!(store.get_entry(0x0101, false).unwrap().app_id(), 11);
        assert_eq!(
            store
                .get_entry(0x0101, false)
                .unwrap()
                .kind()
                .raw_key_blob_size(),
            1028
        );
        assert!(!store
            .get_entry_attributes(0x0101, false)
            .unwrap()
            .common
            .flags
            .session());
        assert!(!store.get_entry(0x0101, false).unwrap().disabled());
        assert!(store
            .get_entry_attributes(0x0101, false)
            .unwrap()
            .common
            .flags
            .local());
        assert_eq!(store.get_entry_blob(0x0101).unwrap(), [0x2; 1028]);

        assert!(matches!(
            store.get_entry(0x0102, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry_blob(0x0102),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry(0x0301, false),
            Err(HsmErr::InvalidKeyIndex)
        ));
        assert!(matches!(
            store.get_entry_blob(0x0301),
            Err(HsmErr::InvalidKeyIndex)
        ));
    }

    #[test]
    fn test_get_table() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0b1010110);

        assert_eq!(store.mask.count_ones(), 4);
        assert!(store.get_table(0).is_err());
        assert!(store.get_table(1).is_ok());
        assert!(store.get_table(2).is_ok());
        assert!(store.get_table(3).is_err());
    }

    #[test]
    fn test_entry_enable_disable() {
        let store_memory = [0u32; (17 * 1024 * 65) / 4];

        let mut store = KeyStoreImpl::new(store_memory.as_ptr() as usize, 0x1);

        assert_eq!(store.mask.count_ones(), 1);

        let mut attributes = EntryAttributes::default();
        attributes.common.flags.set_session(true);

        let kind = EntryKind::AesGcmBulk256Unapproved;
        let session_id_or_key_tag = 5;
        let app_id = 10;

        let aesbulk256_key_blob = [0; 2];
        let result = store.add_entry(
            &attributes,
            session_id_or_key_tag,
            kind,
            app_id,
            &aesbulk256_key_blob,
        );
        assert!(result.is_ok());

        assert!(store.disable_entry(0).is_ok());
        assert!(store.enable_entry(0).is_ok());
    }
}
