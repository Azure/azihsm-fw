// Copyright (c) Microsoft Corporation. All rights reserved.

/// NIST P-256 curve order `n`, in little-endian u64 limbs (limb[0] = LSB).
/// Source: SEC 2: Recommended Elliptic Curve Domain Parameters (Version 2.0)
/// https://www.secg.org/sec2-v2.pdf, section 2.4.1 (secp256r1)
pub const P256_ORDER_U64: [u64; 4] = [
    0xf3b9cac2fc632551,
    0xbce6faada7179e84,
    0xffffffffffffffff,
    0xffffffff00000000,
];

/// NIST P-384 curve order `n`, in little-endian u64 limbs (limb[0] = LSB).
/// Source: SEC 2: Recommended Elliptic Curve Domain Parameters (Version 2.0)
/// https://www.secg.org/sec2-v2.pdf, section 2.5.1 (secp384r1)
pub const P384_ORDER_U64: [u64; 6] = [
    0xecec196accc52973,
    0x581a0db248b0a77a,
    0xc7634d81f4372ddf,
    0xffffffffffffffff,
    0xffffffffffffffff,
    0xffffffffffffffff,
];

/// NIST P-521 curve order `n`, in little-endian u64 limbs (limb[0] = LSB).
/// Source: SEC 2: Recommended Elliptic Curve Domain Parameters (Version 2.0)
/// https://www.secg.org/sec2-v2.pdf, section 2.6.1 (secp521r1)
pub const P521_ORDER_U64: [u64; 9] = [
    0xbb6fb71e91386409,
    0x3bb5c9b8899c47ae,
    0x7fcc0148f709a5d0,
    0x51868783bf2f966b,
    0xfffffffffffffffa,
    0xffffffffffffffff,
    0xffffffffffffffff,
    0xffffffffffffffff,
    0x00000000000001ff,
];
