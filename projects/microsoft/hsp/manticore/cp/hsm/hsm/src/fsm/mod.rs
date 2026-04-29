// Copyright (c) Microsoft Corporation. All rights reserved.

mod aes_enc_dec;
mod aes_gen_key;
mod attest_key;
mod change_pin;
mod close_session;
mod combo_fsm;
mod delete_key;
#[cfg(feature = "mcr_test_hooks")]
mod der_key_import;
mod ecc_gen_key;
mod ecc_sign;
mod ecdh_key_exchange;
mod establish_credential;
mod flush_session;
mod get_api_rev;
mod get_cert_chain_info;
mod get_certificate;
mod get_device_info;
mod get_establish_cred_encryption_key;
#[cfg(feature = "fips_validation_hooks")]
mod get_priv_key;
#[cfg(feature = "fips_validation_hooks")]
mod get_rng;
mod get_sealed_bk3;
mod get_session_encryption_key;
mod get_unwrapping_key;
mod hkdf_derive;
mod hmac;
mod hsm_fsm;
mod init_bk3;
mod kbkdf_derive;
mod open_key;
mod open_session;
mod part_init;
#[cfg(feature = "fips_validation_hooks")]
mod raw_key_import;
mod res_cleanup_fsm;
mod rsa_mod_exp;
mod rsa_unwrap;
#[cfg(feature = "fips_validation_hooks")]
mod rsa_unwrap_kek;
mod set_sealed_bk3;
#[cfg(feature = "fips_validation_hooks")]
mod sha;
#[cfg(feature = "fips_validation_hooks")]
mod soft_aes;
mod test_action;
mod unmask_key;
mod unsupported;

#[cfg(test)]
cfg_if::cfg_if! {
    if #[cfg(test)] {
        mod tests;
    }
}

use bitfield_struct::bitfield;
pub(crate) use combo_fsm::ComboFsm;
pub(crate) use hsm_fsm::HsmFsm;
use mcr_ddi_mbor::MborByteArray;
use mcr_ddi_mbor::MborEncoder;
use mcr_ddi_mbor::MborLenAccumulator;
use mcr_ddi_types::*;
use mcr_io_controller::*;
use mcr_types::*;
use open_enum::open_enum;
pub(crate) use part_init::HsmPartInitFsm;
pub(crate) use res_cleanup_fsm::HsmResCleanupFsm;
use zerocopy::*;

use self::aes_enc_dec::AesEncDecCmd;
use self::aes_gen_key::AesGenKeyCmd;
use self::attest_key::AttestKeyCmd;
use self::change_pin::ChangePinCmd;
use self::close_session::CloseSessionCmd;
use self::delete_key::DeleteKeyCmd;
#[cfg(feature = "mcr_test_hooks")]
use self::der_key_import::DerKeyImportCmd;
use self::ecc_gen_key::EccGenKeyCmd;
use self::ecc_sign::EccSignCmd;
use self::ecdh_key_exchange::EcdhKeyExchangeCmd;
use self::establish_credential::EstablishCredentialCmd;
use self::flush_session::FlushSessionCmd;
use self::get_api_rev::GetApiRevCmd;
use self::get_cert_chain_info::GetCertChainInfoCmd;
use self::get_certificate::GetCertificateCmd;
use self::get_device_info::GetDeviceInfoCmd;
use self::get_establish_cred_encryption_key::GetEstablishCredEncryptionKeyCmd;
#[cfg(feature = "fips_validation_hooks")]
use self::get_priv_key::GetPrivKeyCmd;
#[cfg(feature = "fips_validation_hooks")]
use self::get_rng::GetRngCmd;
use self::get_sealed_bk3::GetSealedBk3Cmd;
use self::get_session_encryption_key::GetSessionEncryptionKeyCmd;
use self::get_unwrapping_key::GetUnwrappingKeyCmd;
use self::hkdf_derive::HkdfDeriveCmd;
use self::hmac::HmacCmd;
use self::init_bk3::InitBk3Cmd;
use self::kbkdf_derive::KbkdfDeriveCmd;
use self::open_key::OpenKeyCmd;
use self::open_session::OpenSessionCmd;
#[cfg(feature = "fips_validation_hooks")]
use self::raw_key_import::RawKeyImportCmd;
use self::rsa_mod_exp::RsaModExpCmd;
use self::rsa_unwrap::RsaUnwrapCmd;
#[cfg(feature = "fips_validation_hooks")]
use self::rsa_unwrap_kek::RsaUnwrapKekTestCmd;
use self::set_sealed_bk3::SetSealedBk3Cmd;
#[cfg(feature = "fips_validation_hooks")]
use self::sha::ShaDigestCmd;
#[cfg(feature = "fips_validation_hooks")]
use self::soft_aes::SoftAesCmd;
#[cfg(any(
    feature = "mcr_test_hooks",
    feature = "mcr_manual_test_hooks",
    feature = "fips_validation_hooks"
))]
use self::test_action::TestActionCmd;
use self::unmask_key::UnmaskKeyCmd;
use self::unsupported::UnsupportedCmd;
use crate::cmd_scheduler::*;
use crate::env::HsmHalTrait;
use crate::error::HsmResult;
use crate::heap::*;
use crate::partition::*;
use crate::*;
pub(crate) type DmaHeap<E> = <<E as HsmEnvTrait>::Hal as HsmHalTrait>::DmaHeap;
pub(crate) type DmaBuffer<E> =
    <<<E as HsmEnvTrait>::Hal as HsmHalTrait>::DmaHeap as HsmDmaHeapTrait>::Alloc;
pub(crate) type ResId = HsmFsmResourceId;
pub(crate) type HsmPartitionEnv<E> =
    <<E as env::HsmEnvTrait>::Partition as partition::HsmPartition>::Env;

/// HSM Submission queue entry command opcode
#[repr(u16)]
#[open_enum]
#[derive(Debug)]
enum HsmSqeCmdOpcode {
    /// Generic command
    Generic = 0,

    /// Flush command
    Flush = 1,

    /// Unknown command
    Unknown = 0x3FF,
}

impl From<u32> for HsmSqeCmdOpcode {
    /// Converts to this type from the input type.
    fn from(value: u32) -> Self {
        match value {
            x if x == HsmSqeCmdOpcode::Generic.into() => HsmSqeCmdOpcode::Generic,
            x if x == HsmSqeCmdOpcode::Flush.into() => HsmSqeCmdOpcode::Flush,
            _ => HsmSqeCmdOpcode::Unknown,
        }
    }
}

impl From<HsmSqeCmdOpcode> for u32 {
    /// Converts to this type from the input type.
    fn from(value: HsmSqeCmdOpcode) -> Self {
        value.0 as u32
    }
}

/// HSM submission queue entry command
#[bitfield(u32)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct HsmSqeCmd {
    /// Command Opcode
    #[bits(10)]
    op: HsmSqeCmdOpcode,

    /// Command Set
    #[bits(4)]
    set: u32,

    /// PRP or SGL
    #[bits(2)]
    psdt: u32,

    /// Command Id
    #[bits(16)]
    id: u16,
}

/// HSM submission queue entry DMA descriptor
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct HsmSqeDmaDesc {
    /// Length of the DMA buffer
    len: u32,

    /// First page
    prp1: MemoryAddr,

    /// Second page or page list
    prp2: MemoryAddr,
}

/// Enumeration for different types of opcodes
#[repr(u8)]
#[derive(Debug, PartialEq)]
pub enum HsmSessionControlKind {
    /// This kind of opcode means that there is no
    /// session id associated with this opcode
    NoSession,

    /// This kind of opcode indicates that a session
    /// is being opened
    Open,

    /// Kind used to indicate opcodes for closing a
    /// session
    Close,

    /// Kind used to indicate opcodes that are part of
    /// a session
    InSession,
}

impl From<HsmSessionControlKind> for u8 {
    fn from(kind: HsmSessionControlKind) -> u8 {
        match kind {
            HsmSessionControlKind::NoSession => 0,
            HsmSessionControlKind::Open => 1,
            HsmSessionControlKind::Close => 2,
            HsmSessionControlKind::InSession => 3,
        }
    }
}

impl From<u8> for HsmSessionControlKind {
    fn from(kind: u8) -> HsmSessionControlKind {
        match kind {
            0 => HsmSessionControlKind::NoSession,
            1 => HsmSessionControlKind::Open,
            2 => HsmSessionControlKind::Close,
            _ => HsmSessionControlKind::InSession,
        }
    }
}

impl From<DdiOp> for HsmSessionControlKind {
    fn from(e: DdiOp) -> Self {
        match e {
            DdiOp::GetApiRev
            | DdiOp::GetDeviceInfo
            | DdiOp::GetEstablishCredEncryptionKey
            | DdiOp::EstablishCredential
            | DdiOp::GetSessionEncryptionKey
            | DdiOp::InitBk3
            | DdiOp::GetSealedBk3
            | DdiOp::SetSealedBk3
            | DdiOp::GetCertChainInfo
            | DdiOp::GetCertificate => HsmSessionControlKind::NoSession,

            DdiOp::OpenSession => HsmSessionControlKind::Open,

            DdiOp::CloseSession | DdiOp::ResetFunction => HsmSessionControlKind::Close,

            _ => HsmSessionControlKind::InSession,
        }
    }
}

/// HSM submission queue entry session flags
#[bitfield(u8)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct HsmSessionFlags {
    /// Command Opcode
    #[bits(2)]
    ctrl: HsmSessionControlKind,

    /// Flag indicating session is valid or not
    #[bits(1)]
    id_valid: bool,

    /// Flag indicating App vault id is valid or not
    #[bits(1)]
    app_vault_id_is_valid: bool,

    /// Flag indicating session was successfully closed
    #[bits(1)]
    session_closed: bool,

    /// Reserved
    #[bits(3)]
    _rsvd: u16,
}

#[bitfield(u16)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct PsfField {
    /// Phase
    phase: bool,

    /// Status
    #[bits(11)]
    status: HostStatusCode,

    /// Reserved
    #[bits(4)]
    _rsvd: u8,
}

/// HSM submission queue entry
#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct HsmSqe {
    /// Command
    cmd: HsmSqeCmd,

    /// Source DMA descriptor
    src: HsmSqeDmaDesc,

    /// Destination DMA descriptor
    dst: HsmSqeDmaDesc,

    /// Session Flags
    session_flags: HsmSessionFlags,

    /// Reserved
    _rsvd1: [u8; 3],

    /// Session ID
    session_id: u16,

    /// Reserved
    _rsvd2: [u8; 14],
}
static_assertions::assert_eq_size!(HsmSqe, IoRxEntry);

impl From<IoRxEntry> for HsmSqe {
    /// Converts to this type from the input type.
    fn from(value: IoRxEntry) -> Self {
        HsmSqe::read_from_bytes(&value[..]).unwrap()
    }
}

impl HsmSqe {
    /// Get source buffer length
    pub fn src_len(&self) -> usize {
        self.src.len as usize
    }

    /// Get destination buffer length
    pub fn dst_len(&self) -> usize {
        self.dst.len as usize
    }
}

#[repr(C)]
#[derive(Default, IntoBytes, Immutable, FromBytes)]
pub(crate) struct HsmCqe {
    /// Length of data copied in destination buffer
    pub dst_len: u16,

    /// Session flags
    pub session_flags: HsmSessionFlags,

    /// Reserved
    _rsvd1: u8,

    /// Session id
    pub session_id: u16,

    /// App vault id
    pub app_vault_id: u8,

    /// Reserved
    _rsvd2: u8,

    /// Submission Queue Head Pointer
    pub sq_head: u16,

    /// Submission Queue ID
    pub sq_id: u16,

    /// Command ID
    pub cmd_id: u16,

    /// Phase status flags
    pub psf: PsfField,
}
static_assertions::assert_eq_size!(HsmCqe, IoTxEntry);

impl From<HsmCqe> for IoTxEntry {
    /// Converts to this type from the input type.
    fn from(value: HsmCqe) -> Self {
        let mut entry: Self = [0u8; IO_TX_ENTRY_SIZE];
        entry.clone_from_slice(value.as_bytes());
        entry
    }
}

pub(crate) trait HsmCmdTrait<E: HsmEnvTrait> {
    /// Take the response buffer if it is available
    ///
    /// # Returns
    ///
    /// * `Option<DmaBuffer<E>>` - DMA buffer containing the response if it
    ///   is available otherwise None
    ///
    /// # Behavior
    ///
    /// This function is called by the HSM FSM to get the response buffer once
    /// the command FSM `on_event`` interface returned Ok(()) or Err(HsmErr)
    /// If Err(HsmErr::Pending) is returned, the command FSM is still processing
    /// the command and has not prepared the response buffer.
    fn take_response(&mut self) -> Option<DmaBuffer<E>>;

    /// Handle an event
    ///
    /// # Arguments
    ///
    /// * `event` - Event to handle
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * `Result<(), HsmErr>` - Result of handling the event
    fn on_event(&mut self, event: HsmFsmEvent, tag: TagId) -> Result<(), HsmErr>;

    /// Acquire a resource
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `res_id` - Resource ID
    ///
    /// # Returns
    ///
    /// * `AdminFsmEvent` - Event to wake the state machine with
    #[allow(unused_variables)]
    fn acquire_resource(&mut self, tag: TagId, res_id: ResId) -> HsmFsmEvent {
        unimplemented!()
    }

    /// Check if the command requires resource
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    /// * `res_id` - Resource ID
    ///
    /// # Returns
    ///
    /// * `bool` - True if the command requires resource
    #[allow(unused_variables)]
    fn requires_resource(&self, tag: TagId, res_id: ResId) -> bool {
        false
    }

    /// Get the session ID this command FSM operates on-behalf of
    ///
    /// # Returns
    ///
    /// * `Option<u16>` - Session ID
    fn session_id(&self) -> Option<u16> {
        None
    }

    /// Get the app vault ID this command FSM operates on-behalf of
    ///
    /// # Returns
    ///
    /// * `Option<u8>` - App Vault ID
    fn app_vault_id(&self) -> Option<u8> {
        None
    }

    /// Perform any rollback in case of error
    ///
    /// # Arguments
    ///
    /// * `tag` - Tag ID
    ///
    /// # Returns
    ///
    /// * Returns `Ok(())` if successful else `Err(HsmError)` if failed
    fn rollback(&mut self, _tag: TagId) -> HsmResult<()> {
        Ok(())
    }

    /// Check if the command needs to be retried
    fn retry(&self) -> bool {
        false
    }
}

/// Encode DMA buffer with command
///
/// # Arguments
///
/// * `cmd` - Command to encode
/// * `ctx` - FSM context
///
/// # Returns
///
/// * `Result<DmaBuffer<E>, HsmErr>` - Result of encoding the command
fn encode_buf<T, D>(cmd: &T, heap: &D) -> Result<D::Alloc, HsmErr>
where
    T: mcr_ddi_mbor::MborEncode + mcr_ddi_mbor::MborLen,
    D: HsmDmaHeapTrait,
{
    let mut acc = MborLenAccumulator::default();
    cmd.mbor_len(&mut acc);
    let len = acc.len();
    let mut buf = heap
        .allocate_from_pool(len)
        .ok_or(HsmErr::DmaAllocFailure)?;
    let mut encoder = MborEncoder::new(buf.as_ref_mut());
    cmd.mbor_encode(&mut encoder)
        .or(Err(HsmErr::DdiEncodeFailed))?;
    Ok(buf)
}

/// Decode command from DMA buffer
///
/// # Arguments
///
/// * `buf` - DMA buffer containing the command
///
/// # Returns
///
/// * `Result<T, HsmErr>` - Result of decoding the command
fn decode_buf<T, E>(buf: &DmaBuffer<E>) -> Result<T, HsmErr>
where
    T: for<'a> mcr_ddi_mbor::MborDecode<'a>,
    E: HsmEnvTrait,
{
    let mut decoder = mcr_ddi_mbor::MborDecoder::new(buf.as_ref());
    T::mbor_decode(&mut decoder).or(Err(HsmErr::DdiDecodeFailed))
}
