// Copyright (c) Microsoft Corporation. All rights reserved.

use core::cmp::Ordering;

use mcr_crypto_pka::*;

use crate::partition::EccCurve;

pub(crate) struct EccPublicKeyRangeValidation {}

impl EccPublicKeyRangeValidation {
    /// Validate the incoming public key x and y meets the condition: 1 <= x <= p - 1
    /// and 1 <= y <= p - 1, where p is the prime of the curve.
    /// To do this we first check from Most Significant Byte to Least Significant Byte.
    ///
    /// x, y and p are big numbers in little endian format.
    pub fn validate(x: &[u8], y: &[u8], curve: EccCurve) -> bool {
        let prime = PkaEccCurve::get_prime(curve.into());
        let p = &prime.p[..curve.len()];

        // Check if x and y are in range [1, p - 1]
        Self::is_in_valid_range(x, p) && Self::is_in_valid_range(y, p)
    }

    /// Validate that a coordinate meets the condition: 1 <= coordinate <= p - 1
    /// This is adapted from the RSA check_valid_input function pattern.
    ///
    /// coordinate and p are big numbers in little endian format.
    fn is_in_valid_range(coordinate: &[u8], p: &[u8]) -> bool {
        // sanity check
        if coordinate.len() != p.len() {
            return false;
        }

        // used to keep track of whether coordinate > 0
        let mut coord_gt_zero = false;

        // Compare bytes from most to least significant (right to left in little-endian)
        // At non-LSB positions (i > 0):
        // - If p[i] > coordinate[i]: The difference is at least 2^8 (256), which guarantees coordinate < p
        // - If coordinate[i] > p[i]: The value coordinate exceeds p, so coordinate >= p is true
        // - If equal: Continue checking lower bytes
        for i in (1..p.len()).rev() {
            // check if coordinate[i] > 0, when i > 0
            coord_gt_zero |= coordinate[i] > 0;

            match p[i].cmp(&coordinate[i]) {
                Ordering::Greater => {
                    if coord_gt_zero {
                        return true;
                    }

                    // Since we know coordinate < p here, we need to check if coordinate >= 1
                    // return true if 1 <= coordinate < p, false if coordinate < 1
                    return coordinate[1..i].iter().rev().any(|&byte| byte > 0)
                        || coordinate[0] >= 1;
                }
                Ordering::Less => {
                    // if coordinate >= p, then coordinate < p is false
                    return false;
                }
                Ordering::Equal => {}
            }
        }

        // The non least significant bytes are equal, so we need to
        // check the least significant byte: coordinate >= 1 and coordinate <= p-1
        // coordinate >= 1: coord_gte_zero || coordinate[0] >= 1
        // coordinate <= p-1: coordinate[0] <= p[0] - 1, which is coordinate[0] < p[0] when p[0] > 0
        // Special case: if p[0] == 0, then p-1 would have p[0] = 255 (due to borrow),
        // but since we're comparing equal non-LSB bytes, p[0] should never be 0 in practice
        (coord_gt_zero || coordinate[0] >= 1) && coordinate[0] < p[0]
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    /// Helper function to create a coordinate equal to 1 (valid lower bound)
    fn create_coordinate_one(curve_len: usize) -> Vec<u8> {
        let mut coord = vec![0u8; curve_len];
        coord[0] = 1; // LSB = 1, all other bytes = 0
        coord
    }

    /// Helper function to create a coordinate equal to 0 (invalid - below lower bound)
    fn create_coordinate_zero(curve_len: usize) -> Vec<u8> {
        vec![0u8; curve_len] // All bytes = 0
    }

    /// Helper function to create a coordinate equal to p-1 (valid upper bound)
    fn create_coordinate_p_minus_one(prime_p: &[u8]) -> Vec<u8> {
        let mut coord = prime_p.to_vec();
        // Subtract 1 from the little-endian number
        let mut borrow = 1u16;
        for byte in coord.iter_mut() {
            let temp = *byte as u16;
            if temp >= borrow {
                *byte = (temp - borrow) as u8;
                break; // No more borrow needed
            } else {
                *byte = (temp + 256 - borrow) as u8;
                borrow = 1;
            }
        }
        coord
    }

    /// Helper function to create a coordinate equal to p (invalid - at upper bound)
    fn create_coordinate_p(prime_p: &[u8]) -> Vec<u8> {
        prime_p.to_vec()
    }

    /// Helper function to create a coordinate greater than p (invalid - above upper bound)
    fn create_coordinate_greater_than_p(prime_p: &[u8]) -> Vec<u8> {
        let mut coord = prime_p.to_vec();
        // Add 1 to the little-endian number
        let mut carry = 1u16;
        for byte in coord.iter_mut() {
            let temp = *byte as u16 + carry;
            *byte = (temp & 0xff) as u8;
            carry = temp >> 8;
            if carry == 0 {
                break;
            }
        }
        coord
    }

    /// Helper function to create a random valid coordinate (somewhere in middle of range)
    fn create_coordinate_mid_range(curve_len: usize) -> Vec<u8> {
        let mut coord = vec![0x80u8; curve_len]; // Set all bytes to 0x80 for mid-range value
        coord[curve_len - 1] = 0x7f; // Ensure it's less than typical prime values
        coord
    }

    #[test]
    fn test_validate_p256_valid_coordinates() {
        let curve = EccCurve::P256;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 1 (valid lower bound)
        let x_one = create_coordinate_one(curve_len);
        let y_one = create_coordinate_one(curve_len);
        assert!(EccPublicKeyRangeValidation::validate(&x_one, &y_one, curve));

        // Test with coordinate = p-1 (valid upper bound)
        let x_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        let y_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        assert!(EccPublicKeyRangeValidation::validate(
            &x_p_minus_one,
            &y_p_minus_one,
            curve
        ));

        // Test with mid-range coordinates (valid)
        let x_mid = create_coordinate_mid_range(curve_len);
        let y_mid = create_coordinate_mid_range(curve_len);
        assert!(EccPublicKeyRangeValidation::validate(&x_mid, &y_mid, curve));

        // Test mixed valid coordinates
        assert!(EccPublicKeyRangeValidation::validate(
            &x_one,
            &y_p_minus_one,
            curve
        ));
        assert!(EccPublicKeyRangeValidation::validate(
            &x_p_minus_one,
            &x_one,
            curve
        ));
    }

    #[test]
    fn test_validate_p256_invalid_coordinates() {
        let curve = EccCurve::P256;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 0 (invalid - below lower bound)
        let x_zero = create_coordinate_zero(curve_len);
        let y_zero = create_coordinate_zero(curve_len);
        let x_valid = create_coordinate_one(curve_len);
        let y_valid = create_coordinate_one(curve_len);

        assert!(!EccPublicKeyRangeValidation::validate(
            &x_zero, &y_valid, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_valid, &y_zero, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_zero, &y_zero, curve
        ));

        // Test with coordinate = p (invalid - at upper bound)
        let x_p = create_coordinate_p(&prime.p[..curve_len]);
        let y_p = create_coordinate_p(&prime.p[..curve_len]);

        assert!(!EccPublicKeyRangeValidation::validate(
            &x_p, &y_valid, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_valid, &y_p, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(&x_p, &y_p, curve));

        // Test with coordinate > p (invalid - above upper bound)
        let x_greater = create_coordinate_greater_than_p(&prime.p[..curve_len]);
        let y_greater = create_coordinate_greater_than_p(&prime.p[..curve_len]);

        assert!(!EccPublicKeyRangeValidation::validate(
            &x_greater, &y_valid, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_valid, &y_greater, curve
        ));
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_greater, &y_greater, curve
        ));
    }

    #[test]
    fn test_validate_p384_valid_coordinates() {
        let curve = EccCurve::P384;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 1 (valid lower bound)
        let x_one = create_coordinate_one(curve_len);
        let y_one = create_coordinate_one(curve_len);
        assert!(EccPublicKeyRangeValidation::validate(&x_one, &y_one, curve));

        // Test with coordinate = p-1 (valid upper bound)
        let x_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        let y_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        assert!(EccPublicKeyRangeValidation::validate(
            &x_p_minus_one,
            &y_p_minus_one,
            curve
        ));

        // Test with mid-range coordinates (valid)
        let x_mid = create_coordinate_mid_range(curve_len);
        let y_mid = create_coordinate_mid_range(curve_len);
        assert!(EccPublicKeyRangeValidation::validate(&x_mid, &y_mid, curve));
    }

    #[test]
    fn test_validate_p384_invalid_coordinates() {
        let curve = EccCurve::P384;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 0 (invalid)
        let x_zero = create_coordinate_zero(curve_len);
        let y_valid = create_coordinate_one(curve_len);
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_zero, &y_valid, curve
        ));

        // Test with coordinate = p (invalid)
        let x_p = create_coordinate_p(&prime.p[..curve_len]);
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_p, &y_valid, curve
        ));

        // Test with coordinate > p (invalid)
        let x_greater = create_coordinate_greater_than_p(&prime.p[..curve_len]);
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_greater, &y_valid, curve
        ));
    }

    #[test]
    fn test_validate_p521_valid_coordinates() {
        let curve = EccCurve::P521;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 1 (valid lower bound)
        let x_one = create_coordinate_one(curve_len);
        let y_one = create_coordinate_one(curve_len);
        assert!(EccPublicKeyRangeValidation::validate(&x_one, &y_one, curve));

        // Test with coordinate = p-1 (valid upper bound)
        let x_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        let y_p_minus_one = create_coordinate_p_minus_one(&prime.p[..curve_len]);
        assert!(EccPublicKeyRangeValidation::validate(
            &x_p_minus_one,
            &y_p_minus_one,
            curve
        ));
    }

    #[test]
    fn test_validate_p521_invalid_coordinates() {
        let curve = EccCurve::P521;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());

        // Test with coordinate = 0 (invalid)
        let x_zero = create_coordinate_zero(curve_len);
        let y_valid = create_coordinate_one(curve_len);
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_zero, &y_valid, curve
        ));

        // Test with coordinate = p (invalid)
        let x_p = create_coordinate_p(&prime.p[..curve_len]);
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_p, &y_valid, curve
        ));
    }

    #[test]
    fn test_validate_p521_valid_coordinates_individual() {
        let curve = EccCurve::P521;

        let x: [u8; 66] = [
            0x01, 0x96, 0x7b, 0x85, 0xda, 0x42, 0x97, 0xa0, 0x79, 0xfd, 0x6f, 0x69, 0x1b, 0x2a,
            0xc2, 0x8a, 0x44, 0xba, 0xb8, 0x2b, 0x78, 0x9b, 0xed, 0x94, 0x12, 0x88, 0x5b, 0x18,
            0x46, 0xb0, 0xe8, 0x77, 0x4b, 0xa6, 0xe2, 0x3c, 0x0d, 0x72, 0x27, 0x2e, 0x14, 0xbe,
            0x17, 0xea, 0x64, 0x98, 0x2e, 0x31, 0x12, 0xf3, 0xb3, 0x43, 0xf7, 0x91, 0xd8, 0xec,
            0xb7, 0x6c, 0x54, 0x51, 0x89, 0xee, 0x51, 0x83, 0x45, 0x01,
        ];
        let y: [u8; 66] = [
            0x11, 0x98, 0x05, 0x14, 0x4b, 0x18, 0x5e, 0xff, 0x5e, 0x98, 0x10, 0x98, 0x60, 0x12,
            0xf2, 0x56, 0x7b, 0xac, 0x05, 0x0e, 0xc5, 0x94, 0xe0, 0xb4, 0x97, 0x72, 0x31, 0x3b,
            0x7c, 0x76, 0xe8, 0xee, 0x88, 0xaf, 0xc4, 0x47, 0x28, 0x12, 0xcc, 0xe3, 0xd2, 0x7e,
            0x6d, 0x7c, 0xa6, 0x5f, 0x96, 0x59, 0xe3, 0x9d, 0xea, 0xd0, 0x7e, 0x19, 0x82, 0xfe,
            0x53, 0x30, 0x39, 0x3e, 0xe0, 0xaa, 0x75, 0x95, 0xe7, 0x00,
        ];

        assert!(EccPublicKeyRangeValidation::validate(&x, &y, curve));
    }

    #[test]
    fn test_validate_length_mismatch() {
        let curve = EccCurve::P256;
        let curve_len = curve.len();

        let x_valid = create_coordinate_one(curve_len);
        let y_valid = create_coordinate_one(curve_len);

        // Test with wrong length x coordinate
        let x_wrong_len = vec![0u8; curve_len + 1];
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_wrong_len,
            &y_valid,
            curve
        ));

        // Test with wrong length y coordinate
        let y_wrong_len = vec![0u8; curve_len - 1];
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_valid,
            &y_wrong_len,
            curve
        ));

        // Test with both wrong lengths
        assert!(!EccPublicKeyRangeValidation::validate(
            &x_wrong_len,
            &y_wrong_len,
            curve
        ));
    }

    #[test]
    fn test_is_in_valid_range_edge_cases() {
        let curve = EccCurve::P256;
        let curve_len = curve.len();
        let prime = PkaEccCurve::get_prime(curve.into());
        let p = &prime.p[..curve_len];

        // Test coordinate = 1 (should be valid)
        let coord_one = create_coordinate_one(curve_len);
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_one, p
        ));

        // Test coordinate = 0 (should be invalid)
        let coord_zero = create_coordinate_zero(curve_len);
        assert!(!EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_zero,
            p
        ));

        // Test coordinate = p-1 (should be valid)
        let coord_p_minus_one = create_coordinate_p_minus_one(p);
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_p_minus_one,
            p
        ));

        // Test coordinate = p (should be invalid)
        let coord_p = create_coordinate_p(p);
        assert!(!EccPublicKeyRangeValidation::is_in_valid_range(&coord_p, p));

        // Test coordinate > p (should be invalid)
        let coord_greater = create_coordinate_greater_than_p(p);
        assert!(!EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_greater,
            p
        ));

        // Test length mismatch
        let coord_wrong_len = vec![1u8; curve_len + 1];
        assert!(!EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_wrong_len,
            p
        ));
    }

    #[test]
    fn test_boundary_values_with_real_primes() {
        // Test P256 specific boundary values
        let curve = EccCurve::P256;
        let prime = PkaEccCurve::get_prime(curve.into());
        let p256 = &prime.p[..curve.len()];

        // P256 prime: FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF
        // In little-endian: FF FF FF FF FF FF FF FF FF FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 FF FF FF FF

        // Test with a coordinate that has non-zero high-order bytes
        let mut coord_high_bytes = vec![0u8; curve.len()];
        coord_high_bytes[curve.len() - 1] = 0x01; // Set MSB
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_high_bytes,
            p256
        ));

        // Test with coordinate having pattern similar to P256 prime but smaller
        let mut coord_similar = p256.to_vec();
        coord_similar[0] = 0xfe; // Make it p-1 by reducing LSB
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_similar,
            p256
        ));

        // Test coordinate with byte values that could cause overflow in comparison
        let mut coord_overflow_test = vec![0xffu8; curve.len()];
        coord_overflow_test[curve.len() - 1] = 0x7f; // Ensure it's less than prime
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_overflow_test,
            p256
        ));
    }

    #[test]
    fn test_specific_prime_boundary_edge_cases() {
        // Test with P256 where prime has specific pattern:
        // High bytes = FF, then 00 bytes, then 01, then FF again
        let curve = EccCurve::P256;
        let prime = PkaEccCurve::get_prime(curve.into());
        let p256 = &prime.p[..curve.len()];

        // Create coordinate that matches prime except for one byte
        let mut coord_almost_p = p256.to_vec();
        coord_almost_p[12] = 0x01; // Change one of the 0x00 bytes to 0x01, making it > p
        assert!(!EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_almost_p,
            p256
        ));

        // Create coordinate that exactly equals p-1
        let coord_exactly_p_minus_1 = create_coordinate_p_minus_one(p256);
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_exactly_p_minus_1,
            p256
        ));

        // Create coordinate where only LSB differs from 0
        let mut coord_lsb_only = vec![0u8; curve.len()];
        coord_lsb_only[0] = 0x01;
        assert!(EccPublicKeyRangeValidation::is_in_valid_range(
            &coord_lsb_only,
            p256
        ));
    }
}
