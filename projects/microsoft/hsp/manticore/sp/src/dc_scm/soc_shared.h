// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef SOC_SHARED_H_
#define SOC_SHARED_H_

#include "rot_memory_map.h"
#include "1sp/manticore_1sp.h"
#include "splibs/inc/spcryptotypes.h"


/**
 * Reset type indicators indicating the SoC reset context.
 */
enum soc_reset_type {
	SOC_RESET_TYPE_SOC = 0,				/**< A full SoC reset. */
	SOC_RESET_TYPE_WARM = 1,			/**< A SoC warm reset. */
	SOC_RESET_TYPE_FIRMWARE_UPDATE = 2,	/**< A SoC warm reset for a firmware update. */
};

/**
 * Structure for storing information that will be shared with all SoC cores.
 */
struct soc_shared_data {
	uint32_t reset_type;	/**< Indication of the type of reset executed by the device. */
};

/**
 * Structure for storing information that will be passed from 1SP to the CP cores.
 */
struct cp_shared_data {
	struct manticore_measurements_log por_log;			/**< Storage for the POR firmware measurements (PCRs 0 and 2). */
	uint8_t fips_certified;								/**< Indicator of FIPS certification for loaded firmware. */
	char fw_version[CERBERUS_PROTOCOL_FW_VERSION_LEN];	/**< Version string for the firmware package.  Padded with spaces. */
	uint8_t socid[32];									/**< SOCID for the device as an ASCII hex string. */
	int32_t alias_key_length;							/**< Length of the CP alias key.  This will be -1 if the key was too long. */
	uint8_t alias_key[MAX_ALIAS_KEY_LENGTH];			/**< Storage for the CP alias private key. */
	int32_t alias_cert_length;							/**< Length of the alias certificate.  This will be -1 if the certificate was too long. */
	uint8_t alias_cert[MAX_ALIAS_CERT_LENGTH];			/**< Storage for the CP alias certificate signed by the device ID. */

	/* If there is need to add more HSP data in the future, the max size of the alias cert will need
	 * to be reduced, but only in this context.  It can't change in the 1SP shared to SPRT. */

	/* These fields begin the next 4k page, which is not owned by HSP. */
	uint32_t cp_logger_lock;				/**< Logger lock for CP. */
	uint8_t non_por_preserve_data[8188];	/**< Data to preserve on non-POR resets. */
};


/**
 * Length of data to erase within CP shared data.
 */
#define CP_SHARED_ERASE_LENGTH      \
	(offsetof (struct cp_shared_data, cp_logger_lock) - \
		offsetof (struct cp_shared_data, fips_certified))

/**
 * Offset of data to erase within CP shared data.
 */
#define CP_SHARED_ERASE_OFFSET      \
	(offsetof (struct cp_shared_data, fips_certified))


/* Make sure the CP shared data is 12KB in length. */
_Static_assert ((sizeof (struct cp_shared_data) == (12 * 1024)),
	"Shared CP data structure is not sized correctly.");

/* Make sure the CP RO data aligns to a 4KB boundary. */
_Static_assert ((offsetof (struct cp_shared_data, cp_logger_lock) % (4 * 1024) == 0),
	"CP RO data is not 4KB aligned.");

/* Make sure that por_log is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	por_log)) == (GSRAM_MEM_MAP_POR_MEASUREMENTS % CP_SHARED_GSRAM_ADDRESS)),
	"por_log is at the wrong offset.");

/* Make sure that fips_certified is at the expected offset.
 * TODO:  This is not part of the CP GSRAM memory map yet and can't be added easily due to the lack
 * of 4-byte alignment.  Enable this check once the GSRAM map is aware of this value. */
// _Static_assert (((offsetof (struct cp_shared_data,
// 	fips_certified)) == (GSRAM_MEM_MAP_FIPS_CERTIFIED % CP_SHARED_GSRAM_ADDRESS)),
// 	"fips_certified is at the wrong offset.");

/* Make sure that fw_version is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	fw_version)) == (GSRAM_MEM_MAP_FW_PACKAGE_VERSION % CP_SHARED_GSRAM_ADDRESS)),
	"fw_version is at the wrong offset.");

/* Make sure that socid is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	socid)) == (GSRAM_MEM_MAP_SOC_ID % CP_SHARED_GSRAM_ADDRESS)), "socid is at the wrong offset.");

/* Make sure that alias_key_length is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	alias_key_length)) == (GSRAM_MEM_MAP_ALIAS_KEY_LENGTH % CP_SHARED_GSRAM_ADDRESS)),
	"alias_key_length is at the wrong offset.");

/* Make sure that alias_key is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	alias_key)) == (GSRAM_MEM_MAP_ALIAS_KEY % CP_SHARED_GSRAM_ADDRESS)),
	"alias_key is at the wrong offset.");

/* Make sure that alias_cert_length is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	alias_cert_length)) == (GSRAM_MEM_MAP_ALIAS_CERT_LENGTH % CP_SHARED_GSRAM_ADDRESS)),
	"alias_cert_length is at the wrong offset.");

/* Make sure that alias_cert is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	alias_cert)) == (GSRAM_MEM_MAP_ALIAS_CERT % CP_SHARED_GSRAM_ADDRESS)),
	"alias_cert is at the wrong offset.");

/* Make sure that cp_logger_lock is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	cp_logger_lock)) == (GSRAM_MEM_MAP_LOGGER_LOCK % CP_SHARED_GSRAM_ADDRESS)),
	"cp_logger_lock is at the wrong offset.");

/* Make sure that non_por_preserve_data is at the expected offset. */
_Static_assert (((offsetof (struct cp_shared_data,
	non_por_preserve_data)) == (GSRAM_MEM_MAP_ADMIN_PCIE_RESOURCE_TABLE % CP_SHARED_GSRAM_ADDRESS)),
	"non_por_preserve_data is at the wrong offset.");


#endif	/* SOC_SHARED_H_ */
