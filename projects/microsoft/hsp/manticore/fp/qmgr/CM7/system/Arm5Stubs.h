// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

/***********************************************************************************
 *
 * @file Arm5Stubs.h
 *
 * The Stub functions for Armcc 5
 *
 ***********************************************************************************/
#pragma once

/**
 *  Get return address
 *
 *  @return     return address
 */
__inline unsigned int __return_address(void)
{
    unsigned int result;
    __asm volatile ("MOV %0, LR\n" : "=r" (result));
    return result;
}
