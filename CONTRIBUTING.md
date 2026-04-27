# Contributing to Azure Integrated HSM Firmware

This project welcomes contributions and suggestions. Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the
instructions provided by the bot. You will only need to do this once across all repos using our CLA.

## Getting Started

1. Fork the repository and create your branch from `main`.
2. Install build dependencies: `cd projects/microsoft/hsp/manticore && ./tools/install_dependencies.sh`
3. Set up the build environment: `source ./tools/menv.sh`
4. Make your changes.
5. Ensure the firmware builds successfully: `./make_manticore.sh`
6. Submit a pull request.

## Build Instructions

See [README.md](README.md) for detailed build instructions.

## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
