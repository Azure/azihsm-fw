// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef OVL3_COMMANDS_H_
#define OVL3_COMMANDS_H_

#include "cmd_interface/omc_background.h"
#include "msft_protocol/msft_mctp_protocol.h"


/**
 * Version of the OVL3 protocol provided by this implementation.
 */
#define	OVL3_PROTOCOL_VERSION			0

/**
 * Command codes for the OVL3 MSFT command set.
 */
enum {
	MSFT_PROTOCOL_OVL3_SOC_FLASH_ERASE = 0x01,
	MSFT_PROTOCOL_OVL3_SOC_FLASH_ERASE_STATUS = 0x02,
	MSFT_PROTOCOL_OVL3_SOC_IMAGE_PARTITIONS_ERASE = 0x03,
	MSFT_PROTOCOL_OVL3_SOC_IMAGE_PARTITIONS_ERASE_STATUS = 0x04,
};

#pragma pack(push, 1)

struct ovl3_soc_flash_erase_request {
	struct msft_mctp_protocol_header header;	/**< Message header */
};

struct ovl3_soc_flash_erase_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
};

struct ovl3_get_soc_flash_erase_status_request {
	struct msft_mctp_protocol_header header;	/**< Message header */
};

struct ovl3_get_soc_flash_erase_status_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
	uint32_t erase_status;								/**< Flash erase status */
};

struct ovl3_soc_image_partitions_erase_request {
	struct msft_mctp_protocol_header header;	/**< Message header */
};

struct ovl3_soc_image_partitions_erase_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
};

struct ovl3_get_soc_image_partitions_erase_status_request {
	struct msft_mctp_protocol_header header;	/**< Message header */
};

struct ovl3_get_soc_image_partitions_erase_status_response {
	struct msft_mctp_protocol_response_header header;	/**< Message header */
	uint32_t erase_status;								/**< Image partitions erase status */
};

#pragma pack(pop)

int ovl3_soc_flash_erase_omc (struct omc_background *handler, struct cmd_interface_msg *request);

int ovl3_get_soc_flash_erase_status_omc (struct omc_background *handler,
	struct cmd_interface_msg *request);

int ovl3_soc_image_partitions_erase_omc (struct omc_background *handler,
	struct cmd_interface_msg *request);

int ovl3_get_soc_image_partitions_erase_status_omc (struct omc_background *handler,
	struct cmd_interface_msg *request);


#endif	/* OVL3_COMMANDS_H_ */
