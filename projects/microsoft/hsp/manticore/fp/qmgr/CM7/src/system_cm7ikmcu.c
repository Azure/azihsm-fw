/*************************************************************************//**
 * @file     system_cm7ikmcu.c
 * @brief    CMSIS-Core(M) Device Peripheral Access Layer Source File
 *           for Cortex-M7 Device
 * @version  V1.0.0
 * @date     20. January 2021
 *****************************************************************************/
/*
 * Copyright (c) 2009-2025 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SysTypes.h"
#include "cm7ikmcu.h"
#include "M7MemMap.h"
#include "M7Partition.h"

/*----------------------------------------------------------------------------
 * Clock configuration
 *----------------------------------------------------------------------------*/
// #define __HSI          (5000000UL)
#define SYSTEM_CLOCK   (50000000UL)

// /*---------------------------------------------------------------------------
//   Exception / Interrupt Vector table
//  *---------------------------------------------------------------------------*/
// extern const VECTOR_TABLE_Type __VECTOR_TABLE[496];

/*---------------------------------------------------------------------------
  System Core Clock Variable
 *---------------------------------------------------------------------------*/
/* ToDo: Initialize SystemCoreClock with the system core clock frequency value
         achieved after system intitialization.
         This means system core clock frequency after call to SystemInit() */
uint32_t SystemCoreClock = SYSTEM_CLOCK;  /* System Clock Frequency (Core Clock)*/

/* Forward declaration */
extern int main(void);

#include "mpu.c"
#include "irq.h"

/*---------------------------------------------------------------------------
  System Core Clock function
 *---------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
/* ToDo: Add code to calculate the system frequency based upon the current
         register settings.
         This function can be used to retrieve the system core clock frequeny
         after user changed register sittings. */
  SystemCoreClock = SYSTEM_CLOCK;
}

/*---------------------------------------------------------------------------
  System initialization function
 *---------------------------------------------------------------------------*/
void SystemInit(void)
{
  /* MPU */
  Init_MPU();

  /* NVIC / VIC */
  Irq_Init(VIC_IRQ_ENABLE_BIT);

  /* Jump to main (this project's entry from startup) */
  main();
}