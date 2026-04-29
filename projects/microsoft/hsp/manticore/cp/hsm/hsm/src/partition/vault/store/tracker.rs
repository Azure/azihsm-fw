// Copyright (c) Microsoft Corporation. All rights reserved.

use super::*;

pub(super) struct UsedBlocksTracker {
    base: usize,
}

impl UsedBlocksTracker {
    pub(super) fn new(base: usize) -> Self {
        UsedBlocksTracker { base }
    }

    pub(super) fn used_blocks(&self) -> &[u32; USED_BLOCK_TRACKING_SIZE_WORDS] {
        unsafe { &*((self.base) as *const [u32; USED_BLOCK_TRACKING_SIZE_WORDS]) }
    }

    pub(super) fn used_blocks_mut(&mut self) -> &mut [u32; USED_BLOCK_TRACKING_SIZE_WORDS] {
        unsafe { &mut *((self.base) as *mut [u32; USED_BLOCK_TRACKING_SIZE_WORDS]) }
    }

    pub(super) fn get(&self, i: usize) -> Option<bool> {
        if i >= USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize {
            return None;
        }

        if i >= BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT {
            return None;
        }

        let word = i / u32::BITS as usize;
        let bit = i % u32::BITS as usize;
        let word = self.used_blocks()[word];
        Some((word & (1 << bit)) != 0)
    }

    pub(super) fn set(&mut self, i: usize, x: bool) {
        if i >= USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize {
            return;
        }

        if i >= BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT {
            return;
        }

        let word = i / u32::BITS as usize;
        let bit = i % u32::BITS as usize;
        let word = &mut self.used_blocks_mut()[word];
        if x {
            *word |= 1 << bit;
        } else {
            *word &= !(1 << bit);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_used_block_tracker_new() {
        let used_block_tracking_memory = [0u32; USED_BLOCK_TRACKING_SIZE_WORDS];
        let used_block_tracker =
            UsedBlocksTracker::new(used_block_tracking_memory.as_ptr() as usize);

        assert_eq!(used_block_tracker.get(0), Some(false));
        assert_eq!(used_block_tracker.get(1), Some(false));
        assert_eq!(used_block_tracker.get(10), Some(false));
        assert_eq!(used_block_tracker.get(100), Some(false));
        assert_eq!(used_block_tracker.get(1000), Some(false));
        assert_eq!(used_block_tracker.get(10000), None);
    }

    #[test]
    fn test_used_block_tracker_get() {
        let used_block_tracking_memory = [0u32; USED_BLOCK_TRACKING_SIZE_WORDS];
        let used_block_tracker =
            UsedBlocksTracker::new(used_block_tracking_memory.as_ptr() as usize);

        assert_eq!(used_block_tracker.get(0), Some(false));
        assert_eq!(used_block_tracker.get(1), Some(false));
        assert_eq!(used_block_tracker.get(10), Some(false));
        assert_eq!(used_block_tracker.get(100), Some(false));
        assert_eq!(used_block_tracker.get(1000), Some(false));
        assert_eq!(
            used_block_tracker.get(USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize),
            None
        );
        assert_eq!(
            used_block_tracker.get(100 + USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize),
            None
        );
        assert_eq!(
            used_block_tracker.get(BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT),
            None
        );
        assert_eq!(
            used_block_tracker.get(10 + BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT),
            None
        );
        assert_eq!(used_block_tracker.get(10000), None);
    }

    #[test]
    fn test_used_block_tracker_set() {
        let used_block_tracking_memory = [0u32; USED_BLOCK_TRACKING_SIZE_WORDS];
        let mut used_block_tracker =
            UsedBlocksTracker::new(used_block_tracking_memory.as_ptr() as usize);

        used_block_tracker.set(0, true);
        used_block_tracker.set(1, true);
        used_block_tracker.set(10, true);
        used_block_tracker.set(100, true);
        used_block_tracker.set(1000, true);

        assert_eq!(used_block_tracker.get(0), Some(true));
        assert_eq!(used_block_tracker.get(1), Some(true));
        assert_eq!(used_block_tracker.get(10), Some(true));
        assert_eq!(used_block_tracker.get(100), Some(true));
        assert_eq!(used_block_tracker.get(1000), Some(true));

        used_block_tracker.set(0, false);
        used_block_tracker.set(1, false);
        used_block_tracker.set(10, false);
        used_block_tracker.set(100, false);
        used_block_tracker.set(1000, false);

        assert_eq!(used_block_tracker.get(0), Some(false));
        assert_eq!(used_block_tracker.get(1), Some(false));
        assert_eq!(used_block_tracker.get(10), Some(false));
        assert_eq!(used_block_tracker.get(100), Some(false));
        assert_eq!(used_block_tracker.get(1000), Some(false));

        used_block_tracker.set(USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize, true);
        assert_eq!(
            used_block_tracker.get(USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize),
            None
        );

        used_block_tracker.set(
            100 + USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize,
            true,
        );
        assert_eq!(
            used_block_tracker.get(100 + USED_BLOCK_TRACKING_SIZE_WORDS * u32::BITS as usize),
            None
        );

        used_block_tracker.set(BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT, true);
        assert_eq!(
            used_block_tracker.get(BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT),
            None
        );

        used_block_tracker.set(
            10 + BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT,
            true,
        );
        assert_eq!(
            used_block_tracker.get(10 + BLOB_MEMORY_SIZE_BYTES / ENTRY_BLOB_BLOCK_ALIGNMENT),
            None
        );

        used_block_tracker.set(10000, true);
        assert_eq!(used_block_tracker.get(10000), None);
    }
}
