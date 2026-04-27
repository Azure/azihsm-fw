// Copyright (c) Microsoft Corporation. All rights reserved.

pub(crate) struct KeyNumber(pub(crate) u16);

impl KeyNumber {
    pub(crate) fn new(table_index: u8, entry_index: u8) -> Self {
        Self(((table_index as u16) << 8) | (entry_index as u16))
    }

    pub(crate) fn table(&self) -> u8 {
        (self.0 >> 8) as u8
    }

    pub(crate) fn entry(&self) -> u8 {
        (self.0 & 0xff) as u8
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_key_num_to_table_index() {
        assert_eq!(KeyNumber(0x0000).table(), 0x00);
        assert_eq!(KeyNumber(0x0100).table(), 0x01);
        assert_eq!(KeyNumber(0x0200).table(), 0x02);
        assert_eq!(KeyNumber(0x0300).table(), 0x03);
        assert_eq!(KeyNumber(0x0400).table(), 0x04);
        assert_eq!(KeyNumber(0x0500).table(), 0x05);
        assert_eq!(KeyNumber(0x0600).table(), 0x06);
        assert_eq!(KeyNumber(0x0700).table(), 0x07);
        assert_eq!(KeyNumber(0x0800).table(), 0x08);
        assert_eq!(KeyNumber(0x0900).table(), 0x09);
        assert_eq!(KeyNumber(0x0a00).table(), 0x0a);
        assert_eq!(KeyNumber(0x0b00).table(), 0x0b);
        assert_eq!(KeyNumber(0x0c00).table(), 0x0c);
        assert_eq!(KeyNumber(0x0d00).table(), 0x0d);
        assert_eq!(KeyNumber(0x0e00).table(), 0x0e);
        assert_eq!(KeyNumber(0x0f00).table(), 0x0f);
        assert_eq!(KeyNumber(0x1000).table(), 0x10);
        assert_eq!(KeyNumber(0x1100).table(), 0x11);
        assert_eq!(KeyNumber(0x1200).table(), 0x12);
        assert_eq!(KeyNumber(0x1300).table(), 0x13);
        assert_eq!(KeyNumber(0x1400).table(), 0x14);
        assert_eq!(KeyNumber(0x1500).table(), 0x15);
    }

    #[test]
    fn test_key_num_to_entry_index() {
        assert_eq!(KeyNumber(0x0000).entry(), 0x00);
        assert_eq!(KeyNumber(0x0001).entry(), 0x01);
        assert_eq!(KeyNumber(0x0002).entry(), 0x02);
        assert_eq!(KeyNumber(0x0003).entry(), 0x03);
        assert_eq!(KeyNumber(0x0004).entry(), 0x04);
        assert_eq!(KeyNumber(0x0005).entry(), 0x05);
        assert_eq!(KeyNumber(0x0006).entry(), 0x06);
        assert_eq!(KeyNumber(0x0007).entry(), 0x07);
        assert_eq!(KeyNumber(0x0008).entry(), 0x08);
        assert_eq!(KeyNumber(0x0009).entry(), 0x09);
        assert_eq!(KeyNumber(0x000a).entry(), 0x0a);
        assert_eq!(KeyNumber(0x000b).entry(), 0x0b);
        assert_eq!(KeyNumber(0x000c).entry(), 0x0c);
        assert_eq!(KeyNumber(0x000d).entry(), 0x0d);
        assert_eq!(KeyNumber(0x000e).entry(), 0x0e);
        assert_eq!(KeyNumber(0x000f).entry(), 0x0f);
        assert_eq!(KeyNumber(0x0010).entry(), 0x10);
        assert_eq!(KeyNumber(0x0011).entry(), 0x11);
        assert_eq!(KeyNumber(0x0012).entry(), 0x12);
        assert_eq!(KeyNumber(0x0013).entry(), 0x13);
        assert_eq!(KeyNumber(0x0014).entry(), 0x14);
        assert_eq!(KeyNumber(0x0015).entry(), 0x15);
    }

    #[test]
    fn test_table_index_and_entry_index_to_key_num() {
        assert_eq!(KeyNumber::new(0x00, 0x00).0, 0x0000);
        assert_eq!(KeyNumber::new(0x00, 0x01).0, 0x0001);
        assert_eq!(KeyNumber::new(0x01, 0x02).0, 0x0102);
        assert_eq!(KeyNumber::new(0x01, 0x03).0, 0x0103);
        assert_eq!(KeyNumber::new(0x02, 0x04).0, 0x0204);
        assert_eq!(KeyNumber::new(0x02, 0x05).0, 0x0205);
        assert_eq!(KeyNumber::new(0x10, 0x10).0, 0x1010);
        assert_eq!(KeyNumber::new(0x10, 0x11).0, 0x1011);
        assert_eq!(KeyNumber::new(0x21, 0x12).0, 0x2112);
        assert_eq!(KeyNumber::new(0x21, 0x13).0, 0x2113);
        assert_eq!(KeyNumber::new(0x38, 0x14).0, 0x3814);
        assert_eq!(KeyNumber::new(0x38, 0x15).0, 0x3815);
    }
}
