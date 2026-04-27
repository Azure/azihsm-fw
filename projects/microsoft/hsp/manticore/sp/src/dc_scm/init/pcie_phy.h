// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PCIE_PHY_H_
#define PCIE_PHY_H_

#include <stdint.h>
#include "crypto/ecc_hw.h"
#include "crypto/hash.h"
#include "marvell/RegComphy0Soc.h"
#include "marvell/RegComphy1Soc.h"
#include "marvell/RegPcieEp.h"
#include "marvell/RegPcieTop.h"


/**
 * Base SoC address for the PCIe PHY registers.
 */
#define	PCIE_PHY_REGS_BASE_ADDR			0xb0100000

/**
 * Register space for the PCIe PHY.
 */
struct pcie_phy_regs {
	union {
		Comphy0Soc_t comphy0;			/**< PHY0 registers. */
		uint8_t comphy0_mem[0x20000];	/**< Address space for PHY0 registers. */
	};

	uint8_t pad0[0x20000];				/**< Padding between PHY registers spaces. */
	union {
		Comphy1Soc_t comphy1;			/**< PHY1 registers. */
		uint8_t comphy1_mem[0x20000];	/**< Address space for PHY1 registers. */
	};

	union {
		PcieTop_t pcie_top;				/**< Top-level PCIe  registers. */
		uint8_t pcie_top_mem[0x1000];	/**< Address space for PCIe top registers. */
	};

	uint8_t pad1[0x1f000];				/**< Padding after PCIe top registers. */
	union {
		PcieEp_t pcie_ep;				/**< PCIe endpoint registers. */
		uint8_t pcie_ep_mem[0x10000];	/**< Address space for PCIe endpoint registers. */
	};
};

/**
 * Bit number for PCIE_X4_COLD_RST from PcieResetControl_t.
 */
#define	PCIE_TOP_RESET_CONTROL_PCIE_X4_COLD_RST_BIT					1

/**
 * Bit number for DIRECT_ACCESS_EN_P0 and DIRECT_ACCESS_EN_P1 from Ctrl1P0_t and Ctrl1P1_t.
 */
#define	PCIE_TOP_CTRL1PX_DIRECT_ACCESS_EN_PX_BIT					3

/**
 * Bit number for PROG_RAM_SEL_1_0 from Comphy0SocmemoryControl0_t.
 */
#define	COMPHY_MEMORY_CTRL0_PROG_RAM_SEL_1_0_BIT					3

/**
 * Bit count for PROG_RAM_SEL_1_0 from Comphy0SocmemoryControl0_t.
 */
#define	COMPHY_MEMORY_CTRL0_PROG_RAM_SEL_1_0_COUNT					2

/**
 * Bit number for PMEM_CHECKSUM_PASS from Comphy0SocmemoryControl4_t.
 */
#define	COMPHY_MEMORY_CTRL4_PMEM_CHECKSUM_PASS_BIT					1

/**
 * Bit number for XDATA_MEM_CHECKSUM_RESET_CMN from Comphy0SocxdataMemChecksumCmn2_t.
 */
#define	COMPHY_CHKSUM_COMMON2_XDATA_MEM_CHECKSUM_RESET_CMN_BIT		0

/**
 * Bit number for XDATA_MEM_CHECKSUM_PASS_CMN from Comphy0SocxdataMemChecksumCmn2_t.
 */
#define	COMPHY_CHKSUM_COMMON2_XDATA_MEM_CHECKSUM_PASS_CMN_BIT		1

/**
 * Bit number for XDATA_MEM_CHECKSUM_PASS_LANE from Comphy0SocmcuMemReg2Lane_t.
 */
#define COMPHY_MEM_REG2_LANE_XDATA_MEM_CHECKSUM_PASS_LANE_BIT		29


_Static_assert ((offsetof (struct pcie_phy_regs, comphy0) == 0x00000), "COMPHY0 offset is wrong.");
_Static_assert ((offsetof (struct pcie_phy_regs, comphy1) == 0x40000), "COMPHY1 offset is wrong.");
_Static_assert ((offsetof (struct pcie_phy_regs, pcie_top) == 0x60000),
	"PCIE_TOP offset is wrong.");
_Static_assert ((offsetof (struct pcie_phy_regs, pcie_ep) == 0x80000), "PCIE_EP offset is wrong.");


/**
 * Markers identifying the different PCIe firmware images.
 */
enum pcie_phy_fw_type {
	PCIE_PHY_FW_TYPE_MAIN = 0x6d61696e,		/**< Main MCU firmware image. */
	PCIE_PHY_FW_TYPE_COMMON = 0x636f6d6e,	/**< Common MCU firmware image. */
	PCIE_PHY_FW_TYPE_LANE = 0x6c616e65,		/**< Lane MCU firmware image. */
};

/**
 * Maximum data size of the Main PCIe PHY firmware image.  This does not include any descriptor
 * header.
 */
#define	PCIE_PHY_FW_MAIN_MAX_SIZE		(64 * 1024)

/**
 * Maximum data size of the Common PCIe PHY firmware image.  This does not include any descriptor
 * header.
 */
#define	PCIE_PHY_FW_COMMON_MAX_SIZE		(2 * 1024)

/**
 * Maximum data size of the Lane PCIe PHY firmware image.  This does not include any descriptor
 * header.
 */
#define	PCIE_PHY_FW_LANE_MAX_SIZE		(4 * 1024)

/**
 * Length of the header added to a PCIe PHY firmware image.
 */
#define	PCIE_PHY_FW_HEADER_LENGTH		16

/**
 * Firmware image data that will be present in GSRAM.
 */
struct pcie_phy_fw {
	uint32_t type;			/**< Type of PHY MCU firmware image. */
	uint32_t length;		/**< Length of the firmware image data, in dwords. */
	uint32_t reserved[2];	/**< Unused.  Needed for 16-byte alignment. */
	uint32_t data;			/**< First dword of the MCU firmware image. */
};

/**
 * Descriptor for initializing the PCIe PHY.
 */
struct pcie_phy {
	/**
	 * The firmware images in GSRAM.
	 * 0: Main
	 * 1: Common
	 * 2: Lane
	 */
	struct pcie_phy_fw *fw[3];
};


/* For use in 1SP:  Load the firmware from flash. */
int pcie_phy_load_firmware (const struct pcie_phy *pcie);


#endif	/* PCIE_PHY_H_ */
