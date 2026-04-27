// Copyright (c) Microsoft Corporation. All rights reserved.

use mcr_profiler::CallTreeNode;

#[test]
fn test_call_tree() {
    let root = CallTreeNode::new("root", 1);

    let child1 = root.begin_child_call("child1", 5);
    child1.end_call(10);

    let child2 = root.begin_child_call("child2", 10);
    child2.end_call(20);

    root.begin_child_call("child1", 20);
    child1.end_call(30);

    let child2_log = child2.get_log(30);
    assert_eq!(child2_log, "child2:D:1/10(10,10);");

    root.begin_child_call("child2", 40);

    let child2_log = child2.get_log(55);
    assert_eq!(child2_log, "child2:R:2/25(10,15);");

    let child1_log = child1.get_log(55);
    assert_eq!(child1_log, "child1:D:2/15(5,10);");

    let root_log = root.get_log(55);
    assert_eq!(
        root_log,
        "root:R:1/54(54,54){child1:D:2/15(5,10);child2:R:2/25(10,15);};"
    );

    let child2_1 = child2.begin_child_call("child2_1", 60);
    child2_1.end_call(70);

    let root_log = root.get_log(85);
    assert_eq!(
        root_log,
        "root:R:1/84(84,84){child1:D:2/15(5,10);child2:R:2/55(10,45){child2_1:D:1/10(10,10);};};"
    );
}
