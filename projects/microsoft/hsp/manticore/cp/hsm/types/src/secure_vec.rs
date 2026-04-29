// Copyright (c) Microsoft Corporation. All rights reserved.

extern crate alloc;

use core::ops::{Deref, DerefMut};

use alloc::vec::Vec;
use zeroize::Zeroize;

#[derive(Clone, PartialEq, Eq)]
pub struct SecureByteVec {
    inner: Vec<u8>,
}

impl SecureByteVec {
    /// Creates a new `SecureByteVec` with the default capacity.
    pub fn new() -> Self {
        SecureByteVec { inner: Vec::new() }
    }

    /// Wrapper for `Vec::with_capacity` that creates a new `SecureByteVec` with the specified capacity.
    pub fn with_capacity(capacity: usize) -> Self {
        SecureByteVec {
            inner: Vec::with_capacity(capacity),
        }
    }

    pub fn zeroed(len: usize) -> Self {
        Self {
            inner: alloc::vec![0u8; len],
        }
    }

    /// Wrapper for `Vec::push` that adds a value to the vector.
    pub fn push(&mut self, value: u8) {
        self.inner.push(value);
    }

    /// Wrapper for `Vec::as_slice` that converts the vector to a slice.
    pub fn as_slice(&self) -> &[u8] {
        self.inner.as_slice()
    }

    /// Wrapper for `Vec::as_mut_slice` that converts the vector to a mutable slice.
    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        self.inner.as_mut_slice()
    }

    /// Wrapper for `Vec::clear` that returns the length of the vector.
    pub fn clear(&mut self) {
        self.inner.fill(0);
        self.inner.clear();
    }

    /// Wrapper for `Vec::len` that returns the length of the vector.
    pub fn len(&self) -> usize {
        self.inner.len()
    }

    /// Wrapper for `Vec::is_empty` that checks if the vector is empty.
    pub fn is_empty(&self) -> bool {
        self.inner.is_empty()
    }
}

impl Default for SecureByteVec {
    fn default() -> Self {
        Self::new()
    }
}

/// Dref trait implementation for `SecureByteVec`.
impl Deref for SecureByteVec {
    type Target = Vec<u8>;

    fn deref(&self) -> &Self::Target {
        &self.inner
    }
}

/// DrefMut trait implementation for `SecureByteVec`.
impl DerefMut for SecureByteVec {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.inner
    }
}

/// Drop trait implementation for `SecureByteVec`.
impl Drop for SecureByteVec {
    fn drop(&mut self) {
        self.inner.zeroize();
    }
}

/// Conversion trait implementation for `SecureByteVec` from a slice.
impl From<&[u8]> for SecureByteVec {
    fn from(slice: &[u8]) -> Self {
        SecureByteVec {
            inner: slice.to_vec(),
        }
    }
}

/// Conversion trait implementation for `SecureByteVec` from a mutable slice.
impl From<Vec<u8>> for SecureByteVec {
    fn from(vec: Vec<u8>) -> Self {
        SecureByteVec { inner: vec }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_secure_byte_vec() {
        let mut secure_vec = SecureByteVec::new();
        secure_vec.push(1);
        secure_vec.push(2);
        secure_vec.push(3);

        assert_eq!(secure_vec.len(), 3);
        assert_eq!(secure_vec.as_slice(), &[1, 2, 3]);

        secure_vec.clear();
        assert_eq!(secure_vec.len(), 0);
        assert_eq!(secure_vec.as_slice(), &[]);
    }

    #[test]
    fn test_secure_byte_vec_default() {
        let secure_vec: SecureByteVec = Default::default();
        assert_eq!(secure_vec.len(), 0);
        assert_eq!(secure_vec.as_slice(), &[]);
    }

    #[test]
    fn test_secure_byte_vec_with_capacity() {
        let mut secure_vec = SecureByteVec::with_capacity(10);
        secure_vec.push(1);
        secure_vec.push(2);
        secure_vec.push(3);

        assert_eq!(secure_vec.len(), 3);
        assert_eq!(secure_vec.as_slice(), &[1, 2, 3]);

        secure_vec.clear();
        assert_eq!(secure_vec.len(), 0);
        assert_eq!(secure_vec.as_slice(), &[]);
    }

    #[test]
    fn test_secure_byte_vec_zeroed() {
        let mut secure_vec = SecureByteVec::zeroed(5);
        assert_eq!(secure_vec.len(), 5);
        assert_eq!(secure_vec.as_slice(), &[0, 0, 0, 0, 0]);

        secure_vec.push(1);
        assert_eq!(secure_vec.len(), 6);
        assert_eq!(secure_vec.as_slice(), &[0, 0, 0, 0, 0, 1]);

        secure_vec.clear();
        assert_eq!(secure_vec.len(), 0);
    }

    #[test]
    fn test_secure_byte_vec_from_slice() {
        let slice: &[u8] = &[1, 2, 3];
        let secure_vec: SecureByteVec = SecureByteVec::from(slice);
        assert_eq!(secure_vec.len(), 3);
        assert_eq!(secure_vec.as_slice(), &[1, 2, 3]);
    }

    #[test]
    fn test_secure_byte_vec_from_vec() {
        let vec: Vec<u8> = alloc::vec![1, 2, 3];
        let secure_vec: SecureByteVec = SecureByteVec::from(vec);
        assert_eq!(secure_vec.len(), 3);
        assert_eq!(secure_vec.as_slice(), &[1, 2, 3]);
    }

    #[test]
    fn test_secure_byte_vec_deref() {
        let mut secure_vec = SecureByteVec::new();
        secure_vec.push(1);
        secure_vec.push(2);
        secure_vec.push(3);

        let slice: &[u8] = &secure_vec;
        assert_eq!(slice, &[1, 2, 3]);

        let mutable_slice: &mut [u8] = &mut secure_vec;
        mutable_slice[0] = 4;
        assert_eq!(secure_vec.as_slice(), &[4, 2, 3]);
    }

    #[test]
    fn test_secure_byte_vec_len() {
        let secure_vec = SecureByteVec::new();
        assert_eq!(secure_vec.len(), 0);

        let mut secure_vec = SecureByteVec::zeroed(5);
        assert_eq!(secure_vec.len(), 5);

        secure_vec.push(1);
        assert_eq!(secure_vec.len(), 6);

        secure_vec.clear();
        assert_eq!(secure_vec.len(), 0);
    }

    #[test]
    fn test_secure_byte_vec_is_empty() {
        let secure_vec = SecureByteVec::new();
        assert!(secure_vec.is_empty());

        let mut secure_vec = SecureByteVec::zeroed(5);
        assert!(!secure_vec.is_empty());

        secure_vec.clear();
        assert!(secure_vec.is_empty());
    }
}
