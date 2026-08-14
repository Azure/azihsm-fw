// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::rc::Rc;
use core::cell::Ref;
use core::cell::RefCell;
use core::cell::RefMut;
use mcr_crypto_rng::RngTrait;

use bitfield::Bit;
use cdma_vault::CdmaKeyVault;
#[cfg(feature = "mcr_test_hooks")]
use mcr_ddi_types::DdiTestAction;
use mcr_types::*;

use super::session_table::SessionTable;
use super::*;
use crate::env::HsmEnvTrait;
use crate::lm_key_derive::BK3_SIZE_BYTES;

#[repr(transparent)]
#[derive(Copy, Clone, Default)]
pub(crate) struct ResourceGroups(u128);

impl ResourceGroups {
    pub fn mask(&self) -> u128 {
        self.0
    }

    pub fn set_mask(&mut self, mask: u128) {
        self.0 = mask;
    }
}

pub(crate) struct PartState<E: HsmEnvTrait + 'static> {
    pub(super) rimpl: Rc<RefCell<PartStateImpl<E>>>,
}

impl<E: HsmEnvTrait> Clone for PartState<E> {
    fn clone(&self) -> Self {
        Self {
            rimpl: self.rimpl.clone(),
        }
    }
}

impl<E: HsmEnvTrait> PartState<E> {
    pub fn new(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        Self {
            rimpl: Rc::new(RefCell::new(PartStateImpl::new(pfn, ctx))),
        }
    }

    pub fn new_with_resource_table(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        let state = Self::new(pfn, ctx);

        state.rimpl.borrow_mut().restore_res_mask();

        state
    }

    pub fn restore(pfn: PcieFunction, ctx: PartEnv<E>) -> Self {
        let index: usize = pfn.into();
        let data_store_addr = ctx.data_store_addr();
        let data_store = unsafe { &*(data_store_addr as *const HsmPartDataStore) };
        let part_store = &data_store.part[index];

        let state = Self::new(pfn, ctx);

        state.rimpl.borrow_mut().enabled = part_store.enabled;
        state.rimpl.borrow_mut().unwrapping_key_id = part_store.unwrapping_key_id;
        state.set_establish_cred_encryption_key_id(part_store.establish_cred_encryption_key_id);
        state.set_partition_mk_id(part_store.masking_key);
        state.set_session_encryption_key_id(part_store.session_encryption_key_id);

        state.rimpl.borrow_mut().restore_res_mask();
        state.rimpl.borrow_mut().restore_cred_mgr();
        state.rimpl.borrow_mut().restore_io_queues();

        state.set_partition_cert_valid(false);

        state
    }

    pub fn enable(&self) {
        self.rimpl.borrow_mut().enabled = true;
    }

    pub fn disable(&self) {
        self.rimpl.borrow_mut().enabled = false;
        self.cred_mgr_mut().clear();
        self.rimpl.borrow_mut().unwrapping_key_id.take();
        self.set_establish_cred_encryption_key_id(None);
        self.set_session_encryption_key_id(None);
        self.reset_nonce();
        self.session_table().restore(self.session_table().backup());
        self.vault().clear();
        self.cdma_vault().clear();
        self.part_persistent_store_ref().pin_policy = PinPolicy::default();
        let _ = self.rimpl.borrow_mut().masking_key.take();
        #[cfg(feature = "mcr_test_hooks")]
        {
            self.rimpl.borrow_mut().test_hook_to_trigger_level2_abort = false;
            self.rimpl.borrow_mut().cmd_fsm_test_action = None;
            self.rimpl.borrow_mut().hsm_fsm_test_action = None;
        }
    }

    pub fn enabled(&self) -> bool {
        self.rimpl.borrow().enabled
    }

    pub fn nonce(&self) -> [u8; 32] {
        self.part_persistent_store_ref().nonce
    }

    pub fn reset_nonce(&self) {
        self.rimpl.borrow_mut().reset_nonce()
    }

    pub fn verify_user_cred_is_set(&self) -> bool {
        self.rimpl.borrow().verify_user_cred_is_set()
    }

    pub fn change_user_cred(&self, id: &[u8], pin: &[u8]) -> HsmResult<()> {
        self.rimpl.borrow_mut().change_user_cred(id, pin)
    }

    pub fn rgs(&self) -> Ref<'_, ResourceGroups> {
        Ref::map(self.rimpl.borrow(), |s| &s.rgs)
    }

    pub fn rgs_mut(&self) -> RefMut<'_, ResourceGroups> {
        RefMut::map(self.rimpl.borrow_mut(), |s| &mut s.rgs)
    }

    pub fn rgs_reset(&self) {
        self.rimpl.borrow_mut().rgs = ResourceGroups::default();
    }

    pub fn ioq_mgr(&self) -> Ref<'_, IoQueueMgr> {
        Ref::map(self.rimpl.borrow(), |s| &s.ioq_mgr)
    }

    pub fn ioq_mgr_mut(&self) -> RefMut<'_, IoQueueMgr> {
        RefMut::map(self.rimpl.borrow_mut(), |s| &mut s.ioq_mgr)
    }

    pub fn cred_mgr(&self) -> Ref<'_, CredentialMgr> {
        Ref::map(self.rimpl.borrow(), |s| &s.cred_mgr)
    }

    pub fn cred_mgr_mut(&self) -> RefMut<'_, CredentialMgr> {
        RefMut::map(self.rimpl.borrow_mut(), |s| &mut s.cred_mgr)
    }

    pub fn pin_policy_mgr(&self) -> Ref<'_, PinPolicyMgr<E>> {
        Ref::map(self.rimpl.borrow(), |s| &s.pin_policy_mgr)
    }

    pub fn pin_policy_mgr_mut(&self) -> RefMut<'_, PinPolicyMgr<E>> {
        RefMut::map(self.rimpl.borrow_mut(), |s| &mut s.pin_policy_mgr)
    }

    pub fn env(&self) -> Ref<'_, PartEnv<E>> {
        Ref::map(self.rimpl.borrow(), |s| &s.env)
    }

    pub fn vault(&self) -> KeyVault {
        KeyVault::new(self.env().vault_addr(), self.rgs().mask())
    }

    pub fn session_table(&self) -> SessionTable {
        let pfn_index: usize = self.rimpl.borrow().pfn.into();

        let session_table_base = self
            .env()
            .part_persistent_store_ref(pfn_index)
            .session_table
            .as_ptr() as usize;

        SessionTable::new(session_table_base)
    }

    pub fn part_persistent_store_ref(&self) -> &'static mut HsmPartPersistentStore {
        let pfn_index: usize = self.rimpl.borrow().pfn.into();
        self.env().part_persistent_store_ref(pfn_index)
    }

    pub fn set_vm_launch_guid(&self, guid: &VmLaunchGuid) {
        self.part_persistent_store_ref()
            .vm_launch_guid
            .copy_from_slice(guid);
    }

    /// Arm/disarm Gate 1 (`unwrapping_key_required`) for this PFN; CP-only writer, SP reads.
    pub fn set_unwrapping_key_required(&self, required: bool) {
        self.part_persistent_store_ref().unwrapping_key_required = required;
    }

    pub fn vm_launch_guid(&self) -> VmLaunchGuid {
        self.part_persistent_store_ref().vm_launch_guid
    }

    pub fn cdma_vault(&self) -> CdmaKeyVault {
        CdmaKeyVault::new(
            self.env().cdma_vault_addr(),
            self.rgs().mask(),
            self.env().cdma_vault_meta_data(),
        )
    }

    pub fn bks_table(&self) -> BksTable {
        #[cfg(feature = "mcr_test_hooks")]
        if let Some(ref table) = self.rimpl.borrow().bks_table {
            return table.clone();
        }
        BksTable::new(self.env().bks_table_addr())
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn set_bks_table(&self, table: BksTable) {
        self.rimpl.borrow_mut().bks_table = Some(table);
    }

    pub fn set_unwrapping_key_id(&self, key_id: Option<KeyId>) {
        self.rimpl.borrow_mut().unwrapping_key_id = key_id;
    }

    pub fn unwrapping_key_id(&self) -> Option<KeyId> {
        self.rimpl.borrow().unwrapping_key_id
    }

    pub fn set_establish_cred_encryption_key_id(&self, key_id: Option<KeyId>) {
        self.rimpl.borrow_mut().establish_cred_encryption_key_id = key_id;
    }

    pub fn get_establish_cred_encryption_key_id(&self) -> Option<KeyId> {
        self.rimpl.borrow().establish_cred_encryption_key_id
    }

    pub fn set_session_encryption_key_id(&self, key_id: Option<KeyId>) {
        self.rimpl.borrow_mut().session_encryption_key_id = key_id;
    }

    pub fn get_session_encryption_key_id(&self) -> Option<KeyId> {
        self.rimpl.borrow().session_encryption_key_id
    }

    pub fn get_cert_chain_lengths_info(&self) -> Option<GetCertChainLengthsInfo> {
        self.rimpl.borrow().cert_chain_len_info
    }

    pub fn set_cert_chain_lengths_info(&self, info: Option<GetCertChainLengthsInfo>) {
        self.rimpl.borrow_mut().cert_chain_len_info = info;
    }

    pub fn migrate(&self) {
        let partition_ref = self.part_persistent_store_ref();

        self.cred_mgr_mut().clear();
        self.rimpl.borrow_mut().unwrapping_key_id.take();
        partition_ref.bk3_session_key.is_valid = false;
        partition_ref.bk3_session_key.key.zeroize();
        partition_ref.pin_policy = PinPolicy::default();
        self.set_establish_cred_encryption_key_id(None);
        self.set_session_encryption_key_id(None);
        self.reset_nonce();
        self.session_table().restore(self.session_table().backup());
        self.vault().clear();
        self.cdma_vault().clear();
        self.set_partition_mk_id(None);
        #[cfg(feature = "mcr_test_hooks")]
        {
            self.rimpl.borrow_mut().bks_table = None;
            self.rimpl.borrow_mut().test_hook_to_trigger_level2_abort = false;
            self.rimpl.borrow_mut().cmd_fsm_test_action = None;
            self.rimpl.borrow_mut().hsm_fsm_test_action = None;
        }
    }

    pub fn clear_partition_info(&self) {
        let partition_ref = self.part_persistent_store_ref();

        partition_ref.vm_launch_guid = VmLaunchGuid::default();
        partition_ref.partition_id_valid = false;
        partition_ref.partition_cert_valid = false;
        partition_ref.partition_cert.length = 0;
        partition_ref.partition_cert.data.zeroize();
        partition_ref.partition_identifier.id.zeroize();
        partition_ref.partition_identifier.priv_key.zeroize();
        partition_ref.partition_identifier.pub_key.zeroize();
        partition_ref.unwrapping_key_required = false;
        partition_ref.unwrapping_key_bk.zeroize();
        partition_ref.unwrapping_key_bk_valid = UnwrappingKeyValidity::Empty as u8;
        partition_ref.masked_bk_boot.len = 0;
        partition_ref.masked_bk_boot.data.zeroize();
        partition_ref.sealed_bk3.len = 0;
        partition_ref.sealed_bk3.data.zeroize();
        partition_ref.bk3_session_key.is_valid = false;
        partition_ref.bk3_session_key.key.zeroize();

        self.set_cert_chain_lengths_info(None);
        self.session_table().restore(0);

        self.set_partition_mk_id(None);
    }

    pub fn store_date(&self) {
        self.rimpl.borrow().store_data()
    }

    pub fn is_fips_approved(&self) -> bool {
        self.rimpl.borrow().is_fips_approved()
    }

    pub fn clear_partition_provisioning_state(&self) {
        self.rimpl.borrow_mut().clear_partition_provisioning_state()
    }

    pub fn partition_id_valid(&self) -> bool {
        self.part_persistent_store_ref().partition_id_valid
    }

    pub fn set_partition_id_valid(&self, valid: bool) {
        self.part_persistent_store_ref().partition_id_valid = valid;
    }

    pub fn partition_id(&self) -> HsmPartitionId {
        self.part_persistent_store_ref().partition_identifier.id
    }

    pub fn set_partition_id(&self, id: &HsmPartitionId) {
        self.part_persistent_store_ref()
            .partition_identifier
            .id
            .copy_from_slice(id);
    }

    pub fn partition_id_priv_key(&self) -> &[u8; EccCurve::P384 as usize] {
        &self
            .part_persistent_store_ref()
            .partition_identifier
            .priv_key
    }

    pub fn set_partition_id_priv_key(&self, key: &[u8]) {
        self.part_persistent_store_ref()
            .partition_identifier
            .priv_key[0..48]
            .copy_from_slice(key);
    }

    pub fn partition_id_pub_key(&self) -> &[u8; 97] {
        &self
            .part_persistent_store_ref()
            .partition_identifier
            .pub_key
    }

    pub fn set_partition_id_pub_key(&self, key: &[u8]) {
        self.part_persistent_store_ref()
            .partition_identifier
            .pub_key[0] = 0x4; // uncompressed point
        self.part_persistent_store_ref()
            .partition_identifier
            .pub_key[1..48 * 2 + 1]
            .copy_from_slice(key);
    }

    pub fn is_partition_cert_valid(&self) -> bool {
        self.part_persistent_store_ref().partition_cert_valid
    }

    pub fn set_partition_cert_valid(&self, valid: bool) {
        self.part_persistent_store_ref().partition_cert_valid = valid;
    }

    pub fn get_partition_cert(&self) -> IoMemRange {
        let cert_data = &self.part_persistent_store_ref().partition_cert.data
            [0..self.part_persistent_store_ref().partition_cert.length as usize];

        IoMemRange::from(cert_data)
    }

    pub fn get_partition_cert_length(&self) -> u32 {
        self.part_persistent_store_ref().partition_cert.length
    }

    pub fn set_partition_cert_length(&self, length: u32) -> HsmResult<()> {
        if length > MAX_PART_CERT_LENGTH {
            return Err(HsmErr::PartitionCertTooLarge);
        }
        self.part_persistent_store_ref().partition_cert.length = length;

        Ok(())
    }

    pub fn get_masked_bk_boot_len(&self) -> u32 {
        self.part_persistent_store_ref().masked_bk_boot.len
    }

    pub fn set_masked_bk_boot_len(&self, len: u32) {
        self.part_persistent_store_ref().masked_bk_boot.len = len;
    }

    pub fn masked_bk_boot(&self) -> IoMemRange {
        let masked_bk_boot =
            &self.part_persistent_store_ref().masked_bk_boot.data[..MASKED_BK_BOOT_SIZE];

        IoMemRange::from(masked_bk_boot)
    }

    pub fn get_sealed_bk3_len(&self) -> u32 {
        self.part_persistent_store_ref().sealed_bk3.len
    }

    pub fn set_sealed_bk3_len(&self, len: u32) {
        self.part_persistent_store_ref().sealed_bk3.len = len;
    }

    pub fn sealed_bk3(&self) -> IoMemRange {
        let sealed_bk3 = &self.part_persistent_store_ref().sealed_bk3.data[..SEALED_BK3_SIZE];

        IoMemRange::from(sealed_bk3)
    }

    pub fn set_partition_mk_id(&self, key_id: Option<KeyId>) {
        self.rimpl.borrow_mut().masking_key = key_id;
    }

    pub fn get_partition_mk_id(&self) -> Option<KeyId> {
        self.rimpl.borrow().masking_key
    }

    pub fn is_partition_provisioned(&self) -> bool {
        self.rimpl.borrow().is_partition_provisioned()
    }

    pub fn get_bk3_session(&self) -> Option<&[u8; BK3_SIZE_BYTES]> {
        if self.part_persistent_store_ref().bk3_session_key.is_valid {
            return Some(&self.part_persistent_store_ref().bk3_session_key.key);
        }
        None
    }

    pub fn set_bk3_session(&self, bk3_session: SecureByteArray<BK3_SIZE_BYTES>) {
        self.part_persistent_store_ref()
            .bk3_session_key
            .key
            .copy_from_slice(bk3_session.as_slice());
        self.part_persistent_store_ref().bk3_session_key.is_valid = true;
    }

    #[cfg(feature = "fips_validation_hooks")]
    pub fn toggle_fips_approved_state(&self) {
        self.rimpl.borrow().toggle_fips_approved_state();
    }

    #[cfg(feature = "fips_validation_hooks")]
    pub fn clear_bk3_info(&self) {
        let partition_ref = self.part_persistent_store_ref();

        partition_ref.masked_bk_boot.len = 0;
        partition_ref.masked_bk_boot.data.zeroize();
        partition_ref.sealed_bk3.len = 0;
        partition_ref.sealed_bk3.data.zeroize();
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn set_test_hook_to_trigger_level2_abort(&self, level2_trigger: bool) {
        self.rimpl.borrow_mut().test_hook_to_trigger_level2_abort = level2_trigger;
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn test_hook_to_trigger_level2_abort(&self) -> bool {
        self.rimpl.borrow_mut().test_hook_to_trigger_level2_abort
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn cmd_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.rimpl.borrow_mut().cmd_fsm_test_action(test_action)
    }

    #[cfg(feature = "mcr_test_hooks")]
    pub fn hsm_fsm_test_action(&self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        self.rimpl.borrow_mut().hsm_fsm_test_action(test_action)
    }

    #[cfg(feature = "fips_validation_hooks")]
    pub fn inject_rng_hw_failure(&self, rng_hw_self_test_id: u32) {
        self.rimpl
            .borrow_mut()
            .inject_rng_hw_failure(rng_hw_self_test_id);
    }

    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    pub fn neg_pct_skip_cnt(&self, cnt: Option<u8>) -> Option<u8> {
        self.rimpl.borrow_mut().neg_pct_skip_cnt(cnt)
    }
}

pub(super) struct PartStateImpl<E: HsmEnvTrait + 'static> {
    /// Pcie Function ID
    pfn: PcieFunction,

    /// Environment
    env: PartEnv<E>,

    /// Flag indicating if the function is enabled
    enabled: bool,

    /// Resource mask for the function
    rgs: ResourceGroups,

    /// IO queue manager
    ioq_mgr: IoQueueMgr,

    /// Credential manager
    cred_mgr: CredentialMgr,

    /// Key id of the unwrapping key
    unwrapping_key_id: Option<KeyId>,

    /// Key id of the establish cred encryption key
    establish_cred_encryption_key_id: Option<KeyId>,

    /// Key id of the session encryption key
    session_encryption_key_id: Option<KeyId>,

    /// Certifcate chain length context
    cert_chain_len_info: Option<GetCertChainLengthsInfo>,

    /// Pin policy manager
    pin_policy_mgr: PinPolicyMgr<E>,

    /// Masking key
    masking_key: Option<KeyId>,

    /// Test hooks to have a local copy of the bks table so that we can manipulate it for testing purposes.
    #[cfg(feature = "mcr_test_hooks")]
    bks_table: Option<BksTable>,

    /// Test hook to trigger Level-2 abort during queue deletion
    #[cfg(feature = "mcr_test_hooks")]
    test_hook_to_trigger_level2_abort: bool,

    /// Test hooks to test IO rollback feature for Cmd Fsm
    #[cfg(feature = "mcr_test_hooks")]
    cmd_fsm_test_action: Option<DdiTestAction>,

    /// Test hooks to test IO roolback feature for Hsm Fsm
    #[cfg(feature = "mcr_test_hooks")]
    hsm_fsm_test_action: Option<DdiTestAction>,

    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    neg_pct_skip_cnt: Option<u8>,
}

impl<E: HsmEnvTrait> PartStateImpl<E> {
    fn new(pfn: PcieFunction, env: PartEnv<E>) -> Self {
        let pin_policy_mgr = PinPolicyMgr::new(env.clone(), pfn);

        Self {
            pfn,
            env,
            enabled: false,
            rgs: ResourceGroups::default(),
            ioq_mgr: IoQueueMgr::default(),
            cred_mgr: CredentialMgr::default(),
            unwrapping_key_id: None,
            establish_cred_encryption_key_id: None,
            session_encryption_key_id: None,
            cert_chain_len_info: None,
            pin_policy_mgr,
            masking_key: None,
            #[cfg(feature = "mcr_test_hooks")]
            bks_table: None,
            #[cfg(feature = "mcr_test_hooks")]
            test_hook_to_trigger_level2_abort: false,
            #[cfg(feature = "mcr_test_hooks")]
            cmd_fsm_test_action: None,
            #[cfg(feature = "mcr_test_hooks")]
            hsm_fsm_test_action: None,
            #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
            neg_pct_skip_cnt: None,
        }
    }

    fn reset_nonce(&mut self) {
        let pfn_index: usize = self.pfn.into();
        self.env
            .rng()
            .bytes(&mut self.env.part_persistent_store_ref(pfn_index).nonce);
    }

    fn verify_user_cred_is_set(&self) -> bool {
        self.cred_mgr.verify_user_cred_is_set()
    }

    fn change_user_cred(&mut self, id: &[u8], pin: &[u8]) -> HsmResult<()> {
        self.cred_mgr.change_user_cred(id, pin)
    }

    fn restore_cred_mgr(&mut self) {
        let data_store_addr = self.env.data_store_addr();
        let data_store = unsafe { &*(data_store_addr as *const HsmPartDataStore) };
        let index: usize = self.pfn.into();
        let part_store = &data_store.part[index];

        self.cred_mgr.restore_user_cred(part_store.user_cred);
    }

    fn restore_res_mask(&mut self) {
        const INIT: u128 = 0;
        let resource = self.env.resource_table();

        let mask = resource.iter().enumerate().fold(INIT, |m, (i, r)| {
            if r.pfn == Some(self.pfn) {
                m | 1 << i
            } else {
                m
            }
        });

        self.rgs.set_mask(mask);

        // Re-arm Gate 1 on warm/IDFU boot since the persisted mask has no replayed `SetRes` IPC.
        if mask != 0 {
            let pfn_index: usize = self.pfn.into();
            self.env
                .part_persistent_store_ref(pfn_index)
                .unwrapping_key_required = true;
        }
    }

    fn restore_io_queues(&mut self) {
        if self.enabled {
            let resource_table = self.env.resource_table();
            let offset = HostSqId::Id1.offset();

            // Recover the state of the queues from PCIe resource table in memory
            for resource in resource_table.iter() {
                if Some(self.pfn) == resource.pfn && resource.alloc_map.bit(offset) {
                    // Queue ID
                    let queue_id = HSM_IO_QUEUE_BASE as u8 + resource.id;

                    self.ioq_mgr
                        .enable_io_queue(DevSqId(queue_id), DevCqId(queue_id));
                }
            }
        }
    }

    fn store_data(&self) {
        let pfn_index: usize = self.pfn.into();
        let data_store_addr = self.env.data_store_addr();
        let data_store = unsafe { &mut *(data_store_addr as *mut HsmPartDataStore) };
        let part_store = &mut data_store.part[pfn_index];

        part_store.enabled = self.enabled;
        part_store.unwrapping_key_id = self.unwrapping_key_id;
        part_store.establish_cred_encryption_key_id = self.establish_cred_encryption_key_id;
        part_store.masking_key = self.masking_key;
        part_store.session_encryption_key_id = self.session_encryption_key_id;
        part_store.user_cred = self.cred_mgr.user_cred();
    }

    fn is_fips_approved(&self) -> bool {
        self.env.is_fips_approved(self.pfn)
    }

    fn is_partition_provisioned(&self) -> bool {
        self.masking_key.is_some()
    }

    fn clear_partition_provisioning_state(&mut self) {
        self.masking_key = None;
    }

    #[cfg(feature = "fips_validation_hooks")]
    pub fn toggle_fips_approved_state(&self) {
        self.env.toggle_fips_approved_state(self.pfn);
    }

    #[cfg(all(feature = "mcr_test_hooks", feature = "fips_validation_hooks"))]
    fn neg_pct_skip_cnt(&mut self, cnt: Option<u8>) -> Option<u8> {
        // Take the test action if the incoming argument is `None``
        // Else, set the test action and retuen the same
        if cnt == None {
            self.neg_pct_skip_cnt.take()
        } else {
            self.neg_pct_skip_cnt = cnt;

            self.neg_pct_skip_cnt
        }
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn cmd_fsm_test_action(&mut self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        // Take the test action if the incoming argument is `None``
        // Else, set the test action and retuen the same
        if test_action == None {
            self.cmd_fsm_test_action.take()
        } else {
            self.cmd_fsm_test_action = test_action;

            self.cmd_fsm_test_action
        }
    }

    #[cfg(feature = "mcr_test_hooks")]
    fn hsm_fsm_test_action(&mut self, test_action: Option<DdiTestAction>) -> Option<DdiTestAction> {
        // Take the test action if the incoming argument is `None``
        // Else, set the test action and retuen the same
        if test_action == None {
            self.hsm_fsm_test_action.take()
        } else {
            self.hsm_fsm_test_action = test_action;

            self.hsm_fsm_test_action
        }
    }

    #[cfg(feature = "fips_validation_hooks")]
    pub fn inject_rng_hw_failure(&mut self, rng_hw_self_test_id: u32) {
        self.env.rng().inject_rng_hw_failure(rng_hw_self_test_id);
    }
}
