/*++

    Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    riscvcpu.h

Abstract:

    The RISC-V physical memory protection header. HSP RISC-V PMP is configured
    with 16 regions.

Author:

    Navin Pai (navinp)

--*/
#pragma once

#include "splibs/inc/spstatus.h"
#include "splibs/inc/sptypes.h"

//
// The interrupt attributes supported by riscv compiler. Use this attribute to
// indicate that the specified function is an interrupt handler. The compiler
// generates entry & exit sequence suitable for use in an interrupt handler
//

#define RV_USER_INTERRUPT       __attribute__((interrupt("user")))
#define RV_SUPERVISOR_INTERRUPT __attribute__((interrupt("supervisor")))
#define RV_MACHINE_INTERRUPT    __attribute__((interrupt("machine")))

//
// PSP RISC-V has 16 regions
//
#define RISCV_PMP_REGIONS       16

//
// The retail clock frequency of SP is 500 MHz, 500,000,000 Hz
//
#define RISCV_HSP_CLK_HZ        0x1dcd6500
#define RISCV_HSP_CLK_MHZ       500

//
// RTC clock is RISCV_HSP_CLK / 32
//
#define RISCV_RTC_CLK_HZ        (RISCV_HSP_CLK_HZ / 32)
#define RISCV_RTC_CLK_KHZ       (RISCV_RTC_CLK_HZ / 1000)

static_assert((RISCV_RTC_CLK_KHZ * 1000) == RISCV_RTC_CLK_HZ,
              "RISCV_RTC_CLK_KHZ is not an integral value");

//
// Define some inline assembly instructions
//
#define WFI   __asm__ __volatile__("wfi")
#define NOP   __asm__ __volatile__("nop")
#define DMB   __asm__ __volatile__("fence rw, rw")
#define DMBLD __asm__ __volatile__("fence r, rw")
#define DMBST __asm__ __volatile__("fence w, w")
#define ISB                          \
    __asm__ __volatile__("fence.i"); \
    __asm__ __volatile__("fence r, r")

//
// CEASE is a custom instruction for Microsoft Risc-V core.
// It tells the processor to halt, no more instructions will be executed,
// and the processor shutdown sequence will begin.
// Can only be executed in machine mode.
//
#define CEASE __asm__ __volatile__(".word 0x30500073")

//
// For riscv PMP,
// If no PMP entry matches in M-mode access,  the access succeeds.
// If no PMP entry matches in S-mode or U-mode access, but at least one PMP
// entry is implemented, the access fails. If PMP address is not locked, then it
// doesn't apply to M-mode
//

//
// We are using NAPOT ranges
// wich make use of the low-order bits of the associated address register to
// encode the sizeof the range. example: aaaa...aaa0 8-byte NAPOT range
// aaaa...aa01 16-byte NAPOT range
// aaaa...a011 32-byte NAPOT range
// If pmpaddr = aaaa011, for example, the range is aaaa00000 .. aaaa11111,
// inclusive.
//

#define PMP_CONFIG_NO_RIGHTS \
    {                        \
        .M = RiscvPmpNapot   \
    }
#define PMP_CONFIG_READ_ONLY       \
    {                              \
        .M = RiscvPmpNapot, .R = 1 \
    }
#define PMP_CONFIG_READ_WRITE               \
    {                                       \
        .M = RiscvPmpNapot, .R = 1, .W = 1, \
    }
#define PMP_CONFIG_READ_EXECUTE             \
    {                                       \
        .M = RiscvPmpNapot, .R = 1, .X = 1, \
    }
#define PMP_CONFIG_READ_WRITE_EXECUTE              \
    {                                              \
        .M = RiscvPmpNapot, .R = 1, .W = 1, .X = 1 \
    }

// the PMP address need to be shifted by 2
#define PMP_SHIFT                  2
#define NAPOT_ADDRESS(base, range) (((base) + ((range) / 2 - 1)) >> PMP_SHIFT)
#define TOR_ADDRESS(addr)          ((addr) >> PMP_SHIFT)


//
// PMP Addressing modes supported by RISCV-V
//

typedef enum _RiscvPmpAddressMode
{
    RiscvPmpOff = 0,     // PMP is off
    RiscvPmpTor = 1,     // Top of range
    RiscvPmpNa4 = 2,     // Naturally aligned 4 byte region
    RiscvPmpNapot = 3    // Naturally aligned power-of-two region, >= 8 bytes

} RiscvPmpAddressMode;


typedef enum _RiscvPmpRegionStatus
{
    RiscvPmpUnlocked = 0,    // PMP region is unlocked
    RiscvPmpLocked = 1       // PMP region is locked

} RiscvPmpRegionStatus;


//
// The configuration register format
//

typedef union _RiscvPmpConfig
{
    struct
    {
        uint32_t R             : 1;    // Read enabled
        uint32_t W             : 1;    // Write enabled
        uint32_t X             : 1;    // Execute enabled
        RiscvPmpAddressMode M  : 2;    // Address mode
        uint32_t Reserved      : 2;    // Reserved
        RiscvPmpRegionStatus L : 1;    // Locked, Once locked only a reboot can
                                       // change it
    };

    uint32_t val;

} RiscvPmpConfig;


typedef struct _RiscvPmpSetting
{
    uint32_t PmpAddress;
    RiscvPmpConfig PmpConfig;
} RiscvPmpSetting;


//
// Software Exception codes
// https://static.dev.sifive.com/SiFive-E20-Manual-v1p0.pdf
// https://riscv.org/wp-content/uploads/2016/11/riscv-privileged-v1.9.1.pdf
//
typedef enum
{
    RiscvExceptionIAM = 0,    //  0 - Instruction address misaligned
    RiscvExceptionIAF,        //  1 - Instruction access fault
    RiscvExceptionII,         //  2 - Illegal instruction
    RiscvExceptionBREAK,      //  3 - Breakpoint
    RiscvExceptionLAM,        //  4 - Load address misaligned
    RiscvExceptionLAF,        //  5 - Load access fault
    RiscvExceptionSAMOAM,     //  6 - Store/AMO address misaligned
    RiscvExceptionSAMOAF,     //  7 - Store/AMO access fault
    RiscvExceptionECALL_U,    //  8 - Environment call from U-mode
    RiscvExceptionR9,         //  9 - Reserved - Environment call from S-mode
    RiscvExceptionR10,        // 10 - Reserved
    RiscvExceptionECALL_M,    // 11 - Environment call from M-mode
    RiscvExceptionR12,        // 12 - Reserved - Instruction page fault
    RiscvExceptionR13,        // 13 - Reserved - Load page fault
    RiscvExceptionR14,        // 14 - Reserved
    RiscvExceptionR15,        // 15 - Reserved - Store/AMO page fault
    RiscvExceptionMAX,

    RiscvInterruptR0 = 0,    //  0 - Reserved - User software interrupt
    RiscvInterruptR1,        //  1 - Reserved - Supervisor software interrupt
    RiscvInterruptR2,        //  2 - Reserved
    RiscvInterruptMSI,       //  3 - Machine Software Interrupt
    RiscvInterruptR4,        //  4 - Reserved - User timer interrupt
    RiscvInterruptR5,        //  5 - Reserved - Supervisor timer interrupt
    RiscvInterruptR6,        //  6 - Reserved
    RiscvInterruptMTI,       //  7 - Machine Timer Interrupt
    RiscvInterruptR8,        //  8 - Reserved - User external interrupt
    RiscvInterruptR9,        //  9 - Reserved - Supervisor external interrupt
    RiscvInterruptR10,       // 10 - Reserved
    RiscvInterruptMEI,       // 11 - Machine External Interrupt
    RiscvInterruptCSIP,      // 12 - CLIC Software Interrupt Pending
    RiscvInterruptR13,       // 13 -
    RiscvInterruptR14,       // 14 -
    RiscvInterruptR15,       // 15 -
    RiscvInterruptIRQ,       // 16 - CLIC Local Interrupt 0
    RiscvInterruptFIQ,       // 17 - CLIC Local Interrupt 1
    RiscvInterruptCLIC2,     // 18 - CLIC Local Interrupt 2
    RiscvInterruptCLIC3,     // 19 - CLIC Local Interrupt 3
    RiscvInterruptCLIC4,     // 20 - CLIC Local Interrupt 4
    RiscvInterruptCLIC5,     // 21 - CLIC Local Interrupt 5
    RiscvInterruptCLIC6,     // 22 - CLIC Local Interrupt 6
    RiscvInterruptCLIC7,     // 23 - CLIC Local Interrupt 7
    RiscvInterruptMAX,
} RiscvExceptionCode;


#define READ_CSR(reg)                                       \
    (                                                       \
        {                                                   \
            unsigned long tmp;                              \
            __asm__ volatile("csrr %0, " #reg : "=r"(tmp)); \
            tmp;                                            \
        })

#define WRITE_CSR(reg, val) \
    ({ __asm__ volatile("csrw " #reg ", %0" ::"rK"(val)); })

#define SWAP_CSR(reg, val)                            \
    (                                                 \
        {                                             \
            unsigned long tmp;                        \
            __asm__ volatile("csrrw %0, " #reg ", %1" \
                             : "=r"(tmp)              \
                             : "rK"(val));            \
            tmp;                                      \
        })

#define SET_CSR(reg, val)                             \
    (                                                 \
        {                                             \
            unsigned long tmp;                        \
            __asm__ volatile("csrrs %0, " #reg ", %1" \
                             : "=r"(tmp)              \
                             : "rK"(val));            \
            tmp;                                      \
        })

#define CLEAR_CSR(reg, val)                           \
    (                                                 \
        {                                             \
            unsigned long tmp;                        \
            __asm__ volatile("csrrc %0, " #reg ", %1" \
                             : "=r"(tmp)              \
                             : "rK"(val));            \
            tmp;                                      \
        })

#define READ_CSR_VALUE(reg)                                \
    (                                                      \
        {                                                  \
            unsigned long tmp;                             \
            __asm__ volatile("csrr %0, " reg : "=r"(tmp)); \
            tmp;                                           \
        })

#define WRITE_CSR_VALUE(reg, val) \
    ({ __asm__ volatile("csrw " reg ", %0" ::"rK"(val)); })

typedef enum _RiscvPrivilegeMode
{
    RiscvPrivilegeUser = 0,
    RiscvPrivilegeSuperviser = 1,
    RiscvPrivilegeReserved = 2,
    RiscvPrivilegeMachine = 3
} RiscvPrivilegeMode;


//
// mstatus (Machine Status Register)
//
typedef union _RiscvMStatus
{
    struct
    {
        uint32_t Reserved1     : 3;
        uint32_t MIE           : 1;    // Machine mode interrupt enable
        uint32_t Reserved2     : 3;
        uint32_t MPIE          : 1;    // Machine mode previous interrupt enable
        uint32_t Reserved3     : 3;
        RiscvPrivilegeMode MPP : 2;    // Machine mode previous privilege
        uint32_t Reserved4     : 19;
    } Flags;

    uint32_t Val;

} RiscvMStatus;


//
// Machine interrupt-enable register (mie)
// When in CLIC modes, the mie register is hardwired to zero and individual
// interrupt enables are controlled by clicIntIE CLIC memory mapped registers
//
typedef union _RiscvInterruptEnable
{
    struct
    {
        uint32_t Reserved1 : 3;     // Reserved
        uint32_t MSIE      : 1;     // Machine mode software interrupt enable
        uint32_t Reserved2 : 3;     // Reserved
        uint32_t MTIE      : 1;     // Machine mode timer interrupt enable
        uint32_t Reserved3 : 3;     // Reserved
        uint32_t MEIE      : 1;     // machine mode external interrupt enable
        uint32_t Reserved4 : 20;    // Reserved
    } Flags;

    uint32_t Val;

} RiscvInterruptEnable;


//
// Machine iterrupt-pending register (mip)
// When in CLIC modes, the mip register is hardwired to zero and individual
// interrupt enables are controlled by clicIntIP CLIC memory-mapped registers
//
typedef union _RiscvInterruptPending
{
    struct
    {
        uint32_t Reserved1 : 3;     // Reserved
        uint32_t MSIP      : 1;     // RO Machine software interrrupt pending
        uint32_t Reserved2 : 3;     // Reserved
        uint32_t MTIP      : 1;     // RO Machine timer interrupt-pending
        uint32_t Reserved3 : 3;     // Reserved
        uint32_t MEIP      : 1;     // RO Machine external interrupt-pending
        uint32_t Reserved4 : 20;    // Reserved
    } Flags;

    uint32_t Val;
} RiscvInterruptPending;


//
// Machine Cause Register (mcause)
//
typedef union _RiscvMachineCause
{
    struct
    {
        uint32_t ExceptionCode : 10;    // A code identifying the last exception
        uint32_t Reserved      : 13;    // Reserved
        uint32_t MPIE          : 1;     // Previous interrupt enabled, same as
                                        // mstatus.mpie, CLIC mode only
        uint32_t MPIL  : 4;    // Previous interrupt level. CLIC mode only.
        uint32_t MPP   : 2;    // Previous interrupt privilege mode, same as
                               // mstatus.mpp, CLIC mode only
        uint32_t MINHV : 1;    // Hardware vectoring in progress when set. CLIC
                               // mode only
        uint32_t Interrupt : 1;    // 1, if trap was caused by an interrupt, 0
                                   // otherwise

    } Flags;

    uint32_t Val;
} RiscvMachineCause;


//
// CLIC interrupt configuration
//
typedef union _RiscvIntCfg
{
    struct
    {
        uint8_t Reserved   : 6;    // [0-5]
        uint8_t ClicIntCfg : 2;    // [7:6]
    } Flags;

    uint8_t Val;
} RiscvClicIntCfg;

//
// CLIC Configuration
//
typedef union _RiscvClicCfg
{
    struct
    {
        uint8_t NvBits : 1;    // [0] When set, selective hardware vectoring is
                               // enabled.
        uint8_t NlBits : 4;    // [4:1] Determines the number of Level bits
                               // available in clicintcfg
        uint8_t NmBits : 2;    // [6:5] Determines the number Mode bits
                               // available in clicintcfg.
        uint8_t Reserved : 1;    // [7]
    } Flags;

    uint8_t Val;
} RiscvClicCfg;


#define RISCV_CLIC_BASE_ADDRESS      (0x02000000)
#define RISCV_CLIC_BASE_ADDRESS_SIZE (0x10000)

#define RISCV_CLIC_HART0_MSIP_OFFSET \
    (RISCV_CLIC_BASE_ADDRESS + 0x0000)    // 4Bytes, msip for hart-0
#define RISCV_CLIC_HART0_MTIME_OFFSET \
    (RISCV_CLIC_BASE_ADDRESS + 0xBFF8)    // 4Bytes, mtime
#define RISCV_CLIC_HART0_MTIMEH_OFFSET \
    (RISCV_CLIC_BASE_ADDRESS + 0xBFFC)    // 4Bytes, mtimeh
#define RISCV_CLIC_HART0_MTIMECMP_OFFSET \
    (RISCV_CLIC_BASE_ADDRESS + 0x4000)    // 4Bytes, mtimecmp
#define RISCV_CLIC_HART0_MTIMECMPH_OFFSET \
    (RISCV_CLIC_BASE_ADDRESS + 0x4004)    // 4Bytes, mtimecmph

#define RISCV_CLIC_HART0_BASE_ADDRESS      (0x02800000)
#define RISCV_CLIC_HART0_BASE_ADDRESS_SIZE (0x1000)

#define RISCV_CLIC_HART0_INT_IP \
    (RISCV_CLIC_HART0_BASE_ADDRESS + 0x000)    // 1B per int id
#define RISCV_CLIC_HART0_INT_IE \
    (RISCV_CLIC_HART0_BASE_ADDRESS + 0x400)    // 1B per int id
#define RISCV_CLIC_HART0_INT_CFG \
    (RISCV_CLIC_HART0_BASE_ADDRESS + 0x800)    // 1B per int id
#define RISCV_CLIC_HART0_CFG     (RISCV_CLIC_HART0_BASE_ADDRESS + 0xC00)    // 1B

#define RISCV_CLIC_DIRECT_MODE   0x02
#define RISCV_CLIC_VECTORED_MODE 0x03

#define RISCV_ECALL_SIZE         0x4    // Size of ecall operand
#define RISCV_CONTEXT_SIZE_BYTES 160
#define RISCV_CONTEXT_SIZE_DWORD RISCV_CONTEXT_SIZE_BYTES / sizeof(uint32_t)
//
// RISCV system operation
// https://sifive.cdn.prismic.io/sifive/d1984d2b-c9b9-4c91-8de0-d68a5e64fa0f_sifive-interrupt-cookbook-v1p2.pdf
//
#define RISCV_CSR_MTVT           "0x307"
#define RISCV_CSR_MNXTI          "0x345"

//
// Set the configuration for region
//
void RiscvPmpSetRegion(uint32_t PmpRegion,
                       RiscvPmpConfig PmpConfig,
                       uint32_t Address);


//
// Get the pmp region
//
void RiscvPmpGetRegion(uint32_t PmpRegion,
                       RiscvPmpConfig* PmpConfig,
                       uint32_t* Address);

//
// Configure all pmp regions, clearing unused regions
//
void UpdatePmpSetting(const RiscvPmpSetting* PmpSetting,
                      const uint32_t PmpSettingSize);


//
// Runs the UserModeFunction in user mode
//
typedef HSP_STATUS (*UserModeFunction)(uintptr_t Arg1, uintptr_t Arg2);


HSP_STATUS
ExecuteInUserMode(const RiscvPmpSetting* PmpSetting,
                  const uint32_t PmpSettingSize,
                  UserModeFunction Fn,
                  uintptr_t Arg1,
                  uintptr_t Arg2);


//
// Returns the number of cycles the cpu has executed
//
uint64_t RiscvGetCpuCycles();


//
// Returns the RTC clock value
//
uint64_t RiscvGetCpuTime();


//
// Returns the number of instructions retired
//
uint64_t RiscvGetCpuInstructionsRetired();


//
// Setup default interrupts
//

typedef void (*RiscvTrapHandlerFn)(uint32_t Param0);

// Definition of function pointer to be defined by consumer, syscall number
// cannot be less that Syscall_FixedMaxSyscall, it should manage syscall number
// currently  available at the system at the moment else it should return
// STATUS_UNHANDLED_SYSCALL status.

typedef HSP_STATUS (*RiscvSyscallHandlerFn)(uint32_t StackFrame);

//
// Extern default handler comes from core library. This is because
// we do not know what action to take if we encounter an unexpected
// interrupt or a trap in runtime lib.
//

extern void DefaultTrapHandler(uint32_t Param0);

extern HSP_STATUS UnhandledSyscallHandler(uint32_t StackFrame);

extern void RiscvTrapHandler(uint32_t Param0);

extern void HandleUserEcallAsm(uint32_t Param0);

void RiscvInterruptInit();

void RiscvClicInterruptEnable(RiscvExceptionCode Index, uint8_t Level, bool Enable);

void RiscvMachineInterruptEnable(bool Enable);

HSP_STATUS
RiscvSetInterruptHandler(RiscvExceptionCode Index,
                         RiscvTrapHandlerFn InterruptHandler);

HSP_STATUS
RiscvSetExceptionHandler(RiscvExceptionCode Index,
                         RiscvTrapHandlerFn ExceptionHandler);

// Obtain Syscall from Stack Frame
#define GET_SYSCALL_NR(StackFrame)                   \
    (                                                \
        {                                            \
            RISCV_CPU_SAVE_REGS* regs;               \
            regs = (RISCV_CPU_SAVE_REGS*)StackFrame; \
            regs->a7;                                \
        })

// Calling of syscall with no Arguments passed to caller
// Syscall number shall go into A7
#define SYSCALL_NONE(syscall_nr)                         \
    {                                                    \
        __asm__ volatile("li a7, %0" ::"i"(syscall_nr)); \
        __asm__ volatile("ecall");                       \
    }

// Calling of syscall with 1 Argument on A0.
// Syscall number shall go into A7
#define SYSCALL_ONE(syscall_nr, Arg0)                    \
    {                                                    \
        __asm__ volatile("li a7, %0" ::"i"(syscall_nr)); \
        __asm__ volatile("ecall" ::"r"(Arg0));           \
    }

typedef enum
{
    RiscvSyscallReturnToMachineM = 0,
    RiscvSyscallStackError,
    RiscvSyscallSystemUnhandled,
    RiscvSyscallMaxReservedSyscalls
} RiscvSyscallEcallCode;


HSP_STATUS
RiscvSetSystemSyscallHandler(RiscvSyscallHandlerFn SyscallHandlerFn);

#define RISCV_CPU_USER_MODE    0
#define RISCV_CPU_MACHINE_MODE 1

typedef struct _RISCV_TRAP_STACK_INFO
{
    uint32_t CpuModeOnEntry;    // 0: User Mode 1: Machine Mode
    uint32_t UserStackPointer;
    uint32_t MachineStackSize;    // Size calculated
    uint32_t UserStackSize;       // 0 if User Stack is 0
} RISCV_TRAP_STACK_INFO;

typedef struct _RISCV_CPU_SAVE_REGS
{                        // reg   ABI
    uint32_t zero;       // x0   zero
    uint32_t ra;         // x1    ra - return address
    uint32_t sp;         // x2    sp - stack pointer
    uint32_t gp;         // x3    gp - global pointer
    uint32_t tp;         // x4    tp - thread pointer
    uint32_t t0;         // x5    t0 - temporaries
    uint32_t t1;         // x6    t1
    uint32_t t2;         // x7    t2
    uint32_t s0;         // x8    s0/fp - saved register / frame pointer
    uint32_t s1;         // x9    s1 - save register
    uint32_t a0;         // x10   a0 - function arguments / return value
    uint32_t a1;         // x11   a1 - function arguments / return value
    uint32_t a2;         // x12   a2 - function arguments
    uint32_t a3;         // x13   a3
    uint32_t a4;         // x14   a4
    uint32_t a5;         // x15   a5
    uint32_t a6;         // x16   a6
    uint32_t a7;         // x17   a7
    uint32_t s2;         // x18   s2 - Saved register
    uint32_t s3;         // x19   s3 - Saved register
    uint32_t s4;         // x20   s4 - Saved register
    uint32_t s5;         // x21   s5 - Saved register
    uint32_t s6;         // x22   s6 - Saved register
    uint32_t s7;         // x23   s7 - Saved register
    uint32_t s8;         // x24   s8 - Saved register
    uint32_t s9;         // x25   s9 - Saved register
    uint32_t s10;        // x26   s10 - Saved register
    uint32_t s11;        // x27   s11 - Saved register
    uint32_t t3;         // x28   t3 - Temporaries
    uint32_t t4;         // x29   t4
    uint32_t t5;         // x30   t5
    uint32_t t6;         // x31   t6
    uint32_t mpec;       // Machine exceptin return pc
    uint32_t mcause;     // Mcause
    uint32_t mstatus;    // Machine Status
    uint32_t orig_t0;    // Temporary saved T0 on trap entry.
    RISCV_TRAP_STACK_INFO StackInfo;
} RISCV_CPU_SAVE_REGS;

typedef union _RISCV_CPU_SAVE_CONTEXT
{
    RISCV_CPU_SAVE_REGS Registers;
    uint32_t RawData[RISCV_CONTEXT_SIZE_DWORD];
} RISCV_CPU_SAVE_CONTEXT;
