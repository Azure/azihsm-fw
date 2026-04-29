// Copyright (c) Microsoft Corporation. All rights reserved.

//! The module for `COSE_Key` based on Section 7, Key Objects in <https://www.rfc-editor.org/rfc/rfc9052>.

use mcr_crypto_pka::PkaEccPublicKey;
use minicbor::Encoder;

use crate::error::HsmErr;
use crate::error::HsmResult;
use crate::key_attestation::report::PUBLIC_KEY_MAX_SIZE;
use crate::partition::reverse_copy;
use crate::partition::EccCurve;
use crate::partition::EccPubKey;
use crate::partition::RsaPubKey;

// The following definitions are based on <https://www.iana.org/assignments/cose/cose.xhtml>.
const COSE_KEY_COMMON_PARAMETERS_KTY: u8 = 1;
const COSE_KEY_TYPES_RSA: u8 = 3;
const COSE_KEY_TYPES_EC2: u8 = 2;
const COSE_KEY_TYPE_PARAMETERS_RSA_N: i8 = -1;
const COSE_KEY_TYPE_PARAMETERS_RSA_E: i8 = -2;
const COSE_KEY_TYPE_PARAMETERS_EC2_CRV: i8 = -1;
const COSE_KEY_TYPE_PARAMETERS_EC2_X: i8 = -2;
const COSE_KEY_TYPE_PARAMETERS_EC2_Y: i8 = -3;

/// Encode a RSA public key into `COSE_Key` format.
///
/// # Arguments
/// * `n` - RSA public modulus.
/// * `e` - RSA exponent.
/// * `out` - The output buffer.
///
/// # Returns
/// * `usize` - The size of the output data.
///
/// # Errors
/// * `HsmErr::CoseKeyEncodeFailed` - If encoding fails.
pub fn encode_rsa_public(n: &[u8], e: &[u8], out: &mut [u8]) -> HsmResult<usize> {
    let out_len = out.len();
    let mut encoder = Encoder::new(out);

    encoder
        .map(3)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .u8(COSE_KEY_COMMON_PARAMETERS_KTY)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .u8(COSE_KEY_TYPES_RSA)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(COSE_KEY_TYPE_PARAMETERS_RSA_N)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .bytes(n)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(COSE_KEY_TYPE_PARAMETERS_RSA_E)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .bytes(e)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?;

    // We can safely do the subtraction given that encoder already ensures the size
    // of the encoded data is bounded by the size of the buffer.
    Ok(out_len - encoder.writer().len())
}

/// Encode an ECC public key into `COSE_Key` format.
///
/// # Arguments
/// * `crv` - ECC curve.
/// * `x` - ECC public x coordinate.
/// * `y` - ECC public y coordinate.
/// * `out` - The output buffer.
///
/// # Returns
/// * `usize` - The size of the output data.
///
/// # Errors
/// * `HsmErr::CoseKeyEncodeFailed` - If encoding fails.
pub fn encode_ecc_public(crv: i8, x: &[u8], y: &[u8], out: &mut [u8]) -> HsmResult<usize> {
    let out_len = out.len();
    let mut encoder = Encoder::new(out);

    encoder
        .map(4)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .u8(COSE_KEY_COMMON_PARAMETERS_KTY)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .u8(COSE_KEY_TYPES_EC2)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(COSE_KEY_TYPE_PARAMETERS_EC2_CRV)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(crv)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(COSE_KEY_TYPE_PARAMETERS_EC2_X)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .bytes(x)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .i8(COSE_KEY_TYPE_PARAMETERS_EC2_Y)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?
        .bytes(y)
        .map_err(|_| HsmErr::CoseKeyEncodeFailed)?;

    // We can safely do the subtraction given that encoder already ensures the size
    // of the encoded data is bounded by the size of the buffer.
    Ok(out_len - encoder.writer().len())
}

/// The trait for encoding a public key to COSE Key format.
pub(crate) trait CoseKeyEncoderTrait {
    fn to_cose_key(&self) -> HsmResult<([u8; PUBLIC_KEY_MAX_SIZE], u16)>;
}

impl CoseKeyEncoderTrait for EccPubKey {
    fn to_cose_key(&self) -> HsmResult<([u8; PUBLIC_KEY_MAX_SIZE], u16)> {
        // Based on Table 18, https://www.rfc-editor.org/rfc/rfc9053.html
        let crv = match self.curve() {
            EccCurve::P256 => 1,
            EccCurve::P384 => 2,
            EccCurve::P521 => 3,
        };
        let (x, y) = self.coordinates();

        let mut buffer = [0u8; PUBLIC_KEY_MAX_SIZE];
        let len = encode_ecc_public(crv, x, y, &mut buffer)?;

        Ok((buffer, len as u16))
    }
}

impl CoseKeyEncoderTrait for RsaPubKey {
    fn to_cose_key(&self) -> HsmResult<([u8; PUBLIC_KEY_MAX_SIZE], u16)> {
        let n = self.modulus_be();
        let e = self.exponent_be();

        let mut buffer = [0u8; PUBLIC_KEY_MAX_SIZE];
        let len = encode_rsa_public(&n, &e, &mut buffer)?;

        Ok((buffer, len as u16))
    }
}

impl CoseKeyEncoderTrait for PkaEccPublicKey {
    fn to_cose_key(&self) -> HsmResult<([u8; PUBLIC_KEY_MAX_SIZE], u16)> {
        // Based on Table 18, https://www.rfc-editor.org/rfc/rfc9053.html
        let der_curve: EccCurve = self.curve.into();
        let crv = match der_curve {
            EccCurve::P256 => 1,
            EccCurve::P384 => 2,
            EccCurve::P521 => 3,
        };

        let comp_len = der_curve.len();
        let mut x = [0u8; EccCurve::MAX_LEN];
        reverse_copy(&mut x[..comp_len], &self.x()[..comp_len]);
        let mut y = [0u8; EccCurve::MAX_LEN];
        reverse_copy(&mut y[..comp_len], &self.y()[..comp_len]);

        let mut buffer = [0u8; PUBLIC_KEY_MAX_SIZE];
        let len = encode_ecc_public(crv, &x[..comp_len], &y[..comp_len], &mut buffer)?;

        Ok((buffer, len as u16))
    }
}
