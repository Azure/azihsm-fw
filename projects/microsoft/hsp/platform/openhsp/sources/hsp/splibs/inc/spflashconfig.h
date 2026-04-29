/*++

    Copyright (C) Microsoft Corporation. All rights reserved.

Module Name:

    spflashconfig.h

Abstract:

    This file contains temporary configuration values for spi and the flash
    device. Note that this is incredibly dependent on a specific flash part,
    and cannot be assumed to work for all flash parts.

Author:

    Navin Pai (navinp)
    Timothy Prinz (tiprinz)

--*/

#pragma once

#define SPI_FLASH_WAIT_CYCLES_DEFAULT 0x0
#define SPI_FLASH_WAIT_CYCLES_READ    0x6

#define SPI_FLASH_SAMPLE_DELAY        0x2


#define FLASH_MICROCHIP_IOC_ENABLE    0x02
#define FLASH_WINBOND_QUAD_ENABLE     0x02


typedef enum FLASH_COMMAND
{
    FLASH_CMD_WRITE_STATUS_REGISTER = 0x01,
    FLASH_CMD_PAGE_PROGRAM = 0x02,
    FLASH_CMD_READ = 0x03,
    FLASH_CMD_WRITE_DISABLE = 0x04,
    FLASH_CMD_READ_STATUS_REGISTER = 0x05,
    FLASH_CMD_WRITE_ENABLE = 0x06,
    FLASH_CMD_FAST_READ = 0x0B,
    FLASH_CMD_PAGE_PROGRAM_4B_MODE = 0x12,
    FLASH_CMD_READ_4B_MODE = 0x13,
    FLASH_CMD_SECTOR_ERASE = 0x20,
    FLASH_CMD_SECTOR_ERASE_4B_MODE = 0x21,
    FLASH_CMD_WRITE_STATUS_REGISTER_2 = 0x31,
    FLASH_CMD_READ_CONFIG_REGISTER = 0x35,
    FLASH_CMD_SFDP = 0x5A,
    FLASH_CMD_WRITE_ENH_VOLATILE_CONFIG = 0x61,
    FLASH_CMD_READ_ENH_VOLATILE_CONFIG = 0x65,
    FLASH_CMD_RESET_ENABLE = 0x66,
    FLASH_CMD_QUAD_OUTPUT_FAST_READ = 0x6B,
    FLASH_CMD_QUAD_OUTPUT_FAST_READ_4B_MODE = 0x6C,
    FLASH_CMD_WRITE_VOLATILE_CONFIG = 0x81,
    FLASH_CMD_READ_VOLATILE_CONFIG = 0x85,
    FLASH_CMD_RESET_MEMORY = 0x99,
    FLASH_CMD_READ_ID = 0x9F,
    FLASH_CMD_WRITE_NON_VOLATILE_CONFIG = 0xB1,
    FLASH_CMD_READ_NON_VOLATILE_CONFIG = 0xB5,
    FLASH_CMD_ENTER_4B_MODE = 0xB7,
    FLASH_CMD_WRITE_EXT_ADDR_REGISTER = 0xC5,
    FLASH_CMD_CHIP_ERASE = 0xC7,
    FLASH_CMD_READ_EXT_ADDR_REGISTER = 0xC8,
    FLASH_CMD_EXIT_4B_MODE = 0xE9,

    //
    // Microchip specific command
    //
    FLASH_CMD_READ_BLOCK_PROT_REGISTER = 0x72,
    FLASH_CMD_WRITE_BLOCK_PROT_REGISTER = 0x42,
    FLASH_CMD_GLOBAL_BLOCK_PROT_UNLOCK = 0x98,

    //
    // Micron specific commands
    //
    FLASH_CMD_READ_NV_CONF_REGISTER = 0xB5,

} FLASH_COMMAND;


typedef enum _FLASH_JEDEC_ID
{
    FLASH_ID_UNKNOWN = 0x0,
    FLASH_ID_MICRON = 0x0019BA20,
    FLASH_ID_WINBOND = 0x001940EF,
    FLASH_ID_MICROCHIP = 0x004326BF,    // FPGA onboard flash

} FLASH_JEDEC_ID;
