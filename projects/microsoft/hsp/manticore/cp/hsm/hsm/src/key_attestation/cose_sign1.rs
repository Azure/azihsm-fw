// Copyright (c) Microsoft Corporation. All rights reserved.

//! Support COSE_Sign1 object creation based on <https://www.rfc-editor.org/rfc/rfc9052>.

use alloc::vec::Vec;

use crate::error::HsmErr;
use crate::error::HsmResult;
use crate::key_attestation::report::*;

///  Create the payload.
///
/// # Arguments
/// * `public_key` - The encoded public key using `CoseKey`.
/// * `public_key_size` - The size of the encoded public key.
/// * `flags` - The flags associated with the key.
/// * `app_uuid` - The uuid of the vault application session.
/// * `report_data` - Customized data to be included in the report.
/// * `vm_launch_id` - The launch uuid with which the VM was launched.
/// * `encoded_buffer` - The buffer to hold the encoded payload.
///
/// # Returns
/// * `([u8; REPORT_SIZE], usize)` - The payload buffer and the size of the payload.
///
/// # Errors
/// * `HsmErr::AttestationReportEncodeFailed` - If CBOR encoding fails during creation.
#[allow(clippy::too_many_arguments)]
pub fn create_payload(
    version: u16,
    public_key: &[u8; PUBLIC_KEY_MAX_SIZE],
    public_key_size: u16,
    flags: u32,
    app_uuid: [u8; 16],
    report_data: &[u8; REPORT_DATA_SIZE],
    vm_launch_id: &[u8; VM_LAUNCH_ID_SIZE],
    encoded_buffer: &mut Vec<u8>,
) -> HsmResult<usize> {
    let report = KeyAttestationReportPayload {
        version,
        public_key: *public_key,
        public_key_size,
        flags,
        app_uuid,
        report_data: *report_data,
        vm_launch_id: *vm_launch_id,
    };
    encoded_buffer.resize(minicbor::len(&report), 0);

    let size = report
        .encode(encoded_buffer)
        .map_err(|_| HsmErr::AttestationReportEncodeFailed)?;

    Ok(size)
}

/// Create the to-be-signed buffer based on Section 4.4, <https://www.rfc-editor.org/rfc/rfc9052>.
///
/// # Arguments
/// * `body_protected` - The `body_protected` parameter of the `Sig_structure`.
/// * `payload` - The `payload` parameter of the `Sig_structure`.
///
/// # Returns
/// * `([u8; SIG_STRUCTURE_MAX_SIZE], usize)` - The payload buffer and the size of the payload.
///
/// # Errors
/// * `HsmErr::AttestationReportEncodeFailed` - If CBOR encoding fails during creation.
pub fn create_tbs(
    body_protected: &[u8],
    payload: &[u8],
) -> HsmResult<([u8; SIG_STRUCTURE_MAX_SIZE], usize)> {
    let mut sig_struct_buffer = [0u8; SIG_STRUCTURE_MAX_SIZE];

    let sig_struct_size = encode_sig_struct(body_protected, payload, &mut sig_struct_buffer)
        .map_err(|_| HsmErr::AttestationReportEncodeFailed)?;

    Ok((sig_struct_buffer, sig_struct_size))
}
