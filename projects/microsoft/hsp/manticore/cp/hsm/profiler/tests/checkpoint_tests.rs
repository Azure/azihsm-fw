// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_profiler::Checkpoint;

#[test]
fn test_checkpoint_rev() {
    let cp_start = Checkpoint::start("start", 5);
    let log = cp_start.get_log(20);
    assert_eq!(log, "15");

    let cp_foo = cp_start.record("foo", 10);
    let log = cp_foo.get_log(30);
    assert_eq!(log, "5:foo:20");

    let label2 = "barbar";
    let cp_barbar = cp_foo.record(label2, 20);

    let log = cp_barbar.get_log(40);
    assert_eq!(log, "5:foo:10:barbar:20");
}
