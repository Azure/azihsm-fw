/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spi.h

Abstract:

    This file contains declarations of SPI driver functions,
    for initialization and communication with slave device (flash memory).

Author:

    Timothy Prinz (tiprinz)

--*/


#pragma once


#define SPI_ENUM(reg, setting, value) \
    CREG_REGS_DW_APB_SSI_APB_SLAVE_##reg##_##setting##_##setting##_##value

//
// CTRLR1.NDF is only 16 bit in size which is 64KB
//
#define SPI_MAX_TRANSFER_SIZE 0x10000

void HspSpiSetTxMode(uint32_t TransferMode,
                     uint32_t DataFrameSize,
                     uint32_t FrameFormat,
                     uint32_t NumberRxFrames,
                     uint32_t AddressLength,
                     uint32_t WaitCycle,
                     uint32_t SampleDelay);


void HspSpiEnableXipMode();


bool HspSpiTxData(pvoid_t Data, uint32_t NumFrames);


uint32_t HspSpiRxData32(puint32_t DataOut, uint32_t MaxFrames);


uint32_t HspSpiRxData8(puint8_t DataOut, uint32_t MaxFrames);


void HspSpiWaitForTxDone();


INLINE void HspSpiSetSlaveSel(uint32_t Val)
/*++

Description:

    Sets the manual Spi Slave Select mux and S0_SS
    signal to manually control communication enable/disable with flash slave.

    This is necessary as the S0_SS signal output from the Spi
    is not reliable for multibyte transactions, so we set our own.

Arguments:

    Val - Desired value for S0_SS.  Note that it is active low
        to enable flash slave, and high to disable flash slave

--*/
{
    CREG_SPI_SLAVE_SELECT spiSlaveSelect = {0};

    //
    // Only enable and disable Slave 0 select as we only care about flash on
    // Slave 0. Enable writing to the slave select value, set the slave select
    // mux to use S0_SS value we write, and set the S0_SS value. Note that S0_SS
    // is active low for the flash.
    //
    //
    //  Do we need to initialize and deinitialize for every transaction?
    //  Only certain transactions?
    //

    spiSlaveSelect.S0_Wr = 1;
    spiSlaveSelect.S0_Sel = 1;
    spiSlaveSelect.S0_Ss = Val;
    HspWriteRegister32(CREG_REG(SPI_SLAVE_SELECT), spiSlaveSelect.u);
}
