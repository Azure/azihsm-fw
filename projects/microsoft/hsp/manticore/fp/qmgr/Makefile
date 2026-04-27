# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#subdirs=CM7/ccp2 CM7/ccp CM7/dqp R5
subdirs=CM7/FpsCpu0 CM7/FpsCpu1 CM7/FpsCpu2

# Set Mode <NORMAL | CBYPASS | DBYPASS >
# Where,
# 	NORMAL = Encrypt / Decrypt operation enabled.
# 	CBYPASS = Crypto Bypass
# 	DBYPASS = Data Bypass
MODE = NORMAL

# Tokenized Logging Enable
LOGGING = LOGGING_TOKENIZATION

# Set NEWAESKEYVALIDATION for Production CP (M16 and later) and disable for Validation CP
# This feature when enabled checks for Session and App ID
NEWAESKEYVALIDATION = ENABLE

# Currently PSRAM gets cleared in case of Validation CP and Production CP build with Manticore build scripts (M17 and later).
# Production CP version M16 which works with Lionperf tool doesnt clear the PSRAM so set FPCLEARPSRAM to true.
FPCLEARPSRAM = DISABLE

# Enable MCRTESTHOOKS flag In case of Testing for L1 Abort and Crashdump testing.
MCRTESTHOOKS = DISABLE

#DEBUGBUILD flag for FP Debug build and will also enable extra data in crashdump.
DEBUGBUILD = DISABLE

# Enable flag for testing Pre reset fault injection during Impactless FW Update.
PRERESETFAULTINJECTION = DISABLE

# Enable Flag for testing Post reset fault injection during Impactless FW Update.
POSTRESETFAULTINJECTION = DISABLE

# Enable LIONPERF_SUPPORT flag for Validation CP and M16 MSFT CP
# AES-GCM FIPS Compliance.
LIONPERF_SUPPORT = DISABLE

all:
	for d in $(subdirs); do make -C $$d $@ $(MAKEFLAGS) SIM_PRJ=`pwd` BLDDIR=`pwd`/build PLATFORM=CHIP FLASH_BOOT=1 MODE=$(MODE) LOGGING=$(LOGGING) NEWAESKEYVALIDATION=$(NEWAESKEYVALIDATION) FPCLEARPSRAM=$(FPCLEARPSRAM) MCRTESTHOOKS=$(MCRTESTHOOKS) DEBUGBUILD=$(DEBUGBUILD) PRERESETFAULTINJECTION=$(PRERESETFAULTINJECTION) POSTRESETFAULTINJECTION=$(POSTRESETFAULTINJECTION) LIONPERF_SUPPORT=$(LIONPERF_SUPPORT) || exit 1; done

clean:
	for d in $(subdirs); do make -C $$d $@ $(MAKEFLAGS) SIM_PRJ=`pwd` BLDDIR=`pwd`/build PLATFORM=CHIP FLASH_BOOT=1 MODE=$(MODE) LOGGING=$(LOGGING) NEWAESKEYVALIDATION=$(NEWAESKEYVALIDATION) FPCLEARPSRAM=$(FPCLEARPSRAM) MCRTESTHOOKS=$(MCRTESTHOOKS) DEBUGBUILD=$(DEBUGBUILD) PRERESETFAULTINJECTION=$(PRERESETFAULTINJECTION) POSTRESETFAULTINJECTION=$(POSTRESETFAULTINJECTION) LIONPERF_SUPPORT=$(LIONPERF_SUPPORT) ; done

normal: MODE = NORMAL
NORMAL: MODE = NORMAL
DBYPASS: MODE = DBYPASS
dbypass: MODE = DBYPASS
CBYPASS: MODE = CBYPASS
cbypass: MODE = CBYPASS

normal NORMAL: all
DBYPASS dbypass: all
CBYPASS cbypass: all
