#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Install build dependencies for Manticore firmware.
# Supports x86_64 and ARM64 (aarch64) hosts.
#
# Usage: ./tools/install_dependencies.sh
#
# NOTE: This script must NOT be run with sudo. It will prompt for sudo
# only for apt package installation. Toolchains are downloaded to
# ~/build_tools (or $BUILD_TOOLS if set) under the current user.
set -e

ARCH=$(uname -m)
echo "Host architecture: $ARCH"

# ── Step 1: System packages (needs sudo) ──
echo ""
echo "=== Installing system packages ==="
sudo apt update -qq
sudo apt install -y build-essential cmake ninja-build git python3 python3-pip python-is-python3 curl tar bzip2 mono-complete

# ── Step 2: Toolchain downloads (as current user) ──
if [ -z "$BUILD_TOOLS" ]; then
    BUILD_TOOLS="$HOME/build_tools"
fi
echo ""
echo "BUILD_TOOLS=$BUILD_TOOLS"
mkdir -p "$BUILD_TOOLS"

download_file() {
    local url=$1
    local filename=$(basename "$url")
    if [ -f "$BUILD_TOOLS/$filename" ]; then
        echo "  Already downloaded: $filename"
    else
        echo "  Downloading: $filename"
        curl -fSL -o "$BUILD_TOOLS/$filename" "$url"
    fi
}

echo ""
echo "=== RISC-V toolchain (SP build) ==="
if [ "$ARCH" = "aarch64" ]; then
    echo "  ARM64 host: using apt package + picolibc"
    sudo apt install -y gcc-riscv64-unknown-elf picolibc-riscv64-unknown-elf
    # Create nano.specs/nosys.specs compatibility shims for picolibc
    GCC_DIR=$(riscv64-unknown-elf-gcc -print-search-dirs | grep install | sed 's/install: //')
    if [ ! -f "${GCC_DIR}nano.specs" ]; then
        echo "  Creating nano.specs compatibility shim..."
        sudo bash -c "echo '%rename link picolibc_link
%rename cpp picolibc_cpp
*cpp:
-isystem /usr/lib/picolibc/riscv64-unknown-elf/include %(picolibc_cpp)
*link:
-L/usr/lib/picolibc/riscv64-unknown-elf/lib/%M %(picolibc_link)
*lib:
--start-group -lc --end-group' > '${GCC_DIR}nano.specs'"
        sudo touch "${GCC_DIR}nosys.specs"
    fi
else
    cd "$BUILD_TOOLS"
    download_file https://static.dev.sifive.com/dev-tools/freedom-tools/v2020.12/riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz
    [ ! -d riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14 ] &&         tar xzf riscv64-unknown-elf-toolchain-10.2.0-2020.12.8-x86_64-linux-ubuntu14.tar.gz
    cd -
fi

echo ""
echo "=== ARM GCC 9 (CP / 1SP build) ==="
cd "$BUILD_TOOLS"
if [ "$ARCH" = "aarch64" ]; then
    download_file https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-aarch64-linux.tar.bz2
else
    download_file https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major-x86_64-linux.tar.bz2
fi
[ ! -d gcc-arm-none-eabi-9-2019-q4-major ] && tar xjf gcc-arm-none-eabi-9-2019-q4-major-*.tar.bz2

echo ""
echo "=== ARM GCC 7 (FP build) ==="
if [ "$ARCH" = "aarch64" ]; then
    echo "  ARM64 host: using apt package"
    sudo apt install -y gcc-arm-none-eabi
else
    download_file https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/7-2017q4/gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
    [ ! -d gcc-arm-none-eabi-7-2017-q4-major ] && tar xjf gcc-arm-none-eabi-7-2017-q4-major-linux.tar.bz2
fi
cd -

# ── Step 3: Rust toolchain (as current user) ──
echo ""
echo "=== Rust toolchain (CP build) ==="
if ! command -v cargo &> /dev/null; then
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
fi
rustup target add thumbv7em-none-eabihf
cargo install cargo-binutils 2>/dev/null || true
echo "  Rust: $(cargo --version)"

echo ""
echo "================================================"
echo "  All dependencies installed."
echo ""
echo "  Next steps:"
echo "    source ./tools/menv.sh"
echo "    ./make_manticore.sh"
echo "================================================"
