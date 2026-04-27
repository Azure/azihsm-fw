// Copyright (c) Microsoft Corporation. All rights reserved.

#[test]
fn tests() {
    let t = trybuild::TestCases::new();
    t.compile_fail("tests/compile_tests/01-union.rs");
    t.compile_fail("tests/compile_tests/02-struct-invalid.rs");
    t.compile_fail("tests/compile_tests/03-enum-invalid.rs");
}
