// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#include "platform.h"
void Init_MPU(void)
{
    //
    //  MPU
    //  support 8 regions
    //
    //     region 0: back-ground         base=0x00000000, size=4G                 Normal-Dev (R/W, TEX=0,S=0,C=0,B=1, Enable=1)
    //     region 1: IRAM                base=0x00000000  size=512M               Normal-Mem (R/W, TEX=1,S=0,C=0,B=0, Enable=1)
    //     region 2: DRAM                base=0x20000000  size=512M               Normal-Mem (R/W, TEX=1,S=0,C=0,B=0, Enable=1)
    //     region 3: iSRAM               base=0xb0460000, size=128K               Normal-Mem (R/W, TEX=1,S=1,C=0,B=0, Enable=1)
    //     region 4: pSRAM               base=0x00010000, size=32K                Normal-Mem (R/W, TEX=1,S=1,C=0,B=0, Enable=1)
    //     region 5: Alias               base=0x60000000, size=512M               Normal-Mem (R/W, TEX=1,S=1,C=0,B=0, Enable=1)
    //     region 6: Stack_Protect       base=stack_bot,  size=32bytes            Normal-Mem (RO,  TEX=1,S=1,C=0,B=0, Enable=1)
    //     region 7: pSRAM_SOC           base=0xa3e00000, size=32K                Normal-Mem (R/W, TEX=1,S=1,C=1,B=1, Enable=1)
    //

    // region 0
    MPU->RBAR = ((0x00000000UL & MPU_RBAR_ADDR_Msk) | (0x0UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x0UL << MPU_RASR_TEX_Pos) | (0x0UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x1UL << MPU_RASR_B_Pos) | (0x1fUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 1
    MPU->RBAR = ((0x00000000UL & MPU_RBAR_ADDR_Msk) | (0x1UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x0UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x1cUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 2
    MPU->RBAR = ((0x20000000UL & MPU_RBAR_ADDR_Msk) | (0x2UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x0UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x1cUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 3
    MPU->RBAR = ((0xb0460000UL & MPU_RBAR_ADDR_Msk) | (0x3UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x1UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x10UL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 4
    MPU->RBAR = ((0x00010000UL & MPU_RBAR_ADDR_Msk) | (0x4UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x1UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x0eUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 5
    MPU->RBAR = ((0x60000000UL & MPU_RBAR_ADDR_Msk) | (0x5UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x1UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x1cUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

#ifdef ENABLE_STACK_PROTECT
    // region 6
    #ifdef fps_cpu0Core
    MPU->RBAR = ((FPS_CPU0_STACK_PROTECT & MPU_RBAR_ADDR_Msk) | (0x6UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));
    #elif defined (fps_cpu1Core)
    MPU->RBAR = ((FPS_CPU1_STACK_PROTECT & MPU_RBAR_ADDR_Msk) | (0x6UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));
    #else
    MPU->RBAR = ((FPS_CPU2_STACK_PROTECT & MPU_RBAR_ADDR_Msk) | (0x6UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));
    #endif //fps_cpu0Core
#endif
    MPU->RASR = ((0x7UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x0UL << MPU_RASR_S_Pos) | (0x0UL << MPU_RASR_C_Pos) | (0x0UL << MPU_RASR_B_Pos) | (0x04UL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // region 7
    MPU->RBAR = ((0xa3e00000UL & MPU_RBAR_ADDR_Msk) | (0x7UL << MPU_RBAR_REGION_Pos) | (0x1UL << MPU_RBAR_VALID_Pos));

    MPU->RASR = ((0x3UL << MPU_RASR_AP_Pos) | (0x1UL << MPU_RASR_TEX_Pos) | (0x1UL << MPU_RASR_S_Pos) | (0x1UL << MPU_RASR_C_Pos) | (0x1UL << MPU_RASR_B_Pos) | (0x0eUL << MPU_RASR_SIZE_Pos) | (0x1UL << MPU_RASR_ENABLE_Pos));

    // enable MPU
    MPU->CTRL = 0x3;

    __DSB();
    __ISB();
}
