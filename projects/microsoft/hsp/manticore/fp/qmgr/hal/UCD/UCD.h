// SPDX-License-Identifier: MIT
// Copyright (c) 2021-2026 Marvell

#pragma once

#include "platform.h"
#include "RegUcd.h"
#include "assert.h"


#define UCD_DSTRD_DEFAULT 0


#define UCD_PF_MASK    ((1UL << MAX_PF_NUM) - 1)
#define UCD_VF_MASK    ((1ULL << (MAX_VF_NUM / 2)) - 1)

// common_irq_cause/common_irq0_en/common_irq1_en bits
#define UCD_IRQ_COMMON_DATA_PATH_ERR     (1 << 31)
#define UCD_IRQ_NVME_RESET_RCV_VF        (1 << 13)
#define UCD_IRQ_NVME_RESET_RCV_PF        (1 << 12)
#define UCD_IRQ_NVME_SHN_UPD_VF          (1 << 11)
#define UCD_IRQ_NVME_SHN_UPD_PF          (1 << 10)
#define UCD_IRQ_NVME_EN_UPD_VF           (1 << 9)
#define UCD_IRQ_NVME_EN_UPD_PF           (1 << 8)
#define UCD_IRQ_NVME_PF_EVENT_MASK    (UCD_IRQ_NVME_EN_UPD_PF | UCD_IRQ_NVME_SHN_UPD_PF | UCD_IRQ_NVME_RESET_RCV_PF)
#define UCD_IRQ_NVME_VF_EVENT_MASK    (UCD_IRQ_NVME_EN_UPD_VF | UCD_IRQ_NVME_SHN_UPD_VF | UCD_IRQ_NVME_RESET_RCV_VF)

#define UCD_CMN_IRQ_NORMAL       \
    (UCD_IRQ_NVME_EN_UPD_VF |    \
     UCD_IRQ_NVME_EN_UPD_PF |    \
     UCD_IRQ_NVME_RESET_RCV_VF | \
     UCD_IRQ_NVME_RESET_RCV_PF | \
     UCD_IRQ_NVME_SHN_UPD_VF |   \
     UCD_IRQ_NVME_SHN_UPD_PF)

// IBQ registers, XBQ indicates shared IBQ/OBQ fields
#define UCD_XBQ_CFG_SZ_SHFT                     16
#define UCD_IBQ_CFG_DFL_SHFT                    8
#define UCD_IBQ_CFG_PRIO_SHFT                   4

#define UCD_XBQ_CFG_PASSTHROUGH_EN             (1 << 7)  //(1 << 3)
#define UCD_XBQ_CFG_RST                         (1 << 3)  //(1 << 7)
//#define UCD_XBQ_CFG_SHDW_EN                     (1 << 1)
#define UCD_XBQ_CFG_EN                          (1 << 0)

#define UCD_XBQ_CFG1_IFSEL_SHFT                 8

#define UCD_XBQ_CFG1_SHDW_IFSEL_SHFT    0   ///< remove

// OBQ registers
#define UCD_OBQ_CFG_ELSZ_SHFT                   4
#define UCD_OBQ_CFG_PHSBIT_EN                   (1 << 2)
#define UCD_OBQ_CFG_CI_UPDT_EN                  (1 << 8)

#define UCD_OBQ_ICFG0_COAL_TIMEMIN_SHFT         16
#define UCD_OBQ_ICFG0_COAL_TIMEMAX_SHFT         0

#define UCD_OBQ_ICFG1_EN_CI31_TMR_RST           (1 << 19)
#define UCD_OBQ_ICFG1_EN_CIW_REARM              (1 << 18)
#define UCD_OBQ_ICFG1_EN_W_REARM                (1 << 17)
#define UCD_OBQ_ICFG1_EN_INT_COAL               (1 << 16)
#define UCD_OBQ_ICFG1_COAL_COUNT_SHFT           0

#define UCD_OBQ_ICFG2_EN_EXT_TMR_RST            (1 << 31)
#define UCD_OBQ_ICFG2_EN_GEN_MSIX               (1 << 30)
#define UCD_OBQ_ICFG2_MSIX_TBL_SHFT             4
#define UCD_OBQ_ICFG2_MSIX_VECT_SHFT            0

// DFL registers
#define UCD_DFL_CFG_BUFLEN_SHFT                 16
#define UCD_DFL_CFG_SZ_SHFT                     0x8UL  //8
#define UCD_DFL_CFG_EN                          1

#define UCD_DFL_CFG1_IFSEL_SHFT                 8
#define UCD_DFL_CFG1_BUF_IFSEL_SHFT             0

// OSL registers
#define UCD_OSL_CFG_IFSEL_SHFT                  16
#define UCD_OSL_CFG_SZ_SHFT                     8
#define UCD_OSL_CFG_EN                          1
///< OSL entry
#define UCD_OSL_ENTRY_DW3_QPID_SH    0x10UL
#define UCD_OSL_ENTRY_DW3_CREDIT_SH    0x8UL
#define UCD_CREDIT_CNT_NUM    0x1UL


// CQ registers
#define UCD_CQ_CTRL_IFSEL_SH                    16
#define UCD_CQ_CTRL_SIZE_SH                     8
#define UCD_CQ_CTRL_SHDW_EN                    (1 << 1)
#define UCD_CQ_CTRL_Q_EN                       (1 << 0)
#define UCD_CQ_CI_IRQCLR                       (1 << 30)

//UCD IB CMN register
#define UCD_IB_CMN_SNGL_PAUSE ((uint32_t)1 << 1)
#define UCD_IB_CMN_SNGL_STATUS_BUSY ((uint32_t)1 << 12)

//UCD OB CMN register
#define UCD_OB_CMN_SNGL_PAUSE ((uint32_t)1 << 1)

#define COALSCING_TH 1
#define IFSEL_DEFAULT 0

#define SLOT_IBQ0_VALID_FLAG BIT(0)
#define SLOT_IBQ1_VALID_FLAG BIT(1)


typedef enum PCIeFuctcionInterface_t
{
    cPCIeFuctcionInterfacePf0                   = 0x10,         ///< PCIe Physical fuction 0 interface selection
    cPCIeFuctcionInterfaceVf0                   = 0x20,         ///< PCIe Virtual fuction 0 interface selection
    cPCIeFuctcionInterfaceVfMAX                 = (0x20 + MAX_VF_NUM - 1)          ///< Virtual fuction max interface selection
} PCIeFuctcionInterface_t;

typedef enum VfInstallTearDownAction_t
{
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    cActionTearDown = 0x0,
    cActionVfInstall = 0x1
    #else
    #ifdef NEW_VF_QUEUE_MSG_STRUCTURE
    cActionTearDown = 0x0,
    cActionVfInstall = 0x1
    #else
    cActionVfInstall = 0x0,
    cActionTearDown = 0x1
    #endif
    #endif
} VfInstallTearDownAction_t;

typedef enum VfSlotSQ2CQMapUpdateAction_t
{
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    cActionRemove = 0x0,
    cActionCreate = 0x1,
    #else
    #ifdef NEW_VF_QUEUE_MSG_STRUCTURE
    cActionRemove = 0x0,
    cActionCreate = 0x1,
    #else
    cActionCreate = 0x0,
    cActionRemove = 0x1,
    #endif
    #endif
    cActionForceCompletion = 0x2
} VfSlotSQ2CQMapUpdateAction_t;

typedef enum keyUpdateAction_t
{
    #ifdef NEW_AES_KEY_VALIDATION_SUPPORT
    cActionKeyDisable = 0x0,
    cActionEphemeralKeyForSessionDelete = 0x1,
    cActionAllKeysDeleteForApp = 0x2,
    cActionKeyEnable = 0x3,
    cActionKeyInvalidAction
    #else
    #ifdef NEW_VF_QUEUE_MSG_STRUCTURE
    cActionKeyDisable = 0x0,
    cActionKeyEnable = 0x1
    #else
    cActionKeyEnable = 0x0,
    cActionKeyDisable = 0x1
    #endif
    #endif
} keyUpdateAction_t;

typedef enum SlotStatusFlag_t
{
    cStsInit = 0,
    cStsValid = BIT(0),
    cStsForceCompletion = BIT(1),
    cStsDelete = BIT(2),
    cStsTearDown = BIT(3),
    cStsFwUpdate = BIT(4),
}SlotStatusFlag_t;

typedef enum UcdDir_t
{
    cUcdIb = 0,
    cUcdOb = 1
} UcdDir_t;

typedef enum UcdHiuId_t
{
    cUcdHiuVf0 = 0,
    cUcdHiuVfNum = MAX_VF_NUM,
    cUcdHiuPf0 = cUcdHiuVfNum,
    cUcdHiuNum = 65,
} UcdHiuId_t;

typedef enum UcdCqIds_t
{
    cUcdCqFp1,
    cUcdCqFp2,
    cUcdCqRr1,
    cUcdCqRr2,
    cUcdCqRR3
} UcdCqIds_t;


typedef struct UcdPfVf_t
{
    uint32_t PfAll;
    union
    {
        struct
        {
            uint32_t Vf0;
            uint32_t Vf1;
        };
        uint64_t VfAll;
    };
} UcdPfVf_t;

/*
 * OSL : Outbound Source List
 *
 */
typedef struct UcdOslEntry_t
{
    union
    {
        uint64_t                Addr;   //address of the CQ entry to be sent
        struct
        {
            uint32_t addrLow;
            uint32_t addrHi;
        };
    };

    union
    {
        struct
        {
            uint32_t        Dw3;
            uint32_t        Dw4;
        };
        struct
        {
            #if 1
            uint32_t        SQPId : 8;
            uint32_t        Reserved : 8;
            #else
            uint32_t        Reserved : 16;
            #endif
            uint32_t        QPId : 8;
            uint32_t        IFSsel : 8;   //memory addr refers to
            uint32_t        Tag : 24;    //SW cookie
            uint32_t        Ctrl : 8;
        };
    };
} UcdOslEntry_t;

typedef struct UcdOslDesc_t
{
    UcdOslEntry_t*     pEntries;
    uint32_t               Pi;
    uint32_t               Ci;
    uint32_t               IMask;
} UcdOslDesc_t;

typedef struct UcdCqEntry_t
{
    union
    {
        uint64_t        Addr;
        struct
        {
            uint32_t AddrLow;
            uint32_t AddrHigh;
        };
    };
    union
    {
        struct
        {
            uint32_t    Dw3;
            uint32_t    Dw4;
        };
        struct
        {
            uint32_t    Ci : 16;
            uint32_t    QPId : 8;
            uint32_t    IFSel : 8;
            union
            {
                uint32_t Status;
                struct /* ib_status */
                {
                    uint32_t    ListId : 3;/* IB: DFL index */
                    uint32_t    Abort : 1; /* 1: abort cmd */
                    uint32_t    Reserved : 25;
                    uint32_t    ErrSts : 2;
                    uint32_t    Good : 1;/* 0 = error */
                };
                struct /* ob_status */
                {
                    uint32_t    Tag : 24;/* tag in OSL entry */
                    uint32_t    NoData : 1;
                    uint32_t    OQ_Offline : 1;
                    uint32_t    OQ_Overflow : 1;
                    uint32_t    Skpd : 1;
                    uint32_t    Reserved3 : 1;
                    uint32_t    OB_ErrSts : 2;
                    uint32_t    OB_Good : 1;
                };
            };
        };
    };
}UcdCqEntry_t;

#define UCD_IFSEL_TO_VFID(ifsel) ((ifsel) - cPCIeFuctcionInterfaceVf0)
#define UCD_VFID_TO_IFSEL(vfid) ((vfid) + cPCIeFuctcionInterfaceVf0)
#define UCD_IFSEL_TO_PFID(ifsel) ((ifsel) - cPCIeFuctcionInterfacePf0)
#define UCD_PFID_TO_IFSEL(pfid) (((pfid) == 0) ? (cPCIeFuctcionInterfacePf0) : 0xFF)
#define UCD_IFSEL_TO_PFVFID(ifsel) (((ifsel) < 0x20) ? UCD_IFSEL_TO_PFID(ifsel) : UCD_IFSEL_TO_VFID(ifsel))
#define UCD_IFSEL_TO_HIUID(ifsel) (((ifsel) < 0x20) ? (UCD_IFSEL_TO_PFID(ifsel) + MAX_VF_NUM) : UCD_IFSEL_TO_VFID(ifsel))
#define UCD_HIUID_TO_IFSEL(hiu) (((hiu) == cUcdHiuPf0) ? (UCD_PFID_TO_IFSEL((hiu) - MAX_VF_NUM)) : UCD_VFID_TO_IFSEL(hiu))

#define UCD_GET_GQPID(CoreId, QPId) ((CoreId) ? ((QPId) + MAX_UCD_Q_NUM_BY_CORE) : (QPId))


