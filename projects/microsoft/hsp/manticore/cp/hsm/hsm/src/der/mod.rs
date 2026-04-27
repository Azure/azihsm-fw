// Copyright (c) Microsoft Corporation. All rights reserved.

use alloc::vec;
use alloc::vec::Vec;

use asn1::BigInt;
use asn1::BitString;
use asn1::Explicit;
use asn1::Null;
use asn1::ObjectIdentifier;
use mcr_types::SecureByteArray;
use mcr_types::SecureByteVec;

use crate::error::HsmErr;
use crate::partition::EccCurve;
use crate::partition::EccPrivKey;
use crate::partition::EccPubKey;
use crate::partition::RsaPrivKey;
use crate::partition::RsaPrivKeyCrt;
use crate::partition::RsaPubKey;
use crate::partition::RsaSize;

mod tests;

/// Structure to hold ECC curve parameters.
#[derive(Eq, PartialEq, asn1::Asn1Read, asn1::Asn1Write)]
pub enum EcParameters {
    /// Elliptic curve named by a particular OID.
    ///
    /// > namedCurve identifies all the required values for a particular
    /// > set of elliptic curve domain parameters to be represented by an
    /// > object identifier.
    NamedCurve(ObjectIdentifier),
}

/// Parse a DER length at `data[offset]`.  
/// Returns `(length, length_of_length_field)` if successful.
pub fn parse_der_length(data: &[u8], offset: usize) -> Option<(usize, usize)> {
    let byte0 = *data.get(offset)?;
    if byte0 & 0x80 == 0 {
        // short form
        Some((byte0 as usize, 1))
    } else {
        // long form: lower 7 bits = number of following length bytes
        let n = (byte0 & 0x7F) as usize;
        if n == 0 || offset + 1 + n > data.len() {
            return None;
        }
        let mut val = 0usize;
        for i in 0..n {
            val = (val << 8) | (data[offset + 1 + i] as usize);
        }
        Some((val, n + 1))
    }
}

/// Finds the Subject Common Name (= OID 2.5.4.3) printable/UTF8 string.
pub(crate) fn get_subject_cn(der: &[u8]) -> Option<&[u8]> {
    // OID TLV for 2.5.4.3 = 06 03 55 04 03
    const CN_OID: [u8; 5] = [0x06, 0x03, 0x55, 0x04, 0x03];

    let mut first_pos = None;
    let mut second_pos = None;
    let wnd = CN_OID.len();
    let max = der.len().saturating_sub(wnd);

    // scan for up to two matches
    for i in 0..=max {
        if der[i..i + wnd] == CN_OID {
            if first_pos.is_none() {
                first_pos = Some(i);
            } else {
                second_pos = Some(i);
                break;
            }
        }
    }

    // choose second if present, else first
    let pos = second_pos.or(first_pos)?;

    // skip past the OID TLV header+value
    let (oid_len, oid_len_bytes) = parse_der_length(der, pos + 1)?;
    let idx = pos + 1 + oid_len_bytes + oid_len;

    // at idx: the string tag (e.g. 0x0C or 0x13); length follows at idx+1
    let (val_len, val_len_bytes) = parse_der_length(der, idx + 1)?;
    let start = idx + 1 + val_len_bytes;
    let end = start + val_len;

    // // slice out and UTF-8 validate
    // der.get(start..end)
    //    .and_then(|bytes| core::str::from_utf8(bytes).ok())
    der.get(start..end)
        .and_then(|bytes| if bytes.is_empty() { None } else { Some(bytes) })
}

/// Finds the Subject Key Identifier extension (OID 2.5.29.35) and returns its keyIdentifier bytes.
pub(crate) fn get_subject_key_identifier(der: &[u8]) -> Option<&[u8]> {
    // DER for OID 2.5.29.14 is: 06 03 55 1D 0E
    const SKI_OID_TLV: &[u8; 5] = &[0x06, 0x03, 0x55, 0x1D, 0x0E];

    // 1) locate the OID TLV
    let pos = der
        .windows(SKI_OID_TLV.len())
        .position(|w| w == SKI_OID_TLV)?;

    // 2) skip past the OID’s length+value
    let (oid_len, oid_len_bytes) = parse_der_length(der, pos + 1)?;
    let mut idx = pos + 1 + oid_len_bytes + oid_len;

    // 3) skip OPTIONAL critical BOOLEAN (tag 0x01)
    if der.get(idx) == Some(&0x01) {
        let (bool_len, bool_len_bytes) = parse_der_length(der, idx + 1)?;
        idx += 1 + bool_len_bytes + bool_len;
    }

    // 4) next must be the OCTET STRING (tag 0x04) wrapping the SKI
    if der.get(idx) != Some(&0x04) {
        return None;
    }
    let (ext_len, ext_len_bytes) = parse_der_length(der, idx + 1)?;
    let ext_start = idx + 1 + ext_len_bytes;
    let ext_end = ext_start + ext_len;
    let ext_bytes = der.get(ext_start..ext_end)?;

    // 5) ext_bytes is DER of SubjectKeyIdentifier ::= OCTET STRING
    //    so ext_bytes[0] == 0x04, then length, then the keyIdentifier
    if ext_bytes.first() != Some(&0x04) {
        return None;
    }
    let (ki_len, ki_len_bytes) = parse_der_length(ext_bytes, 1)?;
    let ki_start = 1 + ki_len_bytes;
    let ki_end = ki_start + ki_len;
    ext_bytes.get(ki_start..ki_end)
}

// RSA Private Key ASN1 format:
// https://datatracker.ietf.org/doc/html/rfc8017#appendix-A.1.2
//
// RSAPrivateKey ::= SEQUENCE {
//     version           Version,
//     modulus           INTEGER,  -- n
//     publicExponent    INTEGER,  -- e
//     privateExponent   INTEGER,  -- d
//     prime1            INTEGER,  -- p
//     prime2            INTEGER,  -- q
//     exponent1         INTEGER,  -- d mod (p-1)
//     exponent2         INTEGER,  -- d mod (q-1)
//     coefficient       INTEGER,  -- (inverse of q) mod p
//     otherPrimeInfos   OtherPrimeInfos OPTIONAL
// OtherPrimeInfos ::= SEQUENCE SIZE(1..MAX) OF OtherPrimeInfo
// OtherPrimeInfo ::= SEQUENCE {
//     prime             INTEGER,  -- ri
//     exponent          INTEGER,  -- di
//     coefficient       INTEGER   -- ti
// }
#[derive(asn1::Asn1Read, asn1::Asn1Write)]
pub struct RsaOtherPrimeInfos<'a> {
    /// Prime
    prime: BigInt<'a>,

    /// Exponent
    exponent: BigInt<'a>,

    /// Coefficient
    coefficient: BigInt<'a>,
}

pub struct RsaKeyData {
    // RSA Type.
    pub(crate) rsa_type: RsaSize,

    /// Modulus
    modulus: SecureByteVec,

    /// Public exponent
    public_exponent: SecureByteVec,

    /// Private exponent
    private_exponent: SecureByteVec,

    /// Prime 1
    prime1: SecureByteVec,

    /// Prime 2
    prime2: SecureByteVec,

    /// Exponent 1
    exponent1: SecureByteVec,

    /// Exponent 2
    exponent2: SecureByteVec,

    /// Coefficient
    coefficient: SecureByteVec,
}

impl RsaKeyData {
    // RSA Public key in big-endian format.
    pub(crate) fn pub_key(&self) -> RsaPubKey {
        RsaPubKey::from_bytes_be(
            self.rsa_type,
            self.public_exponent.as_slice(),
            self.modulus.as_slice(),
        )
    }

    // RSA Private key in big-endian format.
    pub(crate) fn priv_key(&self) -> RsaPrivKey {
        RsaPrivKey::from_bytes_be(
            self.rsa_type,
            self.private_exponent.as_slice(),
            self.modulus.as_slice(),
            self.public_exponent.as_slice(),
        )
    }

    // RSA CRT Private key in big-endian format.
    pub(crate) fn priv_key_crt(&self) -> RsaPrivKeyCrt {
        RsaPrivKeyCrt {
            rsa_type: self.rsa_type,
            p: self.prime1.clone(),
            q: self.prime2.clone(),
            dp: self.exponent1.clone(),
            dq: self.exponent2.clone(),
            n: self.modulus.clone(),
            n1q: None,
            n2p: None,
            e: self.public_exponent.clone(),
            coefficient: self.coefficient.clone(),
        }
    }
}

#[derive(asn1::Asn1Read, asn1::Asn1Write)]
pub struct Asn1RsaPubKeyParams<'a> {
    modulus: BigInt<'a>,
    exponent: BigInt<'a>,
}

#[derive(asn1::Asn1Read, asn1::Asn1Write)]
pub struct Asn1RsaEncryptionInfo {
    encryption_id: ObjectIdentifier,
    parameter: Null,
}

/// Structure to facilitate ASN1 decoding operations.
pub struct Asn1Decoder {}

impl Asn1Decoder {
    pub fn new() -> Self {
        Self {}
    }

    /// Convert an ECC DER private key to raw format.
    ///
    /// Note: PKCS#8 implementation
    /// PKCS#8 Private Key ASN.1 format:
    /// https://datatracker.ietf.org/doc/html/rfc5208#section-5
    /// Note that the RFC spec says that the public key option is marked
    /// IMPLICIT. But that is not how openssl formats the key. Hence, this implementation
    /// uses EXPLICIT tag to parse the public key.
    ///  PrivateKeyInfo ::= SEQUENCE {
    ///     version                   Version,
    ///     privateKeyAlgorithm       PrivateKeyAlgorithmIdentifier,
    ///     privateKey                PrivateKey,
    ///     attributes           [0]  IMPLICIT Attributes OPTIONAL }
    ///   Version ::= INTEGER
    ///   PrivateKeyAlgorithmIdentifier ::= AlgorithmIdentifier
    ///   PrivateKey ::= OCTET STRING
    ///   Attributes ::= SET OF Attribute
    ///
    /// # Arguments
    ///
    /// * `buf` - Input buffer
    ///
    /// # Returns
    ///
    /// Returns a `Result` containing a `Vec` of bytes representing the DER-decoded public key
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn ecc_priv_key_pkcs8_der_to_raw(&self, buf: &[u8]) -> Result<EccKeyData, HsmErr> {
        let result: asn1::ParseResult<_> = asn1::parse(buf, |d| {
            d.read_element::<asn1::Sequence>()?.parse(|d| {
                let _version = d.read_element::<BigInt>()?;
                let ec_oid = d.read_element::<asn1::Sequence>()?.parse(|d| {
                    let _ecpubkey_oid = d.read_element::<ObjectIdentifier>()?;
                    let ec_oid = d.read_element::<ObjectIdentifier>()?;
                    Ok(ec_oid)
                })?;
                let key_data_buf = d.read_element::<&[u8]>()?;

                Ok((ec_oid, key_data_buf))
            })
        });

        if let Ok((ec_oid, key_data_buf)) = result {
            let key_data: asn1::ParseResult<_> = asn1::parse(key_data_buf, |d| {
                d.read_element::<asn1::Sequence>()?.parse(|d| {
                    let _version = d.read_element::<BigInt>()?;
                    let private_key = d.read_element::<&[u8]>()?;
                    let public_key = d.read_element::<Explicit<Option<BitString>, 1>>()?;
                    Ok((private_key, public_key.as_inner().clone()))
                })
            });

            if let Ok((priv_key_slice, pub_key_bitstring)) = key_data {
                let pub_key_slice = pub_key_bitstring.ok_or(HsmErr::DerDecodeFailed)?.as_bytes();

                // Index 0 points to the form of key compression which should be ignored.
                // 0x04 = Uncompressed format.
                // Index 1+ may some times have a 0x00 byte if the MS bit of the number itself is 1,
                // but the number is positive. The 0x00 byte is prepended in this case for DER encoding
                // to explicitly indicate that the number is positive. This byte should be detected and ignored as well.
                let ecc_curve = get_ecc_curve(ec_oid)?;
                let (pub_key_vec_x, pub_key_vec_y, ecc_pub_key) =
                    get_ecc_pub_key_params(ecc_curve, pub_key_slice)?;

                // Index 0 points to the form of key compression which should be ignored.
                // 0x04 = Uncompressed format.
                // Index 1+ may some times have a 0x00 byte if the MS bit of the number itself is 1,
                // but the number is positive. The 0x00 byte is prepended in this case for DER encoding
                // to explicitly indicate that the number is positive. This byte should be detected and ignored as well.
                let mut index = 0;
                let mut priv_key_zero_count = 0;
                while priv_key_slice[index] == 0 {
                    index += 1;
                    priv_key_zero_count += 1;
                }

                return Ok(EccKeyData {
                    orig_priv_key: priv_key_slice[priv_key_zero_count..].into(),
                    orig_pub_key_x: pub_key_vec_x,
                    orig_pub_key_y: pub_key_vec_y,
                    priv_key: Some(EccPrivKey {
                        curve: ecc_curve,
                        k: {
                            let mut k = SecureByteArray::<{ EccCurve::MAX_LEN }>::new(
                                [0u8; EccCurve::MAX_LEN],
                            );
                            k[EccCurve::MAX_LEN - priv_key_slice[priv_key_zero_count..].len()
                                ..EccCurve::MAX_LEN]
                                .copy_from_slice(&priv_key_slice[priv_key_zero_count..]);
                            k
                        },
                    }),
                    pub_key: Some(ecc_pub_key),
                });
            }
        }

        Err(HsmErr::DerDecodeFailed)?
    }

    fn ecc_priv_key_pkcs1_der_to_raw(&self, buf: &[u8]) -> Result<EccKeyData, HsmErr> {
        let result: asn1::ParseResult<_> = asn1::parse(buf, |d| {
            d.read_element::<asn1::Sequence>()?.parse(|d| {
                let _version = d.read_element::<BigInt>()?;
                let private_key = d.read_element::<&[u8]>()?;
                let parameters = d.read_element::<Explicit<Option<EcParameters>, 0>>()?;
                let public_key_parse_result = d.read_element::<Explicit<Option<BitString>, 1>>();
                let public_key = if let Ok(val) = public_key_parse_result {
                    val.into_inner()
                } else {
                    None
                };

                Ok((private_key, public_key, parameters))
            })
        });

        if let Ok(result_data) = result {
            let ecc_curve: EccCurve = if let Some(ec_params) = result_data.2.into_inner() {
                let p256 = EcParameters::NamedCurve(asn1::oid!(1, 2, 840, 10045, 3, 1, 7));
                let p384 = EcParameters::NamedCurve(asn1::oid!(1, 3, 132, 0, 34));

                if ec_params == p256 {
                    EccCurve::P256
                } else if ec_params == p384 {
                    EccCurve::P384
                } else {
                    EccCurve::P521
                }
            } else {
                Err(HsmErr::DerDecodeFailed)?
            };

            let mut pub_key_vec_x: Vec<u8> = vec![];
            let mut pub_key_vec_y: Vec<u8> = vec![];
            let mut ecc_pub_key: Option<EccPubKey> = None;
            if let Some(pubkey) = result_data.1 {
                // Index 0 points to the form of key compression which should be ignored.
                // 0x04 = Uncompressed format.
                // Index 1+ may some times have a 0x00 byte if the MS bit of the number itself is 1,
                // but the number is positive. The 0x00 byte is prepended in this case for DER encoding
                // to explicitly indicate that the number is positive. This byte should be detected and ignored as well.

                let pub_key: EccPubKey;
                (pub_key_vec_x, pub_key_vec_y, pub_key) =
                    get_ecc_pub_key_params(ecc_curve, pubkey.as_bytes())?;

                ecc_pub_key = Some(pub_key);
            }

            let priv_key_arr = result_data.0;
            // Index 0 points to the form of key compression which should be ignored.
            // 0x04 = Uncompressed format.
            // Index 1+ may some times have a 0x00 byte if the MS bit of the number itself is 1,
            // but the number is positive. The 0x00 byte is prepended in this case for DER encoding
            // to explicitly indicate that the number is positive. This byte should be detected and ignored as well.
            let mut index = 0;
            let mut priv_key_zero_count = 0;
            while priv_key_arr[index] == 0 {
                index += 1;
                priv_key_zero_count += 1;
            }

            return Ok(EccKeyData {
                orig_priv_key: priv_key_arr[priv_key_zero_count..].into(),
                orig_pub_key_x: pub_key_vec_x,
                orig_pub_key_y: pub_key_vec_y,
                priv_key: Some(EccPrivKey {
                    curve: ecc_curve,
                    k: {
                        let mut k =
                            SecureByteArray::<{ EccCurve::MAX_LEN }>::new([0u8; EccCurve::MAX_LEN]);
                        k[EccCurve::MAX_LEN - priv_key_arr[priv_key_zero_count..].len()
                            ..EccCurve::MAX_LEN]
                            .copy_from_slice(&priv_key_arr[priv_key_zero_count..]);
                        k
                    },
                }),
                pub_key: ecc_pub_key,
            });
        }

        Err(HsmErr::DerDecodeFailed)?
    }

    /// SEQUENCE (2 elem)
    ///   AlgorithmIdentifier SEQUENCE (2 elem)
    ///     OBJECT IDENTIFIER 1.2.840.10045.2.1 ecPublicKey (ANSI X9.62 public key type)
    ///     ANY OBJECT IDENTIFIER 1.2.840.10045.3.1.7 prime256v1 (ANSI X9.62 named elliptic curve)
    ///   BIT STRING 0000010010111101001011000101100101100110001111011010000100001110110000…
    #[allow(unused)]
    fn ecc_pub_key_der_to_raw(&self, buf: &[u8]) -> Result<EccKeyData, HsmErr> {
        let result: asn1::ParseResult<_> = asn1::parse(buf, |d| {
            d.read_element::<asn1::Sequence>()?.parse(|d| {
                let ec_oid = d.read_element::<asn1::Sequence>()?.parse(|d| {
                    let _ecpubkey_oid = d.read_element::<ObjectIdentifier>()?;
                    let ec_oid = d.read_element::<ObjectIdentifier>()?;
                    Ok(ec_oid)
                })?;
                let pubkey_bitstring = d.read_element::<BitString>()?;

                Ok((ec_oid, pubkey_bitstring))
            })
        });

        if let Ok((ec_oid, pubkey_bitstring)) = result {
            let pub_key_slice = pubkey_bitstring.as_bytes();

            // Index 0 points to the form of key compression which should be ignored.
            // 0x04 = Uncompressed format.
            // Index 1+ may some times have a 0x00 byte if the MS bit of the number itself is 1,
            // but the number is positive. The 0x00 byte is prepended in this case for DER encoding
            // to explicitly indicate that the number is positive. This byte should be detected and ignored as well.
            let ecc_curve = get_ecc_curve(ec_oid)?;
            let (pub_key_vec_x, pub_key_vec_y, ecc_pub_key) =
                get_ecc_pub_key_params(ecc_curve, pub_key_slice)?;

            Ok(EccKeyData {
                orig_priv_key: SecureByteVec::new(),
                orig_pub_key_x: pub_key_vec_x,
                orig_pub_key_y: pub_key_vec_y,
                priv_key: None,
                pub_key: Some(ecc_pub_key),
            })
        } else {
            Err(HsmErr::DerDecodeFailed)?
        }
    }

    /// Convert an RSA DER private key to raw format.
    ///
    /// # Arguments
    ///
    /// * `buf` - Input buffer
    ///
    /// # Returns
    ///
    /// Returns a `Result` containing a `Vec` of bytes representing the DER-decoded key
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn rsa_priv_key_der_to_raw(&self, buf: &[u8]) -> Result<RsaKeyData, HsmErr> {
        let result: asn1::ParseResult<_> = asn1::parse(buf, |d| {
            d.read_element::<asn1::Sequence>()?.parse(|d| {
                let _version = d.read_element::<BigInt>()?;
                let _alg_id = d.read_element::<Asn1RsaEncryptionInfo>()?;
                let octet_string = d.read_element::<&[u8]>()?;

                Ok(octet_string)
            })
        });

        let buf = result.map_err(|_| HsmErr::DerDecodeFailed)?;

        self.rsa_priv_key_der_to_raw_internal(buf)
    }

    fn rsa_priv_key_der_to_raw_internal(&self, buf: &[u8]) -> Result<RsaKeyData, HsmErr> {
        let result: asn1::ParseResult<_> = asn1::parse(buf, |d| {
            d.read_element::<asn1::Sequence>()?.parse(|d| {
                let _version_rsa_privkey = d.read_element::<BigInt>()?;
                let modulus = d.read_element::<BigInt>()?;
                let public_exponent = d.read_element::<BigInt>()?;
                let private_exponent = d.read_element::<BigInt>()?;
                let prime1 = d.read_element::<BigInt>()?;
                let prime2 = d.read_element::<BigInt>()?;
                let exponent1 = d.read_element::<BigInt>()?;
                let exponent2 = d.read_element::<BigInt>()?;
                let coefficient = d.read_element::<BigInt>()?;
                let _other_prime_infos = d.read_element::<Option<RsaOtherPrimeInfos>>()?;

                let modulus_trimmed = trim_leading_zeros(modulus.as_bytes());

                Ok(RsaKeyData {
                    rsa_type: RsaSize::Rsa2k, // This is updated to the correct size below.
                    modulus: modulus_trimmed,
                    public_exponent: trim_leading_zeros(public_exponent.as_bytes()),
                    private_exponent: trim_leading_zeros(private_exponent.as_bytes()),
                    prime1: trim_leading_zeros(prime1.as_bytes()),
                    prime2: trim_leading_zeros(prime2.as_bytes()),
                    exponent1: trim_leading_zeros(exponent1.as_bytes()),
                    exponent2: trim_leading_zeros(exponent2.as_bytes()),
                    coefficient: trim_leading_zeros(coefficient.as_bytes()),
                })
            })
        });

        if let Ok(mut result_data) = result {
            result_data.rsa_type = result_data.modulus.len().try_into()?;
            return Ok(result_data);
        }

        Err(HsmErr::DerDecodeFailed)?
    }
}

fn get_ecc_pub_key_params(
    ecc_curve: EccCurve,
    pub_key_slice: &[u8],
) -> Result<(Vec<u8>, Vec<u8>, EccPubKey), HsmErr> {
    let comp_len: usize = ecc_curve.into();
    let mut x_zero_count = 0;
    let mut index = 1;
    while pub_key_slice[index] == 0 {
        index += 1;
        x_zero_count += 1;
    }
    let pub_key_vec_x: Vec<u8> = Vec::from(&pub_key_slice[1 + x_zero_count..=comp_len]);
    let mut y_zero_count = 0;
    index = comp_len + 1;
    while pub_key_slice[index] == 0 {
        index += 1;
        y_zero_count += 1;
    }
    let pub_key_vec_y: Vec<u8> = Vec::from(&pub_key_slice[comp_len + y_zero_count + 1..]);
    let ecc_pub_key = EccPubKey::from_bytes_be(
        ecc_curve,
        pub_key_vec_x.as_slice(),
        pub_key_vec_y.as_slice(),
    );
    Ok((pub_key_vec_x, pub_key_vec_y, ecc_pub_key))
}

fn get_ecc_curve(ec_oid: ObjectIdentifier) -> Result<EccCurve, HsmErr> {
    let ecc_curve: EccCurve = {
        let p256 = asn1::oid!(1, 2, 840, 10045, 3, 1, 7);
        let p384 = asn1::oid!(1, 3, 132, 0, 34);
        let p521 = asn1::oid!(1, 3, 132, 0, 35);

        if ec_oid == p256 {
            EccCurve::P256
        } else if ec_oid == p384 {
            EccCurve::P384
        } else if ec_oid == p521 {
            EccCurve::P521
        } else {
            Err(HsmErr::DerDecodeFailed)?
        }
    };
    Ok(ecc_curve)
}

#[allow(unused)]
pub(crate) trait DerEncoderTrait {
    /// Converts the data into DER encoded format.
    ///
    /// # Returns
    /// Returns a `Result` containing a `Vec` of bytes representing the DER-encoded private key
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn to_der(&self) -> Result<Vec<u8>, HsmErr> {
        Err(HsmErr::DerEncodeFailed)?
    }
}

// Structure to contain ECC Key data.
#[allow(dead_code)]
pub(crate) struct EccKeyData {
    /// Original Private Key without converting to any custom formats. (Big endian)
    orig_priv_key: SecureByteVec,

    /// Original public key X component (Big endian)
    orig_pub_key_x: Vec<u8>,

    /// Original public key Y component (Big endian)
    orig_pub_key_y: Vec<u8>,

    /// Private key (Big endian)
    priv_key: Option<EccPrivKey>,

    // Public Key (Big endian)
    pub_key: Option<EccPubKey>,
}

impl EccKeyData {
    // ECC Public Key in big-endian format.
    pub(crate) fn pub_key(&self) -> Option<EccPubKey> {
        self.pub_key.clone()
    }

    // ECC Private key in big-endian format.
    pub(crate) fn priv_key(&self) -> Option<EccPrivKey> {
        self.priv_key.clone()
    }
}

pub(crate) trait DerDecoderTrait {
    /// Converts the DER encoded data into raw data.(Big endian)
    ///
    /// # Returns
    /// Returns a `Result` containing the raw private key and public key respectively
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn ecc_priv_key_pkcs8_der_to_raw(&self) -> Result<EccKeyData, HsmErr> {
        Err(HsmErr::DerDecodeFailed)?
    }

    /// Converts the PKCS1 DER encoded data into raw data. (Big endian)
    ///
    /// # Returns
    /// Returns a `Result` containing the raw private key and public key respectively
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn ecc_priv_key_pkcs1_der_to_raw(&self) -> Result<EccKeyData, HsmErr> {
        Err(HsmErr::DerDecodeFailed)?
    }

    /// Converts the DER encoded data into raw data. (Big endian)
    ///
    /// # Returns
    /// Returns a `Result` containing the raw private key and public key respectively
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    #[allow(unused)]
    fn ecc_pub_key_der_to_raw(&self) -> Result<EccKeyData, HsmErr>;

    /// Converts the DER encoded data into raw data. (Big endian)
    ///
    /// # Returns
    /// Returns a `Result` containing the raw private key and public key respectively
    /// if the conversion is successful, otherwise returns a `HsmErr` error.
    fn rsa_der_to_raw(&self) -> Result<RsaKeyData, HsmErr> {
        Err(HsmErr::DerDecodeFailed)?
    }
}

impl DerDecoderTrait for &[u8] {
    fn ecc_priv_key_pkcs8_der_to_raw(&self) -> Result<EccKeyData, HsmErr> {
        let decoder = Asn1Decoder::new();
        decoder.ecc_priv_key_pkcs8_der_to_raw(self)
    }

    fn ecc_priv_key_pkcs1_der_to_raw(&self) -> Result<EccKeyData, HsmErr> {
        let decoder = Asn1Decoder::new();
        decoder.ecc_priv_key_pkcs1_der_to_raw(self)
    }

    fn ecc_pub_key_der_to_raw(&self) -> Result<EccKeyData, HsmErr> {
        let decoder = Asn1Decoder::new();
        decoder.ecc_pub_key_der_to_raw(self)
    }

    fn rsa_der_to_raw(&self) -> Result<RsaKeyData, HsmErr> {
        let decoder = Asn1Decoder::new();
        decoder.rsa_priv_key_der_to_raw(self)
    }
}

// The leading 00 octet ensures that the encryption block, converted to an integer, is less
// than the modulus. This byte needs to be ignored, which this helper does.
// Refer https://www.rfc-editor.org/rfc/rfc2313.
fn trim_leading_zeros(data: &[u8]) -> SecureByteVec {
    let first_non_zero = data.iter().position(|&x| x != 0).unwrap_or(data.len());
    SecureByteVec::from(&data[first_non_zero..])
}
