// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef VERSION_H_
#define VERSION_H_


/* Version number components. */
#define	FW_VERSION_MAJOR			0
#define	FW_VERSION_MINOR			0
#define	FW_VERSION_RELEASE			0
#define	FW_VERSION_BUILD			0


/**
 * @brief List of sha256sum of shared memory mapping auto-generated CSV files.
 * DO NOT UPDATE below comments fields manually it will be updated during manticore build.
 *
 * GSRAM_MEM_MAP : 8397801e6afbebcd7e8c92a4fa93cab0f205c21e8c1bea79408c70aa70ea8a2f
 * PSRAM_MEM_MAP : da4e8866f351c25b6dbc4603932983d0008c61fefcd6abb4d0f62907e299d69f
 * CDMA_MEM_MAP : 01d4195406103d302ffa329cfa54e6f578b0ab9139e290fed35dbc2e1a9deae2
 * PHY_FW : 0ede59904d47b6eb24e61cef20ae7eaf65548a08265187216191080ae9a4cacf
 */
#define FW_VERSION_IDFU				20


/* Identifier for beta vs. release builds. */
#define	FW_VERSION_IS_RELEASE		0
#if FW_VERSION_IS_RELEASE
#define	FW_VERSION_TYPE		"rel"
#else
#define	FW_VERSION_TYPE		"beta"
#endif


/* String macros to convert version number. */
#define	FW_STRING(x)		#x
#define	FW_TO_STRING(x)		FW_STRING (x)


/**
 * The version number for the firmware.
 */
#define	FW_VERSION_NUM      \
		((FW_VERSION_MAJOR << 24) | (FW_VERSION_MINOR << 16) | (FW_VERSION_RELEASE << 8) | FW_VERSION_BUILD)

/**
 * The version string for the firmware.
 */
#define	FW_VERSION_STRING   \
		FW_TO_STRING (FW_VERSION_MAJOR) "." FW_TO_STRING (FW_VERSION_MINOR) "." FW_TO_STRING (FW_VERSION_RELEASE) "." FW_TO_STRING (FW_VERSION_BUILD) FW_VERSION_TYPE


#endif	/* VERSION_H_ */
