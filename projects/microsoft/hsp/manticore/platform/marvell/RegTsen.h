//-----------------------------------------------------------------------------
//
// Copyright (c) 2022 Marvell. All rights reserved.
// The following file is subject to the limited use license agreement by and
// between Marvell and you, your employer or other entity on behalf of whom
// you act. In the absence of such license agreement the following file is
// subject to Marvell's standard Limited Use License Agreement.
//
//-----------------------------------------------------------------------------

//=============================================================================
//!
//! @brief TSEN Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.3.

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SharedStruct.h"
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0x0
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH_SEL                 :3;      ///<BIT [2:0] TSEN_CH_SEL
        uint32_t TSEN_ADC_CH_SEL             :3;      ///<BIT [5:3] TSEN_ADC_CH_SEL
        uint32_t TSEN_ADC_MODE               :2;      ///<BIT [7:6] TSEN_ADC_MODE
        uint32_t TSEN_ADC_OSR                :2;      ///<BIT [9:8] TSEN_ADC_OSR
        uint32_t TSEN_ADC_CHOP_SEL           :2;      ///<BIT [11:10] TSEN_ADC_CHOP_SEL
        uint32_t TSEN_ADC_RESET              :1;      ///<BIT [12] TSEN_ADC_RESET
        uint32_t TSEN_ADC_ATEST_SEL          :2;      ///<BIT [14:13] TSEN_ADC_ATEST_SEL
        uint32_t TSEN_ADC_RAW_SEL            :2;      ///<BIT [16:15] TSEN_ADC_RAW_SEL
        uint32_t TSEN_ADC_CAL                :2;      ///<BIT [18:17] TSEN_ADC_CAL
        uint32_t TSEN_BG_TRIM                :4;      ///<BIT [22:19] TSEN_BG_TRIM
        uint32_t TSEN_ADC_CHOP_EN            :2;      ///<BIT [24:23] TSEN_ADC_CHOP_EN
        uint32_t TSEN_DEM_EN                 :1;      ///<BIT [25] TSEN_DEM_EN
        uint32_t TSEN_ADC_AVG_BYPASS         :1;      ///<BIT [26] TSEN_ADC_AVG_BYPASS
        uint32_t RSVD                        :2;      ///<BIT [28:27] RSVD_0
        uint32_t TSEN_ADC_START              :1;      ///<BIT [29] TSEN_ADC_START
        uint32_t TSEN_ADC_EN                 :1;      ///<BIT [30] TSEN_ADC_EN
        uint32_t TSEN_BIAS                   :1;      ///<BIT [31] TSEN_BIAS
    } b;
} TemperatureSensorControl_t;

/// @brief 0x4
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_ADC_DATA               :10;     ///<BIT [9:0] TSEN_ADC_DATA
        uint32_t RSVD_1                      :6;      ///<BIT [15:10] RSVD_1
        uint32_t TSEN_ADC_DATA_RAW           :10;     ///<BIT [25:16] TSEN_ADC_DATA_RAW
        uint32_t RSVD                        :5;      ///<BIT [30:26] RSVD_0
        uint32_t TSEN_ADC_RDY                :1;      ///<BIT [31] TSEN_ADC_RDY
    } b;
} TemperatureSensorStatus_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_SAMPLE_DATA            :10;     ///<BIT [9:0] TSEN_SAMPLE_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorSampleStatus_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_TRIP1_THR              :10;     ///<BIT [9:0] TSEN_TRIP1_THR
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorTrip1_t;

/// @brief 0x14
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_TRIP2_THR              :10;     ///<BIT [9:0] TSEN_TRIP2_THR
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorTrip2_t;

/// @brief 0x20
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :1;      ///<BIT [0] RSVD_1
        uint32_t TSEN_CH1_INT_PEND           :1;      ///<BIT [1] TSEN_CH1_INT_PEND
        uint32_t TSEN_CH2_INT_PEND           :1;      ///<BIT [2] TSEN_CH2_INT_PEND
        uint32_t TSEN_CH3_INT_PEND           :1;      ///<BIT [3] TSEN_CH3_INT_PEND
        uint32_t TSEN_CH4_INT_PEND           :1;      ///<BIT [4] TSEN_CH4_INT_PEND
        uint32_t TSEN_CH5_INT_PEND           :1;      ///<BIT [5] TSEN_CH5_INT_PEND
        uint32_t TSEN_CH6_INT_PEND           :1;      ///<BIT [6] TSEN_CH6_INT_PEND
        uint32_t TSEN_CH7_INT_PEND           :1;      ///<BIT [7] TSEN_CH7_INT_PEND
        uint32_t RSVD                        :22;     ///<BIT [29:8] RSVD_0
        uint32_t TSEN_TRIP2_ERR_INT_PEND     :1;      ///<BIT [30] TSEN_TRIP2_ERR_INT_PEND
        uint32_t TSEN_SAMPLE_RDY_INT_PEND    :1;      ///<BIT [31] TSEN_SAMPLE_RDY_INT_PEND
    } b;
} TemperatureSensorIntrPending_t;

/// @brief 0x24
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_1                      :1;      ///<BIT [0] RSVD_1
        uint32_t TSEN_CH1_INT_MSK            :1;      ///<BIT [1] TSEN_CH1_INT_MSK
        uint32_t TSEN_CH2_INT_MSK            :1;      ///<BIT [2] TSEN_CH2_INT_MSK
        uint32_t TSEN_CH3_INT_MSK            :1;      ///<BIT [3] TSEN_CH3_INT_MSK
        uint32_t TSEN_CH4_INT_MSK            :1;      ///<BIT [4] TSEN_CH4_INT_MSK
        uint32_t TSEN_CH5_INT_MSK            :1;      ///<BIT [5] TSEN_CH5_INT_MSK
        uint32_t TSEN_CH6_INT_MSK            :1;      ///<BIT [6] TSEN_CH6_INT_MSK
        uint32_t TSEN_CH7_INT_MSK            :1;      ///<BIT [7] TSEN_CH7_INT_MSK
        uint32_t RSVD                        :22;     ///<BIT [29:8] RSVD_0
        uint32_t TSEN_TRIP2_ERR_INT_MSK      :1;      ///<BIT [30] TSEN_TRIP2_ERR_INT_MSK
        uint32_t TSEN_SAMPLE_RDY_INT_MSK     :1;      ///<BIT [31] TSEN_SAMPLE_RDY_INT_MSK
    } b;
} TemperatureSensorIntrMask_t;

/// @brief 0x28
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t RSVD_2                      :1;      ///<BIT [0] RSVD_2
        uint32_t TSEN_CH1_INT                :1;      ///<BIT [1] TSEN_CH1_INT
        uint32_t TSEN_CH2_INT                :1;      ///<BIT [2] TSEN_CH2_INT
        uint32_t TSEN_CH3_INT                :1;      ///<BIT [3] TSEN_CH3_INT
        uint32_t TSEN_CH4_INT                :1;      ///<BIT [4] TSEN_CH4_INT
        uint32_t TSEN_CH5_INT                :1;      ///<BIT [5] TSEN_CH5_INT
        uint32_t TSEN_CH6_INT                :1;      ///<BIT [6] TSEN_CH6_INT
        uint32_t TSEN_CH7_INT                :1;      ///<BIT [7] TSEN_CH7_INT
        uint32_t RSVD_1                      :22;     ///<BIT [29:8] RSVD_1
        uint32_t TSEN_TRIP2_ERR_INT          :1;      ///<BIT [30] TSEN_TRIP2_ERR_INT
        uint32_t TSEN_SAMPLE_RDY_INT         :1;      ///<BIT [31] TSEN_SAMPLE_RDY_INT
    } b;
} TemperatureSensorIntrStatus_t;

/// @brief 0x34
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH1_DATA               :10;     ///<BIT [9:0] TSEN_CH1_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel1Status_t;

/// @brief 0x38
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH2_DATA               :10;     ///<BIT [9:0] TSEN_CH2_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel2Status_t;

/// @brief 0x3C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH3_DATA               :10;     ///<BIT [9:0] TSEN_CH3_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel3Status_t;

/// @brief 0x40
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH4_DATA               :10;     ///<BIT [9:0] TSEN_CH4_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel4Status_t;

/// @brief 0x44
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH5_DATA               :10;     ///<BIT [9:0] TSEN_CH5_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel5Status_t;

/// @brief 0x48
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH6_DATA               :10;     ///<BIT [9:0] TSEN_CH6_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel6Status_t;

/// @brief 0x4C
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH7_DATA               :10;     ///<BIT [9:0] TSEN_CH7_DATA
        uint32_t RSVD                        :22;     ///<BIT [31:10] RSVD_0
    } b;
} TemperatureSensorChannel7Status_t;

/// @brief 0x50
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TSEN_CH_MEAS_BEGIN          :3;      ///<BIT [2:0] TSEN_CH_MEAS_BEGIN
        uint32_t RSVD_3                      :1;      ///<BIT [3] RSVD_3
        uint32_t TSEN_CH_MEAS_END            :3;      ///<BIT [6:4] TSEN_CH_MEAS_END
        uint32_t RSVD_2                      :1;      ///<BIT [7] RSVD_2
        uint32_t TSEN_CH_MEAS_START          :1;      ///<BIT [8] TSEN_CH_MEAS_START
        uint32_t TSEN_CH_MEAS_STOP           :1;      ///<BIT [9] TSEN_CH_MEAS_STOP
        uint32_t RSVD_1                      :6;      ///<BIT [15:10] RSVD_1
        uint32_t TSEN_CH_MEAS_LOOP_CNT       :4;      ///<BIT [19:16] TSEN_CH_MEAS_LOOP_CNT
        uint32_t RSVD                        :12;     ///<BIT [31:20] RSVD_0
    } b;
} TemperatureSensorControl2_t;

typedef struct
{
    TemperatureSensorControl_t temperatureSensorControl;                    // 0x0 : Temperature_Sensor_Control / 
    TemperatureSensorStatus_t temperatureSensorStatus;                      // 0x4 : Temperature_Sensor_Status / 
    TemperatureSensorSampleStatus_t temperatureSensorSampleStatus;          // 0x8 : Temperature_Sensor_Sample_Status / 
    uint8_t rsvdC[4];                                                       // 0xC : rsvd_c / rsvd_c
    TemperatureSensorTrip1_t temperatureSensorTrip1;                        // 0x10 : Temperature_Sensor_Trip1 / 
    TemperatureSensorTrip2_t temperatureSensorTrip2;                        // 0x14 : Temperature_Sensor_Trip2 / 
    uint8_t rsvd18[8];                                                      // 0x18 : rsvd_18 / rsvd_18
    TemperatureSensorIntrPending_t temperatureSensorIntrPending;            // 0x20 : Temperature_Sensor_Interrupt_Pending / 
    TemperatureSensorIntrMask_t temperatureSensorIntrMask;                  // 0x24 : Temperature_Sensor_Interrupt_Mask / 
    TemperatureSensorIntrStatus_t temperatureSensorIntrStatus;              // 0x28 : Temperature_Sensor_Interrupt_Status / 
    uint8_t rsvd2c[8];                                                      // 0x2C : rsvd_2c / rsvd_2c
    TemperatureSensorChannel1Status_t temperatureSensorChannel1Status;      // 0x34 : Temperature_Sensor_Channel_1_Status / 
    TemperatureSensorChannel2Status_t temperatureSensorChannel2Status;      // 0x38 : Temperature_Sensor_Channel_2_Status / 
    TemperatureSensorChannel3Status_t temperatureSensorChannel3Status;      // 0x3C : Temperature_Sensor_Channel_3_Status / 
    TemperatureSensorChannel4Status_t temperatureSensorChannel4Status;      // 0x40 : Temperature_Sensor_Channel_4_Status / 
    TemperatureSensorChannel5Status_t temperatureSensorChannel5Status;      // 0x44 : Temperature_Sensor_Channel_5_Status / 
    TemperatureSensorChannel6Status_t temperatureSensorChannel6Status;      // 0x48 : Temperature_Sensor_Channel_6_Status / 
    TemperatureSensorChannel7Status_t temperatureSensorChannel7Status;      // 0x4C : Temperature_Sensor_Channel_7_Status / 
    TemperatureSensorControl2_t temperatureSensorControl2;                  // 0x50 : Temperature_Sensor_Control_2 / 
} Tsen_t;

COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorControl)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorStatus)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorSampleStatus)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorTrip1)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorTrip2)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorIntrPending)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorIntrMask)==0x24,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorIntrStatus)==0x28,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel1Status)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel2Status)==0x38,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel3Status)==0x3C,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel4Status)==0x40,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel5Status)==0x44,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel6Status)==0x48,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorChannel7Status)==0x4C,"check register structure offset");
COMPILE_ASSERT(offsetof(Tsen_t,temperatureSensorControl2)==0x50,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Tsen_t rTsen; ///< 0xB0008000
