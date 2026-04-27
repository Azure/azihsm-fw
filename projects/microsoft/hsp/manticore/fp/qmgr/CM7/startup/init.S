// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "M7MemMap.h"


.thumb
.syntax unified
.global _start
_start:
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    movw r8 , #0
    movw r9 , #0
    movw r10 , #0
    movw r11 , #0
    movw r12 , #0

    B jumpSystemInit

jumpSystemInit:
    ldr r0,=SystemInit
    bx r0


.end
