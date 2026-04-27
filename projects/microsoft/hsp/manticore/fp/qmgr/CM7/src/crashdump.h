// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell


#ifndef __CRASHDUMP_H
#define __CRASHDUMP_H

#include "MessageHandler.h"

#define DUMP_HEADER_MAGIC_COMITTED 0x4D446D70 // MDmp
#define DUMP_HEADER_MAGIC_DIRTY 0x2BB2928F  // ~MDmp
#define FAILURE_CODE_MASK 0x3f

/*
 * ExceptionStackFrame - Used capture the ARM registers on Exception ISR during Wakeup1 interrupt  handing.
 */
typedef struct
{
    uint32_t R0;   // Register R0
    uint32_t R1;   // Register R1
    uint32_t R2;   // Register R2
    uint32_t R3;   // Register R3
    uint32_t R12;  // Register R12
    uint32_t LR;   // Link Register (R14)
    uint32_t PC;   // Program Counter (R15)
    uint32_t xPSR; // Program Status Register
} ExceptionStackFrame;

// CrashCatcherExceptionRegisters - Used to capture ARM registers during Hard fault.
typedef struct
{
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t msp;
    uint32_t exceptionpsr;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} CrashCatcherExceptionRegisters;

/*
 * Fault Code - Fault code as defined by HSP to be added in Crashdump
 */
typedef enum {
    UNKNOWN = 0,                         /**< Unknown failure code. */
    NONMASKABLE_INTERRUPT = 1,           /**< Non-maskable interrupt. */
    HARDFAULT = 2,                       /**< Hard fault. */
    MEMORY_FAULT = 3,                    /**< Memory fault. */
    BUS_FAULT = 4,                       /**< Bus fault. */
    USAGE_FAULT = 5,                     /**< Usage fault. */
    SECURE_FAULT = 6,                    /**< Secure fault. */
    SV_CALL = 7,                         /**< SVCall. */
    DEBUG_MONITOR = 8,                   /**< Debug monitor. */
    PEND_SV = 9,                         /**< PendSV. */
    SYS_TICK = 10,                       /**< SysTick. */
    PANIC = 11,                          /**< Panic. */
    WATCHDOG = 12,                       /**< Watchdog reset as sent by HSP. */
    STACK_OVER_FLOW = 13,                /**< Stack overflow detected. */
    DOUBLE_FAULT = 14,                   /**< Double fault. */
    OTHER_CORE = 15,                     /**< Triggered by other other core. */
    EXPLICIT_FAILURE = 16,               /**< Explicitly triggered on unrecoverable failure. */
    UNCORRECTABLE_ECC_FAILURE = 17,      /**< Trigger crash if an ECC Uncorrectable error occured. */
    CORRECTABLE_ECC_FAILURE = 18,        /**< Trigger crash if an ECC Correctable error(s) occured and exceed the correctable threshold. */
} CrashFaultCode;

/*
 * Fault Code - System defined Hard fault code.
 */
typedef enum {
    SYS_RESET = 1,
    SYS_NMI,
    SYS_HARDFAULT,
    SYS_MEMFAULT,
    SYS_BUSFAULT,
    SYS_USAGEFAULT,
    SYS_SVCALL = 11,
    SYS_DEBUGMON,
    SYS_PENDSV = 14,
} SystemFaultCode;

typedef enum
{
    Release = 0,
    Debug
} DumpType;

#ifdef MCR_TEST_HOOKS
/*
 * TriggerCrashCoreType - Core type as defined in the Trigger Crash IPC DDI testcases
 */
typedef enum
{
    Admin = 0,
    HSM,
    FP0,
    FP1,
    FP2
} TriggerCrashCoreType;

/*
 * MemType - Memory type either DTCM or ITCM
 */
typedef enum
{
    MemTypeItcm = 0,
    MemTypeDtcm
} MemType;

/*
 * MemErrType - Memory type either DTCM or ITCM
 */
typedef enum
{
    MemErrUncorrectable = 0,
    MemErrCorrectable
} MemErrType;

/*
 * InjErrSequence - Sequence of the values to be sets
 */
typedef enum
{
    InjInitMem      = 0, ///< Clear the Memory
    InjSetMem       = 3, ///< Initial Memory Pattern 
    InjCorrErrMem   = 2, ///< 1 bit change to introduce Correctable Error
    InjUncorrErrMem = 5  ///< 2 bit change to introduce Uncorrectable Error
} InjErrSequence;
#endif

/**
* Crashdump Core Types as required by HSP for Crashdump format
*/
typedef enum {
    HSP_TYPE = 0,                     /**< HSP Type. */
    CP0_TYPE,                         /**< Admin Type. */
    CP1_TYPE,                         /**< HSM Type. */
    FP0_TYPE,                         /**< FP0 Type. */
    FP1_TYPE,                         /**< FP1 Type. */
    FP2_TYPE,                         /**< FP2 Type. */
} CrashDumpCoreType;

/*
 * Arm Stack frame to be added to Crashdump
 */
typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;

} CrashDumpArmStackFrame;

/**
 * Structure for crash dump information from an exception.
 */
typedef struct {
    void *stack_ptr;                         /**< The address of the stack frame during exception handling. */
    uint32_t handler_xpsr;                   /**< The xPSR value during exception handling.  This indicates the type of exception. */
    CrashDumpArmStackFrame frame;            /**< The stack frame for the exception that was triggered. */
    uint32_t xpsr;                           /**< Value in register xPSR > */
    uint32_t hfsr;                           /**< Value of the HardFault Status Register (HFSR). */
    uint32_t cfsr;                           /**< Value of the Configurable Fault Status Register (CFSR). */
    uint32_t mmfar;                          /**< Value of the MemManage Fault Address Register (MMFAR). */
    uint32_t bfar;                           /**< Value of the BusFault Address Register (BFAR). */
    uint32_t afsr;                                /**< Value of the BusFault Address Register */
} CrashDumpArm;

typedef enum {
    CRASH_TYPE_NORMAL = 0,
    CRASH_TYPE_CRASH,
    CRASH_TYPE_HANG,
    CRASH_TYPE_PANIC
} CrashType;
/**
 * Crashdump ARM Core Production Code Payload.
 */
typedef struct  {
    CrashDumpArm common_regs;            /**< ARM core register sets defined for crashdump by Cerberus Core */
} CrashDumpPacketArmPayload;

typedef struct {
    uint32_t magic;                                        /**< Magic number in the head of a crashdump */
    uint32_t faultCode;                                    /**< A fault code that sorts the failures fetched from rsgisters to different categories */
    uint16_t version;                                      /**< Version of crashdump packet */
    uint8_t coreType;                                      /**< 0: HSP, 1: CP-Admin, 2: CP-HSM, 3: FP0: 4: FP1, 5: FP2 */
    uint8_t dumpType;                                      /**< 0: Release, 1: Development */
    uint8_t crashType;                                     /**< 0: normal, 1: crash, 2: hanging 3: panic */
    uint8_t reserved;                                      /**< Reserved, must be 0 */
    uint16_t payloadSize;                                  /**< Payload size, number of bytes */
} CrashDumpPacketHeader;

#ifdef DEBUG_BUILD
/*
 * Structure for FP specific information dump. Only available in debug build.
 */
typedef struct {
    uint32_t fpInitSts;
    #if defined (fps_cpu0Core)
    uint32_t psramFp0toFp2Pi;
    uint32_t psramFp2toFp0Ci;
    uint32_t psramFp0toFp1Pi;
    uint32_t psramFp1toFp0Ci;
    #elif defined (fps_cpu1Core)
    uint32_t psramFp1toFp2Pi;
    uint32_t psramFp2toFp1Ci;
    uint32_t psramFp1toFp0Pi;
    uint32_t psramFp0toFp1Ci;
    #elif defined (fps_cpu2Core)
    uint32_t psramFp2toFp0Pi;
    uint32_t psramFp0toFp2Ci;
    uint32_t psramFp2toFp1Pi;
    uint32_t psramFp1toFp2Ci;

    uint32_t psramCp0toFp2ReqPi;
    uint32_t psramCp0toFp2ReqCi;
    uint32_t psramFp2toCp0ResPi;
    uint32_t psramFp2toCp0ResCi;

    uint32_t psramCp1toFp2ReqPi;
    uint32_t psramCp1toFp2ReqCi;
    uint32_t psramFp2toCp1ResPi;
    uint32_t psramFp2toCp1ResCi;

    uint32_t psramFp2toCp0ReqPi;
    uint32_t psramFp2toCp0ReqCi;
    uint32_t psramCp0toFp2ResPi;
    uint32_t psramCp0toFp2ResCi;

    uint32_t psramFp2toCp1ReqPi;
    uint32_t psramFp2toCp1ReqCi;
    uint32_t psramCp1toFp2ResPi;
    uint32_t psramCp1toFp2ResCi;

    #endif
} FpPayload;
#endif

typedef struct {
    CrashDumpPacketArmPayload armPayload;
#ifdef DEBUG_BUILD
    FpPayload fpPayload;
#endif
} CrashDumpPayload;

/*
 * Complete crashdump structure
 */
typedef struct
{
    CrashDumpPacketHeader header;
    CrashDumpPayload payload;
} CrashDumpInfo;

void CrashDump_StartDump(const CrashCatcherExceptionRegisters* pExceptionRegisters, bool sendIrq, bool explicitCrash);
void Explicit_CrashCatcher_Entry(void);
#ifdef MCR_TEST_HOOKS
void TriggerCrash(InjectErrorType errorType);
#endif
#endif
