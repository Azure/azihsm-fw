# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include Path
#------------------------------------------------------------------------------
FPSCPU2_INCLUDE_DIR  += -IFP3Core/Common/
FPSCPU2_INCLUDE_DIR  += -IFP3Core/FpsCpu2/
FPSCPU2_INCLUDE_DIR  += -IHal/Common/
FPSCPU2_INCLUDE_DIR  += -IHal/UCD/
FPSCPU2_INCLUDE_DIR  += -IHal/CDMA/
FPSCPU2_INCLUDE_DIR  += -IHal/GDMA/
FPSCPU2_INCLUDE_DIR  += -IHal/FPS/
INCLUDE_DIR += -IHal/TCON/
INCLUDE_DIR += -IHal/Cortexm7/
#------------------------------------------------------------------------------
# Include Component Make File
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# Object File List
#------------------------------------------------------------------------------
FPSCPU2_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu2/FpsCpu2.o
FPSCPU2_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu2/FpsCpu2ErrorHandle.o
FPSCPU2_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu2/FpsCpu2ErrorInjection.o
FPSCPU2_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu2/FpsCpu2MessageHandler.o
FPSCPU2_OBJS   += $(OBJ_DIR)/FP3Core/FpsCpu2/FpsCpu2MessageOperate.o
#------------------------------------------------------------------------------
# Dependencies
#------------------------------------------------------------------------------
