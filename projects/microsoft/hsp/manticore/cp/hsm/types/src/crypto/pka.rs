// Copyright (c) Microsoft Corporation. All rights reserved.

/// The maximum size of the PKA result buffer.
pub const PKA_RESULT_MAX_SIZE_BYTES: usize = 516;
pub const PKA_INPUT_MAX_SIZE_BYTES: usize = 516 * 2;
pub const PKA_CONST_MAX_SIZE_BYTES: usize = 444;
pub const PKA_SELF_TEST_MAX_SIZE_BYTES: usize = 1536;

/// Enumeration of PKA command codes
#[derive(PartialEq, Eq, Copy, Clone)]
#[repr(u32)]
pub enum PkaCommandCode {
    /// Command code for ECC verify operation with 256-bit key
    EccVerify256 = 0x1000_0000,

    /// Command code for ECC verify operation with 384-bit key
    EccVerify384 = 0x1000_0001,

    /// Command code for ECC verify operation with 521-bit key
    EccVerify521 = 0x1000_0008,

    /// Command code for ECC sign operation with 256-bit key
    EccSign256 = 0x1001_0000,

    /// Command code for ECC sign operation with 384-bit key
    EccSign384 = 0x1001_0001,

    /// Command code for ECC sign operation with 521-bit key
    EccSign521 = 0x1001_0008,

    /// Command to validate if a given point is in the ECC P256 Curve
    EccPointValidation256 = 0x1005_0000,

    /// Command to validate if a given point is in the ECC P384 Curve
    EccPointValidation384 = 0x1005_0001,

    /// Command to validate if a given point is in the ECC P521 Curve
    EccPointValidation521 = 0x1005_0008,

    /// Command code for ECC key generate operation with 256-bit key
    EccKeyGenerate256 = 0x1006_0000,

    /// Command code for ECC key generate operation with 384-bit key
    EccKeyGenerate384 = 0x1006_0001,

    /// Command code for ECC key generate operation with 521-bit key
    EccKeyGenerate521 = 0x1006_0008,

    /// Command code for ECC point multiplication with 256-bit key
    EccPointMultiplication256 = 0x1002_0000,

    /// Command code for ECC point multiplication with 384-bit key
    EccPointMultiplication384 = 0x1002_0001,

    /// Command code for ECC point multiplication with 521-bit key
    EccPointMultiplication521 = 0x1002_0008,

    /// Command code for modular multiplication 256 operation
    ModMultiplication256 = 0x5004_0000,

    /// Command code for modular multiplication 384 operation
    ModMultiplication384 = 0x5004_0001,

    /// Command code for modular multiplication 521 operation
    ModMultiplication521 = 0x5004_0008,

    /// Command code for modular addition 256 operation
    PkaModAddition256 = 0x5005_0000,

    /// Command code for modular addition 384 operation
    PkaModAddition384 = 0x5005_0001,

    /// Command code for modular addition 521 operation
    PkaModAddition521 = 0x5005_0008,

    /// Command code for Montgomery constant calculation for 256-bit prime
    MontgomeryConstCalc256 = 0x500c_0000,

    /// Command code for Montgomery constant calculation for 384-bit prime
    MontgomeryConstCalc384 = 0x500c_0001,

    /// Command code for Montgomery constant calculation for 521-bit prime
    MontgomeryConstCalc521 = 0x500c_0008,

    /// Command code for Rsa 2k private key operation
    RsaPrivateKeyOp2k = 0x5000_0003,

    /// Command code for Rsa 2k private key operation using CRT
    RsaPrivateKeyOp2kCrt = 0x500e_0003,

    /// Command code for Rsa 3k private key operation
    RsaPrivateKeyOp3k = 0x5000_0004,

    /// Command code for Rsa 3k private key operation using CRT
    RsaPrivateKeyOp3kCrt = 0x500e_0004,

    /// Command code for Rsa 4k private key operation
    RsaPrivateKeyOp4k = 0x5000_0005,

    /// Command code for Rsa 4k private key operation using CRT
    RsaPrivateKeyOp4kCrt = 0x500e_0005,

    /// Command code for Rsa 2k public key operation
    RsaPublicKeyOp2k = 0x5001_0003,

    /// Command code for Rsa 3k public key operation
    RsaPublicKeyOp3k = 0x5001_0004,

    /// Command code for Rsa 4k public key operation
    RsaPublicKeyOp4k = 0x5001_0005,

    /// Command code for modular inverse 256 operation
    PkaModInverse256 = 0x5007_0000,

    /// Command code for modular inverse 384 operation
    PkaModInverse384 = 0x5007_0001,

    /// Command code for modular inverse 521 operation
    PkaModInverse521 = 0x5007_0008,

    /// Command code for modular inverse 1k operation
    PkaModInverse1k = 0x5007_0002,

    /// Command code for modular inverse 2k operation
    PkaModInverse2k = 0x5007_0003,

    /// Command code for modular reduction 256 operation
    PkaModReduction256 = 0x5009_0000,

    /// Command code for modular reduction 384 operation
    PkaModReduction384 = 0x5009_0001,

    /// Command code for modular reduction 521 operation
    PkaModReduction521 = 0x5009_0008,

    /// Command code for conversion of Montgomery representation to normal operand for 256 sized operation output
    PkaMontgomeryReprOut256 = 0x500a_0000,

    /// Command code for conversion of Montgomery representation to normal operand for 384 sized operation output
    PkaMontgomeryReprOut384 = 0x500a_0001,

    /// Command code for conversion of Montgomery representation to normal operand for 521 sized operation output
    PkaMontgomeryReprOut521 = 0x500a_0008,

    /// Command code for conversion of Montgomery input to normal operand for 1k sized operation
    PkaMontgomeryReprOut1k = 0x500a_0002,

    /// Command code for conversion of Montgomery input to normal operand for 2k sized operation
    PkaMontgomeryReprOut2k = 0x500a_0003,

    /// Command code for conversion of Montgomery input to normal operand for 3k sized operation
    PkaMontgomeryReprOut3k = 0x500a_0004,

    /// Command code for conversion of normal operand to Montgomery input for 1k sized operation
    PkaMontgomeryReprIn1k = 0x500b_0002,

    /// Command code for conversion of normal operand to Montgomery input for 2k sized operation
    PkaMontgomeryReprIn2k = 0x500b_0003,

    /// Command code for conversion of normal operand to Montgomery input for 3k sized operation
    PkaMontgomeryReprIn3k = 0x500b_0004,

    /// Command code for conversion of normal operand to Montgomery input for 4k sized operation
    PkaMontgomeryReprIn4k = 0x500b_0005,

    /// Command code for conversion of Montgomery input to normal operand for 4k sized operation
    PkaMontgomeryReprOut4k = 0x500a_0005,

    /// Command code for conversion of normal operand to Montgomery input for 256 sized operation
    PkaMontgomeryReprIn256 = 0x500b_0000,

    // Command code for conversion of normal operand to Montgomery input for 384 sized operation
    PkaMontgomeryReprIn384 = 0x500b_0001,

    /// Command code for conversion of normal operand to Montgomery input for 521 sized operation
    PkaMontgomeryReprIn521 = 0x500b_0008,

    /// Command code for montgomery 1k constant calculation operation
    PkaMontgomeryConstCalc1k = 0x500c_0002,

    /// Command code for montgomery 2k constant calculation operation
    PkaMontgomeryConstCalc2k = 0x500c_0003,

    /// Command code for montgomery 3k constant calculation operation
    PkaMontgomeryConstCalc3k = 0x500c_0004,

    /// Command code for montgomery 4k constant calculation operation
    PkaMontgomeryConstCalc4k = 0x500c_0005,

    /// Command code for modular multiplication 2k operation
    PkaModMultiplication2k = 0x5004_0003,

    /// Command code for modular multiplication 3k operation
    PkaModMultiplication3k = 0x5004_0004,

    /// Command code for modular multiplication 4k operation
    PkaModMultiplication4k = 0x5004_0005,

    /// Command for memory wipe
    PkaMemWipe = 0x500F_0000,

    /// Unknown Command code
    Unknown = 0xffff_ffff,
}

/// PKA Command structure
#[repr(C)]
pub struct PkaCommand {
    /// PKA Command Code
    pub command_code: PkaCommandCode,

    /// Memory address for the result
    pub result_addr: u32,

    /// Memory address of the first argument
    pub arg1_addr: u32,

    /// Memory address of the second argument
    pub arg2_addr: u32,

    /// Memory address of the third argument
    pub arg3_addr: u32,
}
