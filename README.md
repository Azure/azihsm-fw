# Azure Integrated HSM Firmware

This repository contains the open-source firmware for
[Azure Integrated HSM](https://techcommunity.microsoft.com/blog/azureinfrastructureblog/securing-azure-infrastructure-with-silicon-innovation/4293834),
Microsoft's dedicated Hardware Security Module (HSM) designed to be integrated
directly into every new server in Microsoft's datacenters.

## About Azure Integrated HSM

Azure Integrated HSM is a custom security chip that strengthens key protection
by enabling the use of encryption and signing keys while they remain within the
bounds of a hardware security module — without incurring the typical network
access latencies of traditional cloud HSM services.

**Key capabilities:**

- **FIPS 140-3 Level 3** — Designed to meet the Federal Information Processing
  Standards (FIPS) 140-3 Level 3 Security Requirements for Cryptographic Modules,
  providing strong physical and logical tamper protection and detection.
- **Locally deployed with minimum latency** — Dedicated hardware cryptographic
  accelerators perform encryption, decryption, signing, and verification
  operations locally on the server, eliminating network round-trip latency to
  remote HSM services.
- **Per-workload HSM partitions** — Hardware-isolated partitions allow only
  oracle access to keys from the workload environment, supporting both
  confidential and general-purpose virtual machines and containers.
- **Secure by design** — Keys remain isolated from all software, including both
  guest and host software, within dedicated hardware. Azure Integrated HSM will
  be installed in every new server in Microsoft's datacenters.

Azure Integrated HSM eliminates the classic tradeoff between increased network
round-trip latency to remote HSM services and the reduced security of releasing
keys from those services. As a server-local HSM that securely binds to workload
environments, it provides industry-leading in-use key protection without latency
drawbacks.

For more details, see the
[announcement blog post](https://techcommunity.microsoft.com/blog/azureinfrastructureblog/securing-azure-infrastructure-with-silicon-innovation/4293834).

## Firmware Architecture

This firmware is based on
[Project Cerberus](https://github.com/opencomputeproject/Project_Olympus/tree/master/Project_Cerberus),
a hardware root of trust (RoT) for server platforms that provides secure boot
enforcement and firmware attestation capabilities.

The firmware consists of four processor components:

| Component | Processor | Language | Description |
|-----------|-----------|----------|-------------|
| **SP** (Security Processor) | RISC-V | C | Root of Trust Firmware, secure boot, attestation, SPDM |
| **CP** (Control/Crypto Processor) | ARM Cortex-M7 | Rust | Cryptographic operations, key management, NVMe admin |
| **FP** (Fast-Path Processor) | ARM Cortex-M (3 CPUs) | C | NVMe fastpath data plane, host interface |

## Getting the Source Code

```bash
git clone --recurse-submodules https://github.com/microsoft/azihsm-fw.git
cd azihsm-fw
```

If you already cloned without `--recurse-submodules`:
```bash
git submodule update --init --recursive
```

## Source Layout

| Folder     | Description                                                    |
|------------|----------------------------------------------------------------|
| `core/`    | Platform-agnostic Cerberus code (crypto, attestation, SPDM).  |
| `external/`| External dependencies (git submodules).                        |
| `projects/`| Platform-specific implementations (SP, CP, FP).               |
| `cmake/`   | CMake build infrastructure.                                    |

## Building the Firmware

### Prerequisites

**Ubuntu 22.04 (x86_64)** is the supported build environment. The repository
includes an automated dependency installer:

```bash
cd projects/microsoft/hsp/manticore

# Install all build toolchains (RISC-V, ARM GCC 9, ARM GCC 7, Rust):
./tools/install_dependencies.sh
```

This downloads cross-compilers into `~/build_tools/` by default. Override with
`BUILD_TOOLS=/your/path ./tools/install_dependencies.sh`.

### Build

```bash
cd projects/microsoft/hsp/manticore

# Set up build environment (adds compilers to PATH):
source ./tools/menv.sh

# Build the complete firmware image:
./make_manticore.sh
```

This produces `manticore.bin` in the `build-prod/` directory — the complete
firmware image containing SP, CP, FP, and PCIe components.

### Build Individual Components

**SP (Security Processor):**
```bash
cd projects/microsoft/hsp/manticore/sp
bash make_sp.sh
```

**CP (Control/Crypto Processor):**
```bash
cd projects/microsoft/hsp/manticore/cp/hsm
cargo xtask app-release
```

**FP (Fast-Path Processor):**
```bash
cd projects/microsoft/hsp/manticore/fp/qmgr
make
```

### Clean Build

```bash
./make_manticore.sh --rebuild
```

## External Dependencies

Included as git submodules — pulled automatically with `--recurse-submodules`:

| Dependency | Repository | License |
|------------|-----------|---------|
| Mbed TLS | [Mbed-TLS/mbedtls](https://github.com/Mbed-TLS/mbedtls) | Apache 2.0 |
| FreeRTOS Kernel | [FreeRTOS/FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) | MIT |
| TPM 2.0 Reference | [microsoft/ms-tpm-20-ref](https://github.com/microsoft/ms-tpm-20-ref) | MIT |
| printf | [mpaland/printf](https://github.com/mpaland/printf) | MIT |
| ACVP Parser | [smuellerDD/acvpparser](https://github.com/smuellerDD/acvpparser) | BSD 3-Clause |

## License

See [LICENSE](./LICENSE) for details.

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft
trademarks or logos is subject to and must follow
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
