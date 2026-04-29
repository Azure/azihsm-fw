// Copyright (c) Microsoft Corporation. All rights reserved.

cfg_if::cfg_if! {
    if #[cfg(not(feature = "std"))] {
        use std::path::Path;
        use std::path::PathBuf;

        // Gets the project root directory path.
        fn project_root() -> PathBuf {
            Path::new(&env!("CARGO_MANIFEST_DIR"))
                .ancestors()
                .nth(1)
                .unwrap()
                .to_path_buf()
        }
    }
}

fn main() {
    #[cfg(not(feature = "std"))]
    {
        let out = &std::path::PathBuf::from(std::env::var_os("OUT_DIR").unwrap());

        // Put `memory.x` in our output directory and ensure it's on the linker search path.
        std::io::Write::write_all(
            &mut std::fs::File::create(out.join("memory.x")).unwrap(),
            include_bytes!("memory.x"),
        )
        .unwrap();

        let device_x_path = project_root()
            .join("exception-handlers")
            .join("src")
            .join("device.x");

        let device_x_content = std::fs::read(&device_x_path).unwrap_or_else(|e| {
            panic!(
                "Failed to read device.x from {}: {}",
                device_x_path.display(),
                e
            )
        });

        // Put `device.x` in our output directory and ensure it's on the linker search path.
        std::io::Write::write_all(
            &mut std::fs::File::create(out.join("device.x")).unwrap(),
            &device_x_content,
        )
        .unwrap();

        // Set the linker search path to `out` directory.
        println!("cargo:rustc-link-search={}", out.display());

        // By default, Cargo will re-run a build script whenever
        // any file in the project changes. By specifying `memory.x`
        // here, we ensure the build script is only re-run when
        // `memory.x` is changed.
        println!("cargo:rerun-if-changed=memory.x");

        // Specify linker arguments.

        // `--nmagic` is required if memory section addresses are not aligned to 0x10000,
        // for example the FLASH and RAM sections in your `memory.x`.
        // See https://github.com/rust-embedded/cortex-m-quickstart/pull/95
        println!("cargo:rustc-link-arg=--nmagic");

        // Set the linker script to the one provided by cortex-m-rt.
        println!("cargo:rustc-link-arg=-Tlink.x");
    }
}
