// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef HSP_AEB_H_
#define HSP_AEB_H_

#include <stddef.h>
#include <stdint.h>
#include "platform_api.h"
#include "status/msft_module_id.h"


/* Configurable RoT parameters.  Defaults can be overridden in platform_config.h. */
#include "platform_config.h"
#ifndef HSP_AEB_TOTAL_AEBS
/**
 * Total number of individual AEBs available in hardware.
 *
 * Maybe there's a better way to detect this value besides relying on platform_config, but this
 * value is currently the same everywhere.  If the number ever changes, a better way to detect the
 * differences may be available.
 *
 * Perhaps parameterize this and make it part of the driver instance structure.
 */
#define	HSP_AEB_TOTAL_AEBS				128
#endif

/**
 * The maximum number of AEB word groups supported by the hardware.
 */
#define	HSP_AEB_MAX_GROUPS				(HSP_AEB_TOTAL_AEBS / 32)

/**
 * Determine the group that contains the specified AEB.
 */
#define	HSP_AEB_GET_GROUP(x)			((x) >> 5)

/**
 * Determine the bit number within the group for an AEB.
 */
#define	HSP_AEB_GET_BIT(x)				((x) & 0x1f)

/**
 * Get the bit mask for a single AEB within a group.
 */
#define	HSP_AEB_GET_BIT_MASK(x)			(1U << HSP_AEB_GET_BIT (x))


struct Creg_regs_aeb_regs;	/* Defined in HSP register definition. */

/**
 * Variable context for the AEB driver.
 */
struct hsp_aeb_state {
	platform_mutex lock;	/**< Synchronization for AEB operations. */
};

/**
 * Driver for controlling Access Enablement Bits (AEB) from HSP.
 */
struct hsp_aeb {
	/**
	 * Determine the state of a single AEB.  This is the AEB state being used by hardware for access
	 * decisions.
	 *
	 * @param aeb The driver for the AEB to query.
	 * @param num The AEB number to query.
	 *
	 * @return 0 if the AEB is disabled, 1 if enabled, or an error code.
	 */
	int (*get_aeb_state) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Explicitly enable a single AEB by setting the corresponding enable bit.  Whether the AEB
	 * state changes depends on whether the AEB has been disabled by either hardware or software.
	 *
	 * @param aeb The driver for the AEB to enable.
	 * @param num The AEB number to enable.
	 *
	 * @return 0 if the AEB was enabled or an error code.
	 */
	int (*enable_aeb) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Determine if a single AEB has been explicitly enabled.  This does not necessarily match the
	 * current state of the AEB.
	 *
	 * @param aeb The driver for the AEB to query.
	 * @param num The AEB number to query.
	 *
	 * @return 1 if the AEB has been enabled, 0 if not, or an error code.
	 */
	int (*is_enabled) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Explicitly disable a single AEB by setting the corresponding disable bit.  The AEB will be
	 * disabled as long as hardware does not force the AEB to an enabled state.  Future modification
	 * of the AEB state is still permitted.
	 *
	 * @param aeb The driver for the AEB to disable.
	 * @param num The AEB number to disable.
	 *
	 * @return 0 if the AEB was disabled or an error code.
	 */
	int (*disable_aeb) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Disable a single AEB and lock it from any further modification by software.
	 *
	 * @param aeb The driver for the AEB to lock.
	 * @param num The AEB number to lock.
	 *
	 * @return 0 if the AEB was lock from modification or an error code.
	 */
	int (*disable_and_lock_aeb) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Determine if a single AEB has been explicitly disabled, with or without locking the AEB.
	 * This does not necessarily match the current state of the AEB.
	 *
	 * @param aeb The driver for the AEB to query.
	 * @param num The AEB number to query.
	 *
	 * @return 1 if the AEB has been disable, 0 if not, or an error code.
	 */
	int (*is_disabled) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Determine if a single AEB has been disabled and locked from modification.
	 *
	 * @param aeb The driver for the AEB to query.
	 * @param num The AEB number to query.
	 *
	 * @return 1 if the AEB has been locked, 0 if not, or an error code.
	 */
	int (*is_locked) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Restore the hardware default state for a single AEB.
	 *
	 * @param aeb The driver for the AEB to restore.
	 * @param num The AEB number to restore.
	 *
	 * @return 0 if the AEB default was restored or an error code.
	 */
	int (*restore_default) (const struct hsp_aeb *aeb, unsigned int num);

	/**
	 * Get the current state of multiple AEBs.  This represents the AEB state being used by hardware
	 * for access decisions.
	 *
	 * @param aeb The driver for the AEBs to query.
	 * @param state Output for the current AEB state.  Each bit corresponds to a single AEB, mapped
	 * directly to bit position.
	 * @param word_count The number of AEB words that can be returned.  If this is more than the
	 * number of AEB words supported by the device, remaining words will be set to 0.
	 *
	 * @return 0 if the AEB status was retrieved successfully or an error code.
	 */
	int (*get_multiple_aeb_state) (const struct hsp_aeb *aeb, uint32_t *state, size_t word_count);

	/**
	 * Retrieve the locked state of multiple AEBs.
	 *
	 * @param aeb The driver for the AEBs to query.
	 * @param locked Output for the current AEB locked state.  Each bit corresponds to a single AEB,
	 * mapped directly to bit position.
	 * @param word_count The number of AEB words that can be returned.  If this is more than the
	 * number of AEB words supported by the device, remaining words will be set to 0.
	 *
	 * @return 0 if the AEB locked status was retrieved successfully or an error code.
	 */
	int (*get_multiple_aeb_locked) (const struct hsp_aeb *aeb, uint32_t *locked, size_t word_count);

	/**
	 * Configure multiple AEBs.  The resulting state will be a combination of the applied
	 * configuration, hardware capabilities, security state, and prior software AEB configuration.
	 *
	 * In all lists, each bit represents a single AEB.  If the bit is set, the corresponding
	 * configuration is applied for that AEB.  If the bit is clear, the configuration is skipped for
	 * that AEB.
	 *
	 * AEBs are always configured starting with AEB 0.
	 *
	 * @param aeb The driver for the AEBs to configure.
	 * @param enable List of AEBs to enable.  This can be null to skip all enable configuration.
	 * @param disable List of AEBs to disable.  This can be null to skip all disable configuration.
	 * @param lock List of AEBs to disable and lock from future modification.  This can be null to
	 * skip all lock configuration.
	 * @param word_count The number of AEBs words in the configuration lists.  All provided lists
	 * must be the same length.
	 *
	 * @return 0 if the AEBs were configured successfully or an error code.  A successful return
	 * does not mean the AEB state matches the applied configuration.
	 */
	int (*configure_multiple_aeb) (const struct hsp_aeb *aeb, const uint32_t *enable,
		const uint32_t *disable, const uint32_t *lock, size_t word_count);

	struct hsp_aeb_state *state;		/**< Variable context for the AEB driver. */
	struct Creg_regs_aeb_regs *regs;	/**< Register interface for the AEBs. */
};


int hsp_aeb_init (struct hsp_aeb *aeb, struct hsp_aeb_state *state,
	struct Creg_regs_aeb_regs *regs);
int hsp_aeb_init_state (const struct hsp_aeb *aeb);
void hsp_aeb_release (const struct hsp_aeb *aeb);


#define	HSP_AEB_ERROR(code)		ROT_ERROR (MSFT_MODULE_HSP_AEB, code)

/**
 * Error codes that can be generated by the HSP AEBs.
 */
enum {
	HSP_AEB_INVALID_ARGUMENT = HSP_AEB_ERROR (0x00),	/**< Input parameter is null or not valid. */
	HSP_AEB_NO_MEMORY = HSP_AEB_ERROR (0x01),			/**< Memory allocation failed. */
	HSP_AEB_GET_STATE_FAILED = HSP_AEB_ERROR (0x02),	/**< Failed to get a single AEB state. */
	HSP_AEB_ENABLE_FAILED = HSP_AEB_ERROR (0x03),		/**< Failed to enable an AEB. */
	HSP_AEB_DISABLE_FAILED = HSP_AEB_ERROR (0x04),		/**< Failed to disable an AEB. */
	HSP_AEB_DEFAULT_FAILED = HSP_AEB_ERROR (0x05),		/**< Failed to restore the AEB default status. */
	HSP_AEB_LOCK_FAILED = HSP_AEB_ERROR (0x06),			/**< Failed to lock an AEB from modification. */
	HSP_AEB_GET_MULTIPLE_FAILED = HSP_AEB_ERROR (0x07),	/**< Failed to get state for multiple AEBs. */
	HSP_AEB_CONFIGURE_FAILED = HSP_AEB_ERROR (0x08),	/**< Failed to configure multiple AEBs. */
	HSP_AEB_LOCKED = HSP_AEB_ERROR (0x09),				/**< The AEB cannot be modified. */
	HSP_AEB_OUT_OF_RANGE = HSP_AEB_ERROR (0x0a),		/**< The specified AEB is not supported by the hardware. */
};


#endif	/* HSP_AEB_H_ */
