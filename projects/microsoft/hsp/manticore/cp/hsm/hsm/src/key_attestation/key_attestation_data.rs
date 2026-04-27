// Copyright (c) Microsoft Corporation. All rights reserved.

//! The module for `KeyAttestationData` that includes necessary data for
//! creating an attestation report.

use alloc::vec::Vec;

use crate::error::HsmErr;
use crate::error::HsmResult;
use crate::key_attestation::report::*;

/// Include the necessary data for creating an attestation report in `CoseSign1Object`.
pub struct KeyAttestationData {
    /// ProtectedHeader of a COSE_Sign1 object
    pub protected_header: [u8; PROTECTED_HEADER_SIZE],
    /// Unprotected header of a COSE_Sign1 object
    pub unprotected_header: UnprotectedHeader,
    /// Payload of a COSE_Sign1 object
    pub payload: Vec<u8>,
    /// Length of the payload
    pub payload_len: usize,
    /// Indicate whether if data is available for signing
    pub ready_to_sign: bool,
}

impl KeyAttestationData {
    ///  Create and initialize a `KeyAttestationData` instance.
    ///
    /// # Returns
    /// * `KeyAttestationData` - An initialized `KeyAttestationData` instance.
    pub fn new() -> Self {
        let unprotected_header = UnprotectedHeader {};

        Self {
            protected_header: PROTECTED_HEADER,
            unprotected_header,
            payload: Vec::new(),
            payload_len: 0,
            ready_to_sign: false,
        }
    }

    ///  Create the report payload.
    ///
    /// # Arguments
    /// * `public_key` - The encoded public key using `CoseKey`.
    /// * `public_key_len` - The length of the encoded public key.
    /// * `flags` - The flags associated with the key.
    /// * `app_uuid` - The uuid of the vault application session.
    /// * `report_data` - Customized data to be included in the report payload.
    /// * `vm_launch_id` - The launch uuid with which the VM was launched.
    ///
    /// # Returns
    /// * `()` - If the creation succeeds.
    ///
    /// # Errors
    /// * `HsmErr::AttestationReportEncodeFailed` - If CBOR encoding fails during creation.
    pub fn create_report_payload(
        &mut self,
        public_key: &[u8; PUBLIC_KEY_MAX_SIZE],
        public_key_len: u16,
        flags: KeyFlags,
        app_uuid: [u8; 16],
        report_data: &[u8; REPORT_DATA_SIZE],
        vm_launch_id: &[u8; VM_LAUNCH_ID_SIZE],
    ) -> HsmResult<()> {
        self.payload_len = crate::key_attestation::cose_sign1::create_payload(
            REPORT_VERSION,
            public_key,
            public_key_len,
            flags.into(),
            app_uuid,
            report_data,
            vm_launch_id,
            &mut self.payload,
        )?;

        self.ready_to_sign = true;

        Ok(())
    }

    /// Encode the data into an attestation report (i.e., a COSE_Sign1 object).
    ///
    /// # Arguments
    /// * `signature` - ES384 signature.
    /// * `quote_buffer` - slice to output encoded data to
    ///
    /// # Returns
    /// * `([u8; COSE_SIGN1_OBJECT_MAX_SIZE], usize)` - The CoseSign1 object data and the length of the data.
    ///
    /// # Errors
    /// * `HsmErr::AttestationReportEncodeFailed` - If CBOR encoding fails.
    pub fn encode(
        &self,
        signature: [u8; SIGNATURE_SIZE],
        quote_buffer: &mut [u8],
    ) -> HsmResult<()> {
        // Add an untagged COSE_Sign1 object after the tag
        let cose_sign1_object = CoseSign1Object {
            protected_header: self.protected_header,
            unprotected_header: self.unprotected_header,
            payload: &self.payload[..self.payload_len],
            signature,
        };

        let cose_sign1_object_buffer_len = cose_sign1_object
            .encode(quote_buffer)
            .map_err(|_| HsmErr::AttestationReportEncodeFailed)?;
        if cose_sign1_object_buffer_len != quote_buffer.len() {
            Err(HsmErr::AttestKeyInternalErr)?
        }

        Ok(())
    }

    /// Return expected length of quote_buffer output by encode()
    ///
    /// # Returns
    /// * `usize` - Expected length of CoseSign1 object data.
    pub fn encode_len(&self) -> usize {
        COSE_SIGN1_TAG_SIZE
            + PROTECTED_HEADER_SIZE
            + self.payload_len
            + SIGNATURE_SIZE
            + COSE_SIGN1_ENCODING_BYTES
    }
}
