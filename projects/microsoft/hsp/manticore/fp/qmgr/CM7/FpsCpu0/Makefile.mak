# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include Path
#------------------------------------------------------------------------------
FPSCPU0_INCLUDE_DIR  += -IFP3Core/Common/
FPSCPU0_INCLUDE_DIR  += -IFP3Core/FpsCpu0/
FPSCPU0_INCLUDE_DIR  += -IHal/Common/
FPSCPU0_INCLUDE_DIR  += -IHal/UCD/
FPSCPU0_INCLUDE_DIR  += -IHal/FPS/
FPSCPU0_INCLUDE_DIR  += -IHal/CDMA/
INCLUDE_DIR += -IHal/TCON/
INCLUDE_DIR += -IHal/Cortexm7/
#------------------------------------------------------------------------------
# Include Component Make File
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Object File List
#------------------------------------------------------------------------------
FPSCPU0_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu0/FpsCpu0.o
FPSCPU0_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu0/FpsCpu0Misc.o
FPSCPU0_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu0/FpsCpu0MessageHandler.o
#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
