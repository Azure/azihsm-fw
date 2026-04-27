# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include Path
#------------------------------------------------------------------------------
FPSCPU1_INCLUDE_DIR  += -IFP3Core/Common/
FPSCPU1_INCLUDE_DIR  += -IFP3Core/FpsCpu1/
FPSCpU1_INCLUDE_DIR  += -IHal/Common/
FPSCpU1_INCLUDE_DIR  += -IHal/UCD/
FPSCPU1_INCLUDE_DIR  += -IHal/CDMA/
INCLUDE_DIR += -IHal/TCON/
INCLUDE_DIR += -IHal/Cortexm7/
#------------------------------------------------------------------------------
# Include Component Make File
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Object File List
#------------------------------------------------------------------------------
FPSCPU1_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu1/FpsCpu1.o
FPSCPU1_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu1/FpsCpu1ErrorHandle.o
FPSCPU1_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu1/FpsCpu1MessageHandler.o
#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
