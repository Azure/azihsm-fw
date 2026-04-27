// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2026 Marvell

//=============================================================================
//!
//! @brief CORTEXM7 Registers
//!
//=============================================================================

// Generated with Dullahan v2.4.2.cfa8763

#pragma once

//-----------------------------------------------------------------------------
//  Dependencies
//-----------------------------------------------------------------------------

#include <stddef.h>
#include <stdint.h>
#include "SysTypes.h"

//-----------------------------------------------------------------------------
//  Public Constant Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Public Data Type Definitions
//-----------------------------------------------------------------------------


/// @brief 0xFF000
typedef struct
{
    uint32_t entry0;                      //ENTRY_0
    uint32_t entry1;                      //ENTRY_1
    uint32_t entry2;                      //ENTRY_2
    uint32_t entry3;                      //ENTRY_3
    uint32_t entry4;                      //ENTRY_4
    uint32_t entry5;                      //ENTRY_5
    uint32_t entry6;                      //ENTRY_6
    uint8_t rsvd1c[4016];                 //rsvd_1c
    uint32_t memType;                     //MEM_TYPE
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} PPBROMTable_t;

/// @brief 0xFE000
typedef struct
{
    uint32_t entry0;                      //ENTRY_0
    uint32_t entry1;                      //ENTRY_1
    uint32_t entry2;                      //ENTRY_2
    uint32_t entry3;                      //ENTRY_3
    uint32_t entry4;                      //ENTRY_4
    uint8_t rsvd14[4024];                 //rsvd_14
    uint32_t memType;                     //MEM_TYPE
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} ProcessorROMTable_t;

/// @brief 0x42000
typedef struct
{
    uint32_t cticontrol;                  //CTICONTROL
    uint8_t rsvd4[12];                    //rsvd_4
    uint32_t ctiintack;                   //CTIINTACK
    uint32_t ctiappset;                   //CTIAPPSET
    uint32_t ctiappclear;                 //CTIAPPCLEAR
    uint32_t ctiapppulse;                 //CTIAPPPULSE
    uint32_t ctiinen0;                    //CTIINEN0
    uint32_t ctiinen1;                    //CTIINEN1
    uint32_t ctiinen2;                    //CTIINEN2
    uint32_t ctiinen3;                    //CTIINEN3
    uint32_t ctiinen4;                    //CTIINEN4
    uint32_t ctiinen5;                    //CTIINEN5
    uint32_t ctiinen6;                    //CTIINEN6
    uint32_t ctiinen7;                    //CTIINEN7
    uint8_t rsvd40[96];                   //rsvd_40
    uint32_t ctiouten0;                   //CTIOUTEN0
    uint32_t ctiouten1;                   //CTIOUTEN1
    uint32_t ctiouten2;                   //CTIOUTEN2
    uint32_t ctiouten3;                   //CTIOUTEN3
    uint32_t ctiouten4;                   //CTIOUTEN4
    uint32_t ctiouten5;                   //CTIOUTEN5
    uint32_t ctiouten6;                   //CTIOUTEN6
    uint32_t ctiouten7;                   //CTIOUTEN7
    uint8_t rsvdC0[112];                  //rsvd_c0
    uint32_t ctitriginstatus;             //CTITRIGINSTATUS
    uint32_t ctitrigoutstatus;            //CTITRIGOUTSTATUS
    uint32_t ctichinstatus;               //CTICHINSTATUS
    uint32_t ctichoutstatus;              //CTICHOUTSTATUS
    uint32_t ctigate;                     //CTIGATE
    uint32_t asicctl;                     //ASICCTL
    uint8_t rsvd148[3512];                //rsvd_148
    uint32_t ctiitctrl;                   //CTIITCTRL
    uint8_t rsvdF04[156];                 //rsvd_f04
    uint32_t cticlaimset;                 //CTICLAIMSET
    uint32_t cticlaimclr;                 //CTICLAIMCLR
    uint8_t rsvdFa8[8];                   //rsvd_fa8
    uint32_t ctilar;                      //CTILAR
    uint32_t ctilsr;                      //CTILSR
    uint32_t ctiauthstatus;               //CTIAUTHSTATUS
    uint8_t rsvdFbc[12];                  //rsvd_fbc
    uint32_t ctidevid;                    //CTIDEVID
    uint32_t ctidevtype;                  //CTIDEVTYPE
    uint32_t ctipidr4;                    //CTIPIDR4
    uint32_t ctipidr5;                    //CTIPIDR5
    uint32_t ctipidr6;                    //CTIPIDR6
    uint32_t ctipidr7;                    //CTIPIDR7
    uint32_t ctipidr0;                    //CTIPIDR0
    uint32_t ctipidr1;                    //CTIPIDR1
    uint32_t ctipidr2;                    //CTIPIDR2
    uint32_t ctipidr3;                    //CTIPIDR3
    uint32_t cticidr0;                    //CTICIDR0
    uint32_t cticidr1;                    //CTICIDR1
    uint32_t cticidr2;                    //CTICIDR2
    uint32_t cticidr3;                    //CTICIDR3
} Cti_t;

/// @brief 0x41000
typedef struct
{
    uint8_t rsvd0[4];                     //rsvd_0
    uint32_t trcprgctlr;                  //TRCPRGCTLR
    uint32_t trcprocselr;                 //TRCPROCSELR
    uint32_t trcstatr;                    //TRCSTATR
    uint32_t trcconfigr;                  //TRCCONFIGR
    uint8_t rsvd14[12];                   //rsvd_14
    uint32_t trceventctl0r;               //TRCEVENTCTL0R
    uint32_t trceventctl1r;               //TRCEVENTCTL1R
    uint8_t rsvd28[4];                    //rsvd_28
    uint32_t trcstallctlr;                //TRCSTALLCTLR
    uint32_t trctsctlr;                   //TRCTSCTLR
    uint32_t trcsyncpr;                   //TRCSYNCPR
    uint32_t trcccctlr;                   //TRCCCCTLR
    uint32_t trcbbctlr;                   //TRCBBCTLR
    uint32_t trctraceidr;                 //TRCTRACEIDR
    uint8_t rsvd44[60];                   //rsvd_44
    uint32_t trcvictlr;                   //TRCVICTLR
    uint32_t trcviiectlr;                 //TRCVIIECTLR
    uint32_t trcvissctlr;                 //TRCVISSCTLR
    uint32_t trcvipcssctlr;               //TRCVIPCSSCTLR
    uint8_t rsvd90[16];                   //rsvd_90
    uint32_t trcvdctlr;                   //TRCVDCTLR
    uint32_t trcvdsacctlr;                //TRCVDSACCTLR
    uint32_t trcvdarcctlr;                //TRCVDARCCTLR
    uint8_t rsvdAc[84];                   //rsvd_ac
    uint32_t trcseqevr0;                  //TRCSEQEVR0
    uint32_t trcseqevr1;                  //TRCSEQEVR1
    uint32_t trcseqevr2;                  //TRCSEQEVR2
    uint8_t rsvd10c[12];                  //rsvd_10c
    uint32_t trcseqrstevr;                //TRCSEQRSTEVR
    uint32_t trcseqstr;                   //TRCSEQSTR
    uint32_t trcextinselr;                //TRCEXTINSELR
    uint8_t rsvd124[28];                  //rsvd_124
    uint32_t trccntrldvr0;                //TRCCNTRLDVR0
    uint32_t trccntrldvr1;                //TRCCNTRLDVR1
    uint8_t rsvd148[8];                   //rsvd_148
    uint32_t trccntctlr0;                 //TRCCNTCTLR0
    uint32_t trccntctlr1;                 //TRCCNTCTLR1
    uint8_t rsvd158[8];                   //rsvd_158
    uint32_t trccntvr0;                   //TRCCNTVR0
    uint32_t trccntvr1;                   //TRCCNTVR1
    uint8_t rsvd168[24];                  //rsvd_168
    uint32_t trcidr8;                     //TRCIDR8
    uint32_t trcidr9;                     //TRCIDR9
    uint32_t trcidr10;                    //TRCIDR10
    uint32_t trcidr11;                    //TRCIDR11
    uint32_t trcidr12;                    //TRCIDR12
    uint32_t trcidr13;                    //TRCIDR13
    uint8_t rsvd198[40];                  //rsvd_198
    uint32_t trcimspec0;                  //TRCIMSPEC0
    uint8_t rsvd1c4[28];                  //rsvd_1c4
    uint32_t trcidr0;                     //TRCIDR0
    uint32_t trcidr1;                     //TRCIDR1
    uint32_t trcidr2;                     //TRCIDR2
    uint32_t trcidr3;                     //TRCIDR3
    uint32_t trcidr4;                     //TRCIDR4
    uint32_t trcidr5;                     //TRCIDR5
    uint8_t rsvd1f8[16];                  //rsvd_1f8
    uint32_t trcrsctlr2;                  //TRCRSCTLR2
    uint32_t trcrsctlr3;                  //TRCRSCTLR3
    uint32_t trcrsctlr4;                  //TRCRSCTLR4
    uint32_t trcrsctlr5;                  //TRCRSCTLR5
    uint32_t trcrsctlr6;                  //TRCRSCTLR6
    uint32_t trcrsctlr7;                  //TRCRSCTLR7
    uint32_t trcrsctlr8;                  //TRCRSCTLR8
    uint32_t trcrsctlr9;                  //TRCRSCTLR9
    uint32_t trcrsctlr10;                 //TRCRSCTLR10
    uint32_t trcrsctlr11;                 //TRCRSCTLR11
    uint32_t trcrsctlr12;                 //TRCRSCTLR12
    uint32_t trcrsctlr13;                 //TRCRSCTLR13
    uint32_t trcrsctlr14;                 //TRCRSCTLR14
    uint32_t trcrsctlr15;                 //TRCRSCTLR15
    uint8_t rsvd240[64];                  //rsvd_240
    uint32_t trcssccr0;                   //TRCSSCCR0
    uint8_t rsvd284[28];                  //rsvd_284
    uint32_t trcsscsr0;                   //TRCSSCSR0
    uint8_t rsvd2a4[28];                  //rsvd_2a4
    uint32_t trcsspcicr0;                 //TRCSSPCICR0
    uint8_t rsvd2c4[64];                  //rsvd_2c4
    uint32_t trcoslsr;                    //TRCOSLSR
    uint8_t rsvd308[8];                   //rsvd_308
    uint32_t trcpdcr;                     //TRCPDCR
    uint32_t trcpdsr;                     //TRCPDSR
    uint8_t rsvd318[232];                 //rsvd_318
    uint32_t trcacvr0310;                 //TRCACVR0_31_0
    uint8_t rsvd404[4];                   //rsvd_404
    uint32_t trcacvr1310;                 //TRCACVR1_31_0
    uint8_t rsvd40c[4];                   //rsvd_40c
    uint32_t trcacvr2310;                 //TRCACVR2_31_0
    uint8_t rsvd414[4];                   //rsvd_414
    uint32_t trcacvr3310;                 //TRCACVR3_31_0
    uint8_t rsvd41c[4];                   //rsvd_41c
    uint32_t trcacvr4310;                 //TRCACVR4_31_0
    uint8_t rsvd424[4];                   //rsvd_424
    uint32_t trcacvr5310;                 //TRCACVR5_31_0
    uint8_t rsvd42c[4];                   //rsvd_42c
    uint32_t trcacvr6310;                 //TRCACVR6_31_0
    uint8_t rsvd434[4];                   //rsvd_434
    uint32_t trcacvr7310;                 //TRCACVR7_31_0
    uint8_t rsvd43c[68];                  //rsvd_43c
    uint32_t trcacatr0;                   //TRCACATR0
    uint8_t rsvd484[4];                   //rsvd_484
    uint32_t trcacatr1;                   //TRCACATR1
    uint8_t rsvd48c[4];                   //rsvd_48c
    uint32_t trcacatr2;                   //TRCACATR2
    uint8_t rsvd494[4];                   //rsvd_494
    uint32_t trcacatr3;                   //TRCACATR3
    uint8_t rsvd49c[4];                   //rsvd_49c
    uint32_t trcacatr4;                   //TRCACATR4
    uint8_t rsvd4a4[4];                   //rsvd_4a4
    uint32_t trcacatr5;                   //TRCACATR5
    uint8_t rsvd4ac[4];                   //rsvd_4ac
    uint32_t trcacatr6;                   //TRCACATR6
    uint8_t rsvd4b4[4];                   //rsvd_4b4
    uint32_t trcacatr7;                   //TRCACATR7
    uint8_t rsvd4bc[68];                  //rsvd_4bc
    uint32_t trcdvcvr0;                   //TRCDVCVR0
    uint8_t rsvd504[12];                  //rsvd_504
    uint32_t trcdvcvr1;                   //TRCDVCVR1
    uint8_t rsvd514[108];                 //rsvd_514
    uint32_t trcdvcmr0;                   //TRCDVCMR0
    uint8_t rsvd584[12];                  //rsvd_584
    uint32_t trcdvcmr1;                   //TRCDVCMR1
    uint8_t rsvd594[2376];                //rsvd_594
    uint32_t trcitmiscoutr;               //TRCITMISCOUTR
    uint32_t trcitmiscinr;                //TRCITMISCINR
    uint32_t trcitatbidr;                 //TRCITATBIDR
    uint32_t trcitddatar;                 //TRCITDDATAR
    uint32_t trcitidatar;                 //TRCITIDATAR
    uint32_t trcitdatbinr;                //TRCITDATBINR
    uint32_t trcitiatbinr;                //TRCITIATBINR
    uint32_t trcitdatboutr;               //TRCITDATBOUTR
    uint32_t trcitiatboutr;               //TRCITIATBOUTR
    uint32_t trcitctrl;                   //TRCITCTRL
    uint8_t rsvdF04[156];                 //rsvd_f04
    uint32_t trcclaimset;                 //TRCCLAIMSET
    uint32_t trcclaimclr;                 //TRCCLAIMCLR
    uint8_t rsvdFa8[8];                   //rsvd_fa8
    uint32_t trclar;                      //TRCLAR
    uint32_t trclsr;                      //TRCLSR
    uint32_t trcauthstatus;               //TRCAUTHSTATUS
    uint32_t trcdevarch;                  //TRCDEVARCH
    uint8_t rsvdFc0[8];                   //rsvd_fc0
    uint32_t trcdevid;                    //TRCDEVID
    uint32_t trcdevtype;                  //TRCDEVTYPE
    uint32_t trcpidr4;                    //TRCPIDR4
    uint32_t trcpidr5;                    //TRCPIDR5
    uint32_t trcpidr6;                    //TRCPIDR6
    uint32_t trcpidr7;                    //TRCPIDR7
    uint32_t trcpidr0;                    //TRCPIDR0
    uint32_t trcpidr1;                    //TRCPIDR1
    uint32_t trcpidr2;                    //TRCPIDR2
    uint32_t trcpidr3;                    //TRCPIDR3
    uint32_t trccidr0;                    //TRCCIDR0
    uint32_t trccidr1;                    //TRCCIDR1
    uint32_t trccidr2;                    //TRCCIDR2
    uint32_t trccidr3;                    //TRCCIDR3
} Etm_t;

/// @brief 0xE000
typedef struct
{
    uint8_t rsvd0[4];                     //rsvd_0
    uint32_t ictr;                        //ICTR
    uint32_t actlr;                       //ACTLR
    uint8_t rsvdC[4];                     //rsvd_c
    uint32_t systCsr;                     //SYST_CSR
    uint32_t systRvr;                     //SYST_RVR
    uint32_t systCvr;                     //SYST_CVR
    uint32_t systCalib;                   //SYST_CALIB
    uint8_t rsvd20[224];                  //rsvd_20
    uint32_t nvicIser0;                   //NVIC_ISER0
    uint32_t nvicIser1;                   //NVIC_ISER1
    uint32_t nvicIser2;                   //NVIC_ISER2
    uint32_t nvicIser3;                   //NVIC_ISER3
    uint32_t nvicIser4;                   //NVIC_ISER4
    uint32_t nvicIser5;                   //NVIC_ISER5
    uint32_t nvicIser6;                   //NVIC_ISER6
    uint32_t nvicIser7;                   //NVIC_ISER7
    uint8_t rsvd120[96];                  //rsvd_120
    uint32_t nvicIcer0;                   //NVIC_ICER0
    uint32_t nvicIcer1;                   //NVIC_ICER1
    uint32_t nvicIcer2;                   //NVIC_ICER2
    uint32_t nvicIcer3;                   //NVIC_ICER3
    uint32_t nvicIcer4;                   //NVIC_ICER4
    uint32_t nvicIcer5;                   //NVIC_ICER5
    uint32_t nvicIcer6;                   //NVIC_ICER6
    uint32_t nvicIcer7;                   //NVIC_ICER7
    uint8_t rsvd1a0[96];                  //rsvd_1a0
    uint32_t nvicIspr0;                   //NVIC_ISPR0
    uint32_t nvicIspr1;                   //NVIC_ISPR1
    uint32_t nvicIspr2;                   //NVIC_ISPR2
    uint32_t nvicIspr3;                   //NVIC_ISPR3
    uint32_t nvicIspr4;                   //NVIC_ISPR4
    uint32_t nvicIspr5;                   //NVIC_ISPR5
    uint32_t nvicIspr6;                   //NVIC_ISPR6
    uint32_t nvicIspr7;                   //NVIC_ISPR7
    uint8_t rsvd220[96];                  //rsvd_220
    uint32_t nvicIcpr0;                   //NVIC_ICPR0
    uint32_t nvicIcpr1;                   //NVIC_ICPR1
    uint32_t nvicIcpr2;                   //NVIC_ICPR2
    uint32_t nvicIcpr3;                   //NVIC_ICPR3
    uint32_t nvicIcpr4;                   //NVIC_ICPR4
    uint32_t nvicIcpr5;                   //NVIC_ICPR5
    uint32_t nvicIcpr6;                   //NVIC_ICPR6
    uint32_t nvicIcpr7;                   //NVIC_ICPR7
    uint8_t rsvd2a0[96];                  //rsvd_2a0
    uint32_t nvicIabr0;                   //NVIC_IABR0
    uint32_t nvicIabr1;                   //NVIC_IABR1
    uint32_t nvicIabr2;                   //NVIC_IABR2
    uint32_t nvicIabr3;                   //NVIC_IABR3
    uint32_t nvicIabr4;                   //NVIC_IABR4
    uint32_t nvicIabr5;                   //NVIC_IABR5
    uint32_t nvicIabr6;                   //NVIC_IABR6
    uint32_t nvicIabr7;                   //NVIC_IABR7
    uint8_t rsvd320[224];                 //rsvd_320
    uint32_t nvicIpr0;                    //NVIC_IPR0
    uint32_t nvicIpr1;                    //NVIC_IPR1
    uint32_t nvicIpr2;                    //NVIC_IPR2
    uint32_t nvicIpr3;                    //NVIC_IPR3
    uint32_t nvicIpr4;                    //NVIC_IPR4
    uint32_t nvicIpr5;                    //NVIC_IPR5
    uint32_t nvicIpr6;                    //NVIC_IPR6
    uint32_t nvicIpr7;                    //NVIC_IPR7
    uint32_t nvicIpr8;                    //NVIC_IPR8
    uint32_t nvicIpr9;                    //NVIC_IPR9
    uint32_t nvicIpr10;                   //NVIC_IPR10
    uint32_t nvicIpr11;                   //NVIC_IPR11
    uint32_t nvicIpr12;                   //NVIC_IPR12
    uint32_t nvicIpr13;                   //NVIC_IPR13
    uint32_t nvicIpr14;                   //NVIC_IPR14
    uint32_t nvicIpr15;                   //NVIC_IPR15
    uint32_t nvicIpr16;                   //NVIC_IPR16
    uint32_t nvicIpr17;                   //NVIC_IPR17
    uint32_t nvicIpr18;                   //NVIC_IPR18
    uint32_t nvicIpr19;                   //NVIC_IPR19
    uint32_t nvicIpr20;                   //NVIC_IPR20
    uint32_t nvicIpr21;                   //NVIC_IPR21
    uint32_t nvicIpr22;                   //NVIC_IPR22
    uint32_t nvicIpr23;                   //NVIC_IPR23
    uint32_t nvicIpr24;                   //NVIC_IPR24
    uint32_t nvicIpr25;                   //NVIC_IPR25
    uint32_t nvicIpr26;                   //NVIC_IPR26
    uint32_t nvicIpr27;                   //NVIC_IPR27
    uint32_t nvicIpr28;                   //NVIC_IPR28
    uint32_t nvicIpr29;                   //NVIC_IPR29
    uint32_t nvicIpr30;                   //NVIC_IPR30
    uint32_t nvicIpr31;                   //NVIC_IPR31
    uint32_t nvicIpr32;                   //NVIC_IPR32
    uint32_t nvicIpr33;                   //NVIC_IPR33
    uint32_t nvicIpr34;                   //NVIC_IPR34
    uint32_t nvicIpr35;                   //NVIC_IPR35
    uint32_t nvicIpr36;                   //NVIC_IPR36
    uint32_t nvicIpr37;                   //NVIC_IPR37
    uint32_t nvicIpr38;                   //NVIC_IPR38
    uint32_t nvicIpr39;                   //NVIC_IPR39
    uint32_t nvicIpr40;                   //NVIC_IPR40
    uint32_t nvicIpr41;                   //NVIC_IPR41
    uint32_t nvicIpr42;                   //NVIC_IPR42
    uint32_t nvicIpr43;                   //NVIC_IPR43
    uint32_t nvicIpr44;                   //NVIC_IPR44
    uint32_t nvicIpr45;                   //NVIC_IPR45
    uint32_t nvicIpr46;                   //NVIC_IPR46
    uint32_t nvicIpr47;                   //NVIC_IPR47
    uint32_t nvicIpr48;                   //NVIC_IPR48
    uint32_t nvicIpr49;                   //NVIC_IPR49
    uint32_t nvicIpr50;                   //NVIC_IPR50
    uint32_t nvicIpr51;                   //NVIC_IPR51
    uint32_t nvicIpr52;                   //NVIC_IPR52
    uint32_t nvicIpr53;                   //NVIC_IPR53
    uint32_t nvicIpr54;                   //NVIC_IPR54
    uint32_t nvicIpr55;                   //NVIC_IPR55
    uint32_t nvicIpr56;                   //NVIC_IPR56
    uint32_t nvicIpr57;                   //NVIC_IPR57
    uint32_t nvicIpr58;                   //NVIC_IPR58
    uint32_t nvicIpr59;                   //NVIC_IPR59
    uint8_t rsvd4f0[2064];                //rsvd_4f0
    uint32_t cpuid;                       //CPUID
    uint32_t icsr;                        //ICSR
    uint32_t vtor;                        //VTOR
    uint32_t aircr;                       //AIRCR
    uint32_t scr;                         //SCR
    uint32_t ccr;                         //CCR
    uint32_t shpr1;                       //SHPR1
    uint32_t shpr2;                       //SHPR2
    uint32_t shpr3;                       //SHPR3
    uint32_t shcsr;                       //SHCSR
    uint32_t cfsr;                        //CFSR
    uint32_t hfsr;                        //HFSR
    uint32_t dfsr;                        //DFSR
    uint32_t mmfar;                       //MMFAR
    uint32_t bfar;                        //BFAR
    uint32_t afsr;                        //AFSR
    uint32_t idPfr0;                      //ID_PFR0
    uint32_t idPfr1;                      //ID_PFR1
    uint32_t idDfr0;                      //ID_DFR0
    uint32_t idAfr0;                      //ID_AFR0
    uint32_t idMmfr0;                     //ID_MMFR0
    uint32_t idMmfr1;                     //ID_MMFR1
    uint32_t idMmfr2;                     //ID_MMFR2
    uint32_t idMmfr3;                     //ID_MMFR3
    uint32_t idIsar0;                     //ID_ISAR0
    uint32_t idIsar1;                     //ID_ISAR1
    uint32_t idIsar2;                     //ID_ISAR2
    uint32_t idIsar3;                     //ID_ISAR3
    uint32_t idIsar4;                     //ID_ISAR4
    uint8_t rsvdD74[4];                   //rsvd_d74
    uint32_t clidr;                       //CLIDR
    uint32_t ctr;                         //CTR
    uint32_t ccsidr;                      //CCSIDR
    uint32_t csselr;                      //CSSELR
    uint32_t cpacr;                       //CPACR
    uint8_t rsvdD8c[4];                   //rsvd_d8c
    uint32_t mpuType;                     //MPU_TYPE
    uint32_t mpuCtrl;                     //MPU_CTRL
    uint32_t mpuRnr;                      //MPU_RNR
    uint32_t mpuRbar;                     //MPU_RBAR
    uint32_t mpuRasr;                     //MPU_RASR
    uint32_t mpuRbarA1;                   //MPU_RBAR_A1
    uint32_t mpuRasrA1;                   //MPU_RASR_A1
    uint32_t mpuRbarA2;                   //MPU_RBAR_A2
    uint32_t mpuRasrA2;                   //MPU_RASR_A2
    uint32_t mpuRbarA3;                   //MPU_RBAR_A3
    uint32_t mpuRasrA3;                   //MPU_RASR_A3
    uint8_t rsvdDbc[324];                 //rsvd_dbc
    uint32_t stir;                        //STIR
    uint8_t rsvdF04[48];                  //rsvd_f04
    uint32_t fpccr;                       //FPCCR
    uint32_t fpcar;                       //FPCAR
    uint32_t fpdscr;                      //FPDSCR
    uint8_t rsvdF40[16];                  //rsvd_f40
    uint32_t iciallu;                     //ICIALLU
    uint8_t rsvdF54[4];                   //rsvd_f54
    uint32_t icimvau;                     //ICIMVAU
    uint32_t dcimvac;                     //DCIMVAC
    uint32_t dcisw;                       //DCISW
    uint32_t dccmvau;                     //DCCMVAU
    uint32_t dccmvac;                     //DCCMVAC
    uint32_t dccsw;                       //DCCSW
    uint32_t dccimvac;                    //DCCIMVAC
    uint32_t dccisw;                      //DCCISW
    uint32_t bpiall;                      //BPIALL
    uint8_t rsvdF7c[20];                  //rsvd_f7c
    uint32_t itcmcr;                      //ITCMCR
    uint32_t dtcmcr;                      //DTCMCR
    uint32_t ahbpcr;                      //AHBPCR
    uint32_t cacr;                        //CACR
    uint32_t ahbscr;                      //AHBSCR
    uint8_t rsvdFa4[4];                   //rsvd_fa4
    uint32_t abfsr;                       //ABFSR
    uint8_t rsvdFac[4];                   //rsvd_fac
    uint32_t iebr0;                       //IEBR0
    uint32_t iEBR1h;                      //IEBR1h
    uint32_t dEBR0h;                      //DEBR0h
    uint32_t dEBR1h;                      //DEBR1h
    uint8_t rsvdFc0[16];                  //rsvd_fc0
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} SystemControl_t;

/// @brief 0x2000
typedef struct
{
    uint32_t fpCtrl;                      //FP_CTRL
    uint8_t rsvd4[4];                     //rsvd_4
    uint32_t fpComp0;                     //FP_COMP0
    uint32_t fpComp1;                     //FP_COMP1
    uint32_t fpComp2;                     //FP_COMP2
    uint32_t fpComp3;                     //FP_COMP3
    uint32_t fpComp4;                     //FP_COMP4
    uint32_t fpComp5;                     //FP_COMP5
    uint32_t fpComp6;                     //FP_COMP6
    uint32_t fpComp7;                     //FP_COMP7
    uint8_t rsvd28[4008];                 //rsvd_28
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} Fpb_t;

/// @brief 0x1000
typedef struct
{
    uint32_t dwtCtrl;                     //DWT_CTRL
    uint32_t dwtCyccnt;                   //DWT_CYCCNT
    uint32_t dwtCpicnt;                   //DWT_CPICNT
    uint32_t dwtExccnt;                   //DWT_EXCCNT
    uint32_t dwtSleepcnt;                 //DWT_SLEEPCNT
    uint32_t dwtLsucnt;                   //DWT_LSUCNT
    uint32_t dwtFoldcnt;                  //DWT_FOLDCNT
    uint32_t dwtPcsr;                     //DWT_PCSR
    uint32_t dwtComp0;                    //DWT_COMP0
    uint32_t dwtMask0;                    //DWT_MASK0
    uint32_t dwtFunction0;                //DWT_FUNCTION0
    uint8_t rsvd2c[4];                    //rsvd_2c
    uint32_t dwtComp1;                    //DWT_COMP1
    uint32_t dwtMask1;                    //DWT_MASK1
    uint32_t dwtFunction1;                //DWT_FUNCTION1
    uint8_t rsvd3c[4];                    //rsvd_3c
    uint32_t dwtComp2;                    //DWT_COMP2
    uint32_t dwtMask2;                    //DWT_MASK2
    uint32_t dwtFunction2;                //DWT_FUNCTION2
    uint8_t rsvd4c[4];                    //rsvd_4c
    uint32_t dwtComp3;                    //DWT_COMP3
    uint32_t dwtMask3;                    //DWT_MASK3
    uint32_t dwtFunction3;                //DWT_FUNCTION3
    uint8_t rsvd5c[3956];                 //rsvd_5c
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} Dwt_t;

/// @brief 0x0
typedef struct
{
    uint32_t itmStim0;                    //ITM_STIM0
    uint32_t itmStim1;                    //ITM_STIM1
    uint32_t itmStim2;                    //ITM_STIM2
    uint32_t itmStim3;                    //ITM_STIM3
    uint32_t itmStim4;                    //ITM_STIM4
    uint32_t itmStim5;                    //ITM_STIM5
    uint32_t itmStim6;                    //ITM_STIM6
    uint32_t itmStim7;                    //ITM_STIM7
    uint32_t itmStim8;                    //ITM_STIM8
    uint32_t itmStim9;                    //ITM_STIM9
    uint32_t itmStim10;                   //ITM_STIM10
    uint32_t itmStim11;                   //ITM_STIM11
    uint32_t itmStim12;                   //ITM_STIM12
    uint32_t itmStim13;                   //ITM_STIM13
    uint32_t itmStim14;                   //ITM_STIM14
    uint32_t itmStim15;                   //ITM_STIM15
    uint32_t itmStim16;                   //ITM_STIM16
    uint32_t itmStim17;                   //ITM_STIM17
    uint32_t itmStim18;                   //ITM_STIM18
    uint32_t itmStim19;                   //ITM_STIM19
    uint32_t itmStim20;                   //ITM_STIM20
    uint32_t itmStim21;                   //ITM_STIM21
    uint32_t itmStim22;                   //ITM_STIM22
    uint32_t itmStim23;                   //ITM_STIM23
    uint32_t itmStim24;                   //ITM_STIM24
    uint32_t itmStim25;                   //ITM_STIM25
    uint32_t itmStim26;                   //ITM_STIM26
    uint32_t itmStim27;                   //ITM_STIM27
    uint32_t itmStim28;                   //ITM_STIM28
    uint32_t itmStim29;                   //ITM_STIM29
    uint32_t itmStim30;                   //ITM_STIM30
    uint32_t itmStim31;                   //ITM_STIM31
    uint8_t rsvd80[3456];                 //rsvd_80
    uint32_t itmTer;                      //ITM_TER
    uint8_t rsvdE04[60];                  //rsvd_e04
    uint32_t itmTpr;                      //ITM_TPR
    uint8_t rsvdE44[60];                  //rsvd_e44
    uint32_t itmTcr;                      //ITM_TCR
    uint8_t rsvdE84[332];                 //rsvd_e84
    uint32_t pid4;                        //PID4
    uint32_t pid5;                        //PID5
    uint32_t pid6;                        //PID6
    uint32_t pid7;                        //PID7
    uint32_t pid0;                        //PID0
    uint32_t pid1;                        //PID1
    uint32_t pid2;                        //PID2
    uint32_t pid3;                        //PID3
    uint32_t cid0;                        //CID0
    uint32_t cid1;                        //CID1
    uint32_t cid2;                        //CID2
    uint32_t cid3;                        //CID3
} Itm_t;

typedef struct
{
    Itm_t itm;                                                              // 0x0 : ITM /
    Dwt_t dwt;                                                              // 0x1000 : DWT /
    Fpb_t fpb;                                                              // 0x2000 : FPB /
    uint8_t rsvd3000[45056];                                                // 0x3000 : rsvd_3000 / rsvd_3000
    SystemControl_t systemControl;                                          // 0xE000 : SystemControl /
    uint8_t rsvdF000[204800];                                               // 0xF000 : rsvd_f000 / rsvd_f000
    Etm_t etm;                                                              // 0x41000 : ETM /
    Cti_t cti;                                                              // 0x42000 : CTI /
    uint8_t rsvd43000[765952];                                              // 0x43000 : rsvd_43000 / rsvd_43000
    ProcessorROMTable_t processorROMTable;                                  // 0xFE000 : ProcessorROMTable /
    PPBROMTable_t pPBROMTable;                                              // 0xFF000 : PPBROMTable /
} Cortexm7_t;

COMPILE_ASSERT(offsetof(Cortexm7_t, itm) == 0x0, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, dwt) == 0x1000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, fpb) == 0x2000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, systemControl) == 0xE000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, etm) == 0x41000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, cti) == 0x42000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, processorROMTable) == 0xFE000, "check register structure offset");
COMPILE_ASSERT(offsetof(Cortexm7_t, pPBROMTable) == 0xFF000, "check register structure offset");

//-----------------------------------------------------------------------------
//  Exported register reference (Volatile type)
//-----------------------------------------------------------------------------

//extern volatile Cortexm7_t rCortexm7; ///< 0xE0000000
