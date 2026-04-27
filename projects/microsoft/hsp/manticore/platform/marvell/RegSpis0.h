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
//! @brief SPIS_0 Registers
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
        uint32_t RSVD8                       :1;      ///<BIT [0] RSVD8
        uint32_t TIMEOUT_ERR_STS             :1;      ///<BIT [1] TIMEOUT_ERR_STS
        uint32_t CS_ERR_STS                  :1;      ///<BIT [2] CS_ERR_STS
        uint32_t RSVD7                       :1;      ///<BIT [3] RSVD7
        uint32_t BUS_ERR_STS                 :1;      ///<BIT [4] BUS_ERR_STS
        uint32_t LACK_DATA_ERR_STS           :1;      ///<BIT [5] LACK_DATA_ERR_STS
        uint32_t ADDR_ALIG_ERR_STS           :1;      ///<BIT [6] ADDR_ALIG_ERR_STS
        uint32_t RSVD6                       :1;      ///<BIT [7] RSVD6
        uint32_t CMD_ERROR_STS               :1;      ///<BIT [8] CMD_ERROR_STS
        uint32_t CMD_DONE_STS                :1;      ///<BIT [9] CMD_DONE_STS
        uint32_t RSVD5                       :1;      ///<BIT [10] RSVD5
        uint32_t SIZE_ERR_STS                :1;      ///<BIT [11] SIZE_ERR_STS
        uint32_t ABORT_DONE_STS              :1;      ///<BIT [12] ABORT_DONE_STS
        uint32_t RSVD4                       :2;      ///<BIT [14:13] RSVD4
        uint32_t TPM_CA_RCVD_STS             :1;      ///<BIT [15] TPM_CA_RCVD_STS
        uint32_t RSVD3                       :1;      ///<BIT [16] RSVD3
        uint32_t TIMEOUT_ERR_MASK            :1;      ///<BIT [17] TIMEOUT_ERR_MASK
        uint32_t CS_ERR_MASK                 :1;      ///<BIT [18] CS_ERR_MASK
        uint32_t RSVD2                       :1;      ///<BIT [19] RSVD2
        uint32_t BUS_ERR_MASK                :1;      ///<BIT [20] BUS_ERR_MASK
        uint32_t LACK_DATA_ERR_MASK          :1;      ///<BIT [21] LACK_DATA_ERR_MASK
        uint32_t ADDR_ALIG_ERR_MASK          :1;      ///<BIT [22] ADDR_ALIG_ERR_MASK
        uint32_t RSVD1                       :1;      ///<BIT [23] RSVD1
        uint32_t CMD_ERROR_MASK              :1;      ///<BIT [24] CMD_ERROR_MASK
        uint32_t CMD_DONE_MASK               :1;      ///<BIT [25] CMD_DONE_MASK
        uint32_t RSVD0                       :1;      ///<BIT [26] RSVD0
        uint32_t SIZE_ERR_MASK               :1;      ///<BIT [27] SIZE_ERR_MASK
        uint32_t ABORT_DONE_MASK             :1;      ///<BIT [28] ABORT_DONE_MASK
        uint32_t RSVD                        :2;      ///<BIT [30:29] RSVD
        uint32_t TPM_CA_RCVD_MASK            :1;      ///<BIT [31] TPM_CA_RCVD_MASK
    } b;
} Spis0SpiTpmInt_t;

/// @brief 0x8
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t SPI_TPM_ERR_SIZE            :8;      ///<BIT [7:0] SPI_TPM_ERR_SIZE
        uint32_t SPI_TPM_ERR_CMD             :8;      ///<BIT [15:8] SPI_TPM_ERR_CMD
        uint32_t SPI_TPM_ERR_ADDR            :16;     ///<BIT [31:16] SPI_TPM_ERR_ADDR
    } b;
} Spis0SpiTpmErrState_t;

/// @brief 0x10
typedef union
{
    uint32_t all;///< All bits.
    struct
    {
        uint32_t TPM_XMIT_RDY                :1;      ///<BIT [0] TPM_XMIT_RDY
        uint32_t RSVD                        :2;      ///<BIT [2:1] RSVD
        uint32_t SPI_TPM_ABORT               :1;      ///<BIT [3] SPI_TPM_ABORT
        uint32_t TPM_CLEAN_BUF               :1;      ///<BIT [4] TPM_CLEAN_BUF
        uint32_t RSVD0                       :3;      ///<BIT [7:5] RSVD0
        uint32_t TPM_IF_MODE                 :1;      ///<BIT [8] TPM_IF_MODE
        uint32_t RSVD_9_11                   :3;      ///<BIT [11:9] rsvd_9_11
        uint32_t TPM_REG_BYPASS              :1;      ///<BIT [12] TPM_REG_BYPASS
        uint32_t SPIS_AXI_ENABLE             :1;      ///<BIT [13] SPIS_AXI_ENABLE
        uint32_t RSVD2                       :18;     ///<BIT [31:14] RSVD2
    } b;
} Spis0SpiTpmXferCtrl_t;

typedef struct
{
    Spis0SpiTpmInt_t spiTpmInt;                                             // 0x0 : SPI_TPM_INT / 
    uint32_t spiTpmTimeout;                                                 // 0x4 : SPI_TPM_TIMEOUT / 
    Spis0SpiTpmErrState_t spiTpmErrState;                                   // 0x8 : SPI_TPM_ERR_STATE / 
    uint8_t rsvdC[4];                                                       // 0xC : rsvd_c / rsvd_c
    Spis0SpiTpmXferCtrl_t spiTpmXferCtrl;                                   // 0x10 : SPI_TPM_XFER_CTRL / 
    uint32_t tpmRxData0;                                                    // 0x14 : TPM_RX_DATA_0 / 
    uint32_t tpmRxData1;                                                    // 0x18 : TPM_RX_DATA_1 / 
    uint8_t rsvd1c[4];                                                      // 0x1C : rsvd_1c / rsvd_1c
    uint32_t tpmTxData;                                                     // 0x20 : TPM_TX_DATA / 
    uint8_t rsvd24[12];                                                     // 0x24 : rsvd_24 / rsvd_24
    uint32_t tpmRxBufAddr;                                                  // 0x30 : TPM_RX_BUF_ADDR / 
    uint32_t tpmTxBufAddr;                                                  // 0x34 : TPM_TX_BUF_ADDR / 
    uint8_t rsvd38[200];                                                    // 0x38 : rsvd_38 / rsvd_38
    uint32_t tpmAccess0;                                                    // 0x100 : TPM_ACCESS_0 / 
    uint32_t tpmSts0;                                                       // 0x104 : TPM_STS_0 / 
    uint32_t tpmIntfCapability0;                                            // 0x108 : TPM_INTF_CAPABILITY_0 / 
    uint32_t tpmIntEnable0;                                                 // 0x10C : TPM_INT_ENABLE_0 / 
    uint32_t tpmIntStatus0;                                                 // 0x110 : TPM_INT_STATUS_0 / 
    uint32_t tpmIntVector0;                                                 // 0x114 : TPM_INT_VECTOR_0 / 
    uint32_t tpmDidVid0;                                                    // 0x118 : TPM_DID_VID_0 / 
    uint32_t tpmRid0;                                                       // 0x11C : TPM_RID_0 / 
    uint8_t rsvd120[32];                                                    // 0x120 : rsvd_120 / rsvd_120
    uint32_t tpmAccess1;                                                    // 0x140 : TPM_ACCESS_1 / 
    uint32_t tpmSts1;                                                       // 0x144 : TPM_STS_1 / 
    uint32_t tpmIntfCapability1;                                            // 0x148 : TPM_INTF_CAPABILITY_1 / 
    uint32_t tpmIntEnable1;                                                 // 0x14C : TPM_INT_ENABLE_1 / 
    uint32_t tpmIntStatus1;                                                 // 0x150 : TPM_INT_STATUS_1 / 
    uint32_t tpmIntVector1;                                                 // 0x154 : TPM_INT_VECTOR_1 / 
    uint32_t tpmDidVid1;                                                    // 0x158 : TPM_DID_VID_1 / 
    uint32_t tpmRid1;                                                       // 0x15C : TPM_RID_1 / 
    uint8_t rsvd160[32];                                                    // 0x160 : rsvd_160 / rsvd_160
    uint32_t tpmAccess2;                                                    // 0x180 : TPM_ACCESS_2 / 
    uint32_t tpmSts2;                                                       // 0x184 : TPM_STS_2 / 
    uint32_t tpmIntfCapability2;                                            // 0x188 : TPM_INTF_CAPABILITY_2 / 
    uint32_t tpmIntEnable2;                                                 // 0x18C : TPM_INT_ENABLE_2 / 
    uint32_t tpmIntStatus2;                                                 // 0x190 : TPM_INT_STATUS_2 / 
    uint32_t tpmIntVector2;                                                 // 0x194 : TPM_INT_VECTOR_2 / 
    uint32_t tpmDidVid2;                                                    // 0x198 : TPM_DID_VID_2 / 
    uint32_t tpmRid2;                                                       // 0x19C : TPM_RID_2 / 
    uint8_t rsvd1a0[32];                                                    // 0x1A0 : rsvd_1a0 / rsvd_1a0
    uint32_t tpmAccess3;                                                    // 0x1C0 : TPM_ACCESS_3 / 
    uint32_t tpmSts3;                                                       // 0x1C4 : TPM_STS_3 / 
    uint32_t tpmIntfCapability3;                                            // 0x1C8 : TPM_INTF_CAPABILITY_3 / 
    uint32_t tpmIntEnable3;                                                 // 0x1CC : TPM_INT_ENABLE_3 / 
    uint32_t tpmIntStatus3;                                                 // 0x1D0 : TPM_INT_STATUS_3 / 
    uint32_t tpmIntVector3;                                                 // 0x1D4 : TPM_INT_VECTOR_3 / 
    uint32_t tpmDidVid3;                                                    // 0x1D8 : TPM_DID_VID_3 / 
    uint32_t tpmRid3;                                                       // 0x1DC : TPM_RID_3 / 
    uint8_t rsvd1e0[32];                                                    // 0x1E0 : rsvd_1e0 / rsvd_1e0
    uint32_t tpmAccess4;                                                    // 0x200 : TPM_ACCESS_4 / 
    uint32_t tpmSts4;                                                       // 0x204 : TPM_STS_4 / 
    uint32_t tpmIntfCapability4;                                            // 0x208 : TPM_INTF_CAPABILITY_4 / 
    uint32_t tpmIntEnable4;                                                 // 0x20C : TPM_INT_ENABLE_4 / 
    uint32_t tpmIntStatus4;                                                 // 0x210 : TPM_INT_STATUS_4 / 
    uint32_t tpmIntVector4;                                                 // 0x214 : TPM_INT_VECTOR_4 / 
    uint32_t tpmDidVid4;                                                    // 0x218 : TPM_DID_VID_4 / 
    uint32_t tpmRid4;                                                       // 0x21C : TPM_RID_4 / 
} Spis0_t;

COMPILE_ASSERT(offsetof(Spis0_t,spiTpmInt)==0x0,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,spiTpmTimeout)==0x4,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,spiTpmErrState)==0x8,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,spiTpmXferCtrl)==0x10,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRxData0)==0x14,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRxData1)==0x18,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmTxData)==0x20,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRxBufAddr)==0x30,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmTxBufAddr)==0x34,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmAccess0)==0x100,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmSts0)==0x104,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntfCapability0)==0x108,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntEnable0)==0x10C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntStatus0)==0x110,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntVector0)==0x114,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmDidVid0)==0x118,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRid0)==0x11C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmAccess1)==0x140,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmSts1)==0x144,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntfCapability1)==0x148,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntEnable1)==0x14C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntStatus1)==0x150,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntVector1)==0x154,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmDidVid1)==0x158,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRid1)==0x15C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmAccess2)==0x180,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmSts2)==0x184,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntfCapability2)==0x188,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntEnable2)==0x18C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntStatus2)==0x190,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntVector2)==0x194,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmDidVid2)==0x198,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRid2)==0x19C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmAccess3)==0x1C0,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmSts3)==0x1C4,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntfCapability3)==0x1C8,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntEnable3)==0x1CC,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntStatus3)==0x1D0,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntVector3)==0x1D4,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmDidVid3)==0x1D8,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRid3)==0x1DC,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmAccess4)==0x200,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmSts4)==0x204,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntfCapability4)==0x208,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntEnable4)==0x20C,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntStatus4)==0x210,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmIntVector4)==0x214,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmDidVid4)==0x218,"check register structure offset");
COMPILE_ASSERT(offsetof(Spis0_t,tpmRid4)==0x21C,"check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------
extern volatile Spis0_t rSpis0; ///< 0xB000A000
