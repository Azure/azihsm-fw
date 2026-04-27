// Copyright (c) Microsoft Corporation. All rights reserved.

#[allow(unused)]
use std::env;
#[allow(unused)]
use std::fs::File;
#[allow(unused)]
use std::io::Write;
#[allow(unused)]
use std::path::PathBuf;

fn main() {
    #[cfg(target_arch = "arm")]
    {
        // Put `memory.x` in our output directory and ensure it's
        // on the linker search path.
        let out = &PathBuf::from(env::var_os("OUT_DIR").unwrap());

        File::create(out.join("memory.x"))
            .unwrap()
            .write_all(include_bytes!("../../../../test/harness/memory.x"))
            .unwrap();

        File::create(out.join("device.x"))
            .unwrap()
            .write_all(include_bytes!("../../../../test/harness/device.x"))
            .unwrap();
        println!("cargo:rustc-link-search={}", out.display());
        println!("cargo:rerun-if-changed=memory.x");
        println!("cargo:rustc-link-arg=--nmagic");
        println!("cargo:rustc-link-arg=-Tlink.x");
    }
}
