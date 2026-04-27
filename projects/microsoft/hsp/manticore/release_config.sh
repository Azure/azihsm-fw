# Copyright (c) Microsoft Corporation. All rights reserved.

# Configure the release parameters for use with manticore.
PRODUCT='Manticore'
BRANCH_PREFIX='manticore_branch_'
TAG_PREFIX='manticore_release_'
RELEASE_PREFIX='manticore'
VERSION_FILE="$manticore/version.h"
VERSION_REPO='manticore'
MANIFEST_MAIN_BRANCH='main'
MANIFESTS='default.xml'

MSFT_REPOS="cerberus-core Cerberus onefleet-hsp pluton-openhsp pluton-splibs \
	pluton-shared pluton-utils lion-fp-qmgr manticore mcr-hsm"
EXTERNAL_REPOS='mbedtls FreeRTOS-Kernel ms-tpm-20-ref printf acvpparser'
