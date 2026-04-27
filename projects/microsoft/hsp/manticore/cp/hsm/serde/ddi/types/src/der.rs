// Copyright (c) Microsoft Corporation. All rights reserved.

// This contains helper functions for pre_encode and post_decode functions

#[cfg(any(feature = "pre_encode", feature = "post_decode"))]
extern crate alloc;
#[cfg(any(feature = "pre_encode", feature = "post_decode"))]
use alloc::vec::Vec;

#[cfg(feature = "post_decode")]
use mcr_ddi_mbor::MborDecodeError;
#[cfg(feature = "pre_encode")]
use mcr_ddi_mbor::MborEncodeError;

#[cfg(any(feature = "pre_encode", feature = "post_decode"))]
use crate::DdiEccCurve;
#[cfg(feature = "post_decode")]
use crate::MAX_ECC_DER_COMPONENT_SIZE;

/// Reverse copy a slice from src to destination
/// Helper function for implementing pre_encode_fn and post_decode_fn
pub fn reverse_copy(dst: &mut [u8], src: &[u8]) {
    for (item1, item2) in src.iter().rev().zip(dst.iter_mut()) {
        *item2 = *item1;
    }
}

/// Structure to represent Ecc key data (Big endian format)
/// Helper struct for pre_encode_fn and post_decode_fn
#[cfg(any(feature = "pre_encode", feature = "post_decode"))]
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EccPublicKeyData {
    pub x: [u8; MAX_ECC_DER_COMPONENT_SIZE],
    pub y: [u8; MAX_ECC_DER_COMPONENT_SIZE],
    pub curve: DdiEccCurve,
}

/// Structure to represent Ecc key data (Big endian format)
/// Helper struct for pre_encode_fn and post_decode_fn
#[cfg(feature = "post_decode")]
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RsaPublicKeyData {
    pub e: Vec<u8>,
    pub n: Vec<u8>,
    pub little_endian: bool,
}

/// Parse DER to create Ecc Public key data
/// Helper function for implementing pre_encode_fn
#[cfg(feature = "pre_encode")]
pub fn ecc_pub_key_der_to_raw(buf: &[u8]) -> Result<EccPublicKeyData, MborEncodeError> {
    let spki = spki::SubjectPublicKeyInfoRef::try_from(buf).map_err(|sec1_error_stack| {
        tracing::error!(?sec1_error_stack);
        MborEncodeError::DerDecodeFailed
    })?;
    let spki_parameters = spki
        .algorithm
        .parameters
        .ok_or(MborEncodeError::DerDecodeFailed)?;
    let algo_oid = spki_parameters
        .decode_as::<spki::ObjectIdentifier>()
        .map_err(|sec1_error_stack| {
            tracing::error!(?sec1_error_stack);
            MborEncodeError::DerDecodeFailed
        })?;

    use p256::elliptic_curve::pkcs8::AssociatedOid;
    let p256_oid: spki::ObjectIdentifier = <p256::NistP256 as AssociatedOid>::OID;
    let p384_oid: spki::ObjectIdentifier = <p384::NistP384 as AssociatedOid>::OID;
    let p521_oid: spki::ObjectIdentifier = <p521::NistP521 as AssociatedOid>::OID;

    if algo_oid == p256_oid {
        let point = sec1::point::EncodedPoint::<sec1::consts::U32>::from_bytes(
            spki.subject_public_key.raw_bytes(),
        )
        .map_err(|sec1_error_stack| {
            tracing::error!(?sec1_error_stack);
            MborEncodeError::DerDecodeFailed
        })?;
        match point.coordinates() {
            sec1::point::Coordinates::Uncompressed { x, y } => {
                let mut x_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                let mut y_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                x_array[32 - x.len()..32].copy_from_slice(x);
                y_array[32 - y.len()..32].copy_from_slice(y);
                Ok(EccPublicKeyData {
                    x: x_array,
                    y: y_array,
                    curve: DdiEccCurve::P256,
                })
            }
            unexpected_coordinates => {
                tracing::error!("Unexpected coordinates {:?}", unexpected_coordinates);
                Err(MborEncodeError::DerDecodeFailed)?
            }
        }
    } else if algo_oid == p384_oid {
        let point = sec1::point::EncodedPoint::<sec1::consts::U48>::from_bytes(
            spki.subject_public_key.raw_bytes(),
        )
        .map_err(|sec1_error_stack| {
            tracing::error!(?sec1_error_stack);
            MborEncodeError::DerDecodeFailed
        })?;
        match point.coordinates() {
            sec1::point::Coordinates::Uncompressed { x, y } => {
                let mut x_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                let mut y_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                x_array[48 - x.len()..48].copy_from_slice(x);
                y_array[48 - y.len()..48].copy_from_slice(y);
                Ok(EccPublicKeyData {
                    x: x_array,
                    y: y_array,
                    curve: DdiEccCurve::P384,
                })
            }
            unexpected_coordinates => {
                tracing::error!("Unexpected coordinates {:?}", unexpected_coordinates);
                Err(MborEncodeError::DerDecodeFailed)?
            }
        }
    } else if algo_oid == p521_oid {
        let point = sec1::point::EncodedPoint::<sec1::consts::U66>::from_bytes(
            spki.subject_public_key.raw_bytes(),
        )
        .map_err(|sec1_error_stack| {
            tracing::error!(?sec1_error_stack);
            MborEncodeError::DerDecodeFailed
        })?;
        match point.coordinates() {
            sec1::point::Coordinates::Uncompressed { x, y } => {
                let mut x_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                let mut y_array = [0u8; MAX_ECC_DER_COMPONENT_SIZE];
                x_array[66 - x.len()..66].copy_from_slice(x);
                y_array[66 - y.len()..66].copy_from_slice(y);
                Ok(EccPublicKeyData {
                    x: x_array,
                    y: y_array,
                    curve: DdiEccCurve::P521,
                })
            }
            unexpected_coordinates => {
                tracing::error!("Unexpected coordinates {:?}", unexpected_coordinates);
                Err(MborEncodeError::DerDecodeFailed)?
            }
        }
    } else {
        tracing::error!("Unexpected algorithm oid {:?}", algo_oid);
        Err(MborEncodeError::DerDecodeFailed)?
    }
}

/// Convert Ecc public key data to DER
/// Helper function for implementing post_decode_fn
#[cfg(feature = "post_decode")]
pub fn ecc_pub_key_raw_to_der(key_data: EccPublicKeyData) -> Result<Vec<u8>, MborDecodeError> {
    use spki::EncodePublicKey;
    let document = match key_data.curve {
        DdiEccCurve::P256 => {
            let point = sec1::point::EncodedPoint::<sec1::consts::U32>::from_affine_coordinates(
                &p256::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.x[..32],
                ),
                &p256::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.y[..32],
                ),
                false,
            );
            use p256::elliptic_curve::sec1::FromEncodedPoint;
            let public_key: p256::PublicKey =
                Option::from(p256::PublicKey::from_encoded_point(&point))
                    .ok_or(MborDecodeError::InvalidKeyData)?;
            public_key.to_public_key_der().map_err(|sec1_error_stack| {
                tracing::error!(?sec1_error_stack);
                MborDecodeError::InvalidKeyData
            })?
        }
        DdiEccCurve::P384 => {
            let point = sec1::point::EncodedPoint::<sec1::consts::U48>::from_affine_coordinates(
                &p384::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.x[..48],
                ),
                &p384::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.y[..48],
                ),
                false,
            );
            use p384::elliptic_curve::sec1::FromEncodedPoint;
            let public_key: p384::PublicKey =
                Option::from(p384::PublicKey::from_encoded_point(&point))
                    .ok_or(MborDecodeError::InvalidKeyData)?;
            public_key.to_public_key_der().map_err(|sec1_error_stack| {
                tracing::error!(?sec1_error_stack);
                MborDecodeError::InvalidKeyData
            })?
        }
        DdiEccCurve::P521 => {
            let point = sec1::point::EncodedPoint::<sec1::consts::U66>::from_affine_coordinates(
                &p521::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.x[..66],
                ),
                &p521::elliptic_curve::generic_array::GenericArray::clone_from_slice(
                    &key_data.y[..66],
                ),
                false,
            );
            use p521::elliptic_curve::sec1::FromEncodedPoint;
            let public_key: p521::PublicKey =
                Option::from(p521::PublicKey::from_encoded_point(&point))
                    .ok_or(MborDecodeError::InvalidKeyData)?;
            public_key.to_public_key_der().map_err(|sec1_error_stack| {
                tracing::error!(?sec1_error_stack);
                MborDecodeError::InvalidKeyData
            })?
        }
    };

    Ok(document.as_bytes().to_vec())
}

/// Convert RSA public key data to DER
/// Helper function for implementing post_decode_fn
#[cfg(feature = "post_decode")]
pub fn rsa_pub_key_raw_to_der(key_data: RsaPublicKeyData) -> Result<Vec<u8>, MborDecodeError> {
    use rsa::RsaPublicKey;
    let (n_uint, e_uint) = if key_data.little_endian {
        (
            rsa::BigUint::from_bytes_le(&key_data.n),
            rsa::BigUint::from_bytes_le(&key_data.e),
        )
    } else {
        (
            rsa::BigUint::from_bytes_be(&key_data.n),
            rsa::BigUint::from_bytes_be(&key_data.e),
        )
    };
    let public_key = RsaPublicKey::new(n_uint, e_uint).map_err(|error_stack| {
        tracing::error!(?error_stack);
        MborDecodeError::InvalidKeyData
    })?;
    let document =
        rsa::pkcs8::EncodePublicKey::to_public_key_der(&public_key).map_err(|error_stack| {
            tracing::error!(?error_stack);
            MborDecodeError::InvalidKeyData
        })?;

    Ok(document.as_bytes().to_vec())
}
