// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use bitfield::Bit;
use bitfield::BitMut;
use core::cell::RefCell;
use mcr_types::SESSION_TABLE_LEN;
use zeroize::Zeroize;

use crate::error::HsmResult;
use crate::HsmErr;

const MAX_SESSIONS: usize = 8;

/// Session indirection table that maps Virtual session ids to Physical sessions
/// ids (in the vault).
/// This aids in live migration, disaster recovery etc. Virtual session ids will
/// remain same to the customer even if the physical session id gets changed.
pub struct SessionTable {
    rimpl: Rc<RefCell<SessionTableImpl>>,
}

impl SessionTable {
    /// Create Session Table Object
    ///
    /// # Arguments
    ///
    /// * `base` - Base address of the session table for the current partition
    ///
    /// # Returns
    ///
    /// `SessionTable` - SessionTable Instance
    pub fn new(base: usize) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(SessionTableImpl::new(base))),
        }
    }

    /// Get the count of number of sessions that can still be created
    pub fn get_available_session_count(&self) -> usize {
        self.rimpl
            .borrow()
            .get_table()
            .get_available_session_count() as usize
    }

    /// Create a virtual session in the SessionTable
    ///
    /// # Arguments
    ///
    /// * `target_id` - Physical session id in the vault
    ///
    /// # Returns
    ///
    /// `u16` - Virtual session id
    pub fn create_session(&self, target_id: u16) -> HsmResult<u16> {
        self.rimpl
            .borrow_mut()
            .get_table_mut()
            .create_session(target_id)
    }

    /// Get Physical Session Id for the passed virtual id.
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    ///
    /// # Returns
    ///
    /// `u16` - Physical session id in the vault
    pub fn get_target_session(&self, id: u16) -> HsmResult<u16> {
        self.rimpl.borrow().get_table().get_target_session(id)
    }

    /// Delete the entry from SessionTable.
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    pub fn delete(&self, id: u16) {
        self.rimpl.borrow_mut().get_table_mut().delete(id);
    }

    /// Rollback the change from recreation session action
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    pub fn rollback_recreation(&self, id: u16) {
        self.rimpl
            .borrow_mut()
            .get_table_mut()
            .rollback_recreation(id);
    }

    /// Back up the current SessionTable allocation mask.
    ///
    /// # Returns
    ///
    /// * `u8` - Mask for which sessions are currently created
    #[allow(unused)]
    pub fn backup(&self) -> u8 {
        self.rimpl.borrow().get_table().backup()
    }

    /// Restore the SessionTable mask. This ensures that the session ids remain reserved.
    /// The restored sessions must be reestablished. To reestablish, reestablish credential
    /// ddi api needs to be used by the client.
    ///
    /// # Arguments
    ///
    /// * `mask` - Mask for which sessions were currently created and need to be reestablished
    pub fn restore(&self, mask: u8) {
        self.rimpl.borrow_mut().get_table_mut().restore(mask);
    }

    /// Returns whether the session is valid or not. A session is valid if it is currently
    /// present but does not need to be reestablished.
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    ///
    /// # Returns
    ///
    /// * `HsmResult<()>` - Ok if valid, Err with reason if not valid
    pub fn valid(&self, id: u16) -> HsmResult<()> {
        self.rimpl.borrow().get_table().valid(id)
    }

    /// Returns whether the session needs to be reestablished e.g. after a live migration
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    ///
    /// # Returns
    ///
    /// * `bool` - True if needs to be reestablished, false otherwise
    pub fn needs_renegotiation(&self, id: u16) -> bool {
        self.rimpl.borrow().get_table().needs_renegotiation(id)
    }

    /// Similar to create_session but allows to map a specific virtual session to
    /// the physical session id. This is used for sessions that need renegotiation.
    ///
    /// # Arguments
    ///
    /// * `id` - Virtual session id in the SessionTable
    /// * `target_id` - Physical session id in the vault
    pub fn recreate_session(&self, id: u16, target_id: u16) {
        self.rimpl
            .borrow_mut()
            .get_table_mut()
            .recreate_session(id, target_id);
    }
}

struct SessionTableImpl {
    base: usize,
}

impl SessionTableImpl {
    fn new(base: usize) -> Self {
        Self { base }
    }

    fn get_table(&self) -> &HsmPartSessionTable {
        unsafe { &*(self.base as *const HsmPartSessionTable) }
    }

    fn get_table_mut(&mut self) -> &mut HsmPartSessionTable {
        unsafe { &mut *(self.base as *mut HsmPartSessionTable) }
    }
}

#[repr(C)]
struct HsmPartSessionTable {
    /// Session Allocation Mask
    pub session_allocation_mask: u8,

    /// Session Renegotiation Mask
    pub session_renegotiation_mask: u8,

    /// Session Indirection Table
    pub table: [u16; MAX_SESSIONS],
}
static_assertions::const_assert_eq!(size_of::<HsmPartSessionTable>(), SESSION_TABLE_LEN);

impl HsmPartSessionTable {
    #[allow(unused)]
    fn backup(&self) -> u8 {
        self.session_allocation_mask
    }

    fn restore(&mut self, mask: u8) {
        self.session_allocation_mask = mask;
        self.session_renegotiation_mask = mask;
        self.table.zeroize();
    }

    fn get_available_session_count(&self) -> u32 {
        self.session_allocation_mask.count_zeros()
    }

    fn needs_renegotiation(&self, id: u16) -> bool {
        if id as usize > MAX_SESSIONS - 1 {
            return false;
        }

        self.session_allocation_mask.bit(id.into())
            && self.session_renegotiation_mask.bit(id.into())
    }

    fn valid(&self, id: u16) -> HsmResult<()> {
        if id as usize > MAX_SESSIONS - 1 {
            Err(HsmErr::SessionLimitReached)?
        }

        if !self.session_allocation_mask.bit(id.into()) {
            Err(HsmErr::SessionNotFound)?
        }

        if self.session_renegotiation_mask.bit(id.into()) {
            Err(HsmErr::SessionNeedsRenegotiation)?
        }

        Ok(())
    }

    fn recreate_session(&mut self, id: u16, target_id: u16) {
        if (id as usize) < MAX_SESSIONS && self.needs_renegotiation(id) {
            self.session_renegotiation_mask.set_bit(id.into(), false);
            self.table[id as usize] = target_id;
        }
    }

    fn create_session(&mut self, target_id: u16) -> HsmResult<u16> {
        let id = self.session_allocation_mask.trailing_ones() as u16;

        if id as usize > MAX_SESSIONS - 1 {
            Err(HsmErr::SessionLimitReached)?
        }

        self.table[id as usize] = target_id;
        self.session_allocation_mask.set_bit(id.into(), true);
        Ok(id)
    }

    fn get_target_session(&self, id: u16) -> HsmResult<u16> {
        self.valid(id)?;

        Ok(self.table[id as usize])
    }

    fn rollback_recreation(&mut self, id: u16) {
        if (id as usize) < MAX_SESSIONS {
            self.session_allocation_mask.set_bit(id.into(), true);
            self.session_renegotiation_mask.set_bit(id.into(), true);
            self.table[id as usize] = 0;
        }
    }

    fn delete(&mut self, id: u16) {
        if (id as usize) < MAX_SESSIONS {
            self.session_allocation_mask.set_bit(id.into(), false);
            self.session_renegotiation_mask.set_bit(id.into(), false);
            self.table[id as usize] = 0;
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_session_create_new_table() {
        let store_memory = [0u8; SESSION_TABLE_LEN];
        let session_table = SessionTable::new(store_memory.as_ptr() as usize);

        let session_id = session_table.create_session(0);
        assert_eq!(session_id, Ok(0));
    }

    #[test]
    fn test_session_create_max_new_table() {
        let store_memory = [0u8; SESSION_TABLE_LEN];
        let session_table = SessionTable::new(store_memory.as_ptr() as usize);

        for i in 0..MAX_SESSIONS as u16 {
            let session_id = session_table.create_session(i);
            assert_eq!(session_id, Ok(i));
        }

        let session_id = session_table.create_session(130);
        assert_eq!(session_id, Err(HsmErr::SessionLimitReached));
    }

    #[test]
    fn test_get_target_session_new_table() {
        let store_memory = [0u8; SESSION_TABLE_LEN];
        let session_table = SessionTable::new(store_memory.as_ptr() as usize);

        let session_id = session_table.create_session(4);
        assert_eq!(session_id, Ok(0));

        let target_session = session_table.get_target_session(session_id.unwrap());
        assert_eq!(target_session, Ok(4));

        assert_eq!(
            session_table.get_target_session(1),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session(4),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 1) as u16),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session(MAX_SESSIONS as u16),
            Err(HsmErr::SessionLimitReached)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS + 20) as u16),
            Err(HsmErr::SessionLimitReached)
        );
    }

    #[test]
    fn test_get_target_session_max_new_table() {
        let store_memory = [0u8; SESSION_TABLE_LEN];
        let session_table = SessionTable::new(store_memory.as_ptr() as usize);

        for i in 0..MAX_SESSIONS as u16 {
            let session_id = session_table.create_session(i);
            assert_eq!(session_id, Ok(i));
        }

        for i in 0..MAX_SESSIONS as u16 {
            let target_session = session_table.get_target_session(i);
            assert_eq!(target_session, Ok(i));
        }

        assert_eq!(
            session_table.get_target_session(MAX_SESSIONS as u16),
            Err(HsmErr::SessionLimitReached)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS + 50) as u16),
            Err(HsmErr::SessionLimitReached)
        );
    }

    #[test]
    fn test_backup_restore_renegotiate() {
        let store_memory = [0u8; SESSION_TABLE_LEN];
        let session_table = SessionTable::new(store_memory.as_ptr() as usize);

        assert!(!session_table.needs_renegotiation(500));

        for i in 0..MAX_SESSIONS as u16 {
            let session_id = session_table.create_session(i);
            assert_eq!(session_id, Ok(i));
        }

        assert!(session_table.valid(1).is_ok());
        assert!(session_table.valid((MAX_SESSIONS - 1) as u16).is_ok());
        assert!(session_table.valid(0).is_ok());
        assert!(session_table.valid((MAX_SESSIONS - 2) as u16).is_ok());
        assert!(!session_table.needs_renegotiation(1));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 1) as u16));
        assert!(!session_table.needs_renegotiation(0));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 2) as u16));
        assert_eq!(session_table.get_target_session(1), Ok(1));
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 1) as u16),
            Ok((MAX_SESSIONS - 1) as u16)
        );
        assert_eq!(session_table.get_target_session(0), Ok(0));
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 2) as u16),
            Ok((MAX_SESSIONS - 2) as u16)
        );

        for i in (0..MAX_SESSIONS as u16).step_by(2) {
            session_table.delete(i);
        }

        let backup = session_table.backup();
        assert_eq!(backup, 0b10101010);
        assert!(session_table.valid(1).is_ok());
        assert!(session_table.valid((MAX_SESSIONS - 1) as u16).is_ok());
        assert_eq!(session_table.valid(0), Err(HsmErr::SessionNotFound));
        assert_eq!(
            session_table.valid((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );
        assert!(!session_table.needs_renegotiation(1));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 1) as u16));
        assert!(!session_table.needs_renegotiation(0));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 2) as u16));
        assert_eq!(session_table.get_target_session(1), Ok(1));
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 1) as u16),
            Ok((MAX_SESSIONS - 1) as u16)
        );
        assert_eq!(
            session_table.get_target_session(0),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );

        session_table.restore(backup);
        assert_eq!(
            session_table.valid(1),
            Err(HsmErr::SessionNeedsRenegotiation)
        );
        assert_eq!(
            session_table.valid((MAX_SESSIONS - 1) as u16),
            Err(HsmErr::SessionNeedsRenegotiation)
        );
        assert_eq!(session_table.valid(0), Err(HsmErr::SessionNotFound));
        assert_eq!(
            session_table.valid((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );
        assert!(session_table.needs_renegotiation(1));
        assert!(session_table.needs_renegotiation((MAX_SESSIONS - 1) as u16));
        assert!(!session_table.needs_renegotiation(0));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 2) as u16));
        assert_eq!(
            session_table.get_target_session(1),
            Err(HsmErr::SessionNeedsRenegotiation)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 1) as u16),
            Err(HsmErr::SessionNeedsRenegotiation)
        );
        assert_eq!(
            session_table.get_target_session(0),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );

        session_table.recreate_session(1, 101);
        session_table.recreate_session((MAX_SESSIONS - 1) as u16, 227);
        session_table.recreate_session(0, 100);
        session_table.recreate_session((MAX_SESSIONS - 2) as u16, 226);

        assert!(session_table.valid(1).is_ok());
        assert!(session_table.valid((MAX_SESSIONS - 1) as u16).is_ok());
        assert_eq!(session_table.valid(0), Err(HsmErr::SessionNotFound));
        assert_eq!(
            session_table.valid((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );
        assert!(!session_table.needs_renegotiation(1));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 1) as u16));
        assert!(!session_table.needs_renegotiation(0));
        assert!(!session_table.needs_renegotiation((MAX_SESSIONS - 2) as u16));
        assert_eq!(session_table.get_target_session(1), Ok(101));
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 1) as u16),
            Ok(227)
        );
        assert_eq!(
            session_table.get_target_session(0),
            Err(HsmErr::SessionNotFound)
        );
        assert_eq!(
            session_table.get_target_session((MAX_SESSIONS - 2) as u16),
            Err(HsmErr::SessionNotFound)
        );
    }
}
