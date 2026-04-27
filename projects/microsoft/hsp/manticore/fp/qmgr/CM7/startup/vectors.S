// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "M7MemMap.h"

/* vectors.s */
.cpu cortex-m7
.syntax unified
.thumb_func
.section ".interrupt_vector"

.extern CrashCatcher_Entry
.extern CrashCatcher_Entry_Send_IRQ_to_Other_CPU

.word   _stack_top  /* stack top address */

.word   _start+1          /* 1 Reset */
.word   HardFault_Handler /* 2 NMI */
.word   HardFault_Handler /* 3 HardFault */
.word   HardFault_Handler /* 4 MemManage */
.word   HardFault_Handler /* 5 BusFault */
.word   HardFault_Handler /* 6 UsageFault */
.word   0                 /* 7 RESERVED */
.word   0                 /* 8 RESERVED */
.word   0                 /* 9 RESERVED*/
.word   0                 /* 10 RESERVED */
.word   SVC_Handler       /* 11 SVCall */
.word   DebugMon_Handler  /* 12 Debug Monitor */
.word   0                 /* 13 RESERVED */
.word   PendSV_Handler    /* 14 PendSV */
.word   Irq_SysTickHandler   /* 15 SysTick */
.word   Irq_EmptyHandler  /* 16 External Interrupt(0 ) */
.word   Irq_EmptyHandler  /* 17 External Interrupt(1 ) */
.word   Irq_EmptyHandler  /* 18 External Interrupt(2 ) */
.word   Irq_EmptyHandler  /* 19 External Interrupt(3 ) */
.word   Irq_EmptyHandler  /* 20 External Interrupt(4 ) */
.word   Irq_EmptyHandler  /* 21 External Interrupt(5 ) */
.word   Irq_EmptyHandler  /* 22 External Interrupt(6 ) */
.word   Irq_EmptyHandler  /* 23 External Interrupt(7 ) */
.word   Irq_EmptyHandler  /* 24 External Interrupt(8 ) */
.word   Irq_EmptyHandler  /* 25 External Interrupt(9 ) */
.word   Irq_EmptyHandler /* 26 External Interrupt(10) */
.word   Irq_EmptyHandler /* 27 External Interrupt(11) */
.word   Irq_EmptyHandler /* 28 External Interrupt(12) */
.word   Irq_EmptyHandler /* 29 External Interrupt(13) */
.word   Irq_EmptyHandler /* 20 External Interrupt(14) */
.word   Irq_EmptyHandler /* 21 External Interrupt(15) */
.word   Irq_EmptyHandler /* 22 External Interrupt(16) */
.word   Irq_EmptyHandler /* 23 External Interrupt(17) */
.word   Irq_EmptyHandler /* 24 External Interrupt(18) */
.word   Irq_EmptyHandler /* 25 External Interrupt(19) */
.word   Irq_EmptyHandler /* 26 External Interrupt(20) */
.word   Irq_EmptyHandler /* 27 External Interrupt(21) */
.word   Irq_EmptyHandler /* 28 External Interrupt(22) */
.word   Irq_EmptyHandler /* 29 External Interrupt(23) */
.word   Irq_EmptyHandler /* 30 External Interrupt(24) */
.word   Irq_EmptyHandler /* 31 External Interrupt(25) */
.word   Irq_EmptyHandler /* 32 External Interrupt(26) */
.word   Irq_EmptyHandler /* 33 External Interrupt(27) */
.word   Irq_EmptyHandler /* 34 External Interrupt(28) */
.word   Irq_EmptyHandler /* 35 External Interrupt(29) */
.word   Irq_EmptyHandler /* 36 External Interrupt(30) */
.word   Irq_EmptyHandler /* 37 External Interrupt(31) */
.word   Irq_EmptyHandler /* 38 External Interrupt(32) */
.word   Irq_EmptyHandler /* 39 External Interrupt(33) */
.word   Irq_EmptyHandler /* 40 External Interrupt(34) */
.word   Irq_EmptyHandler /* 41 External Interrupt(35) */
.word   Irq_EmptyHandler /* 42 External Interrupt(36) */
.word   Irq_EmptyHandler /* 43 External Interrupt(37) */
.word   Irq_EmptyHandler /* 44 External Interrupt(38) */
.word   Irq_EmptyHandler /* 45 External Interrupt(39) */
.word   Irq_EmptyHandler /* 46 External Interrupt(40) */
.word   Irq_EmptyHandler /* 47 External Interrupt(41) */
.word   Irq_EmptyHandler /* 48 External Interrupt(42) */
.word   Irq_EmptyHandler /* 49 External Interrupt(43) */
.word   Irq_EmptyHandler /* 50 External Interrupt(44) */
.word   Irq_EmptyHandler /* 51 External Interrupt(45) */
.word   Irq_EmptyHandler /* 52 External Interrupt(46) */
.word   Irq_EmptyHandler /* 53 External Interrupt(47) */
.word   Irq_EmptyHandler /* 54 External Interrupt(48) */
.word   Irq_EmptyHandler /* 55 External Interrupt(49) */
.word   Irq_EmptyHandler /* 56 External Interrupt(50) */
.word   Irq_EmptyHandler /* 57 External Interrupt(51) */
.word   Irq_EmptyHandler /* 58 External Interrupt(52) */
.word   Irq_EmptyHandler /* 59 External Interrupt(53) */
.word   Irq_EmptyHandler /* 60 External Interrupt(54) */
.word   Irq_EmptyHandler /* 61 External Interrupt(55) */
.word   Irq_EmptyHandler /* 62 External Interrupt(56) */
.word   Irq_EmptyHandler /* 63 External Interrupt(57) */
.word   Irq_EmptyHandler /* 64 External Interrupt(58) */
.word   ext_irq59_handler /* 65 External Interrupt(59) */
.word   Irq_EmptyHandler /* 66 External Interrupt(60) */
.word   Irq_EmptyHandler /* 67 External Interrupt(61) */
.word   Irq_EmptyHandler /* 68 External Interrupt(62) */
.word   Irq_EmptyHandler /* 69 External Interrupt(63) */
.word   Irq_EmptyHandler /* 70 External Interrupt(64) */
.word   Irq_EmptyHandler /* 71 External Interrupt(65) */
.word   Irq_EmptyHandler /* 72 External Interrupt(66) */
.word   Irq_EmptyHandler /* 73 External Interrupt(67) */
.word   Irq_EmptyHandler /* 74 External Interrupt(68) */
.word   Irq_EmptyHandler /* 75 External Interrupt(69) */
.word   Irq_EmptyHandler /* 76 External Interrupt(70) */
.word   Irq_EmptyHandler /* 77 External Interrupt(71) */
.word   Irq_EmptyHandler /* 78 External Interrupt(72) */
.word   Irq_EmptyHandler /* 79 External Interrupt(73) */
.word   Irq_EmptyHandler /* 80 External Interrupt(74) */
.word   Irq_EmptyHandler /* 81 External Interrupt(75) */
.word   Irq_EmptyHandler /* 82 External Interrupt(76) */
.word   Irq_EmptyHandler /* 83 External Interrupt(77) */
.word   Irq_EmptyHandler /* 84 External Interrupt(78) */
.word   Irq_EmptyHandler /* 85 External Interrupt(79) */
.word   Irq_EmptyHandler /* 86 External Interrupt(80) */
.word   Irq_EmptyHandler /* 87 External Interrupt(81) */
.word   Irq_EmptyHandler /* 88 External Interrupt(82) */
.word   Irq_EmptyHandler /* 89 External Interrupt(83) */
.word   Irq_EmptyHandler /* 90 External Interrupt(84) */
.word   Irq_EmptyHandler /* 91 External Interrupt(85) */
.word   Irq_EmptyHandler /* 92 External Interrupt(86) */
.word   Irq_EmptyHandler /* 93 External Interrupt(87) */
.word   ext_irq88_handler /* 94 External Interrupt(88) */
.word   ext_irq89_handler /* 95 External Interrupt(89) */
#.word   Irq_EmptyHandler /* 95 External Interrupt(89) */
.word   Irq_EmptyHandler /* 96 External Interrupt(90) */
.word   Irq_EmptyHandler /* 97 External Interrupt(91) */
.word   Irq_EmptyHandler /* 98 External Interrupt(92) */
.word   Irq_EmptyHandler /* 99 External Interrupt(93) */
.word   Irq_EmptyHandler /* 100 External Interrupt(94) */
.word   Irq_EmptyHandler /* 101 External Interrupt(95) */
.word   Irq_EmptyHandler /* 102 External Interrupt(96) */
.word   Irq_EmptyHandler /* 103 External Interrupt(97) */
.word   Irq_EmptyHandler /* 104 External Interrupt(98) */
.word   Irq_EmptyHandler /* 105 External Interrupt(99) */
.word   Irq_EmptyHandler /* 106 External Interrupt(100) */
.word   Irq_EmptyHandler /* 107 External Interrupt(101) */
.word   Irq_EmptyHandler /* 108 External Interrupt(102) */
.word   Irq_EmptyHandler /* 109 External Interrupt(103) */
.word   Irq_EmptyHandler /* 110 External Interrupt(104) */
.word   Irq_EmptyHandler /* 111 External Interrupt(105) */
.word   Irq_EmptyHandler /* 112 External Interrupt(106) */
.word   Irq_EmptyHandler /* 113 External Interrupt(107) */
.word   Irq_EmptyHandler /* 114 External Interrupt(108) */
.word   Irq_EmptyHandler /* 115 External Interrupt(109) */
.word   Irq_EmptyHandler /* 116 External Interrupt(110) */
.word   Irq_EmptyHandler /* 117 External Interrupt(111) */
.word   Irq_EmptyHandler /* 118 External Interrupt(112) */
.word   Irq_EmptyHandler /* 119 External Interrupt(113) */
.word   Irq_EmptyHandler /* 120 External Interrupt(114) */
.word   Irq_EmptyHandler /* 121 External Interrupt(115) */
.word   Irq_EmptyHandler /* 122 External Interrupt(116) */
.word   Irq_EmptyHandler /* 123 External Interrupt(117) */
.word   Irq_EmptyHandler /* 124 External Interrupt(118) */
.word   Irq_EmptyHandler /* 125 External Interrupt(119) */
.word   Irq_EmptyHandler /* 126 External Interrupt(120) */
.word   Irq_EmptyHandler /* 127 External Interrupt(121) */
.word   Irq_EmptyHandler /* 128 External Interrupt(122) */
.word   Irq_EmptyHandler /* 129 External Interrupt(123) */
.word   Irq_EmptyHandler /* 130 External Interrupt(124) */
.word   Irq_EmptyHandler /* 131 External Interrupt(125) */
.word   Irq_EmptyHandler /* 132 External Interrupt(126) */
.word   Irq_EmptyHandler /* 133 External Interrupt(127) */
.word   ext_irq128_handler /* 134 External Interrupt(128) */
.word   ext_irq129_handler /* 135 External Interrupt(129) */
.word   ext_irq130_handler /* 136 External Interrupt(130) */
.word   ext_irq131_handler /* 137 External Interrupt(131) */
.word   ext_irq132_handler /* 138 External Interrupt(132) */
.word   ext_irq133_handler /* 139 External Interrupt(133) */

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
  .macro    def_irq_handler    handler_name
  .align 1
  .thumb_func
  .weak    \handler_name
  .type    \handler_name, %function
\handler_name :
  b    .
  .size    \handler_name, . - \handler_name
  .endm

/*  Crash handler
 *   pass MSP, R4-R11 to CrashCatcher_Entry_Send_IRQ_to_Other_CPU.
 *   M7 stacks registers [xPSR, ReturnAddress(), LR (R14), R12, R3, R2, R1, and R0] when it enters an exception handler
 *       R4
 *       R5
 *       R6
 *       R7
 *       R8
 *       R9
 *       R10
 *       R11
 *       MSP
 *       ExceptionPSR
 *       R0
 *       R1
 *       R2
 *       R3
 *       R12
 *       LR
 *       PC
 *       PSR
 *
 */
  .thumb_func
  .weak   HardFault_Handler
  .type   HardFault_Handler, %function
  HardFault_Handler:
    /* Push msp into stack */
    mrs     r1, MSP
    add     r1, r1, #32
    mrs     r2, xpsr
    push    {r1, r2}
    push    {r4-r11}
    mov     r0, sp
    bl      CrashCatcher_Entry
    b       .

@  .thumb_func
@  .weak    Irq_SysTickHandler
@  .type    handler_name, %function
@   Irq_SysTickHandler:
@        b    .
@  .endm



@  def_irq_handler    NMI_Handler
@  def_irq_handler    HardFault_Handler
@  def_irq_handler    Handler_For_Other_CPU_HardFault
@  def_irq_handler    MemManage_Handler
@  def_irq_handler    BusFault_Handler
@  def_irq_handler    UsageFault_Handler
  def_irq_handler    SVC_Handler
  def_irq_handler    DebugMon_Handler
  def_irq_handler    PendSV_Handler
  def_irq_handler    Irq_SysTickHandler


.end
