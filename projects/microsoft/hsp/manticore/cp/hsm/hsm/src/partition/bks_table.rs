// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::RefCell;
use core::mem::offset_of;
use mcr_types::BksTableEntry;
use mcr_types::SecureByteArray;

const NUM_BKS1_ENTRIES: usize = 11;

/// A table storing BKS1 and BKS2 values
/// First key will be BKS1 for current firmware
/// Last key will be BKS2
#[derive(Clone)]
pub struct BksTable {
    rimpl: Rc<RefCell<BksTableImpl>>,
}

impl BksTable {
    /// Create BKS Table Object
    ///
    /// # Arguments
    ///
    /// * `base` - Base address of the bks table for the current partition
    ///
    /// # Returns
    ///
    /// `BksTable` - BksTable Instance
    pub fn new(base: usize) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(BksTableImpl::new(base))),
        }
    }

    pub fn get_bks1_current(&self) -> SecureByteArray<32> {
        self.rimpl.borrow().get_bks1_current().into()
    }

    pub fn get_bks1(&self, svn: u64) -> Option<SecureByteArray<32>> {
        self.rimpl.borrow().get_bks1(svn).map(|bks| bks.into())
    }

    pub fn get_bks2(&self) -> SecureByteArray<32> {
        self.rimpl.borrow().get_bks2().into()
    }

    pub fn get_current_svn(&self) -> u64 {
        self.rimpl.borrow().get_current_svn()
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn set_current_svn(&self, svn: u64) {
        self.rimpl.borrow_mut().set_current_svn(svn)
    }
}

struct BksTableImpl {
    /// BK1 entries
    pub bks1_entries: [BksTableEntry; NUM_BKS1_ENTRIES],

    /// BK2 entry
    pub bks2_entry: BksTableEntry,
}

impl BksTableImpl {
    fn new(base: usize) -> Self {
        Self {
            bks1_entries: unsafe { *(base as *const [BksTableEntry; 11]) },
            bks2_entry: unsafe {
                *((base + offset_of!(BksTableImpl, bks2_entry)) as *const BksTableEntry)
            },
        }
    }

    fn get_bks1_current(&self) -> [u8; 32] {
        self.bks1_entries[0].bks
    }

    fn get_bks1(&self, svn: u64) -> Option<[u8; 32]> {
        self.bks1_entries
            .iter()
            .find(|entry| entry.svn == svn.to_le_bytes())
            .map(|entry| entry.bks)
    }

    fn get_current_svn(&self) -> u64 {
        u64::from_le_bytes(self.bks1_entries[0].svn)
    }

    fn get_bks2(&self) -> [u8; 32] {
        self.bks2_entry.bks
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn set_current_svn(&mut self, svn: u64) {
        // Overwrite the last entry with the first entry
        self.bks1_entries[NUM_BKS1_ENTRIES - 1] = self.bks1_entries[0];

        // Update the first entry with the new SVN and zeroed BKS
        self.bks1_entries[0].svn = svn.to_le_bytes();
        self.bks1_entries[0].bks = [0u8; 32];
        self.bks1_entries[0].valid = 1;
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    const BKS_TABLE_LEN: usize = 12;

    #[test]
    fn test_get_current_svn() {
        let store_memory = [0u8; BKS_TABLE_LEN * size_of::<BksTableEntry>()];
        let bks_table = BksTable::new(store_memory.as_ptr() as usize);

        let svn = bks_table.get_current_svn();
        assert_eq!(svn, 0);
    }

    #[test]
    fn test_get_first_bks1() {
        let store_memory = [0u8; BKS_TABLE_LEN * size_of::<BksTableEntry>()];
        let bks_table = BksTable::new(store_memory.as_ptr() as usize);

        let bks1 = bks_table.get_bks1(0);
        assert_eq!(bks1.unwrap().as_slice(), &[0; 32]);
    }

    #[test]
    fn test_get_bks1_by_svn() {
        let mut store_memory = [0u8; BKS_TABLE_LEN * size_of::<BksTableEntry>()];
        for i in 1..=NUM_BKS1_ENTRIES + 1 {
            let table_entry = BksTableEntry {
                valid: 1u8,
                svn: (i as u64).to_le_bytes(),
                bks: [i as u8; 32],
            };
            let entry = &mut store_memory
                [(i - 1) * size_of::<BksTableEntry>()..i * size_of::<BksTableEntry>()];
            entry.copy_from_slice(unsafe {
                core::slice::from_raw_parts(
                    &table_entry as *const BksTableEntry as *const u8,
                    size_of::<BksTableEntry>(),
                )
            });
        }
        let bks_table = BksTable::new(store_memory.as_ptr() as usize);

        assert_eq!(bks_table.get_current_svn(), 1);
        for i in 0..NUM_BKS1_ENTRIES {
            let bks1 = bks_table.get_bks1((i + 1) as u64);
            assert_eq!(bks1.unwrap().as_slice(), &[(i + 1) as u8; 32]);
        }
        assert_eq!(bks_table.get_bks2().as_slice(), &[BKS_TABLE_LEN as u8; 32]);
    }

    #[test]
    fn test_get_bks2() {
        let store_memory = [0u8; BKS_TABLE_LEN * size_of::<BksTableEntry>()];
        let bks_table = BksTable::new(store_memory.as_ptr() as usize);

        let bks2 = bks_table.get_bks2();
        assert_eq!(bks2.as_slice(), [0; 32]);
    }
}
