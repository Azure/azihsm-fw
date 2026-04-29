# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include Path
#------------------------------------------------------------------------------
INCLUDE_DIR += -IFP3Core/System
INCLUDE_DIR += -IFP3Core/ldscripts
INCLUDE_DIR += -IFP3Core/Common
INCLUDE_DIR += -IFP3Core/FpsCpu0
INCLUDE_DIR += -IFP3Core/FpsCpu1
INCLUDE_DIR += -IFP3Core/FpsCpu2
#------------------------------------------------------------------------------
# Include Component Make File
#------------------------------------------------------------------------------
# TODO: Change SHARE_OBJS += xxx.o into SHARE_OBJS += $(xxx_OBJS)
FP_SHARE_OBJS =
include FP3Core/System/Fiber/M7Fiber.mak                                  # Build Fiber
include FP3Core/System/Debug/Debug.mak                                    # Build Debug
#------------------------------------------------------------------------------
# Object File List
#------------------------------------------------------------------------------

FP_SHARE_OBJS += $(M7FIBER_OBJS)
FP_SHARE_OBJS += $(M7DEBUG_OBJS)

#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
