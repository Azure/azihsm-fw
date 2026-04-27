#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Source this script to add build toolchains to PATH.
# Usage: source ./tools/menv.sh

ARCH=$(uname -m)

if [ -z "$BUILD_TOOLS" ]; then
    BUILD_TOOLS=$(realpath ~/build_tools)
fi

# RISC-V compiler (SP build)
if [ "$ARCH" = "aarch64" ]; then
    # On ARM64, riscv gcc is installed via apt (already in PATH)
    command -v riscv64-unknown-elf-gcc &>/dev/null && echo "RISC-V: $(riscv64-unknown-elf-gcc --version | head -1)"
else
    RV_DIR=$BUILD_TOOLS/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14/bin
    if [ -d "$RV_DIR" ]; then
        export PATH=$PATH:$RV_DIR
        echo "RISC-V: $(riscv64-unknown-elf-gcc --version | head -1)"
    fi
fi

# ARM GCC 9 (CP / 1SP build)
ARM9=$BUILD_TOOLS/gcc-arm-none-eabi-9-2019-q4-major/bin
if [ -d "$ARM9" ]; then
    export PATH=$PATH:$ARM9
    echo "ARM GCC 9: $(arm-none-eabi-gcc --version | head -1)"
fi

# ARM GCC 7 (FP build)
if [ "$ARCH" != "aarch64" ]; then
    ARM7=$BUILD_TOOLS/gcc-arm-none-eabi-7-2017-q4-major/bin
    if [ -d "$ARM7" ]; then
        export PATH=$PATH:$ARM7
    fi
fi

# Rust
[ -f "$HOME/.cargo/env" ] && source "$HOME/.cargo/env"

echo "Build environment ready."
