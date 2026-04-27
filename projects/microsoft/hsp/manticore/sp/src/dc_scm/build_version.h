// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef BUILD_VERSION_H_
#define BUILD_VERSION_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* Macros to extract major.minor.release.build components from a version number. */
#define	BUILD_VERSION_MAJOR(x)				((x)[3])
#define	BUILD_VERSION_MINOR(x)				((x)[2])
#define	BUILD_VERSION_RELEASE(x)			((x)[1])
#define	BUILD_VERSION_BUILD(x)				((x)[0])

/**
 * The integer value representing the full version number, without any extensions.
 */
#define	BUILD_VERSION_FW_VERSION(x)         \
	(((uint32_t) BUILD_VERSION_MAJOR (x) << 24) | ((uint32_t) BUILD_VERSION_MINOR (x) << 16) | \
		((uint32_t) BUILD_VERSION_RELEASE (x) << 8) | BUILD_VERSION_BUILD (x))

/**
 * Get the identifier indicating the type of extension being applied to the version string.  5 bits
 * are reserved for identifying the type of extension added to the version number.
 */
#define	BUILD_VERSION_EXTENSION_TYPE(x)		((x)[4] & 0x1f)

/**
 * Extension type identifiers for the build version.
 */
enum {
	BUILD_VERSION_EXTENSION_PROD = 0,	/**< A production firmware image. */
	BUILD_VERSION_EXTENSION_RC = 1,		/**< A release candidate image. */
	BUILD_VERSION_EXTENSION_DEV = 2,	/**< A firmware image containing development keys. */
	BUILD_VERSION_EXTENSION_BETA = 3,	/**< An untracked, test build. */
	BUILD_VERSION_EXTENSION_EVB = 4,	/**< A firmware image built for the Marvell EVB. */
	BUILD_VERSION_EXTENSION_REC = 8,	/**< A firmware image built for recovery. */
	BUILD_VERSION_EXTENSION_TEST = 12,	/**< A firmware image built with test capabilities enabled. */
};

/**
 * Get the numerical extention value appended to builds.  27 bits are reserved for storing an opaque
 * numerical value to append to the version.
 */
#define	BUILD_VERSION_EXTENSION_NUMBER(x)   \
	(((uint32_t) (x)[7] << 19) | ((uint32_t) (x)[6] << 11) | ((uint32_t) (x)[5] << 3) | \
		(((x)[4] >> 5) & 0x7))


int build_version_to_string (const uint8_t *version, bool secure, bool fips, char *str,
	size_t length);
void build_version_debug_log (const uint8_t *version, bool secure, bool fips,
	const char *service_indicator);


#endif	/* BUILD_VERSION_H_ */
