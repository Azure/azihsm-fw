# SPDX-License-Identifier: MIT
# Copyright (c) 2021-2026 Marvell

#------------------------------------------------------------------------------
# Include path
#------------------------------------------------------------------------------
INCLUDE_DIR += -IFP3Core/System/Debug
INCLUDE_DIR += -IFP3Core/ldscripts
INCLUDE_DIR += -IPcSim/MrvlDefinition
INCLUDE_DIR += -ISystem/Inc
#------------------------------------------------------------------------------
# Object file list
#------------------------------------------------------------------------------

M7DEBUG_OBJS += $(OBJ_DIR)/FP3Core/System/Debug/LoggingDebug.o
#M7DEBUG_OBJS += $(OBJ_DIR)/FP3Core/System/Debug/xxx.o

#------------------------------------------------------------------------------
# Include component make file
#------------------------------------------------------------------------------
