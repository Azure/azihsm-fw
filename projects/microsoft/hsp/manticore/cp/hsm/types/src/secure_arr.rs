// Copyright (c) Microsoft Corporation. All rights reserved.

use core::ops::{Deref, DerefMut};

use zeroize::Zeroize;

/// A secure, fixed-size byte array that zeroizes on drop.
#[derive(Clone, PartialEq)]
pub struct SecureByteArray<const N: usize> {
    data: [u8; N],
}

impl<const N: usize> SecureByteArray<N> {
    /// Create a new SecureByteArray from a [u8; N]
    pub fn new(data: [u8; N]) -> Self {
        Self { data }
    }

    /// Access the contents as an immutable slice
    pub fn as_slice(&self) -> &[u8] {
        &self.data
    }

    /// Access the contents as a mutable slice
    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        &mut self.data
    }

    /// Consume and return the inner array
    pub fn into_inner(self) -> [u8; N] {
        self.data
    }
}

impl<const N: usize> Deref for SecureByteArray<N> {
    type Target = [u8; N];

    fn deref(&self) -> &Self::Target {
        &self.data
    }
}

impl<const N: usize> DerefMut for SecureByteArray<N> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.data
    }
}

impl<const N: usize> From<[u8; N]> for SecureByteArray<N> {
    fn from(data: [u8; N]) -> Self {
        Self::new(data)
    }
}

impl<const N: usize> Drop for SecureByteArray<N> {
    fn drop(&mut self) {
        self.data.zeroize(); // securely wipe memory
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_secure_byte_array() {
        let data = [1, 2, 3, 4, 5];
        let secure_array = SecureByteArray::new(data);
        assert_eq!(secure_array.as_slice(), &[1, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_as_slice() {
        let data = [1, 2, 3, 4, 5];
        let mut secure_array = SecureByteArray::new(data);
        assert_eq!(secure_array.as_slice(), &[1, 2, 3, 4, 5]);

        secure_array.as_mut_slice()[0] = 10;
        assert_eq!(secure_array.as_slice(), &[10, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_as_mut_slice() {
        let data = [1, 2, 3, 4, 5];
        let mut secure_array = SecureByteArray::new(data);
        assert_eq!(secure_array.as_mut_slice(), &[1, 2, 3, 4, 5]);

        secure_array.as_mut_slice()[0] = 10;
        assert_eq!(secure_array.as_mut_slice(), &[10, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_deref() {
        let data = [1, 2, 3, 4, 5];
        let secure_array = SecureByteArray::new(data);
        assert_eq!(*secure_array, [1, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_deref_mut() {
        let data = [1, 2, 3, 4, 5];
        let mut secure_array = SecureByteArray::new(data);
        secure_array[0] = 10;
        assert_eq!(*secure_array, [10, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_into_inner() {
        let data = [1, 2, 3, 4, 5];
        let secure_array = SecureByteArray::new(data);
        let inner_array = secure_array.into_inner();
        assert_eq!(inner_array, [1, 2, 3, 4, 5]);
    }

    #[test]
    fn test_secure_byte_array_from() {
        let data = [1, 2, 3, 4, 5];
        let secure_array: SecureByteArray<5> = SecureByteArray::from(data);
        assert_eq!(secure_array.as_slice(), &[1, 2, 3, 4, 5]);
    }
}
