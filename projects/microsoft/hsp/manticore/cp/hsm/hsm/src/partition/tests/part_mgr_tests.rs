// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_types::*;

use crate::mock::MockEnv;
use crate::mock::MockPartition;
use crate::partition::HsmPartitionMgr;

#[test]
fn test_clone() {
    let part_mgr = HsmPartitionMgr::<MockEnv>::new(mock_partition_creator_with_no_expectation);
    let _ = part_mgr.clone();
}

#[test]
fn test_partition() {
    let part_mgr = HsmPartitionMgr::<MockEnv>::new(mock_partition_creator);

    for pfn in PcieFunction::iter() {
        part_mgr.partition(pfn);
    }
}

#[test]
fn test_partition_shutdown() {
    let part_mgr = HsmPartitionMgr::<MockEnv>::new(mock_partition_with_store_data_expectation);

    part_mgr.prepare_for_shutdown();
}

fn mock_partition_creator(_pfn: PcieFunction) -> MockPartition {
    let mut partition = MockPartition::new();
    partition
        .expect_clone()
        .once()
        .returning(MockPartition::new);

    partition
}

fn mock_partition_creator_with_no_expectation(_pfn: PcieFunction) -> MockPartition {
    MockPartition::new()
}

fn mock_partition_with_store_data_expectation(_pfn: PcieFunction) -> MockPartition {
    let mut partition = MockPartition::new();
    partition.expect_store_data().once().returning(|| ());

    partition
}
